#include "DHT.h"
#include <EEPROM.h>
#include <Wire.h>                           // Подключаем библиотеку Wire                           
#include <TimeLib.h>                        // Подключаем библиотеку TimeLib
#include <DS1307RTC.h>    
//#include <ZMPT101B.h>
#define SENSITIVITY 500.0f
#include "EmonLib.h"
EnergyMonitor emon1;

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

void loop() {

  emon1.calcVI(20,100);
  float supplyVoltage   = emon1.Vrms;
  //Serial.print(emon1.Vrms);
  //Serial.println("");

  int h = dht.readHumidity(); //Измеряем влажность
  float t = dht.readTemperature(); //Измеряем температуру
  //int voltage = voltageSensor.getRmsVoltage();
  
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
      Serial.println(supplyVoltage);
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

}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}