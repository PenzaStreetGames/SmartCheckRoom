from server.mqtt_handlers.module_handlers.module_topic_handler import ModuleTopicHandler


class NfcTest(ModuleTopicHandler):

    def __init__(self, id):
        topic_in = f"/nfc/{id}"
        topic_out = f"/server/nfc/{id}"
        super().__init__("nfc", id, topic_in, topic_out)

    def send_tag(self, tag):
        body = {
            "tag": tag
        }
        self.send_message(body)


if __name__ == '__main__':
    nfc = NfcTest(1)
    tag = int(input())
    while tag > 0:
        nfc.send_tag(tag)
        tag = int(input())
