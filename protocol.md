# Протокол взаимодействия модулей

## NFC -> Сервер

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

## Сервер -> NFC
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
    "status": "new|block|exist"
  }
}
```

## Сервер -> Крючки
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
    "power": true
  }
}
```

## Сервер -> пульт
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
    "status": "new"
  }
}
```

## Пульт -> Сервер
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
