#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "UA-Alumnos"
#define WIFI_PASSWORD "41umn05WLC"
const int Trigger = 14;
const int Echo = 12;
const int Echo2 = 27;
const int Trigger2 = 26;
const int ROOM_LED = 25;
const int MICROPHONE_PIN = 33;
bool clap_trigger = false;
unsigned long lastClapTime = 0;
const unsigned long debounceDelay = 200;
// Insert Firebase SETUP
#define API_KEY "AIzaSyA2gv3pjLAAw3Euu2XaU22TCI5RwufHOYQ"
#define DATABASE_URL "https://light-colors-12927-default-rtdb.firebaseio.com/"
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
volatile bool clap_trigger_digital = false;
void setup() {
  Serial.begin(9600);          //iniciailzamos la comunicación
  pinMode(Trigger, OUTPUT);    //pin como salida
  pinMode(Echo, INPUT);        //pin como entrada
  digitalWrite(Trigger, LOW);  //Inicializamos el pin con 0
  pinMode(MICROPHONE_PIN, INPUT_PULLUP);
  pinMode(Trigger2, OUTPUT);
  pinMode(Echo2, INPUT);
  digitalWrite(Trigger2, LOW);
  attachInterrupt(MICROPHONE_PIN, clapInterrupt, CHANGE);
  connectWifi();

  calibrate();

  Serial.print("initDist1");
  Serial.println(initDist_1); 
  Serial.print("initDist2");
  Serial.println(initDist_2);


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
  config.api_key = API_KEY;
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
void clapInterrupt() {
  unsigned long currentTime = millis();
  if (currentTime - lastClapTime >= debounceDelay) {
    lastClapTime = currentTime;
    clap_trigger_digital = !clap_trigger_digital;
    digitalWrite(ROOM_LED, clap_trigger_digital);
    Serial.print("Clap value: ");
    Serial.println(clap_trigger_digital);
  }
}
void calibrate() {
  initDist_1 = calculateDistance(Trigger, Echo);
  initDist_2 = calculateDistance(Trigger2, Echo2);
  isCalibrated = true;
}

bool inThreashold(int measuredDist, int initDist){
  return measuredDist <= initDist + initDist * 0.2 + 1 && 
            measuredDist >= initDist - initDist * 0.2 + 1;
}

void distance() {

  Serial.println(person_passing);
  // Read distance from Sensor 1
  long distance_1 = calculateDistance(Trigger, Echo);
  // Read distance from Sensor 2
  long distance_2 = calculateDistance(Trigger2, Echo2);

  if (inThreashold(distance_1, initDist_1) && inThreashold(distance_2, initDist_2)) {
    person_passing = false;
  }

  if (person_passing) {return;}

  // Compare distance values to threshold
  if (distance_1 < initDist_1 - initDist_1 * 0.2 + 1) {
    Serial.println("Leaving");
    person_passing = true;
    people_count--;
  }
  if (distance_2 < initDist_2 - initDist_2 * 0.2 + 1) {
    Serial.println("Entering");
    person_passing = true;
    people_count++;
  }
  Serial.print("People count: ");
  Serial.println(people_count);

  delay(300);  // wait a little bit before next reading
}

int calculateDistance(int trigg, int ech) {
  digitalWrite(trigg, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigg, LOW);
  long t = pulseIn(ech, HIGH);
  return t / 58;
}
void colorUpdater(char color[], int pin) {
  if (Firebase.RTDB.getInt(&fbdo, color)) {
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
  }
}

void adjustLedToLight(int led_pin, int trigger_pin) {
  int analogValue = analogRead(PWM_PIN);
  if (clap_trigger_digital) {
    analogWrite(led_pin, 0);
  } else if (analogValue <= 1800) {
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
  distance();
  //adjustLedToLight(ROOM_LED, MICROPHONE_PIN);
}