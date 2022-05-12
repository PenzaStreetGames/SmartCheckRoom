import traceback

import paho.mqtt.client as mqtt
from server.mqtt_handlers.server_handlers.handlers import HooksHandler, NfcHandler, ControlBoxHandler
from server.settings import MQTT_HOST, MQTT_PORT
from server.services.services import NfcService, ControlBoxService
from datetime import datetime


class AllTopicsHandler:
    """
    Обработчик, подписанный на все топики MQTT-сервера.

    Выполняет распределительную роль.
    """

    def __init__(self):

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.connect(MQTT_HOST, MQTT_PORT, 30)
        self.client.loop_start()

        self.hooks_handler = HooksHandler()
        self.nfc_handler = NfcHandler()
        self.control_box_handler = ControlBoxHandler()
        self.nfc_service = NfcService()
        self.control_box_service = ControlBoxService()

    def on_connect(self, client, userdata, flags, rc):
        """Подписка на все топики после успешного подключения к MQTT-серверу"""
        print("Connected with result code " + str(rc))
        self.client.subscribe("#")

    def on_message(self, client, userdata, msg):
        """Обработка нового сообщения. Обрабатываются только сообщения, адресованные серверу"""
        topic: str = msg.topic
        try:
            if topic.startswith("/server/nfc/"):
                self.nfc_service.handle_request(msg)
            elif topic.startswith("/server/control/"):
                self.control_box_service.handle_request(msg)
        except Exception as ex:
            print(traceback.format_exc())
        print(str(datetime.utcnow()) + " " + msg.topic + " " + str(msg.payload))


