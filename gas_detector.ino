#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(6, 7); // SoftwareSerial for GSM module
SoftwareSerial displaySerial(8, 9);  // Software Serial for display


Adafruit_SSD1306 myDisplay(128, 64, &Wire); // OLED display

String writeApiKey = "api_key"; // ThingSpeak API Key

const int gasThreshold = 70;

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
  delay(100);
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
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
#include <MQ2.h> 
int redLed = 11; // to include Red led
int greenLed = 10; // to include Green led
int buzzer = 9; // to include Buzzer
int LPG = A1; // to include MQ-2 sensor
int lm35 = A0;
float temp;           // to include LM 35 sensor
int sensorThres = 170; // Your threshold value
MQ2 mq2(LPG);

void setup() {
     mySerial.begin(9600);  //Module baude rate, this is on max
  analogReference(INTERNAL);
  lcd.begin(16, 2);
  lcd.init();    // initialize the lcd 
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("SENSING REPORT..");
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(LPG, INPUT);
  Serial.begin(9600);
}

void loop() {
  int analogSensor = analogRead(LPG);
  Serial.print("Pin A1: ");
  Serial.println(analogSensor);
  if (analogSensor > sensorThres)   // Checks if it has reached the threshold value
  {
     lcd.setCursor(0,1);
    lcd.print("Gas Detected");
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
    digitalWrite(buzzer,HIGH);
    delay(5);
   temp = analogRead(A0);  //temp = temp * 0.48828125;  //temp=temp*(5.0/1023.0)*100;
 temp=temp*1100/(1024*10);
   lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temp);
      delay(5000);
      lcd.clear();
     mySerial.println("AT+CMGF=1");   //Set the module to SMS mode
  delay(400);
    mySerial.println("AT+CMGS=\"+2349025295326\"\r");  // Phone number here but don't forget to include your country code
    delay(400);
     mySerial.println("GAS LEAK ALERT!");   //This is the text to send to the phone number
     delay(100);
     mySerial.println((char)26);// ASCII code (required according to the datasheet)
  delay(2000);
  }
  else
  {
    lcd.setCursor(0,0);
  lcd.print("Gas Not Detected");
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
digitalWrite(buzzer,LOW);
  }
  delay(100);
}
