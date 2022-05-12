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
cd python_server
pip3 install -r requirements.txt
```

Запуск
```
python3 main.py
```

Сборка колеса
```
python3 -m build
```

Установка колеса
```
cd dist
pip install SmartCheckRoom-0.0.1-py3-none-any.whl 
```

Использование колеса
```
python3 -m server.main
```

Сборка документации
```
cd ..
python setup.py build_sphinx
```
