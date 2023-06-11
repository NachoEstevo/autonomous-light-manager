#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//Firebase Setup
#define WIFI_SSID "WIFI-NAME"
#define WIFI_PASSWORD "WIFI-PASSWORD"
#define API_KEY "API"
#define DATABASE_URL "DB_URL"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//Declared PINS
const int Trigger = 14;
const int Echo = 12;
const int Echo2 = 27;
const int Trigger2 = 26;
const int ROOM_LED = 25;
const int MICROPHONE_PIN = 33;
const int PWM_PIN = 34;
const int PIN_RED = 18;
const int PIN_GREEN = 19;
const int PIN_BLUE = 21;

//Global Variables
bool clap_trigger = false;
unsigned long lastClapTime = 0;
const unsigned long debounceDelay = 200;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
int people_count = 0;
long initDist_1, initDist_2;
bool isCalibrated = false;
bool person_passing = false;
volatile bool clap_trigger_digital = false;
void firebase(void* parameter);
TaskHandle_t firebaseHandle;

void setup() {
  Serial.begin(9600);
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
  digitalWrite(Trigger, LOW);
  pinMode(MICROPHONE_PIN, INPUT_PULLUP);
  pinMode(Trigger2, OUTPUT);
  pinMode(Echo2, INPUT);
  digitalWrite(Trigger2, LOW);
  attachInterrupt(MICROPHONE_PIN, clapInterrupt, CHANGE);
  connectWifi();
  calibrate();
  xTaskCreate(firebase, "Firebase Task", 10000, NULL, 1, &firebaseHandle);
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

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  delay(1500);
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void firebase(void* parameter) {
  while (true) {
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      colorUpdater("RED", PIN_RED);
      colorUpdater("GREEN", PIN_GREEN);
      colorUpdater("BLUE", PIN_BLUE);
      vTaskDelay(pdMS_TO_TICKS(1500));
    }
  }
}
void calibrate() {
  initDist_1 = calculateDistance(Trigger, Echo);
  initDist_2 = calculateDistance(Trigger2, Echo2);
  isCalibrated = true;
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

bool inThreashold(int measuredDist, int initDist) {
  return measuredDist <= initDist + initDist * 0.2 + 1 && measuredDist >= initDist - initDist * 0.2 + 1;
}

void distance() {
  // Read distance from Sensor 1
  long distance_1 = calculateDistance(Trigger, Echo);
  // Read distance from Sensor 2
  long distance_2 = calculateDistance(Trigger2, Echo2);

  if (inThreashold(distance_1, initDist_1) && inThreashold(distance_2, initDist_2)) {
    person_passing = false;
  }

  if (!person_passing) {

    if (distance_1 <= 40 && distance_2 >= 40) {
      Serial.println("Leaving");
      person_passing = true;
      people_count--;
      Serial.print("People count: ");
      Serial.println(people_count);
      delay(650);
      return;
    }
    if (distance_2 < 40) {
      Serial.println("Entering");
      person_passing = true;
      people_count++;
      Serial.print("People count: ");
      Serial.println(people_count);
      delay(650);
      return;
    }
    delay(0);
  }
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


void adjustLedToLight(int led_pin, int trigger_pin) {
  if (people_count <= 0) {
    analogWrite(led_pin, 0);
  } else {
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
}

void loop() {
  adjustLedToLight(ROOM_LED, MICROPHONE_PIN);
  distance();
}
