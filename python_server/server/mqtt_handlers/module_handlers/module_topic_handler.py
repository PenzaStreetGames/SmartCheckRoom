import paho.mqtt.client as mqtt
from json import dumps, loads
from server.settings import MQTT_HOST, MQTT_PORT


class ModuleTopicHandler:

    def __init__(self, module, id, topic_in=None, topic_out=None):
        self.module = module
        self.id = id
        self.topic_in = topic_in
        self.topic_out = topic_out

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.connect(MQTT_HOST, MQTT_PORT, 30)
        self.client.loop_start()

    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code " + str(rc))
        client.subscribe(self.topic_in)

    def on_message(self, client, userdata, msg):
        print(msg.topic + "\n" + str(msg.payload))

    def send_message(self, body):
        message = {
            "from": {
                "type": self.module,
                "id": self.id
            },
            "to": {
                "type": "server",
                "id": 1
            },
            "topic": self.topic_out,
            "body": body
        }
        self.client.publish(topic=self.topic_out, payload=dumps(message))

