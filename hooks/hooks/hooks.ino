#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <String> 
#include "Adafruit_NeoPixel.h"

#define SERIAL_SPEED 9600 // скорость serial-порта
#define CPU_SPEED 240 // скорость работы мк

#define PIN 19 // пин DI
#define NUM_hooks 50 // число диодов

#define MY_SSID "1102/3" // имя wi-fi сети
#define MY_PASSWD "54597549" // пароль wi-fi сети

#define MQTT_SERVER "51.250.105.61" // mqtt сервер - адрес
#define MQTT_PORT 1884 // mqtt сервер - порт

#define SERVER_ID "1" // ID устройства "сервер"
#define SERVER_NAME "server" // Имя устройства "сервер"
#define DEVICE_ID "1" // ID устройства "крючки"
#define DEVICE_NAME "hooks" // Имя устройства "крючки"

#define RED   0xff0000
#define GREEN 0x0000ff
#define BLUE  0x00ff00
#define BLACK 0x000000

#define hook_from 1
#define hook_to 32

#define NUM_HOOKS hook_to - hook_from + 1

String TOPIC_READ = (String)"/" + (String)DEVICE_NAME + (String)"/" + (String)DEVICE_ID;

WiFiClient wifiClient; // объект доступа к wi-fi
PubSubClient MQTTclient(wifiClient); // объект общения с mqtt сервером
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_hooks, PIN, NEO_GRB + NEO_KHZ800); // инициализация ленты

SemaphoreHandle_t MQTTSemaphoreKeepAlive; // семафор для работы с mqtt
SemaphoreHandle_t HooksSemaphore; // семафор для работы с hooks

// многозадачность (multi-tasking)
TaskHandle_t TaskLed; // обновление свечения кнопок
TaskHandle_t TaskMQTT; // общение с mqtt-сервером

typedef enum {
    OFF,
    LIGHT,
    BLINK
} states;

typedef struct {
  short hook_id; // номер крючка в гардеробе
  short led_id;  // номер светодиода на ленте
  states state;   // состояние крючка (off|light|blink)
  bool is_light; // должна ли гореть крючек
  int color;     // цвет свечения
} hook;

static volatile hook hooks[NUM_HOOKS];

void hookInit(short index, short hook_id, short led_id, states state, bool is_light, int color) {
    cli(); // magic tool
    hooks[index].hook_id = hook_id;
    hooks[index].led_id = led_id;
    hooks[index].state = state;
    hooks[index].is_light = is_light;
    hooks[index].color = color;
    sei(); // magic tool
}

void setup() {
    delay(3000);
    Serial.begin(SERIAL_SPEED); // Serial для отладки
    setCpuFrequencyMhz(CPU_SPEED); // установка скорости CPU
    Serial.println("start");
  
    strip.begin();
    strip.setBrightness(50); // яркость, от 0 до 255
    strip.clear(); // очистить
    strip.show(); // отправить на ленту

    MQTTSemaphoreKeepAlive = xSemaphoreCreateBinary(); // Семафор !!! нужно останавливать во время "публикации" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    xSemaphoreGive(MQTTSemaphoreKeepAlive);

    // Определение семафора
    HooksSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(HooksSemaphore); // Деактивация семафора

    // Определение структуры
    // https://stackoverflow.com/questions/41347474/how-to-initialise-a-volatile-structure-with-a-non-volatile-structure

    xSemaphoreTake(HooksSemaphore, portMAX_DELAY); // Активация семафора с большой задержкой активации

    for (short i = 0; i < NUM_HOOKS; i++) {
        hookInit(i, hook_from + i, i, OFF, false, BLACK);
    }

    xSemaphoreGive(HooksSemaphore); // Деактивация семафора

    // Определение задач
    xTaskCreatePinnedToCore(
        blinkHandler,
        "TaskBlink",
        10000,
        NULL,
        0,
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
        if (MQTTclient.connect(
            (
                (String)DEVICE_ID + (String)DEVICE_NAME
            ).c_str())
         ){
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
    strip.clear();
    for (int i = 0; i < NUM_HOOKS; i++) {
        if (hooks[i].is_light)
            strip.setPixelColor(hooks[i].led_id, hooks[i].color);
        else
            strip.setPixelColor(hooks[i].led_id, BLACK);
    }
    strip.show();
}

// обработка свечения кнопок
void blinkHandler(void * pv_parameters) {
    int led_ms[NUM_hooks]; // по счётчику на кнопку для поддержки мигания
    for (short i = 0; i < NUM_hooks; i++) {
        led_ms[i] = 0;  
    }
    int blink_period = 500; // период мигания
    int handle_delay = 100; // задержка между итерациями
    
    for(;;) {

        xSemaphoreTake(HooksSemaphore, portMAX_DELAY);

        for(short i = 0; i < NUM_HOOKS; i++) {
            switch (hooks[i].state) {
                case OFF:                // крючок не горит
                    hooks[i].color = BLACK;
                    hooks[i].is_light = false;
                    break;
                case LIGHT:              // крючок горит
                    hooks[i].is_light = true;
                    break;
                case BLINK:              // крючок мигает
                    led_ms[i] += handle_delay;
                    if (led_ms[i] > blink_period) {
                        hooks[i].is_light = !hooks[i].is_light;
                        led_ms[i] = 0; 
                    }
                    break;
            }
        }
        
        update_tape();
        
        xSemaphoreGive(HooksSemaphore);

        delay(handle_delay); // задержка на всякий случай - чтобы пины быстро не переключать
    }
}

void loop() {
    // тесты (удалены)
    vTaskDelete(NULL); // Удаление лишней задачи loop
}

int charColorToIntColor(const char* color) {
    if (strcmp(color, "red") == 0) {
        return RED;
    }
    else if (strcmp(color, "green") == 0) {
        return GREEN;
    }
    else if (strcmp(color, "blue") == 0) {
        return BLUE;
    }
    else if (strcmp(color, "black") == 0) {
        return BLACK;
    }
    return BLACK;
}

states charStateToStatesState(const char* stat) {
    if (strcmp(stat, "push") == 0) {
        return LIGHT;
    }
    else if (strcmp(stat, "pushed") == 0) {
        return OFF;
    }
    else if (strcmp(stat, "pull") == 0) {
        return BLINK;
    }
    else if (strcmp(stat, "pulled") == 0) {
        return OFF;
    }
    return OFF;
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

        short hook_id = doc["body"]["hook_id"];
        states stat = charStateToStatesState(doc["body"]["status"]);
        int color = charColorToIntColor(doc["body"]["color"]);

        Serial.print(hook_id);
        Serial.print(" ");
        Serial.print(stat);
        Serial.print(" ");
        Serial.print(color);
        Serial.println();

        short led_id = hook_id - hook_from;

        xSemaphoreTake( HooksSemaphore, portMAX_DELAY );

        hookInit(led_id, hooks[led_id].hook_id, hooks[led_id].led_id, stat, hooks[led_id].is_light, color);

        xSemaphoreGive( HooksSemaphore);
    }
}
