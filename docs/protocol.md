# Протокол взаимодействия модулей

## Общая структура запросов
```json
{
  "from": {
    "type": "type",
    "id": 1
  },
  "to": {
    "type": "type",
    "id": 1
  },
  "topic": "/topic",
  "body": {
    "...": "..."
  }
}
```

`from` - данные отправителя сообщения

`to` - данные получателя сообщения

`topic` - MQTT-топик, в который будет отправлено сообщение

`body` - полезная нагрузка сообщения

`type` - тип устройства

`id` - номер устройства (уникальный для всех типов устройств)

Типы устройств:
* `server` - сервер 
* `nfc` - NFC-считыватель
* `hooks` - RGB-подсветка крючков
* `control` - пульт управления гардеробщика

## NFC - Сервер

Датчик NFC отправляет на сервер данные считанные с карточки.

```json
{
  "from": {
    "type": "nfc",
    "id": 123
  },
  "to": {
    "type": "server",
    "id": 1
  },
  "topic": "/server/nfc/{nfc_id}",
  "body": {
    "tag": 123456
  }
}
```

`tag` - данные, считанные с метки

## Сервер - NFC

Сервер обрабатывает сообщение считывателя и присылает ему ответ 

```json
{
  "from": {
    "type": "server",
    "id": 1
  },
  "to": {
    "type": "nfc",
    "id": 123
  },
  "topic": "/nfc/{nfc_id}",
  "body": {
    "tag": 123456,
    "status": "push|block|pull"
  }
}
```

`tag` - метка, по которой прислан ответ

`status` - статус обработки метки

Возможные статусы:
* `new` - метка новая и добавлена в гардероб
* `block` - метка не добавлена в гардероб (очередь заполнена, неправильная метка, в гардеробе нет места)
* `exist` - метка существовала ранее, запрошена выдача

Необязательным аргументом тела этого запроса может быть поле `reason`
В котором указывается причина в отказе обработки метки (используется для отладки)

```json
{
  "body": {
    "tag": 123456,
    "status": "block",
    "reason": "hooks stuck"
  }
}
```

## Сервер - Крючки

После добавления метки происходит выбор и подсветка крючка

```json
{
  "from": {
    "type": "server",
    "id": 1
  },
  "to": {
    "type": "hooks",
    "id": 234
  },
  "topic": "/hooks/{hooks_id}",
  "body": {
    "hook_id": 42,
    "status": "push|pushed|pull|pulled",
    "color": "red|green|blue|black"
  }
}
```

`hook_id` - номер крючка

`status` - статус подсветки крючка

`color` - цвет подсветки крючка

Возможные статусы:
* `push` - вещь нужно повесить (ровное сияние)
* `pushed` - вещь повешена (отключение подсветки)
* `pull` - вещь нужно снять (мигание)
* `pulled` - вещь выдана (отключение подсветки)

## Сервер - пульт

После обработки метки гардеробщику на пульт приходит запрос на подтверждение занесения/выдачи вещи 

```json
{
  "from": {
    "type": "server",
    "id": 1
  },
  "to": {
    "type": "control",
    "id": 345
  },
  "topic": "/control/{control_id}",
  "body": {
    "tag": 123456,
    "status": "push|pushed|pull|pulled"
  }
}
```

`tag` - метка 

`status` - статус подсветки пульта

Возможные статусы:
* `push` - вещь нужно повесить (ровное сияние)
* `pushed` - вещь повешена (отключение подсветки)
* `pull` - вещь нужно снять (мигание)
* `pulled` - вещь выдана (отключение подсветки)

## Пульт - Сервер

Гардеробщик подтверждает занесение/выдачу вещи нажатием кнопки на пульте

```json
{
  "from": {
    "type": "control",
    "id": 345
  },
  "to": {
    "type": "server",
    "id": 1
  },
  "topic": "/server/control/{control_id}",
  "body": {
    "tag": 123456,
    "status": "pushed|pulled"
  }
}
```

`tag` - метка

`status` - статус нажатия на кнопку

Возможные статусы:
* `pushed` - вещь занесена
* `pulled` - вещь выдана
