from mqtt_handlers.server_handlers.all_topics_handler import AllTopicsHandler
from mqtt_handlers.server_handlers.handlers import ControlBoxHandler, HooksHandler
import time

if __name__ == '__main__':
    all_topics_handler = AllTopicsHandler()
    control_box_handler = ControlBoxHandler()
    hooks_handler = HooksHandler()
    while True:
        pass
        # for i in range(1, 33):
        #     hooks_handler.send_hook_id_status(1, hook_id=i, status="push", color="red")
        #     time.sleep(5)
        #     hooks_handler.send_hook_id_status(1, hook_id=i, status="pushed", color="red")
        #     time.sleep(5)
        #     pass
        # control_box_handler.send_tag_status(345, 123456, "push")
        # time.sleep(30)
        # control_box_handler.send_tag_status(345, 123456, "pushed")
        # time.sleep(30)
        # control_box_handler.send_tag_status(345, 123456, "pull")
        # time.sleep(30)
        # control_box_handler.send_tag_status(345, 123456, "pulled")
        # time.sleep(30)

