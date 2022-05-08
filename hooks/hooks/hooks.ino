#include "Adafruit_NeoPixel.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <String> 

#define PIN 19        // пин DI
#define NUM_LEDS 50   // число диодов

#define SERIAL_SPEED 9600 // скорость serial-порта
#define CPU_SPEED 240 // скорость работы мк

#define MY_SSID "1102/3" // имя wi-fi сети
#define MY_PASSWD "54597549" // пароль wi-fi сети

#define MQTT_SERVER "51.250.105.61" // mqtt сервер - адрес
#define MQTT_PORT 1884 // mqtt сервер - порт

#define SERVER_ID "1" // ID устройства "сервер"
#define SERVER_NAME "server" // Имя устройства "сервер"
#define DEVICE_ID "500" // ID устройства "крючки"
#define HOOKS_ID "1"
#define DEVICE_NAME "hooks" // Имя устройства "крючки"

String TOPIC_READ = (String)"/" + (String)DEVICE_NAME + (String)"/" + (String)HOOKS_ID;

WiFiClient wifiClient; // объект доступа к wi-fi
PubSubClient MQTTclient(wifiClient); // объект общения с mqtt сервером
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800); // инициализация ленты

SemaphoreHandle_t MQTTSemaphoreKeepAlive; // семафор для работы с mqtt
SemaphoreHandle_t HooksSemaphore; // семафор для работы с hooks

// многозадачность (multi-tasking)
TaskHandle_t TaskLed; // обновление свечения кнопок
TaskHandle_t TaskMQTT; // общение с mqtt-сервером

int red = 0xff0000;
int black = 0x000000;

int led_from = 1;
int led_to = 32;

#define NUM_HOOKS led_from - led_to

struct hook {
  short hook_id; // номер крючка в гардеробе
  short led_id;  // номер светодиода на ленте
  char* state;   // состояние крючка (off|light|blink)
  int color;     // цвет свечения
}

volatile hook hooks[NUM_LEDS]; 

void setup() {
    Serial.begin(SERIAL_SPEED); // Serial для отладки
    setCpuFrequencyMhz(CPU_SPEED); // установка скорости CPU
    Serial.println("start");
  
    strip.begin();
    strip.setBrightness(50);    // яркость, от 0 до 255
    strip.clear();                          // очистить
    strip.show();                           // отправить на ленту
    led = 0;

    MQTTSemaphoreKeepAlive = xSemaphoreCreateBinary(); // Семафор !!! нужно останавливать во время "публикации" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    xSemaphoreGive(MQTTSemaphoreKeepAlive);

    // Определение семафора
    ButtonsSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(ButtonsSemaphore); // Деактивация семафора

    // Определение структуры
    // https://stackoverflow.com/questions/41347474/how-to-initialise-a-volatile-structure-with-a-non-volatile-structure

    xSemaphoreTake(HooksSemaphore, portMAX_DELAY); // Активация семафора с большой задержкой активации

    for (int i = 0; i < leds_to - leds_from + 1; i++) {
        leds[i].hook_id = leds_from + i;
        leds[i].led_id = i;
        leds[i].state = "off";
        leds[i].color = black;
    }

    xSemaphoreGive(HooksSemaphore); // Деактивация семафора

    // Определение задач
    xTaskCreatePinnedToCore(
        blinkHandler,
        "TaskBlink",
        10000,
        NULL,
        1,
        &TaskLed,
        0);
    delay(500);

    xTaskCreatePinnedToCore(
        mqttHandler,
        "TaskMQTT",
        50000,
        NULL,
        1,
        &TaskMQTT,
        1);
    delay(500);
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
        if (MQTTclient.connect(DEVICE_ID)) {
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

// обработка свечения кнопок
void blinkHandler(void * pv_parameters) {
    int led_ms[NUM_LEDS]; // по счётчику на кнопку для поддержки мигания
    for (int i = 0; i < led_ms.sixe(); i++) {
        led_ms[i] = 0;  
    }
    blink_period = 500; // период мигания
    handle_delay = 100; // задержка между итерациями
    
    while (true) {

        xSemaphoreTake(HooksSemaphore, portMAX_DELAY);

        for(short i = 0; i < NUM_HOOKS; i++) {
            switch (hooks[i].state) {
                case "off":                // крючок не горит
                    hooks[i].color = black;
                    break;
                case "light":              // крючок горит
                    hooks[i].color = red;
                    break;
                case "blink":              // крючок мигает
                    led_ms[i] += handle_delay;
                    if (led_ms[i] > blink_period) {
                        hooks[i].color = (hooks[i].color == black ? red : black);
                        led_ms[i] = 0; 
                    }
                    break;
            }
        }

        xSemaphoreGive(HooksSemaphore);

        update_tape();
        delay(handle_delay); // задержка на всякий случай - чтобы пины быстро не переключать
    }
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
        MQTTclient.loop(); // важная функция да да - частично отвечает за вызов callback

        xSemaphoreGive( MQTTSemaphoreKeepAlive );

        delay(1000);
    }
}

void update_tape() {
    xSemaphoreTake(HooksSemaphore, portMAX_DELAY);
    for (int i = 0; i < NUM_HOOKS; i++) {
        strip.setPixelColor(hooks[i].led_id, hooks[i].color);
    }
    strip.show();
    xSemaphoreGive(HooksSemaphore);
}

void loop() {
    // тесты (удалены)
    vTaskDelete(NULL); // Удаление лишней задачи loop
}

void stripHandler(void * pv_parameters) { // обработка включения-выключения, нажатия кнопок
    while (true) {
        // смотрим очередь запросов
        // queueMqttCallbackToButtonsHandler
        if (uxQueueMessagesWaiting(queueMqttCallbackToButtonsHandler) >= 2) { // если в очереди есть Id действия + тэг
            short taskId = -1;
            xQueueReceive(queueMqttCallbackToButtonsHandler, &taskId, portMAX_DELAY); // получим Id действия 0 - push, 1 - pull, 2 - pushed, 3 - pulled
            int hook_id = -1;
            xQueueReceive(queueMqttCallbackToButtonsHandler, &hook_id, portMAX_DELAY); // получим тэг
            int led_id = hook_id - led_from; 
            xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY);

            short btnWithTag = -1;
            
            switch(taskId) {
                case 0:
                    hooks[led_id].state = "light";
                    break;
                case 1:
                    hooks[led_id].state = "blink";
                    break;
                case 2:
                    hooks[led_id].state = "off";
                    break;
                case 3:
                    hooks[led_id].state = "off";
                    break;
            }

            xSemaphoreGive(ButtonsSemaphore);
        }
        delay(50);
    };
}

// функция обработки входящих сообщений
void callback(char * topic, byte * message, unsigned int length) { 

    String messageTemp;

    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }

    if (strcmp(topic, TOPIC_READ.c_str()) == 0) { // вроде всегда true, так как мы подписаны на "наш" топик, но на всякий
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, messageTemp);

        int hook_id = doc["body"]["hook_id"]; // LONG !!!
        const char* stat = doc["body"]["status"];
        short taskId = -1;

        Serial.print(tag);
        Serial.print(" ");
        Serial.print(stat);
        Serial.println();

        if (strcmp(stat, "push") == 0) { // запрос на "вещь нужно повесить (ровное сияние)" - 0
            taskId = 0;
            xQueueSend(queueMqttCallbackToButtonsHandler, &taskId, portMAX_DELAY);
            xQueueSend(queueMqttCallbackToButtonsHandler, &tag, portMAX_DELAY);
        }
        else if (strcmp(stat, "pull") == 0) { // запрос на "вещь нужно снять (мигание)" - 1
            taskId = 1;
            xQueueSend(queueMqttCallbackToButtonsHandler, &taskId, portMAX_DELAY);
            xQueueSend(queueMqttCallbackToButtonsHandler, &tag, portMAX_DELAY);
        }
        else if (strcmp(stat, "pushed") == 0) { // запрос на "вещь повешена (отключение подсветки)" - 2
            taskId = 2;
            xQueueSend(queueMqttCallbackToButtonsHandler, &taskId, portMAX_DELAY);
            xQueueSend(queueMqttCallbackToButtonsHandler, &tag, portMAX_DELAY);
        }
        else if (strcmp(stat, "pulled") == 0) { // запрос на "вещь выдана (отключение подсветки)" - 3
            taskId = 3;
            xQueueSend(queueMqttCallbackToButtonsHandler, &taskId, portMAX_DELAY);
            xQueueSend(queueMqttCallbackToButtonsHandler, &tag, portMAX_DELAY);
        }
    }
}
