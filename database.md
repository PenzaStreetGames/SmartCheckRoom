# Структура базы данных
![](resources/db_schema.png)

Предполагаемый диалект - SqlLite

## Таблицы

### Device

Список всех устройств в системе

Поля таблицы:
* `device_id` -
* `type` -
* `name` -

### Hook

Список крючков в гардеробе

Поля таблицы:
* `hook_id` -
* `hanger_id` -
* `status` -

### Nfc

Список NFC-считывателей

Поля таблицы:
* `nfc_id` -
* `device_id` -
* `control_id` -

### Control

Список пультов управления гардеробщиков

Поля таблицы:
* `control_id` -
* `device_id` -

### Hanger

Список пролётов с крючками (контроллеров RGB-лент)

Поля таблицы:
* `hanger_id` -
* `device_id` -
* `control_id` -
* `number_id` -

### Tag

Список NFC-меток в системе

Поля таблицы:
* `tag_id` -
* `code` -
* `status` -

### Tag_state

Список операций, совершаемых с метками

Поля таблицы:
* `tag_state_id` -
* `tag_id` -
* `status` -
* `event_time` -
* `hook_id` -
* `control_id` -

### Control_queue

Состояние очереди пульта управления

Поля таблицы:
* `control_queue_id` - 
* `control_id` -
* `direction` -
* `position` -
* `tag_id` -
* `status` -


