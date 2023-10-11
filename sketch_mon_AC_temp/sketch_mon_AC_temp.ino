// https://robotchip.ru/izmerenie-peremennogo-napryazheniya-s-pomoshchyu-zmpt101b-i-arduino/ - модуль напряжения
// http://sagis.ru/blog/?p=53 - Плата часов и карта памяти
// https://microkontroller.ru/arduino-projects/logger-dannyh-temperatury-vlazhnosti-na-sd-kartu-i-kompyuter-s-pomoshhyu-arduino/ - логгирование с датчика температуры с использованием даты и времени
// https://robotchip.ru/obzor-chasov-realnogo-vremeni-ds1307/ - плата времени
// https://geekelectronics.org/arduino/dht22-podklyuchenie-k-arduino.html - DHT22 подключение
// https://github.com/Abdurraziq/ZMPT101B-arduino/blob/master/examples/simple_usage/simple_usage.ino - библиотека модуля напряжения

#include "DHT.h"
//----------------------
#include "SdFat.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CS_PIN 10

SdFat sd;
//-----------------------
#include <Wire.h>                           // Подключаем библиотеку Wire                           
#include <TimeLib.h>                        // Подключаем библиотеку TimeLib
#include <DS1307RTC.h>    
//-----------------------

#include <ZMPT101B.h>

#define SENSITIVITY 500.0f

File myFile;

// ZMPT101B sensor output connected to analog pin A0
// and the voltage source frequency is 50 Hz.
ZMPT101B voltageSensor(A0, 50.0);

#define DHTPIN 2 // Тот самый номер пина, о котором упоминалось выше
// Одна из следующих строк закоментирована. Снимите комментарий, если подключаете датчик DHT11 к arduino
DHT dht(DHTPIN, DHT22); //Инициация датчика
//DHT dht(DHTPIN, DHT11);
void setup() {

  Serial.begin(9600);

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

}
void loop() {
  delay(4000); // 4 секунды задержки
  //Serial.println(analogRead(A0));
  float h = dht.readHumidity(); //Измеряем влажность
  float t = dht.readTemperature(); //Измеряем температуру

  float voltage = voltageSensor.getRmsVoltage();
  Serial.println(voltage);

  if (isnan(h) || isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
    Serial.println("Ошибка считывания");
    return;
  }

  tmElements_t tm;                          
 
  if (RTC.read(tm)) {

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
    Serial.println();
    Serial.print("Влажность:");
    Serial.print("|");
    Serial.print(h);
    Serial.print("|");
//  Serial.print(" %\t");
    Serial.print("Температура: ");
    Serial.print("|");
    Serial.print(t);
    Serial.print("|");
    Serial.print("Напряжение");
    Serial.println(voltage);
//  Serial.println(" *C "); //Вывод показателей на экран

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
    if (t == 0) {
      myFile.print("Отключение электроэнергии");
    }
    else {
      myFile.print("t");
    }
    myFile.print("t");
    myFile.print("|");
    myFile.print("h");
    myFile.print("|");
    myFile.print(voltage);

//    sd.ls(LS_R | LS_SIZE); //Нужно для отладки чтобы видеть что файл увеличивается

    // close the file:
    myFile.close();
    



  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  


}
 
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}




