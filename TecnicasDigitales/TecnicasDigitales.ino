#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "UA-Alumnos"
#define WIFI_PASSWORD "41umn05WLC"
const int Trigger = 14;  //Pin digital 2 para el Trigger del sensor
const int Echo = 12;     //Pin digital 3 para el Echo del sensor
const int Echo2 = 27;
const int Trigger2 = 26;
const int ROOM_LED = 13;
// Insert Firebase project API Key
#define API_KEY "AIzaSyA2gv3pjLAAw3Euu2XaU22TCI5RwufHOYQ"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://light-colors-12927-default-rtdb.firebaseio.com/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
const int PWM_PIN = 34;
const int PIN_RED = 18;
const int PIN_GREEN = 19;
const int PIN_BLUE = 21;
int people_count = 0;
long initDist_1, initDist_2;
bool isCalibrated = false;
bool person_passing = false;
void setup() {
  Serial.begin(9600);          //iniciailzamos la comunicación
  pinMode(Trigger, OUTPUT);    //pin como salida
  pinMode(Echo, INPUT);        //pin como entrada
  digitalWrite(Trigger, LOW);  //Inicializamos el pin con 0

  pinMode(Trigger2, OUTPUT);
  pinMode(Echo2, INPUT);
  digitalWrite(Trigger2, LOW);
  // xTaskCreate(firebase, "Firebase ", 10000, NULL, 1, NULL);
  // xTaskCreate(ultrasonic, "Ultrasonic Sensors", 10000, NULL, 1, NULL);

  connectWifi();

  calibrate();

  pinMode(PIN_RED, OUTPUT);
  // pinMode(PIN_GREEN, OUTPUT);
  // pinMode(PIN_BLUE,  OUTPUT);

  // analogWrite(PIN_RED,   0);
  // analogWrite(PIN_GREEN, 151);
  // analogWrite(PIN_BLUE,  157);
}
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void calibrate() {
  initDist_1 = calculateDistance(Trigger, Echo);
  initDist_2 = calculateDistance(Trigger2, Echo2);
  isCalibrated = true;
}

void distance(int trigg, int e) {
  // Read distance from Sensor 1
  long distance_1 = calculateDistance(Trigger, Echo);
  Serial.print("dist1 ");
  Serial.println(distance_1);

  // Read distance from Sensor 2
  long distance_2 = calculateDistance(Trigger2, Echo2);
  Serial.print("dist2 ");
  Serial.println(distance_2);

  if (distance_1 == initDist_1 && distance_2 == initDist_2) {
    person_passing = false;
  }

  if (person_passing) {
    return;
  }

  // Compare distance values to threshold
  if (distance_1 < initDist_1 - 10) {
    Serial.println("leaving");
    person_passing = true;
    people_count--;
  }

  if (distance_2 < initDist_2 - 10) {
    person_passing = true;
    people_count++;
  }

  // Print people count to serial monitor
  Serial.print("People count: ");
  Serial.println(people_count);

  delay(100);  // wait a little bit before next reading
}
void regulateLight(int lsens) {
}

int calculateDistance(int trigg, int ech) {
  digitalWrite(trigg, HIGH);
  delayMicroseconds(10);  //Enviamos un pulso de 10us
  digitalWrite(trigg, LOW);
  long t = pulseIn(ech, HIGH);  //obtenemos el ancho del pulso
  return t / 59;
}
void colorUpdater(char color[], int pin) {
  if (Firebase.RTDB.getInt(&fbdo, color)) {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
    Serial.print("Value: ");
    Serial.println(fbdo.intData());
    analogWrite(pin, 255 - fbdo.intData());

  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
  count++;
}

void firebase(/*void* parameter*/) {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    colorUpdater("RED", PIN_RED);
    colorUpdater("GREEN", PIN_GREEN);
    colorUpdater("BLUE", PIN_BLUE);
    // create function and pass color as parameter. Update 3 colors inside if
  }
}
void distancePrinters(/*void* parameter*/) {
  distance(Trigger, Echo);
  distance(Trigger2, Echo2);
}
void adjustLedToLight(int led_pin) {
  int analogValue = analogRead(PWM_PIN);
  Serial.print("Analog reading: ");
  Serial.println(analogValue);  // the raw analog reading
  if (analogValue <= 1800) {
    analogWrite(led_pin, 150);
  } else if (analogValue <= 2300) {
    analogWrite(led_pin, 100);
  } else if (analogValue <= 3000) {
    analogWrite(led_pin, 50);
  } else {
    analogWrite(led_pin, 0);
  }
}

void loop() {
  //firebase();
  distancePrinters();

  //adjustLedToLight(ROOM_LED);
  delay(500);
}