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
