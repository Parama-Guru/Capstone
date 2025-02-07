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
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Insert your network credentials
#define WIFI_SSID "vivo Y16"
#define WIFI_PASSWORD "dhivya@123"
#define API_KEY "AIzaSyA0N1KJ0FZ6ITBR7Hv7081_ery2MNW0HbQ"
#define DATABASE_URL "https://ipdemoday-default-rtdb.asia-
southeast1.firebasedatabase.app/"
//Define Firebase Data object
FirebaseData fbdo;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0
bool signupOK = false;
void displaySensorDetails(void)
{
    sensor_t sensor;
    accel.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print ("Sensor: "); Serial.println(sensor.name);
    Serial.print ("Driver Ver: "); Serial.println(sensor.version);
    Serial.print ("Unique ID: "); Serial.println(sensor.sensor_id);
    Serial.print ("Max Value: "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
    Serial.print ("Min Value: "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
    Serial.print ("Resolution: "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
    Serial.println("------------------------------------");
    Serial.println("");
}
void displayDataRate(void)
{
Serial.print ("Data Rate: ");
switch(accel.getDataRate())
{
    case ADXL345_DATARATE_3200_HZ:
        Serial.print ("3200 ");
        break;
    case ADXL345_DATARATE_1600_HZ:
        Serial.print ("1600 ");
        break;
    case ADXL345_DATARATE_800_HZ:
        Serial.print ("800 ");
        break;
    case ADXL345_DATARATE_400_HZ:
        Serial.print ("400 ");
        break;
    case ADXL345_DATARATE_200_HZ:
        Serial.print ("200 ");
        break;
    case ADXL345_DATARATE_100_HZ:
        Serial.print ("100 ");
        break;
    case ADXL345_DATARATE_50_HZ:
        Serial.print ("50 ");
        break;
    case ADXL345_DATARATE_25_HZ:
        Serial.print ("25 ");
        break;
    case ADXL345_DATARATE_12_5_HZ:
        Serial.print ("12.5 ")
        break;
    case ADXL345_DATARATE_6_25HZ:
        Serial.print ("6.25 ");
        break;
    case ADXL345_DATARATE_3_13_HZ:
        Serial.print ("3.13 ");
        break;
    case ADXL345_DATARATE_1_56_HZ:
        Serial.print ("1.56 ");
        break;
    case ADXL345_DATARATE_0_78_HZ:
        Serial.print ("0.78 ");
        break;
    case ADXL345_DATARATE_0_39_HZ:
        Serial.print ("0.39 ");
        break;
    case ADXL345_DATARATE_0_20_HZ:
        Serial.print ("0.20 ");
        break;
    case ADXL345_DATARATE_0_10_HZ:
        Serial.print ("0.10 ");
        break;
    default:
        Serial.print ("???? ");
        break;
    }
    Serial.println(" Hz");
}
void displayRange(void)
{
Serial.print ("Range: +/- ");
switch(accel.getRange())
    {
    case ADXL345_RANGE_16_G:
        Serial.print ("16 ");
        break;
    case ADXL345_RANGE_8_G:
        Serial.print ("8 ");
        break;
    case ADXL345_RANGE_4_G:
        Serial.print ("4 ");
        break;
    case ADXL345_RANGE_2_G:
        Serial.print ("2 ");
        break;
    default:
        Serial.print ("?? ");
        break;
    }
Serial.println(" g");
}

void setup(){
Serial.begin(115200);
pinMode(ONE_WIRE_BUS, INPUT);
Serial.println("Accelerometer Test"); Serial.println("");
/* Initialise the sensor */
if(!accel.begin())
{
/* There was a problem detecting the ADXL345 ... check your connections */
Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
while(1);
}
/* Set the range to whatever is appropriate for your project */
//accel.setRange(ADXL345_RANGE_16_G);
// accel.setRange(ADXL345_RANGE_8_G);
// accel.setRange(ADXL345_RANGE_4_G);
accel.setRange(ADXL345_RANGE_2_G);
/* Display some basic information on this sensor */
displaySensorDetails();
/* Display additional settings (outside the scope of sensor_t) */
displayDataRate();
displayRange();
Serial.println("");
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting to Wi-Fi");
while (WiFi.status() != WL_CONNECTED){
Serial.print(".");
}
Serial.println();
Serial.print("Connected with IP: ");
Serial.println(WiFi.localIP());
Serial.println();
/* Assign the api key (required) */
config.api_key = API_KEY;
/* Assign the RTDB URL (required) */
config.database_url = DATABASE_URL
/* Sign up */
FirebaseAuth auth;
auth.user.email = "dhivyark39@gmail.com";
auth.user.password = "cool@262004";
if (Firebase.signUp(&config, &auth, "", "")){
Serial.println("ok");
signupOK = true;
}
else{
Serial.printf("%s\n", config.signer.signupError.message.c_str());
}
/* Assign the callback function for the long running token generation task */
config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
Firebase.begin(&config, &auth);
Firebase.reconnectWiFi(true);
sensors.begin();
}
void loop(){
if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 150 ||
sendDataPrevMillis == 0)){
sendDataPrevMillis = millis();
sensors_event_t event;
accel.getEvent(&event);
/* Display the results (acceleration is measured in m/s^2) */
Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" ");
Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print(" ");
Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print(" ");Serial.println("m/s^2
");
// Write an Int number on the database path test/int
/*if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
count++;*/
// Write an Float number on the database path test/float
sensors.requestTemperatures()
float temp = sensors.getTempCByIndex(0);
Serial.println(temp);
if (Firebase.RTDB.pushFloat(&fbdo, "temp/float",temp)){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
if (Firebase.RTDB.pushFloat(&fbdo, "X/float",(event.acceleration.x))){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
if (Firebase.RTDB.pushFloat(&fbdo, "Y/float",(event.acceleration.y))){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
if (Firebase.RTDB.pushFloat(&fbdo, "Z/float",(event.acceleration.z))){
Serial.println("PASSED");
Serial.println("PATH: " + fbdo.dataPath());
Serial.println("TYPE: " + fbdo.dataType());
}
else {
Serial.println("FAILED");
Serial.println("REASON: " + fbdo.errorReason());
}
}
}