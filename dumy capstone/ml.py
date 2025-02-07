import pandas as pd
import dask.dataframe as dd
import pickle
from dask_ml.model_selection import train_test_split
from dask_ml.preprocessing import StandardScaler
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score
from dask_ml.model_selection import GridSearchCV
# Load the imbalanced dataset
df_imbalanced_pandas = pd.read_excel(r"D:\SIXTHSEMESTER\IP\important\final_dataset.xlsx")
df_imbalanced = dd.from_pandas(df_imbalanced_pandas, npartitions=10) # Adjust npartitions as needed
# Load the normal dataset
df_normal_pandas = pd.read_excel(r"D:\SIXTHSEMESTER\IP\important\final_normal.xlsx")
df_normal = dd.from_pandas(df_normal_pandas, npartitions=10)
# Load additional datasets with failure labeled as 2 and 3
df_failure2_pandas = pd.read_excel(r"D:\SIXTHSEMESTER\IP\important\final_horiz_alignment.xlsx")
df_failure2 = dd.from_pandas(df_failure2_pandas, npartitions=10)
df_failure3_pandas = pd.read_excel(r"D:\SIXTHSEMESTER\IP\important\final_vertical_alignment.xlsx")
df_failure3 = dd.from_pandas(df_failure3_pandas, npartitions=10)
# Sample the data
df_imbalanced_sampled = df_imbalanced.sample(frac=0.1, random_state=42)
df_normal_sampled = df_normal.sample(frac=0.1, random_state=42)
df_failure2_sampled = df_failure2.sample(frac=0.1, random_state=42)
df_failure3_sampled = df_failure3.sample(frac=0.1, random_state=42)
# Add a label column to distinguish between normal and failure samples
df_imbalanced_sampled['label'] = 1
df_normal_sampled['label'] = 0
df_failure2_sampled['label'] = 2
df_failure3_sampled['label'] = 3
# Concatenate the datasets
df_combined = dd.concat([df_imbalanced_sampled, df_normal_sampled,
df_failure2_sampled, df_failure3_sampled])
# Drop the 'failure' feature from the combined dataset
df_combined = df_combined.drop('failure', axis=1)
# Remove the first row (index 0) from the DataFrame
# Remove the first row (index 0) from the DataFrame
df_combined = df_combined.loc[df_combined.index != 0]
# Split the data into features (X) and labels (y)
X = df_combined.drop('label', axis=1)
y = df_combined['label']
# Split the data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42,
shuffle=True)
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
# Perform cross-validation using GridSearchCV
grid_search = GridSearchCV(clf, param_grid, cv=5, scoring='accuracy')
grid_search.fit(X_train_scaled, y_train)
# Get the best estimator
best_clf = grid_search.best_estimator_
# Make predictions
y_pred = best_clf.predict(X_test_scaled)
# Convert Dask arrays to NumPy arrays
X_test_scaled_np = X_test_scaled.compute()
y_test_np = y_test.compute()
# Calculate accuracy
accuracy = accuracy_score(y_test_np, y_pred)
print("Accuracy:", accuracy)
accuracy_percentage = accuracy * 100
print("Accuracy:", accuracy_percentage)
# Save the best model
with open('best_model.pkl', 'wb') as f:
    pickle.dump(best_clf, f)
# Load the saved model
with open('best_model.pkl', 'rb') as f:
    loaded_model = pickle.load(f)
# Save the scaler object
with open('scaler.pkl', 'wb') as f:
    pickle.dump(scaler, f)
# Load the scaler object
with open('scaler.pkl', 'rb') as f:
    scaler = pickle.load(f)