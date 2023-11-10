# DC_Monitor_UNO

Данный прибор на базе ESP8266 контролирует параметры температуры, влажности и наличие напряжения в сети переменного тока. Все параметры считываются два раза в секунду. 
В случае если все нормально, через определенный интервал (10 мин) данные отправляются в БД на сервер, где рисуется график изменений параметров. В случае пропадения электричества 
сервер отправляет ошибку в телеграм бот (и, далее, в группу, где телеграм бот является админом - плюс такого состояния - можно добавлять любых заинтересованных лиц для 
быстрого реагирования на ситуацию)

Проект выполнен с UPS - при пропадении питания он, по-прежнему, будет работать довольно продолжительное время. Роутер с сим-картой тоже запитан от бесперебойника, поэтому связь
не попадет на объекте.

В дальнейшем, бэкэнд будет реагировать на длительное отсутсвие переданных показаний и, тоже, будет отправлять ошибку.

Проект в песочнице: https://wokwi.com/projects/380351279022927873
Проект на github: https://github.com/wizand0/DC_Monitor_UNO
Backend сервера для обработки данных: https://github.com/wizand0/flask_web_for_arduino


## Датчики и приборы:
- ESP8266 ESP-12F
- Источник питания 220/5В https://aliexpress.ru/item/1005006060171604.html?spm=a2g2w.orderdetail.0.0.aaee4aa6rtuntF&sku_id=12000035548325599
- Импульсный источник бесперебойного питания https://aliexpress.ru/item/1005002982442200.html?spm=a2g2w.orderdetail.0.0.2dce4aa6mQpzua&sku_id=12000027774110131
- ZMPT101B https://aliexpress.ru/item/4000908383725.html?spm=a2g2w.orderdetail.0.0.b8214aa6vTbB62&sku_id=10000010494015900
- DHT22 цифровой датчик температуры и влажности AM2302 https://aliexpress.ru/item/33037061522.html?spm=a2g2w.orderdetail.0.0.475b4aa6RRmOYf&sku_id=12000032670222588


## Versions:
### v. 1.0
- отображение фактического напряжения в сети;
- отправка данных на сервер;

### v. 0.9

- контроль температуры;
- контроль влажности;
- контроль напряжения в сети переменного тока;




## ToDo:
- добавить lcd для отображения фактических параметров;
- сохранять наличие ошибки в EEPROM для защиты от потери данных;
- при пропадении напряжения сохранить ошибку, контролировать появление wifi подключения, при появлении соединения отправить email об ошибке;