#include <ESP8266WiFi.h>
#include <stdlib_noniso.h>

//....................... Параметры WIFI ............................
const char* ssid = "buh";        // Введите имя WIFI сети
const char* password = "triton2073";          // Введите пароль WIFI сети

//...................... Параметры сервиса ThingsPeak ...............
const char* host = "api.thingspeak.com";
const char* APIkey   = "H20C8OAJ7KXGE3SS";  // Введите API key thingspeak

//...................... Датчик DHT22 (DHT11) .......................
#include <DHT.h>
#define DHTPIN 2                          // Порт подключения датчика DHT22 или DHT11,
#define DHTTYPE DHT22                     // Тип датчика: DHT22 или DHT11
DHT dht(DHTPIN, DHTTYPE);

//.....................................................................
float h,t, voltage;
//===============================================================
void setup() 
{
  Serial.begin(115200);
  delay(10);
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
}
//================================================================
void loop() 
{
//.........................DHT.......................................
  //h = dht.readHumidity();
  //t = dht.readTemperature();
  h = 60;
  t = 29;
  voltage = 220;
  if (isnan(h) || isnan(t)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temperature dht: ");
  Serial.println(t);
  Serial.print("Humidity dht: ");
  Serial.println(h);
  Serial.print("Humidity dht: ");
  Serial.println(h);
  Serial.print("Voltage ZMPT101B: ");
  Serial.println(voltage);
  //..................................................................
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(host);
  WiFiClient client;
  
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) 
  {
    Serial.println("connection failed");
    return;
  }
 
//......................................................................
  String url = "/update?key=";
         url += APIkey;
         url += "&field1=";
         url += t;                     //DHT Температура
         url += "&field2=";
         url += h;                     //DHT Влажность 
         url += "&field3=";
         url += voltage;     
         url += "\r\n\r\n";
 
  Serial.println("Sending data: ");
  Serial.println(url);
                                       // Передача запроса серверу
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
                                       // Чтение данных от сервера и отправка в последовательный порт
  while (client.available()) 
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");

  
  Serial.println("Waiting");
  for(unsigned int i = 0; i < 20; i++)  // задержка между обновлениями.
  {
    delay(3000);                         
  }
}
