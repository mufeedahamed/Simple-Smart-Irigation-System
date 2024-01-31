/*
RFID PIN TABLE
 * -------------------------------------------------
 *             MFRC522      Arduino       Arduino  
 *             Reader/PCD   Uno/101       Mega      
 * Signal      Pin          Pin           Pin      
 * ------------------------------------------------
 * RST/Reset   RST          9             5         
 * SPI SS      SDA(SS)      10            53      
 * SPI MOSI    MOSI         11 / ICSP-4   51        
 * SPI MISO    MISO         12 / ICSP-1   50      
 * SPI SCK     SCK          13 / ICSP-3   52    
 */


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>
#include <MFRC522.h>
#include <DS3231.h>
#include <SoftwareSerial.h>


#define DHTPIN 4
#define DHTTYPE DHT11 
#define RST_PIN 5
#define SS_PIN 53
#define servo1Pin 6
#define servo2Pin 7

LiquidCrystal_I2C lcd(0x3F, 16, 4);
DHT dht(DHTPIN, DHTTYPE);
DS3231 rtc(SDA, SCL);
SoftwareSerial mySerial(9, 10);
Time t;

int soilPin = A0;
double soilRead;
double soilMoist;
int OnHour = 8;
int OnMin = 0;
String textMessage;
String message, motorState;
volatile int flow_frequency; 

float vol = 0.0,l_minute;
unsigned char flowsensor = 2; 
unsigned long currentTime;
unsigned long cloopTime;


String masterTagID1 = "235DF8B";
String masterTagID2 = "13DA9CA3";
String scanTagID = "";

Servo myservo1;
Servo myservo2;

MFRC522 mfrc522(SS_PIN, RST_PIN);

char Incoming_value = "";

void setup() {

  lcd.begin();
  SPI.begin();
  //rtc.begin();
  mfrc522.PCD_Init();
  pinMode(3,INPUT); // Rain sensor
  myservo1.attach(servo1Pin);
  myservo2.attach(servo2Pin);
  myservo1.write(0); 
  myservo2.write(0);
  SIM900.begin(19200);
  Serial.begin(9600);


// The following lines can be uncommented to set the date and time
//  rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to Wednesday
//  rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
//  rtc.setDate(17, 10, 2018);   // Set the date to October 17st, 2018
  
}

void loop()
{
  int h = dht.readHumidity();
  int t = dht.readTemperature(); 
  soilRead = analogRead(soilPin);
  soilMoist = 100-(soilRead/10);

  lcd.setCursor(0,3);
  lcd.print("ACCESS LOCKED");
  sensorvalues();
  t = rtc.getTime();

  Farming_time();

  if(Serial.available()>0)
  {
    Serial.print("Enter your input: ");
    Incoming_value=Serial.read();
  
    if(Incoming_value=="Stat"){
      Serial.print("Temp: ");
      Serial.print(t);
      Serial.print("C. ");
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print("% ");
      Serial.print("Moisture: ");
      Serial.print(soilMoist);
    }
  }

  while (readTagID())
  {
    if (scanTagID == masterTagID1 || scanTagID == masterTagID2 ) 
    {
      lcd.setCursor(0,3);
      int i = 10;
      lcd.print("ACCESS FOR 10s");
      lcd.setCursor(0,3);
      lcd.print("                          ");
      
      while (i>0){

        if(Serial.available()>0){
          Serial.print("Enter your input: ");
          Incoming_value=Serial.read();
           if(Incoming_value=="Prof 1"){
            OnHour = 9;
            OnMin = 30;
            Incoming_value = "";
            }
            if(Incoming_value=="Prof 2"){
            OnHour = 10;
            OnMin = 30;
            Incoming_value = "";
            }
            if(Incoming_value=="Prof 3"){
            OnHour = 12;
            OnMin = 30;
            Incoming_value = "";
            }
        }
        lcd.setCursor(0,3);
        lcd.print("ACCESS FOR ");
        lcd.print(i);
        lcd.print("s");
        delay(1000);
        i--;
        lcd.setCursor(0,3);
        lcd.print("                       ");
      }
    }
    else{
    lcd.setCursor(0,3);
    lcd.print("ACCESS LOCKED");
    }
  }

  delay(2000);
  lcd.clear();

  currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {
    cloopTime = currentTime; // Updates cloopTime
    if(flow_frequency != 0){
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_minute = (flow_frequency / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      l_minute = l_minute/60;
      vol = vol +l_minute;
      flow_frequency = 0; // Reset Counter
      Serial.print(l_minute, DEC); // Print litres/hour
      Serial.println(" L/Sec");
    }
    else {
      Serial.println(" flow rate = 0 ");
    }
   }
}

void sensorvalues() 
{
  int h = dht.readHumidity();
  int t = dht.readTemperature(); 
  soilRead = analogRead(soilPin);
  soilMoist = 100-(soilRead/10);

  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.print("%");
  lcd.setCursor(0,2);
  lcd.print("Moisture: ");
  lcd.print(soilMoist);
  lcd.print("%");

  if (digitalRead(3)==0)
  {
    lcd.setCursor(11,0);
    lcd.print("Rain: Yes");
  }
  else
  {
    lcd.setCursor(11,0);
    lcd.print("Rain: No");
  }

}

boolean readTagID() 
{
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return false;
  }
  /* Clear the string */
  scanTagID = "";
  for ( uint8_t i = 0; i < 4; i++) 
  {
    scanTagID += String(mfrc522.uid.uidByte[i], HEX);
  }
  scanTagID.toUpperCase();
  mfrc522.PICC_HaltA();
  return true;
}

void Farming_Time(){
   if (digitalRead(3)==1){
     if(t.hour == OnHour && t.min == OnMin){
      myservo1.write(90); 
      myservo2.write(90);
      delay(5000);
      myservo1.write(0);
      myservo2.write(0);
    }
  else{
      myservo1.write(0);
      myservo2.write(0);
    }
  
  }

}