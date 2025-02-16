#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define API_KEY "your_firebase_api_key"
#define DATABASE_URL "your_firebase_database_url"

FirebaseData fbdo;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void displaySensorDetails(void) {
    sensor_t sensor;
    accel.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print("Sensor: "); Serial.println(sensor.name);
    Serial.print("Driver Ver: "); Serial.println(sensor.version);
    Serial.print("Unique ID: "); Serial.println(sensor.sensor_id);
    Serial.print("Max Value: "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
    Serial.print("Min Value: "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
    Serial.print("Resolution: "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
    Serial.println("------------------------------------");
    Serial.println("");
}

void displayDataRate(void) {
    Serial.print("Data Rate: ");
    switch (accel.getDataRate()) {
        case ADXL345_DATARATE_3200_HZ: Serial.print("3200 "); break;
        case ADXL345_DATARATE_1600_HZ: Serial.print("1600 "); break;
        case ADXL345_DATARATE_800_HZ: Serial.print("800 "); break;
        case ADXL345_DATARATE_400_HZ: Serial.print("400 "); break;
        case ADXL345_DATARATE_200_HZ: Serial.print("200 "); break;
        case ADXL345_DATARATE_100_HZ: Serial.print("100 "); break;
        case ADXL345_DATARATE_50_HZ: Serial.print("50 "); break;
        case ADXL345_DATARATE_25_HZ: Serial.print("25 "); break;
        case ADXL345_DATARATE_12_5_HZ: Serial.print("12.5 "); break;
        case ADXL345_DATARATE_6_25HZ: Serial.print("6.25 "); break;
        case ADXL345_DATARATE_3_13_HZ: Serial.print("3.13 "); break;
        case ADXL345_DATARATE_1_56_HZ: Serial.print("1.56 "); break;
        case ADXL345_DATARATE_0_78_HZ: Serial.print("0.78 "); break;
        case ADXL345_DATARATE_0_39_HZ: Serial.print("0.39 "); break;
        case ADXL345_DATARATE_0_20_HZ: Serial.print("0.20 "); break;
        case ADXL345_DATARATE_0_10_HZ: Serial.print("0.10 "); break;
        default: Serial.print("???? "); break;
    }
    Serial.println(" Hz");
}

void displayRange(void) {
    Serial.print("Range: +/- ");
    switch (accel.getRange()) {
        case ADXL345_RANGE_16_G: Serial.print("16 "); break;
        case ADXL345_RANGE_8_G: Serial.print("8 "); break;
        case ADXL345_RANGE_4_G: Serial.print("4 "); break;
        case ADXL345_RANGE_2_G: Serial.print("2 "); break;
        default: Serial.print("?? "); break;
    }
    Serial.println(" g");
}

void setup() {
    Serial.begin(115200);
    pinMode(ONE_WIRE_BUS, INPUT);
    Serial.println("Accelerometer Test");

    if (!accel.begin()) {
        Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
        while (1);
    }

    accel.setRange(ADXL345_RANGE_2_G);
    displaySensorDetails();
    displayDataRate();
    displayRange();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    FirebaseAuth auth;
    auth.user.email = "your_email";
    auth.user.password = "your_password";
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
    } else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());