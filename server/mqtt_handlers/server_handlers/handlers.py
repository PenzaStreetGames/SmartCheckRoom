import paho.mqtt.client as mqtt
from json import dumps, loads
from server.settings import MQTT_HOST, MQTT_PORT


class ServerTopicHandler:

    def __init__(self, modules_type=None, topics_out=None):
        self.modules_type = modules_type
        self.topics_out = topics_out

        self.client = mqtt.Client()
        self.client.connect(MQTT_HOST, MQTT_PORT, 30)
        self.client.loop_start()

    def send_message(self, id, body):
        topic = f"{self.topics_out}/{id}"
        message = {
            "from": {
                "type": "server",
                "id": 1
            },
            "to": {
                "type": self.modules_type,
                "id": id
            },
            "topic": topic,
            "body": body
        }
        self.client.publish(topic=topic, payload=dumps(message))


class ControlBoxHandler(ServerTopicHandler):

    def __init__(self):
        modules_type = "control"
        topics_out = f"/{modules_type}"
        super().__init__(modules_type=modules_type, topics_out=topics_out)

    def send_tag_status(self, id, tag, status):
        body = {
            "tag": tag,
            "status": status
        }
        self.send_message(id, body)


class NfcHandler(ServerTopicHandler):

    def __init__(self):
        modules_type = "nfc"
        topics_out = f"/{modules_type}"
        super().__init__(modules_type=modules_type, topics_out=topics_out)

    def send_tag_status(self, id, tag, status):
        body = {
            "tag": tag,
            "status": status
        }
        self.send_message(id, body)


class HooksHandler(ServerTopicHandler):

    def __init__(self):
        modules_type = "hooks"
        topics_out = f"/{modules_type}"
        super().__init__(modules_type=modules_type, topics_out=topics_out)

    def send_hook_id_status(self, id, hook_id, status):
        body = {
            "hook_id": hook_id,
            "status": status
        }
        self.send_message(id, body)

