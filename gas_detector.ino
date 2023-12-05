#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial gsmSerial(6, 7);
String writeApiKey = "3DTMHW1H20UCVI7B";

const int gpsRxPin = 2;
const int gpsTxPin = 3;

SoftwareSerial gpsSerial(gpsRxPin, gpsTxPin);
const int gasThreshold = 80;
TinyGPSPlus gps;
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
#define BUZZER_PIN 8

SSD1306AsciiWire oled;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000L);

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);

  uint32_t m = micros();
  oled.clear();
  oled.set1X();
  oled.println("Gas Detection!");
  oled.println();
  oled.println("Please Wait!");

  gsmSerial.begin(9600);
  delay(14000);
  gpsSerial.begin(9600);
}

void loop() {
  float sensorValue = analogRead(A0);

  updateDisplay(sensorValue);

  showGpsData();

  String gpsMessage = "Latitude: " + String(gps.location.lat(), 6) +
                      ", Longitude: " + String(gps.location.lng(), 6);

  if (sensorValue > gasThreshold) {
    activateBuzzer();
    sendSMS("Gas detected! Take necessary action " + gpsMessage);
    Serial.println("Sending to ThingSpeak...");
    sendToThingSpeak(sensorValue, gps.location.lat(), gps.location.lng());
    deactivateBuzzer();
    delay(10000);
  }

  delay(2000);
}

void updateDisplay(float value) {
  oled.clear();
  oled.set1X();
  oled.println(F("Gas Value: "));
  oled.println(value);
  oled.println();
  if (gps.location.isValid()) {
    oled.println(F("Latitude: "));
    oled.println(gps.location.lat(), 4);
    oled.println(F("Longitude: "));
    oled.println(gps.location.lng(), 3);
  } else {
    oled.println(F("No GPS fix available"));
  }
}

void sendToThingSpeak(float gasValue, float latitude, float longitude) {
  Serial.println(F("Closing existing connections..."));
  gsmSerial.println(F("AT+CIPSHUT"));
  delay(500);

  Serial.println(F("Setting single connection mode..."));
  gsmSerial.println(F("AT+CIPMUX=0"));
  delay(500);

  Serial.println(F("Setting APN..."));
  gsmSerial.println(F("AT+CSTT=\"internet.ng.airtel.com\""));
  delay(2000);

  Serial.println(F("Bringing up wireless connection..."));
  gsmSerial.println(F("AT+CIICR"));
  delay(2000);

  Serial.println(F("Getting local IP address..."));
  gsmSerial.println(F("AT+CIFSR"));
  delay(2000);

  Serial.println(F("Connecting to ThingSpeak..."));
  gsmSerial.println(F("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\""));
  delay(10000);
  while (gsmSerial.available()) {
    Serial.write(gsmSerial.read());
  }

  Serial.println(writeApiKey);
  Serial.println(gasValue);

  String url = "GET /update?api_key=" + writeApiKey +
               "&field1=" + String(gasValue) +
               "&field2=" + String(latitude, 6) +
               "&field3=" + String(longitude, 6);

  Serial.println(url);
  Serial.print(F("Sending data: "));
  Serial.println(url);
  delay(2000);

  Serial.println("Sending data to ThingSpeak...");
  gsmSerial.print(F("AT+CIPSEND="));
  gsmSerial.println(url.length() + 2);
  delay(500);

  gsmSerial.print(url);
  gsmSerial.write(0x0D);
  gsmSerial.write(0x0A);
  delay(8000);

  while (gsmSerial.available()) {
    Serial.write(gsmSerial.read());
  }

  Serial.println(F("Closing the connection..."));
  gsmSerial.println(F("AT+CIPSHUT"));
  delay(500);
}

void sendSMS(String message) {
  Serial.println(F("Sending SMS..."));
  gsmSerial.println(F("AT+CMGF=1"));
  delay(2000);
  gsmSerial.println(F("AT+CMGS=\"+2347082257453\""));
  delay(100);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.write(26);
  delay(100);
  gsmSerial.println();
  delay(5000);
  Serial.println(F("SMS sent."));
}

void showGpsData() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      Serial.println("I am here");
      if (gps.location.isValid()) {
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print(" | Longitude: ");
        Serial.println(gps.location.lng(), 6);
      } else {
        Serial.println("No GPS fix available");
      }
    }
  }
}

void activateBuzzer() {
  tone(BUZZER_PIN, 1000);
}

void deactivateBuzzer() {
  noTone(BUZZER_PIN);
}
