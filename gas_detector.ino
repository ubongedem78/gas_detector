#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(6, 7); // SoftwareSerial for GSM module

Adafruit_SSD1306 myDisplay(128, 64, &Wire); // OLED display

String writeApiKey = "3DTMHW1H20UCVI7B"; // ThingSpeak API Key

const int gasThreshold = 75;

void setup() {
  Serial.begin(9600);

  gsmSerial.begin(9600);
  delay(2000);

//  myDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//  myDisplay.clearDisplay();
//  myDisplay.setTextSize(2);
//  myDisplay.setTextColor(SSD1306_WHITE);
//  myDisplay.setCursor(0, 0);
//  myDisplay.println(F("Gas Detector System"));
//  myDisplay.display();

  Serial.println(F("Initializing GSM..."));
  delay(2000);

  Serial.println(F("Warming up gas sensor..."));
  delay(10000);
  Serial.println(F("Gas sensor warmed up!"));
}

void loop() {
  float sensorValue = analogRead(A0);

//  updateDisplay(sensorValue);

  Serial.print(F("Gas Value: "));
  Serial.println(sensorValue);

  if (sensorValue > gasThreshold) {
    sendSMS("Gas detected! Take necessary action.");
    Serial.println("Sending to ThingSpeak...");
    sendToThingSpeak(sensorValue);
    delay(5000);  // Wait before checking again
  }

  delay(2000);
}
//
//void updateDisplay(float value) {
//  myDisplay.clearDisplay();
//  myDisplay.setTextSize(2);
//  myDisplay.setTextColor(SSD1306_WHITE);
//  myDisplay.setCursor(0, 0);
//  myDisplay.print(F("Gas Value: "));
//  myDisplay.print(value);
//  myDisplay.display();
//}

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