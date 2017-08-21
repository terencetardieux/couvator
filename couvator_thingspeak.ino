#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMPERATURE_PRECISION 9
#define loopInterval 5 //in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board
#define tempPower 2      
#define led 13
#define resistor 15

#define lowThreshold 37.4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* server = "api.thingspeak.com";
String apiKey ="XR2633AETL9AFBMT";
const char* MY_SSID = "SFR_B0C8"; 
const char* MY_PWD = "evosethertuo4uzeillo";
float lastTemp=0;

void setup() {
  Serial.begin(115200);
  connectWifi();

  pinMode(resistor,OUTPUT);
  digitalWrite(resistor,HIGH);
    
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output
  digitalWrite(tempPower, HIGH); // turn sensor power on
  delay(500);
  
  sensors.begin(); 
}

void loop() {
  float actualTemp;
    
  sensors.requestTemperatures(); 
  actualTemp = sensors.getTempCByIndex(0);

  activeResistor(actualTemp, lastTemp);
  lastTemp = actualTemp;
  
  sendTeperatureTS(actualTemp);
  
  delay(20000);  
}

void connectWifi()
{
  Serial.print("Connecting to "+*MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Connected");
  Serial.println("");  
}

void sendTeperatureTS(float temp)
{  
   WiFiClient client;
  
   if (client.connect(server, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
     //Serial.println("WiFi Client connected ");
     
     String postStr = apiKey;
     postStr += "&field1=";
     postStr += String(temp);
     postStr += "\r\n\r\n";
     
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
   }
   client.stop();
}

void activeResistor(float actualTemp, float lastTemp)
{  
  if (actualTemp > lastTemp) {
     Serial.print("Temparture increase|");
     Serial.print(actualTemp);
     Serial.print("°C|+");
     Serial.print((actualTemp-lastTemp));
     Serial.println("°C");
  } else {
    if (actualTemp < lowThreshold) {
     active(4000);
    } 
    Serial.print("Temparture decrease|");
    Serial.print(actualTemp);
    Serial.print("°C|-");
    Serial.print((lastTemp-actualTemp));
    Serial.println("°C");
  } 
}

void active(int duration)
{  
   Serial.print("Start Resitor | ");
   Serial.print(duration);
   Serial.println("ms");
   digitalWrite(resistor,LOW);
   delay(duration);
   digitalWrite(resistor,HIGH);
   Serial.println("Stop Resitor");
}

