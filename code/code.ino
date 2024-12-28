// Importing libraries
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "arduino_secrets.h"
#include "time.h"

// time settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5*60*60; // EST 
const int   daylightOffset_sec = 3600;


// LED & button connections to the ESP
const int ledPin = 4;
const int buttonPin = 14;
const int buttonLedPin = 25;

// Define Firebase Data object and other necessary objects
bool firebaseData = false;
int buttonState = 1;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long lastFirebaseUpdate = 0;
int count = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonLedPin, OUTPUT);

  connectWiFi();
  connectFirebase();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  digitalWrite(buttonLedPin, HIGH);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - lastFirebaseUpdate > 1000)) {  // timer
    lastFirebaseUpdate = millis();

    buttonState = digitalRead(buttonPin); //checking if our button is pressed
    uploadData(buttonState);
    downloadData();
    manageLED(buttonState, firebaseData); // turning the LED on or off

    printLocalTime();
  }
  delay(5);
}

void connectWiFi() {
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to Wifi...");
    digitalWrite(buttonLedPin, HIGH);
    delay(500);
    digitalWrite(buttonLedPin, LOW);
    delay(500);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void connectFirebase() {
  config.api_key = SECRET_API_KEY;
  config.database_url = SECRET_DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase error");
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void uploadData(int buttonstate) {
  if (Firebase.RTDB.setInt(&fbdo, "/user_1", buttonstate)) {
    Serial.printf("Data UPLOAD successful,  ");
  } else {
    Serial.println("Data UPLOAD failed,  ");
  }
}

void downloadData() {
  if (Firebase.RTDB.getInt(&fbdo, "/user_2")) {
    firebaseData = fbdo.intData();
    Serial.println("Data DOWNLOAD successful");
  } else {
    Serial.println("Data DOWNLOAD failed");
  }
}

void manageLED(int buttonState, int firebaseData) {
  if (buttonState == HIGH or firebaseData == HIGH) {
    digitalWrite(ledPin, HIGH); // turning on the LED
    delay(1000);
    digitalWrite(ledPin, LOW); // turning off the LED
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}