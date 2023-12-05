#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(6, 7);
String writeApiKey = "3DTMHW1H20UCVI7B";
const int gasThreshold = 75;
#define I2C_ADDRESS 0x3C
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
  oled.println("Gas Detector Sys!");
  oled.println();

  gsmSerial.begin(9600);
  delay(14000);
}

void loop() {
  float sensorValue = analogRead(A0);

  updateDisplay(sensorValue);

  Serial.print(F("Gas Value: "));
  Serial.println(sensorValue);

  if (sensorValue > gasThreshold) {
    sendSMS("Gas detected! Take necessary action.");
    Serial.println("Sending to ThingSpeak...");
    sendToThingSpeak(sensorValue);
    // Wait before checking again
    delay(5000);
  }

  delay(2000);
}

// Update the OLED display with the given value.
void updateDisplay(float value) {
  oled.clear();
  oled.set2X();
  oled.println(F("Gas Value: "));
  oled.println(value);
  oled.println();
}

// Send data to ThingSpeak.
void sendToThingSpeak(float gasValue) {
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
               "&field1=" + String(gasValue);

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

// Send SMS
void sendSMS(String message) {
  Serial.println(F("Sending SMS..."));
  gsmSerial.println(F("AT+CMGF=1")); // Set the GSM Module to text mode
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
