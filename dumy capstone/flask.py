from flask import Flask, render_template, request, jsonify
import firebase_admin
from firebase_admin import db, credentials
import pickle
import numpy as np
app = Flask(__name__)
app.secret_key = 'bcdv'
# Initialize Firebase Admin SDK
if not firebase_admin._apps:
    cred = credentials.Certificate("credentials.json")
    firebase_admin.initialize_app(cred, {'databaseURL': 'https://iptempsensor-default-rtdb.asia-southeast1.firebasedatabase.app/'})
else:
    default_app = firebase_admin.get_app()
# Load the model
with open('best_model.pkl', 'rb') as f:
    model = pickle.load(f)
# Load the scaler
with open('scaler.pkl', 'rb') as f:
    scaler = pickle.load(f)
# Function to preprocess data using the scaler
def preprocess_data(a1, a2, a3,temp):
# Scale the input data
    input_data = np.array([[a1, a2, a3,temp]])
    input_data_scaled = scaler.transform(input_data)
    return input_data_scaled
@app.route('/')
def index():
    return render_template('index.html')
@app.route('/latest_data', methods=['GET'])
def get_latest_data():
    try:
# Get the latest acceleration values from Firebase Realtime Database
        ref = db.reference('/')
        latest_data = ref.get()
        if latest_data:
# Extract the last value of each dictionary for X, Y, and Z nodes
            a1 = list(latest_data['X']['float'].values())[-1]
            a2 = list(latest_data['Y']['float'].values())[-1] # Access Y values from the nested dictionary
            a3 = list(latest_data['Z']['float'].values())[-1] # Access Z values from the nested dictionary
            temp = list(latest_data['temp']['float'].values())[-1]
            return jsonify({
            'a1': a1, # Acceleration X
            'a2': a2, # Acceleration Y
            'a3': a3, # Acceleration Z
            'temp' : temp
            })
        else:   
            return jsonify({'error': 'Acceleration data not found'})
    except Exception as e:
        return jsonify({'error': str(e)})
@app.route('/predict', methods=['POST'])
def predict():
    try:
        a1 = float(request.form['a1'])
        a2 = float(request.form['a2'])
        a3 = float(request.form['a3'])
        temp = float(request.form['temp'])
        print("Request Form Data:", request.form)
        input_data = np.array([[a1, a2, a3,temp]])
        input_data_scaled = preprocess_data(a1, a2, a3,temp)
        prediction_proba = model.predict_proba(input_data_scaled)[0]
        predicted_class = model.predict(input_data_scaled)[0] # Get the predicted class
        class_label = {0: 'Normal', 1: 'Imbalance', 2: 'Horizontal Misalignment', 3: 'VerticalMisalignment'}[predicted_class]
        class_proba = prediction_proba[predicted_class] # Access probability of predicted class
        output_data = {'class': class_label, 'probability': class_proba}
        error_signal = None
        if class_label != 'Normal': # Check for non-normal classes
            error_signal = f"Possible {class_label} detected. Take action!"
        output_data['error_signal'] = error_signal
        return jsonify({'class': class_label, 'probability': class_proba,'error_signal': error_signal})
#return jsonify(output_data)
    except Exception as e:
        return jsonify({'error': str(e)})
if __name__ == '__main__':
    app.run(debug=True, use_reloader=False)