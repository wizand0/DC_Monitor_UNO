// https://robotchip.ru/izmerenie-peremennogo-napryazheniya-s-pomoshchyu-zmpt101b-i-arduino/ - модуль напряжения
// http://sagis.ru/blog/?p=53 - Плата часов и карта памяти
// https://microkontroller.ru/arduino-projects/logger-dannyh-temperatury-vlazhnosti-na-sd-kartu-i-kompyuter-s-pomoshhyu-arduino/ - логгирование с датчика температуры с использованием даты и времени
// https://robotchip.ru/obzor-chasov-realnogo-vremeni-ds1307/ - плата времени
// https://geekelectronics.org/arduino/dht22-podklyuchenie-k-arduino.html - DHT22 подключение
// https://github.com/Abdurraziq/ZMPT101B-arduino/blob/master/examples/simple_usage/simple_usage.ino - библиотека модуля напряжения


#include <EEPROM.h>
#include "DHT.h"
#include "SdFat.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CS_PIN 10
SdFat sd;
#include <Wire.h>                           // Подключаем библиотеку Wire                           
#include <TimeLib.h>                        // Подключаем библиотеку TimeLib
#include <DS1307RTC.h>    
#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
#define BUTTON_PIN 6
int ledPin = 7;

#include <TM1637.h>

const int CLK = 2;
const int DIO = 3;

TM1637 tl(CLK, DIO);

File myFile;

// ZMPT101B sensor output connected to analog pin A0
// and the voltage source frequency is 50 Hz.
ZMPT101B voltageSensor(A0, 50.0);

#define DHTPIN 4 // Тот самый номер пина, о котором упоминалось выше
// Одна из следующих строк закоментирована. Снимите комментарий, если подключаете датчик DHT11 к arduino
DHT dht(DHTPIN, DHT22); //Инициация датчика
//DHT dht(DHTPIN, DHT11);
boolean alarm_v = false;
boolean alarm_t = false;
boolean alarm_h = false;

void setup() {

  //EEPROM.write(30, 1);

  tl.init();
  tl.set(BRIGHTEST);

  

  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);

  //Проверка на наличие ошибки по питанию в энергонезависимой памяти
  
  if (EEPROM.read(30) == 0) {
    alarm_v = false;
  }
  else {
    alarm_v = true;
  }


  dht.begin();
  //-----------------------------------
  //  Serial.begin(115200);
  if (!sd.begin(CS_PIN, SPI_SPEED)) {
    if (sd.card()->errorCode()) {
      Serial.println("SD initialization failed.");
    } else if (sd.vol()->fatType() == 0) {
      Serial.println("Can't find a valid FAT16/FAT32 partition.");
    } else {
      Serial.println("Can't determine error type");
    }
    return;
  }
  voltageSensor.setSensitivity(SENSITIVITY);

  tl.display(0, 1);
  delay(1000);
  tl.display(1, 1);
  delay(1000);
  tl.display(2, 1);
  delay(1000);
  tl.display(3, 1);
  delay(1000);
}

//int lastState = HIGH;
//int voltage = 220;

void loop() {
  delay(600); // полсекунды задержки
  
  int h = dht.readHumidity(); //Измеряем влажность
  float t = dht.readTemperature(); //Измеряем температуру
  int voltage = voltageSensor.getRmsVoltage();
  

  if (isnan(h) || isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
    Serial.println("Ошибка считывания");
    return;
  }
  tmElements_t tm;                          
 
  if (RTC.read(tm)) {
    if (tm.Second % 20 == 0) { //вывод в консоль каждые 20 секунд
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
      Serial.print("Влажность:");
      Serial.print("|");
      Serial.print(h);
      Serial.print("|");
      Serial.print("Температура: ");
      Serial.print("|");
      Serial.print(t);
      Serial.print("|");
      Serial.print("Напряжение");
      Serial.println(voltage);
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
  myFile = sd.open("logs.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (tm.Second == 0) { // Запись в файл каждую минуту
    if (myFile) {
      Serial.print("Writing to logs.txt...");
      myFile.print(tm.Day);
      myFile.print('.');
      myFile.print(tm.Month);
      myFile.print('.');
      myFile.print(tmYearToCalendar(tm.Year));
      myFile.print("|");
      myFile.print(tm.Hour);
      myFile.print(':');
      myFile.print(tm.Minute);
      myFile.print(':');
      myFile.print(tm.Second);
      myFile.print("|");
      
      myFile.print("t");
      myFile.print("|");
      myFile.print("h");
      myFile.print("|");
      if (voltage == 0) {
        Serial.print("Отключение электроэнергии");
        myFile.print("Отключение электроэнергии");
      }
      else {
        myFile.print(voltage);
      }
      // close the file:
      myFile.close();
      sd.ls(LS_R | LS_SIZE); //Нужно для отладки чтобы видеть что файл увеличивается

    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening logs.txt");
    }
  }

  int digitalval = digitalRead(BUTTON_PIN);

  if(LOW == digitalval)
  {
    alarm_v = false;
    EEPROM.write(30, 0);
    alarm_t = false;
    EEPROM.write(31, 0);
    alarm_h = false;
    EEPROM.write(32, 0);
  }
  
  


  tl.display(0, "-");
  
  tl.display(1, (voltage / 100) % 10);
  tl.display(2, (voltage / 10) % 10);
  tl.display(3, voltage % 10);


  //if (tm.Second == 35) {
  //  voltage = 0;
  //}

  //if (tm.Second == 5) {
  //  voltage = 210;
  //}

  


  if (voltage == 0) {
    alarm_v = true;
  }

  //if (t > 55) {
  //  alarm_t = true;
  //}

  //if (h > 65) {
  //  alarm_h = true;
  //}

  if (alarm_v == true && EEPROM.read(30) == 0) {
    EEPROM.write(30, 1);
    //Serial.println(EEPROM.read(30));
  }
  if (alarm_v == false && EEPROM.read(30) == 1){
    alarm_v = true;
  }



  //Serial.println(tm.Second);

  //Serial.println(voltage);

  //Serial.println(alarm_v);

  //Serial.println(EEPROM.read(30));


  if (alarm_v == true) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }

  



}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}