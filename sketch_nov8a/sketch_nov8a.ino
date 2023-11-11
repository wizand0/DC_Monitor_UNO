#include <ESP8266WiFi.h>
#include <stdlib_noniso.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#include <user_interface.h>
void system_restart(void);

//....................... Параметры WIFI ............................
const char* ssid = "MGTS_GPON_923A";        // Введите имя WIFI сети
const char* password = "5Y3KX67Q";          // Введите пароль WIFI сети

char* ssid1[] = {"MGTS_GPON_923A","buh","POCOX3Pro"}; //list a necessary wifi networks
char* pass1[] = {"5Y3KX67Q","triton273","a1b2c3d4e5f6"}; //list a passwords

//...................... Параметры сервиса ThingsPeak ...............
const char* host1 = "flask-bot-xabor.amvera.io";
//const char* host1 = "192.168.1.73";
const char* host = "api.thingspeak.com";
const char* APIkey   = "H20C8OAJ7KXGE3SS";  // Введите API key thingspeak

int count = 0;

int count_for_restart = 0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 600000;           // Интервал отправки данных если напряжеине есть и все ОК


//...................... Датчик DHT22 (DHT11) .......................
#include <DHT.h>
#define DHTPIN 4                          // Порт подключения датчика DHT22 или DHT11,
#define DHTTYPE DHT22                     // Тип датчика: DHT22 или DHT11
DHT dht(DHTPIN, DHTTYPE);

//...................... Датчик ZMPT101B .......................
#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
#include "EmonLib.h"

ZMPT101B voltageSensor(A0, 50.0);

//.....................................................................
float t;
int h, voltage;

//===============================================================
void setup() 
{
  Serial.begin(115200);
  delay(100);
  Serial.println();
//................Подключение к WIFI сети......................
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

 //.......................DHT...................................
  dht.begin();

  //.......................ZMPT101B...................................

  voltageSensor.setSensitivity(SENSITIVITY);

}
//================================================================
void loop() 
{
// Если к Wi-Fi подключиться не удалось, то запускается функция перебора известных вайфай сетей
  if (WiFi.status() != WL_CONNECTED) {
    MultiWiFiBegin(); //(auth, ssid, pass);
//    WiFi.begin(ssid, password);
//    delay(5000);
//    Serial.print(".");
//    count_for_restart += 1;
//    if (count_for_restart >= 40) {
//      count_for_restart = 0;
//      system_restart();
//      delay(2000);
//    }
    
  }

  voltage = voltageSensor.getRmsVoltage();

  if (voltage < 10) {
    voltage = 0;
  }
//.........................DHT.......................................
  h = dht.readHumidity();
  t = dht.readTemperature();
  //h = 90;
  //t = 55;
  //voltage = 0;
  if (isnan(h) || isnan(t)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    t = 25.0;
    h = 50;
    //return;
  }
  Serial.print("Temperature dht: ");
  Serial.println(t);
  Serial.print("Humidity dht: ");
  Serial.println(h);
  Serial.print("Voltage ZMPT101B: ");
  Serial.println(voltage);
  //..................................................................
  Serial.println("");
  Serial.print("Connecting to ");
  //Serial.println(host); // Thingspeak
  Serial.println(host1);

  WiFiClientSecure client;
  const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!
  
  client.setInsecure(); // this is the magical line that makes everything work
  
  if (!client.connect(host1, httpPort)) { //works!
    Serial.println("connection failed");
    return;
  }
  else {
    Serial.print("Connected to: ");
    Serial.println(host1);
  }
//......................................................................
  String url = "/update?key=";         // Thingspeak
         url += APIkey;
         url += "&field1=";
         url += t;                     //DHT Температура
         url += "&field2=";
         url += h;                     //DHT Влажность 
         url += "&field3=";
         url += voltage;     
         url += "\r\n\r\n";

  String url1 = "/ard_update?key=";     // String для другого сервера
         url1 += APIkey;
         url1 += "&field1=";
         url1 += t;                     //DHT Температура
         url1 += "&field2=";
         url1 += h;                     //DHT Влажность 
         url1 += "&field3=";
         url1 += voltage;     


  Serial.println("Sending data: ");

  Serial.print(count);
  if (voltage == 0 && count < 3) {
    client.println(String("GET ") + url1 + " HTTP/1.1\r\n" +
                 "Host: " + host1 + "\r\n" + 
                 "Connection: close\r\n\r\n");
    count = count +1;

    while (client.available()) 
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();

    delay(500);
    client.flush();     // ждем отправки всех данных

    Serial.println("closing connection");


    client.stop();

  }

  else if (voltage > 20 && count > 0) {
    count = 0;
  }

  else {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
      previousMillis = currentMillis;
      client.println(String("GET ") + url1 + " HTTP/1.1\r\n" +
                   "Host: " + host1 + "\r\n" + 
                   "Connection: close\r\n\r\n");
      while (client.available()) 
      {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
      Serial.println();

      delay(500);
      client.flush();     // ждем отправки всех данных

      Serial.println("closing connection");


      client.stop();

    }
    
  }

  delay(300);

}

//.........................several wifi.......................................

void MultiWiFiBegin() {
  int ssid_count=0;
  int ssid_mas_size = sizeof(ssid1) / sizeof(ssid1[0]);
  do {
    Serial.println("Trying to connect to wi-fi " + String(ssid[ssid_count]));
    WiFi.begin(ssid1[ssid_count], pass1[ssid_count]);   
    int WiFi_timeout_count=0;
    while (WiFi.status() != WL_CONNECTED && WiFi_timeout_count<50) { //waiting 10 sec
      delay(200);
      Serial.print(".");
      ++WiFi_timeout_count;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi! Now I will check the connection to the Blynk server");
//      Blynk.config(auth);
//      Blynk.connect(5000); //waiting 5 sec
    }
    ++ssid_count;
  }
  while (ssid_count<ssid_mas_size);
  if (ssid_count==ssid_mas_size) {
    Serial.println("I could not connect =( Ignore and move on. but still I will try to connect to wi-fi " + String(ssid[ssid_count-1]));
  }
}