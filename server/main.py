import paho.mqtt.client as mqtt


def from_nfc_to_server(msg):
    topic, payload = msg.topic, msg.payload



def from_control_to_server(msg):
    pass


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("#")
    client.publish("my_topic", payload="Hello, how are you?")


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
client.connect("51.250.105.61", 1884, 30)

client.loop_start()
topic = input("topic: ")
msg = input("message: ")
while topic != "exit":
    client.publish(topic, payload=msg)
    topic = input("topic: ")
    msg = input("message: ")
