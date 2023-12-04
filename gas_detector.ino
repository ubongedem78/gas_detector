#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 myDisplay(128, 64, &Wire);

String writeApiKey = "3DTMHW1H20UCVI7B";

const int gasThreshold = 70;
#define MQ2pin A0
#define BUZZER_PIN 8

void setup() {
  Serial.begin(9600);
  delay(2000);

  pinMode(BUZZER_PIN, OUTPUT);

  myDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  myDisplay.clearDisplay();
  myDisplay.setTextSize(2);
  myDisplay.setTextColor(SSD1306_WHITE);
  myDisplay.setCursor(0, 0);
  myDisplay.println("Gas Detector System");
  myDisplay.display();

  Serial.println(F("Initializing GSM..."));
  delay(2000);

  Serial.println(F("Warming up gas sensor..."));
  delay(10000);
  Serial.println(F("Gas sensor warmed up!"));


}

void loop() {
  float sensorValue = analogRead(MQ2pin);

  updateDisplay(sensorValue);

//  Serial.print(F("Gas Value: "));
//  Serial.println(sensorValue);

  if (sensorValue > gasThreshold) {
    activateBuzzer();
    Serial.println(F("Buzzer Activated"));
    sendSMS(F("Gas detected! Take necessary action."));
    Serial.println("Sending to ThingSpeak...");
    sendToThingSpeak(sensorValue);
    delay(10000);
    deactivateBuzzer();
    Serial.println(F("Buzzer Deactivated"));
    delay(2000);
  }

  delay(2000);
}

void updateDisplay(float value) {
  myDisplay.clearDisplay();
  myDisplay.setTextSize(2);
  myDisplay.setTextColor(SSD1306_WHITE);
  myDisplay.setCursor(0, 0);
  myDisplay.print(F("Gas Value: "));
  myDisplay.print(value);
  myDisplay.display();
  delay(100);
}

void sendToThingSpeak(float gasValue) {
  Serial.println(F("AT+CIPSHUT"));
  delay(500);

  Serial.println(F("AT+CIPMUX=0"));
  delay(500);

  Serial.println(F("AT+CSTT=\"internet.ng.airtel.com\""));
  delay(500);

  Serial.println(F("AT+CIICR"));
  delay(500);

  Serial.println(F("AT+CIFSR"));
  delay(500);

  Serial.println(F("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\""));
  delay(5000);

  Serial.print(F("AT+CIPSEND"));
  delay(5000);
  String str = "GET /update?api_key=" + writeApiKey +
               "&field1=" + String(gasValue);

  Serial.println(str);
  delay(3000);
  
  Serial.write(26);
  delay(5000);

  Serial.println(F("AT+CIPSHUT"));
  delay(500);
}

void sendSMS(String message) {
  Serial.println(F("AT+CMGF=1"));
  delay(3000);
  Serial.println(F("AT+CMGS=\"+2347082257453\""));
  delay(1000);
  Serial.println(message);
  delay(100);
  Serial.write(26);
  delay(5000); 
  Serial.println(F("SMS sent."));
}

void activateBuzzer() {
//  tone(BUZZER_PIN, 1000);
}


void deactivateBuzzer() {
  noTone(BUZZER_PIN);
}