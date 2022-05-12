# Умный гардероб
## Проект Академии Samsung, трек IoT

Модули:
* [Модуль Server](./python_server)
* [Модуль Control_Box](./control_box)
* [Модуль Hooks](./hooks)
* [Модуль NFC_Reader](./nfc_reader)

## Зависимости

### Зависимости сервера (Python)

* `paho-mqtt` - работа с протоколом MQTT
* `SQLAlchemy` - работа с SQLLite с помощью ORM

### Зависимости кода датчиков (C++)

В силу специфики загрузки кода на микроконтроллеры, этот процесс не был автоматизирован

* `PubSubClient.h` - работа с протоколом MQTT
* `ArduinoJson.h` - работа с Json в среде Arduino
* `Adafruit_NeoPixel.h` - работа с адресуемой светодиодной лентой WS2811
* `MFRC522.h` - работа с NFC-датчиком MFRC522
* `WiFi.h` - подключение к WiFi

## Запуск

Установка зависимостей
```

```

Запуск
```
python python_server/main.py
```