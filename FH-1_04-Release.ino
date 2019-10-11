
#define BLYNK_PRINT Serial
#define DHTPIN 19
#define DHTTYPE DHT11

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <NTPClient.h>
#include <LiquidCrystal_I2C.h>

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

int rainPin = 35;
float lowerBorder = 1600.0; // 1800, 2000, 1900, 1500 



char auth[] = "type-your-token-here";
char ssid[] = "wifi-name";
char pass[] = "wifi-password";





float desiredTemperature = 24.00;
int thresholdValue = 3500;





DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

WidgetLED virtualLedPump(V4);
WidgetLED virtualLedHeater(V3);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float maxTemp = -100.0;
float minTemp = 100.0;
String maxTempOut = "---";
String minTempOut = "---";

//релета
int heater = 15; // реле 1
int pump = 2; //реле 2

//диоди на платката
int led1 = 32;
int led2 = 33;

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  String temperature = String(t, 1) + " ℃";
  String humidity = String(h, 0) + " %";
 
  Blynk.virtualWrite(V5, humidity);
  Blynk.virtualWrite(V6, temperature);

    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Temp.: ");
    lcd.print(t);
    
    lcd.setCursor(0, 1);
    lcd.print(F("Humidity: "));
    lcd.print(h);
    lcd.println("%");

Serial.println(t);
Serial.println(h);

MinMaxTemperature();
}

String timeDateFormat(){
  String formattedDate = timeClient.getFormattedDate();
   int splitT = formattedDate.indexOf("T");
   String dayStamp = formattedDate.substring(0, splitT);
   String timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
   return timeStamp + " on " + dayStamp;  
}

String soilMoisture(){

  int sensorValue = analogRead(rainPin);
  float diff = 4095.0 - lowerBorder;

  float calcHumSol = (4095.0 - sensorValue*1.0) / diff * 100.0;
  if (calcHumSol > 100.0) {
    calcHumSol = 100.0;
  }

  String humSolValue = String(calcHumSol, 0) + " %";
  Blynk.virtualWrite(V9, humSolValue);
  
  return humSolValue;
}


void MinMaxTemperature() 
{
  if (dht.readTemperature() > maxTemp) {
    maxTemp = dht.readTemperature();  
    String tmpr = (String) maxTemp;
    maxTempOut =  tmpr + " ℃ at " + timeDateFormat();
  } 

  if (dht.readTemperature() < minTemp) {
    minTemp = dht.readTemperature();
    String tmpr = (String) minTemp;
    minTempOut =  tmpr + " ℃ at " + timeDateFormat();
  } 

  Blynk.virtualWrite(V7, minTempOut);
  Blynk.virtualWrite(V8, maxTempOut);
  Serial.println("Max temp: " + maxTempOut);
  Serial.println("Min temp: " + minTempOut);  
}




void setup() {
    
  pinMode(pump, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  digitalWrite(pump, LOW);
  digitalWrite(led1, LOW);
  virtualLedPump.off();
  
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  dht.begin();
  lcd.init();
  lcd.backlight();

  timeClient.begin();
  timeClient.setTimeOffset(10800);

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
  timeClient.update();
  soilMoisture();
  
  int sensorValue = analogRead(rainPin);
  float currentTemp = dht.readTemperature();

  if(sensorValue < thresholdValue){
    digitalWrite(pump, LOW);
    digitalWrite(led1, LOW);
    virtualLedPump.off();
  }
  else {
    digitalWrite(pump, HIGH);
    digitalWrite(led1, HIGH);
    virtualLedPump.on();
  }

  if(currentTemp > desiredTemperature){
    digitalWrite(heater, LOW);
    digitalWrite(led2, LOW);
    virtualLedHeater.off();
  }
  else {
    digitalWrite(heater, HIGH);
    digitalWrite(led2, HIGH);
    virtualLedHeater.on();
  }


//  Serial.print("Влажност на почвата: ");
//  Serial.println(soilMoisture());
      
  delay(1000);
}