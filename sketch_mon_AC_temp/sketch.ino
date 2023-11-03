// https://robotchip.ru/izmerenie-peremennogo-napryazheniya-s-pomoshchyu-zmpt101b-i-arduino/ - модуль напряжения
// http://sagis.ru/blog/?p=53 - Плата часов и карта памяти
// https://microkontroller.ru/arduino-projects/logger-dannyh-temperatury-vlazhnosti-na-sd-kartu-i-kompyuter-s-pomoshhyu-arduino/ - логгирование с датчика температуры с использованием даты и времени
// https://robotchip.ru/obzor-chasov-realnogo-vremeni-ds1307/ - плата времени
// https://geekelectronics.org/arduino/dht22-podklyuchenie-k-arduino.html - DHT22 подключение
// https://github.com/Abdurraziq/ZMPT101B-arduino/blob/master/examples/simple_usage/simple_usage.ino - библиотека модуля напряжения

//https://wokwi.com/projects/380351279022927873 - working project
// https://github.com/wizand0/DC_Monitor_UNO

#include "DHT.h"
#include <EEPROM.h>
#include <Wire.h>                           // Подключаем библиотеку Wire                           
#include <TimeLib.h>                        // Подключаем библиотеку TimeLib
#include <DS1307RTC.h>    
//#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
#include "EmonLib.h"
EnergyMonitor emon1;

//API keys https://thingspeak.com/channels/2322125/api_keys
// https://fil-tec.ru/page/arduino-uno-esp8266-wifi-get-thingspeak

String ssid ="MGTS_GPON_923A";
String password="5Y3KX67Q";
String API = "H20C8OAJ7KXGE3SS";   
String HOST = "flask-bot-xabor.amvera.io";
String PORT = "80";
// строки, которые пригодятся для тестирования в мониторе порта
// AT+CWJAP="Keenetic-0138","*********"
// AT+CIPSTART=0,"TCP","api.thingspeak.com",80
// AT+CIPSEND=0,90
// GET /update?api_key=***************&field1=1&field2=1&field3=1&field4=0&field5=0&field6=0
boolean found = false;
boolean sendnook = false;
int valSensor = 1;

String fields;
String curRSSI="";
int crssi=0;



//ZMPT101B voltageSensor(A0, 50.0);

#define DHTPIN 4 // Тот самый номер пина, о котором упоминалось выше
// Одна из следующих строк закоментирована. Снимите комментарий, если подключаете датчик DHT11 к arduino
DHT dht(DHTPIN, DHT22); //Инициация датчика
//DHT dht(DHTPIN, DHT11);

void setup() {
  //Serial.begin(9600);
  Serial.begin(115200);
  //Проверка на наличие ошибки по питанию в энергонезависимой памяти

  dht.begin();

  //voltageSensor.setSensitivity(SENSITIVITY);
}








void reset() 
{
  sendC("AT+RST", 5, "OK");    
  delay(1000);
}
void connectWifi() 
{

  sendC("AT+CWMODE=3", 100, "OK");
  while(!found) sendC("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"", 4000, "OK");
  
  Serial.println("ATE0"); // не знаю что это, но так работает лучше
}













float supplyVoltage = 0.00;
float t = 0.00;
int h = 0;


void loop() {

  emon1.calcVI(20,100);
  supplyVoltage  = emon1.Vrms;
  //Serial.print(emon1.Vrms);
  //Serial.println("");

  h = dht.readHumidity(); //Измеряем влажность
  t = dht.readTemperature(); //Измеряем температуру
  //int voltage = voltageSensor.getRmsVoltage();
  

  // Комменты ниже убрать после подключения датчиков
//  if (isnan(h) || isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
//    Serial.println(supplyVoltage);
//    Serial.println(h);
//    Serial.println(t);
//
//    //Serial.write(t);
//
//    Serial.println("Ошибка считывания");
//    delay(10000);
//    return;
//  }
  //Убирать до сюда




  tmElements_t tm;                          
 
  if (RTC.read(tm)) {
    if (tm.Second % 30 == 0) { //вывод в консоль и отпарвка API каждые 30 секунд
      Serial.print("Time");
      Serial.print("|");
      print2digits(tm.Hour);
      Serial.write(':');
      print2digits(tm.Minute);
      Serial.write(':');
      print2digits(tm.Second);
      Serial.print("|");
      Serial.print("Date");
      Serial.print("|");
      Serial.print(tm.Day);
      Serial.write('.');
      Serial.print(tm.Month);
      Serial.write('.');
      Serial.print(tmYearToCalendar(tm.Year));
      Serial.print("|");
      Serial.print("H:");
      Serial.print("|");
      Serial.print(h);
      Serial.print("|");
      Serial.print("T: ");
      Serial.print("|");
      Serial.print(t);
      Serial.print("|");
      Serial.print("U");
      Serial.println(supplyVoltage);
      httppost();
      getRSSI();
    }
    if (supplyVoltage == 0) { //если условие по времени не выполнено, но напряжение пропало, то отправка в API
      httppost();
      getRSSI();
    }
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }


  //httppost();
  //getRSSI();







}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}




void httppost () 
{
  int iserror = 1; // предполагаем что будет ошибка
  found = false; // флаг успешной отправки на шаге итерации
  
  fields = "&field1=" + String(h)+"&field2=" + String(t)+"&field3=" + String(supplyVoltage);
  String data = "GET /update?api_key=" + API + "&" + fields  + "\r\n";  
  

  sendC("AT+CIPMUX=1", 2000, "OK");
  if (found)
  {  
      sendC("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 2000, "OK");
      if (found) 
      {

         sendC("AT+CIPSEND=0," + String(data.length()+2), 2000, ">");
         if (found) 
         {
       
            Serial.println(data);
            sendnook = false;
            delay(2000);
          }
         else Serial.println("AT+CIPCLOSE"); 
      }  
  }
  sendC("AT+CIPMUX=0", 100, "OK");

  
  //lastConnectionTime = millis();

}
void getRSSI()
{
  char c1;
  String val = "";
  int i;
  curRSSI = "";
  Serial.print("AT+CWLAP=\"" + ssid + "\"\r\n");
  delay(500);
  if (Serial.available()) 
  {
    while (Serial.available() > 0) 
    {      
       c1 = (char)Serial.read(); val = val + c1; delay(2);
    }  
    int pos = val.indexOf(ssid);
    pos = pos+ssid.length()+3;
    curRSSI = val.substring(pos, pos+2);
    
    Serial.println(curRSSI);
    crssi = curRSSI.toInt();
     
  }  
}
void sendC(String command, int maxTime, char readReplay[])
{
  found = false;  
 
  Serial.println(command);
  delay(maxTime);

}



//void time(long timeNow)
//{
// int days = timeNow / day ;  /
// int hours = (timeNow % day) / hour;  
// int minutes = ((timeNow % day) % hour) / minute ;  
// int seconds = (((timeNow % day) % hour) % minute) / second;
//}
