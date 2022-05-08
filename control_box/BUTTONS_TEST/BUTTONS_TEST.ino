#include <WiFi.h> // wi-fi доступ
#include <PubSubClient.h> // библиотека для работы с mqtt
#include <ArduinoJson.h> // библиотека для работы с json
#include <String> // библиотека для работы со строками

#define SERIAL_SPEED 9600 // скорость serial-порта
#define CPU_SPEED 240 // скорость работы мк

#define BUTTON_PIN_BLUE_LIGHT 23 // синяя кнопка на прием (горящая) - значение
#define LED_PIN_BLUE_LIGHT 22 // синяя кнопка на прием (горящая) - свет

#define BUTTON_PIN_RED_LIGHT 21 // красная кнопка на прием (горящая) - значение
#define LED_PIN_RED_LIGHT 19 // красная кнопка на прием (горящая) - свет

#define BUTTON_PIN_BLUE_BLINK 4 // синяя кнопка на выдачу (мигающая) - значение
#define LED_PIN_BLUE_BLINK 2 // синяя кнопка на выдачу (мигающая) - свет

#define BUTTON_PIN_RED_BLINK 32 // красня кнопка на выдачу (мигающая) - значение
#define LED_PIN_RED_BLINK 33 // красная кнопка на выдачу (мигающая) - свет

#define BUTTONS_AMOUNT 4 // Количество кнопок в системе (0-1 горят, 2-3 мигают)

#define READ_AMOUNT 10 // количество считываний для противодействия дребезгу

typedef struct {
    short pin_value; // пин для кнопки (значение)
    short pin_light; // пин для кнопки (свет)
    bool btn_value; // кнопка нажата?
    bool light_value; // кнопка должна светится/мигать?
}
buttonStruct;

// горящие
#define BUTTON_BLUE_LIGHT_ID 0
#define BUTTON_RED_LIGHT_ID 1
// мигающие
#define BUTTON_BLUE_BLINK_ID 2
#define BUTTON_RED_BLINK_ID 3

static volatile buttonStruct buttons[BUTTONS_AMOUNT]; // массив с кнопками
// 0 - верхняя левая (синяя прием горящая) - BUTTON_BLUE_LIGHT_ID
// 1 - верхняя правая (красная прием горящая) - BUTTON_RED_LIGHT_ID
// 2 - нижняя левая (синяя выдача мигающая) - BUTTON_BLUE_BLINK_ID
// 3 - нижняя правая (красная выдача мигающая) - BUTTON_RED_BLINK_ID
// первая половина - горящие, вторая - мигающие

#define MY_SSID "RickGuest" // имя wi-fi сети
#define MY_PASSWD "1473690014736900" // пароль wi-fi сети

#define MQTT_SERVER "51.250.105.61" // mqtt сервер - адрес
#define MQTT_PORT 1884 // mqtt сервер - порт

#define SERVER_ID "1" // ID устройства "сервер"
#define SERVER_NAME "server" // Имя устройства "сервер"
#define DEVICE_ID "1" // ID устрйоства "пульт"
#define DEVICE_NAME "control" // Имя устройства "пульт"

String TOPIC_READ = (String)"/" + (String)DEVICE_NAME + (String)"/" + (String)DEVICE_ID;
String TOPIC_WRITE = (String)"/" + (String)SERVER_NAME + (String)"/" + (String)DEVICE_NAME + (String)"/" + (String)DEVICE_ID;

WiFiClient wifiClient; // объект доступа к wi-fi
PubSubClient MQTTclient(wifiClient); // объект общения с mqtt сервером

SemaphoreHandle_t MQTTSemaphoreKeepAlive; // семафор для работы с mqtt

// многозадачность (multi-tasking)
TaskHandle_t TaskButtons; // обновление значений кнопок
TaskHandle_t TaskBlink; // обновление свечения кнопок
TaskHandle_t TaskMQTT; // общение с mqtt-сервером

SemaphoreHandle_t ButtonsSemaphore; // семафор для работы с buttons

float average(bool * arr, short n) { // найти среднее логическое значение
    float avg = 0;
    for (short i = 0; i < n; i++) {
        avg += arr[i];
    }
    avg /= (float) n;
    return avg;
}

bool buttonState(float avg_value) { // устранение дребезга - определение состояня кнопки
    return !(avg_value - 0.5 > 0.001);
}

bool readButtonValue(short pin) { // выдать значение кнопки (по pin)
    xSemaphoreGive(ButtonsSemaphore); // Деактивация семафора

    bool input_array[READ_AMOUNT];

    for (short i = 0; i < READ_AMOUNT; i++) {
        input_array[i] = digitalRead(pin);
        delay(5);
    }

    bool state = buttonState(average(input_array, READ_AMOUNT));

    xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY); // Активация семафора

    return state;
}

void readButtonsValue(volatile buttonStruct * buttons) { // обновить значения всех кнопок структуры
    for (short i = 0; i < BUTTONS_AMOUNT; i++) {
        buttons[i].btn_value = readButtonValue(buttons[i].pin_value); // записываем новое значение
    }
}

// переменные только для buttonsHandler
static volatile long long buttons_to_tags[] = {0, 0, 0, 0};
// переменные только для buttonsHandler

QueueHandle_t queueMqttCallbackToButtonsHandler; // очередь данных callback -> buttonsHandler

void sendMqttMessage(long long tag, String state) {
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
    body["status"] = state; // pushed/pulled

    serializeJson(doc, output);

    Serial.println(output);

    xSemaphoreTake( MQTTSemaphoreKeepAlive, portMAX_DELAY ); // блокируем mqtt

    MQTTclient.publish(TOPIC_WRITE.c_str(), output.c_str());

    xSemaphoreGive( MQTTSemaphoreKeepAlive ); // блокируем mqtt
}

short findAllowedBtn(short startN, short stopN) { // поиск свободного пина в промежутке start <= x < stop
    for (short i = startN; i < stopN; i++) {
        if (buttons[i].light_value == false) {
            return i;
        }
    }
    return -1;
}

void handlePressButton(short startN, short stopN, String request) { // обработка нажатости кнопки в промежутке start <= x < stop
    for (short i = startN; i < stopN; i++) { // Проверяем нажатость кнопок
        if (buttons_to_tags[i] > 0 && buttons[i].btn_value == true) { // Кнопка нажата и на ней была "задача" - какой-то тэг
            // нужно выключить кнопку и отправить запрос
            sendMqttMessage(buttons_to_tags[i], request);
        }
    }
}


short findBtnWithTag(short startN, short stopN,long long tag) { // поиск кнопки с заданным тэгом в промежутке
    for(short i = startN; i < stopN; i++) {
        if (buttons_to_tags[i] == tag) {
            return i;
        }
    }
    return -1;
}

void buttonsHandler(void * pv_parameters) { // обработка включения-выключения, нажатия кнопок
    for (;;) {

        xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY); // Активация семафора
        
        // индексы доступных кнопок для назначения
        short allowed_btn_push = findAllowedBtn(0, BUTTONS_AMOUNT / 2);
        short allowed_btn_pull = findAllowedBtn(BUTTONS_AMOUNT / 2, BUTTONS_AMOUNT);
        
        xSemaphoreGive(ButtonsSemaphore);

        // смотрим очередь запросов
        // queueMqttCallbackToButtonsHandler
        if (uxQueueMessagesWaiting(queueMqttCallbackToButtonsHandler) >= 2) { // если в чоереди есть Id действия + тэг
            short taskId = -1;
            xQueueReceive(queueMqttCallbackToButtonsHandler, &taskId, portMAX_DELAY); // получим Id действия 0 - push, 1 - pull, 2 - pushed, 3 - pulled
            long long tag = -1;
            xQueueReceive(queueMqttCallbackToButtonsHandler, &tag, portMAX_DELAY); // получим тэг

            xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY);

            short btnWithTag = -1;
            
            switch(taskId) {
                case 0:
                    if (allowed_btn_push >= 0) {
                        buttons_to_tags[allowed_btn_push] = tag;
                        buttons[allowed_btn_push].light_value = true;
                    }
                    break;
                case 1:
                    if (allowed_btn_pull >= 0) {
                        buttons_to_tags[allowed_btn_pull] = tag;
                        buttons[allowed_btn_pull].light_value = true;
                    }
                    break;
                case 2:
                    btnWithTag = findBtnWithTag(0, BUTTONS_AMOUNT / 2, tag);
                    if (btnWithTag >= 0) {
                        buttons_to_tags[btnWithTag] = 0;
                        buttons[btnWithTag].light_value =  false;
                    }
                    break;
                case 3:
                    btnWithTag = findBtnWithTag(BUTTONS_AMOUNT / 2, BUTTONS_AMOUNT, tag);
                    if (btnWithTag >= 0) {
                        buttons_to_tags[btnWithTag] = 0;
                        buttons[btnWithTag].light_value =  false;
                    }
                    break;
            }

            xSemaphoreGive(ButtonsSemaphore);
        }

        // обработка нажатий

        xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY); // Активация семафора с большой задержкой активации

        // Делаем работу по считыванию
        readButtonsValue(buttons);
        // Работа по считыванию завершена

        // обработка нажатости кнопок
        handlePressButton(0, BUTTONS_AMOUNT / 2, (String)"pushed");
        handlePressButton(BUTTONS_AMOUNT / 2, BUTTONS_AMOUNT, (String)"pulled");

        xSemaphoreGive(ButtonsSemaphore); // Деактивация семафора

        delay(50);
    };
}

void toggleHandler(void * pv_parameters) { // мигание кнопки
    short * pin = (short *) pv_parameters;
    bool blinkingState = HIGH;
    for (;;) {
        digitalWrite(*pin, blinkingState);
        blinkingState = !blinkingState;
        delay(1000);
    }
}

TaskHandle_t buttons_blinking_handlers[] = {NULL, NULL}; // обработчики мигания кнопок

void blinkHandler(void * pv_parameters) { // обработка свечение кнопок
    for (;;) {

        xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY);

        for(short i = 0; i < BUTTONS_AMOUNT; i++) {
            if (i < BUTTONS_AMOUNT / 2) {
                digitalWrite(buttons[i].pin_light, buttons[i].light_value);
            }
            else {
                if (buttons[i].light_value == true && buttons_blinking_handlers[i - BUTTONS_AMOUNT / 2] == NULL) {
                    xTaskCreate(
                        toggleHandler,
                        ((String)"TaskToggle" + String(i)).c_str(),
                        1000,
                        (void *)&(buttons[i].pin_light), // параметр
                        1,
                        &buttons_blinking_handlers[i - BUTTONS_AMOUNT / 2] // обработчик
                    );
                }
                else if (buttons[i].light_value == false && buttons_blinking_handlers[i - BUTTONS_AMOUNT / 2] != NULL) {
                    vTaskDelete(buttons_blinking_handlers[i - BUTTONS_AMOUNT / 2]);
                    digitalWrite(buttons[i].pin_light, LOW); // выключение моргания
                    buttons_blinking_handlers[i - BUTTONS_AMOUNT / 2] = NULL;
                }
            }
        }

        xSemaphoreGive(ButtonsSemaphore);

        delay(100); // задержка на всякий случай - чтобы пины быстро не переключать
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

        /*  Мусорный вывод для проверки работы MQTT
            MQTTclient.publish(TOPIC_READ.c_str(),
            (
                "s pol pinka" + String(random(300))
            ).c_str()
            );*/

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

void pinInit(volatile buttonStruct * buttons) { // инициализация пинов
    for (short i = 0; i < BUTTONS_AMOUNT; i++) {
        pinMode(buttons[i].pin_value, INPUT_PULLUP);
        pinMode(buttons[i].pin_light, OUTPUT);
    }
}

void buttonInit(short index, short pinBtn, short pinLight, bool clicked, bool light) {
    cli(); // magic tool
    buttons[index].pin_value = pinBtn;
    buttons[index].pin_light = pinLight;
    buttons[index].btn_value = clicked;
    buttons[index].light_value = light;
    sei(); // magic tool
}

void setup() {
    delay(3000); // Чтобы быстро не запускалось

    Serial.begin(SERIAL_SPEED); // Serial для отладки

    setCpuFrequencyMhz(CPU_SPEED); // установка скорости CPU

    // Иницилизация очереди
    queueMqttCallbackToButtonsHandler = xQueueCreate(100, sizeof(long long)); // очередь в 100 long long

    MQTTSemaphoreKeepAlive = xSemaphoreCreateBinary(); // Семафор !!! нужно останавливать во время "публикации" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    xSemaphoreGive(MQTTSemaphoreKeepAlive);

    // Определение семафора
    ButtonsSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(ButtonsSemaphore); // Деактивация семафора

    // Определение структуры
    // https://stackoverflow.com/questions/41347474/how-to-initialise-a-volatile-structure-with-a-non-volatile-structure

    xSemaphoreTake(ButtonsSemaphore, portMAX_DELAY); // Активация семафора с большой задержкой активации

    short buttonsId[] = {BUTTON_BLUE_LIGHT_ID, BUTTON_RED_LIGHT_ID, BUTTON_BLUE_BLINK_ID, BUTTON_RED_BLINK_ID}; // 0 1 2 3
    short buttonPins[] = {BUTTON_PIN_BLUE_LIGHT, BUTTON_PIN_RED_LIGHT, BUTTON_PIN_BLUE_BLINK, BUTTON_PIN_RED_BLINK};
    short ledPins[] = {LED_PIN_BLUE_LIGHT, LED_PIN_RED_LIGHT, LED_PIN_BLUE_BLINK, LED_PIN_RED_BLINK};
    for (short index : buttonsId)
        buttonInit(index, buttonPins[index], ledPins[index], false, false);

    // Определение свойств пинов
    pinInit(buttons);

    xSemaphoreGive(ButtonsSemaphore); // Деактивация семафора

    // Определение задач
    xTaskCreatePinnedToCore(
        buttonsHandler, // Функция задачи
        "TaskButtons", // Имя задачи
        25000, // Размер стека задачи
        NULL, // Параметры для задачи
        1, // Приоритет задачи
        &TaskButtons, //Переменная задачи для отслеживания созданной задачи
        0); // Определение ядра для задачи 0/1
    delay(500);

    xTaskCreatePinnedToCore(
        blinkHandler,
        "TaskBlink",
        10000,
        NULL,
        1,
        &TaskBlink,
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

void loop() {
    // тесты (удалены)

    vTaskDelete(NULL); // Удаление лишней задачи loop
}

void callback(char * topic, byte * message, unsigned int length) { // функция обработки взодящих сообщений

    String messageTemp;

    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }

    if (strcmp(topic, TOPIC_READ.c_str()) == 0) { // вроде всегда true, так как мы подписаны на "наш" топик, но на всякий
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, messageTemp);

        long long tag = doc["body"]["tag"]; // LONG !!!
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
