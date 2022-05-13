import paho.mqtt.client as mqtt
from mqtt_handlers.module_handlers.module_topic_handler import ModuleTopicHandler


class ControlBoxTest(ModuleTopicHandler):
    """
    Обработчик пульта гардеробщика
    """

    def __init__(self, id):
        topic_in = f"/control/{id}"
        topic_out = f"/server/control/{id}"
        super().__init__("control", id, topic_in, topic_out)

    def send_tag_status(self, tag, status):
        """Отправка на сервер сообщения об изменении статуса тега в очереди"""
        body = {
            "tag": tag,
            "status": status
        }
        self.send_message(body)


if __name__ == '__main__':
    box = ControlBoxTest(1)
    tag, status = input().split()
    tag = int(tag)
    while tag > 0:
        box.send_tag_status(tag, status)
        tag, status = input().split()
        tag = int(tag)
