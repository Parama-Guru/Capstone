from flask import Flask, render_template, request, jsonify
import pickle
import numpy as np

app = Flask(__name__)
app.secret_key = 'bcdv'

# Load the model and scaler
with open('/Users/guru/vsc/capstone/model/best_model.pkl', 'rb') as f:
    model = pickle.load(f)
with open('/Users/guru/vsc/capstone/model/scaler.pkl', 'rb') as f:
    scaler = pickle.load(f)

# Preprocess data using the scaler
def preprocess_data(*features):
    input_data = np.array([features])  # Use all input features
    return scaler.transform(input_data)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/predict', methods=['POST'])
def predict():
    try:
        # Get input values and preprocess
        inputs = [float(request.form[f'feature_{i}']) for i in range(1, 9)]  # Collect all 8 features
        input_data_scaled = preprocess_data(*inputs)  # Unpack inputs for scaling

        # Make predictions
        prediction_proba = model.predict_proba(input_data_scaled)[0]
        predicted_class = model.predict(input_data_scaled)[0]
        class_label = {0: 'Normal', 1: 'Imbalance'}[predicted_class]
        class_proba = prediction_proba[predicted_class]

        # Construct response
        error_signal = None
        if class_label != 'Normal':
            error_signal = f"Possible {class_label} detected. Take action!"
        return jsonify({'class': class_label, 'probability': class_proba, 'error_signal': error_signal})

    except Exception as e:
        return jsonify({'error': str(e)})

if __name__ == '__main__':
    app.run(debug=True, use_reloader=False)
