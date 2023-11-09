#include <ESP8266WiFi.h>
#include <stdlib_noniso.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

//....................... Параметры WIFI ............................
const char* ssid = "MGTS_GPON_923A";        // Введите имя WIFI сети
const char* password = "5Y3KX67Q";          // Введите пароль WIFI сети

//...................... Параметры сервиса ThingsPeak ...............
const char* host1 = "flask-bot-xabor.amvera.io";
//const char* host1 = "192.168.1.73";
const char* host = "api.thingspeak.com";
const char* APIkey   = "H20C8OAJ7KXGE3SS";  // Введите API key thingspeak

//...................... Многозадачность на millis() .......................
#define MY_PERIOD 500  // период в мс
//uint32_t tmr1;         // переменная таймера
uint32_t myTimer1, myTimer2;
//...................... Датчик DHT22 (DHT11) .......................
#include <DHT.h>
#define DHTPIN 2                          // Порт подключения датчика DHT22 или DHT11,
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
  WiFiClientSecure client;
  const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!
  client.setInsecure(); // this is the magical line that makes everything work


  // Код опроса датчиков раз в секунду
  if (millis() - myTimer1 >= 1000) {   // таймер на 1000 мс (1 раз в сек)
    myTimer1 = millis();              // сброс таймера
  
    voltage = voltageSensor.getRmsVoltage();
//.........................DHT.......................................
    h = dht.readHumidity();
    t = dht.readTemperature();
  //h = 90;
  //t = 55;
  //voltage = 0;
    if (isnan(h) || isnan(t)) 
    {
      Serial.println("Failed to read from DHT sensor!");
      t = 25;
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
  }
  // Заканчивается код опроса датчиков раз в секунду

  //..................................................................
  // Если voltage == 0 то идет отправка данных на сервер и происходит задержка пока напряжение не появится.
  if (voltage == 0) {
    if (!client.connect(host1, httpPort)) { //works!
      Serial.println("connection failed");
      return;
    }
    else {
      Serial.print("Connected to: ");
      Serial.println(host1);
    }
//......................................................................
    String url1 = "/ard_update?key=";     // String для другого сервера
           url1 += APIkey;
           url1 += "&field1=";
           url1 += t;                     //DHT Температура
           url1 += "&field2=";
           url1 += h;                     //DHT Влажность 
           url1 += "&field3=";
           url1 += voltage;     

    Serial.println("Sending data: ");
    client.println(String("GET ") + url1 + " HTTP/1.1\r\n" +
                   "Host: " + host1 + "\r\n" + 
                   "Connection: close\r\n\r\n");
             
    while (client.available()) 
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();

    delay(10000); //Задержка 10 секунд перед повторной отправкой

    client.println(String("GET ") + url1 + " HTTP/1.1\r\n" +
                   "Host: " + host1 + "\r\n" + 
                   "Connection: close\r\n\r\n");

    client.flush();     // ждем отправки всех данных
    Serial.println("closing connection");
    client.stop();

    while (voltage == 0) {
      delay(10000); // Пока напряжение не появилось происходит задержка на 10 секунд пока напряжение не появится
    }
  }
  //Окончание кода на момент отсутствия напряжения
  //..................................................................

  //Код на работу пока напряжение нормальное
  //..................................................................

  if (millis() - myTimer2 >= 120000) {   // таймер на 120000 мс (1 раз в 120 сек)
    myTimer2 = millis();   
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
//           url1 += "\r\n\r\n"; //строка мешала отправке на сервер на Flask

    Serial.println("Sending data: ");

                                       // Передача запроса серверу Thingspeak
  //  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
  //               "Host: " + host + "\r\n" +
  //               "Connection: close\r\n\r\n");  
                                       // Чтение данных от сервера и отправка в последовательный порт

    client.println(String("GET ") + url1 + " HTTP/1.1\r\n" +
                 "Host: " + host1 + "\r\n" + 
                 "Connection: close\r\n\r\n");
    
    while (client.available()) 
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println();
    client.flush();     // ждем отправки всех данных
    Serial.println("closing connection");
    client.stop();
    Serial.println("Waiting");
    //Окончание кода на работу пока напряжеине нормальное
    //..................................................................
  }

}
