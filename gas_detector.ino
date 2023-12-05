#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial gsmSerial(6, 7); // SoftwareSerial for GSM module
String writeApiKey = "3DTMHW1H20UCVI7B"; // ThingSpeak API Key
const int gpsRxPin = 2; // Connect GPS TX to this pin on Arduino
const int gpsTxPin = 3; // Connect GPS RX to this pin on Arduino
SoftwareSerial gpsSerial(gpsRxPin, gpsTxPin);
const int gasThreshold = 75;
TinyGPSPlus gps;
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiWire oled;
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
 

 // Serial.println(F("Gas sensor warmed up!"));
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
   oled.set2X();
  oled.println("Gas Detection!");
  //oled.println("A long line may be truncated");
  oled.println();

  gsmSerial.begin(9600);
  delay(14000);
  gpsSerial.begin(9600);
}
//------------------------------------------------------------------------------
void loop() {
  float sensorValue = analogRead(A0);

  updateDisplay(sensorValue);

//  Serial.print(F("Gas Value: "));
//  Serial.println(sensorValue);

showGpsData();
    
  String gpsMessage = "Latitude: " + String(gps.location.lat(), 6) +
                            " | Longitude: " + String(gps.location.lng(), 6);


  if (sensorValue > gasThreshold) {
    
    sendSMS("Gas detected! Take necessary action " + gpsMessage);
    Serial.println("Sending to ThingSpeak...");
    sendToThingSpeak(sensorValue, gps.location.lat(), gps.location.lng());
    delay(5000);  // Wait before checking again
  }

  delay(2000);
}
//
void updateDisplay(float value) {
//  myDisplay.clearDisplay();
//  myDisplay.setTextSize(2);
oled.clear();
 oled.set2X();
//  myDisplay.setTextColor(SSD1306_WHITE);
//  myDisplay.setCursor(0, 0);
  oled.println(F("Gas Value: "));
oled.println(value);
oled.println();
//  myDisplay.display();
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
  gsmSerial.println(F("AT+CMGF=1")); // Set the GSM Module to text mode
  delay(2000);
  gsmSerial.println(F("AT+CMGS=\"+2347082257453\""));
  delay(100);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.write(26); // ASCII code of CTRL+Z
  delay(100);
  gsmSerial.println();
  delay(5000); 
  Serial.println(F("SMS sent."));
}

void showGpsData() {
  while (gpsSerial.available() > 0) {
      
    // Read the GPS data
    if (gps.encode(gpsSerial.read())) {
      Serial.println("I am here");
      // Check if GPS data is valid
      if (gps.location.isValid()) {
        // Print latitude and longitude
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print(" | Longitude: ");
        Serial.println(gps.location.lng(), 6);
//        String gpsMessage = "Latitude: " + String(gps.location.lat(), 6) +
//                            " | Longitude: " + String(gps.location.lng(), 6);

      } else {
        // If GPS data is not valid, print a message
        Serial.println("No GPS fix available");
      }
    }
  }
}