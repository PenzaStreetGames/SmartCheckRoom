import paho.mqtt.client as mqtt
from database.database import TagService, TagStateService
from json import loads, dumps
from settings import MQTT_HOST, MQTT_PORT
from mqtt_handlers.server_handlers.handlers import ControlBoxHandler, HooksHandler, NfcHandler


def from_nfc_to_server(msg):
    topic, payload = msg.topic, msg.payload


def from_control_to_server(msg):
    topic, payload = msg.topic, msg.payload
    payload_parsed = loads(payload)
    parsed_body = payload_parsed["body"]
    tag = parsed_body["tag"]
    status = parsed_body["status"]
    if status == "pushed":
        print("pushed")
    elif status == "pulled":
        print("pulled")
    else:
        print(f"unknown status {status}")


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("#")


def on_message(client, userdata, msg):
    topic: str = msg.topic
    if topic.startswith("/server/nfc/"):
        from_nfc_to_server(msg)
    elif topic.startswith("/server/control"):
        from_control_to_server(msg)
    print(msg.topic + " " + str(msg.payload))


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_HOST, MQTT_PORT, 30)

client.loop_start()
topic = input("topic: ")
msg = input("message: ")
while topic != "exit":
    client.publish(topic, payload=msg)
    topic = input("topic: ")
    msg = input("message: ")
