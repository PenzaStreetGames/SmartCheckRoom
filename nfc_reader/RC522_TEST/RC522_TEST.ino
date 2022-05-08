// http://developer.alexanderklimov.ru/arduino/rfid_rc522.php
// https://esp32io.com/tutorials/esp32-rfid-nfc
// 5v vs 3.3v ???

#include <WiFi.h> // wi-fi доступ
#include <PubSubClient.h> // библиотека для работы с mqtt
#include <ArduinoJson.h> // библиотека для работы с json
#include <String> // библиотека для работы со строками

#include <SPI.h> // spi
#include <MFRC522.h> // mfrc522

#define SERIAL_SPEED 9600 // скорость serial-порта
#define CPU_SPEED 240 // скорость работы мк

#define SS_PIN  21 // sda
#define RST_PIN 22 // rst

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Создание экземпляра MFRC522

#define MY_SSID "RickGuest" // имя wi-fi сети
#define MY_PASSWD "1473690014736900" // пароль wi-fi сети

#define MQTT_SERVER "51.250.105.61" // mqtt сервер - адрес
#define MQTT_PORT 1884 // mqtt сервер - порт

#define SERVER_ID "1" // ID устройства "сервер"
#define SERVER_NAME "server" // Имя устройства "сервер"
#define DEVICE_ID "1" // ID устрйоства "nfc"
#define DEVICE_NAME "nfc" // Имя устройства "nfc"

String TOPIC_READ = (String)"/" + (String)DEVICE_NAME + (String)"/" + (String)DEVICE_ID;
String TOPIC_WRITE = (String)"/" + (String)SERVER_NAME + (String)"/" + (String)DEVICE_NAME + (String)"/" + (String)DEVICE_ID;

WiFiClient wifiClient; // объект доступа к wi-fi
PubSubClient MQTTclient(wifiClient); // объект общения с mqtt сервером

SemaphoreHandle_t MQTTSemaphoreKeepAlive; // семафор для работы с mqtt

// многозадачность (multi-tasking)
TaskHandle_t TaskMQTT; // общение с mqtt-сервером
TaskHandle_t TaskNFC; // для считывания NFC

QueueHandle_t queueNfcToMqtt; // очередь данных nfcHandler -> mqttHandler

void sendMqttMessage(long long tag) {
    // нужно отправить запрос
    StaticJsonDocument<1024> doc;
    String output = "";

    JsonObject from = doc.createNestedObject("from");
    from["type"] = DEVICE_NAME;
    from["id"] = DEVICE_ID;

    JsonObject to = doc.createNestedObject("to");
    to["type"] = SERVER_NAME;
    to["id"] = SERVER_ID;
    doc["topic"] = TOPIC_WRITE;

    JsonObject body = doc.createNestedObject("body");
    body["tag"] = tag;

    serializeJson(doc, output);

    Serial.println(output);

    MQTTclient.publish(TOPIC_WRITE.c_str(), output.c_str());
}

void mqttHandler(void * pv_parameters) { // обработка поддержания mqtt соединения (keep alive)
    xSemaphoreTake( MQTTSemaphoreKeepAlive, portMAX_DELAY );

    MQTTclient.setKeepAlive( 45 ); // протестировать (45-90-убрать)

    setup_wifi(); // установка wi-fi соединения
    MQTTclient.setServer(MQTT_SERVER, MQTT_PORT); // установка данных сервера
    MQTTclient.setCallback(callback); // установка функции, обрабатывающей вх. соединения

    xSemaphoreGive( MQTTSemaphoreKeepAlive );

    for (;;) {
        xSemaphoreTake( MQTTSemaphoreKeepAlive, portMAX_DELAY );

        if (!MQTTclient.connected()) { // переподключение
            reconnect();
        }

        if (uxQueueMessagesWaiting(queueNfcToMqtt) >= 1) { // если в очереди есть тэг
            long long tag;
            
            xQueueReceive(queueNfcToMqtt, &tag, portMAX_DELAY);

            sendMqttMessage(tag);
        }

        MQTTclient.loop(); // важная функция да да - частично отвечает за вызов callback

        xSemaphoreGive( MQTTSemaphoreKeepAlive );

        delay(1000);
    }
}

void setup_wifi() {
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(MY_SSID);

    WiFi.begin(MY_SSID, MY_PASSWD);

    while (WiFi.status() != WL_CONNECTED) { // Добиваемся подключения к wi-fi
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    // ожидание подключения к mqtt серверу
    while (!MQTTclient.connected()) {
        Serial.print("Attempting MQTT connection...");
        // попытка подключения
        if (MQTTclient.connect(((String)DEVICE_ID + (String)DEVICE_NAME).c_str())) {
            Serial.println("connected");
            // подписка на входящий топик
            MQTTclient.subscribe(TOPIC_READ.c_str());
        }
        else { // что-то сломалось
            Serial.print("failed, rc=");
            Serial.print(MQTTclient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

long long byteArrayToLongLong(byte *buf, byte sizeB) {
    long long value = 0;
    for (byte i = 0; i < sizeB; i++) {
        value = (value << 8) + (buf[i] & 0xFF);
    }
    return value;
}

void nfcHandler(void * pv_parameters) { // обработка nfc

    for(;;){
        // Ожидание
        while ( !mfrc522.PICC_IsNewCardPresent()) {
            delay(25);
        }
    
        // чтение
        while ( !mfrc522.PICC_ReadCardSerial()) {
            delay(25);
        }

        long long tag = byteArrayToLongLong(mfrc522.uid.uidByte, mfrc522.uid.size);

        xQueueSend(queueNfcToMqtt, &tag, portMAX_DELAY);
        
        delay(1000);
    }

}

void setup() {

    delay(3000); // Чтобы быстро не запускалось

    Serial.begin(SERIAL_SPEED); // Serial для отладки
    SPI.begin(); // запуск spi

    setCpuFrequencyMhz(CPU_SPEED); // установка скорости CPU


    MQTTSemaphoreKeepAlive = xSemaphoreCreateBinary(); // Семафор !!! нужно останавливать во время "публикации" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    xSemaphoreGive(MQTTSemaphoreKeepAlive);

    queueNfcToMqtt = xQueueCreate(100, sizeof(long long)); // очередь в 100 long long


    // инициализация MFRC522
    mfrc522.PCD_Init();
    // выводим номер версии прошивки ридера
    mfrc522.PCD_DumpVersionToSerial();
    

    // Определение задач
    xTaskCreatePinnedToCore(
        mqttHandler,
        "TaskMQTT",
        50000,
        NULL,
        1,
        &TaskMQTT,
        1);

    xTaskCreatePinnedToCore(
        nfcHandler,
        "TaskNFC",
        10000,
        NULL,
        1,
        &TaskNFC,
        0);
    delay(500);
}

void loop() {
    vTaskDelete(NULL); // Удаление лишней задачи loop
}

void callback(char * topic, byte * message, unsigned int length) { // функция обработки взодящих сообщений

    String messageTemp;

    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }

    if (strcmp(topic, TOPIC_READ.c_str()) == 0) { // вроде всегда true, так как мы подписаны на "наш" топик, но на всякий
        /*StaticJsonDocument<1024> doc;
        deserializeJson(doc, messageTemp);

        long long tag = doc["body"]["tag"]; // LONG !!!
        const char* stat = doc["body"]["status"];
        short taskId = -1;

        Serial.print(tag);
        Serial.print(" ");
        Serial.print(stat);
        Serial.println();*/

        Serial.println(messageTemp);

        // обработка входящих сообщений
    }
}
