import pandas as pd
import dask.dataframe as dd
import pickle
from dask_ml.model_selection import train_test_split
from dask_ml.preprocessing import StandardScaler
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score
from dask_ml.model_selection import GridSearchCV

# Load the datasets with specific column names
df_imbalance_pandas = pd.read_csv('/Users/guru/vsc/capstone/imbalance.csv', names=[f'feature_{i}' for i in range(1, 9)], header=None)
df_normal_pandas = pd.read_csv('/Users/guru/vsc/capstone/normal.csv', names=[f'feature_{i}' for i in range(1, 9)], header=None)

# Convert to Dask DataFrames
df_imbalanced = dd.from_pandas(df_imbalance_pandas, npartitions=10)
df_normal = dd.from_pandas(df_normal_pandas, npartitions=10)

# Limit the dataset to a total of 100,000 rows
df_imbalanced_sampled = df_imbalanced.sample(frac=0.1, random_state=42).compute().head(50000)
df_normal_sampled = df_normal.sample(frac=0.1, random_state=42).compute().head(50000)

# Add label columns
df_imbalanced_sampled['label'] = 1  # Imbalanced labeled as failure (1)
df_normal_sampled['label'] = 0      # Normal labeled as normal (0)

# Concatenate the sampled datasets
df_combined = pd.concat([df_imbalanced_sampled, df_normal_sampled])

# Convert the concatenated DataFrame back to Dask
df_combined = dd.from_pandas(df_combined, npartitions=10)

# Split the data into features (X) and labels (y)
X = df_combined.drop('label', axis=1)
y = df_combined['label']

# Split the data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42, shuffle=True)

# Standardize the features
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# Initialize the classifier
clf = RandomForestClassifier(random_state=42)

# Define hyperparameters grid
param_grid = {
    'n_estimators': [100, 200, 300],
    'max_depth': [None, 10, 20]
}

# Perform cross-validation using GridSearchCV (without verbose)
grid_search = GridSearchCV(clf, param_grid, cv=5, scoring='accuracy')
grid_search.fit(X_train_scaled, y_train)

# Get the best estimator and its accuracy
best_clf = grid_search.best_estimator_
best_accuracy = grid_search.best_score_

# Log each model's parameters and accuracy in performance.txt
with open('performance.txt', 'w') as f:
    f.write("Cross-Validation Results:\n")
    for params, mean_score, std_dev in zip(grid_search.cv_results_['params'],
                                           grid_search.cv_results_['mean_test_score'],
                                           grid_search.cv_results_['std_test_score']):
        f.write(f"Params: {params}, Mean Accuracy: {mean_score:.4f}, Std Dev: {std_dev:.4f}\n")
        print(f"Params: {params}, Mean Accuracy: {mean_score:.4f}")

# Make predictions
y_pred = best_clf.predict(X_test_scaled)

# Convert Dask arrays to NumPy arrays for evaluation
y_test_np = y_test.compute()
X_test_scaled_np = X_test_scaled.compute()

# Calculate and log accuracy for the best model
accuracy = accuracy_score(y_test_np, y_pred)
accuracy_percentage = accuracy * 100
print("\nBest Model Accuracy on Test Set:", accuracy_percentage)

# Save the best model accuracy to bestmodel.txt
with open('bestmodel.txt', 'w') as f:
    f.write(f"Best Model Accuracy: {accuracy_percentage:.2f}%\n")

# Save the best model
with open('best_model.pkl', 'wb') as f:
    pickle.dump(best_clf, f)

# Save the scaler object
with open('scaler.pkl', 'wb') as f:
    pickle.dump(scaler, f)
