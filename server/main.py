from mqtt_handlers.server_handlers.all_topics_handler import AllTopicsHandler
from mqtt_handlers.server_handlers.handlers import ControlBoxHandler
import time

if __name__ == '__main__':
    all_topics_handler = AllTopicsHandler()
    control_box_handler = ControlBoxHandler()
    while True:
        pass
        # control_box_handler.send_tag_status(345, 123456, "push")
        # time.sleep(30)
        # control_box_handler.send_tag_status(345, 123456, "pushed")
        # time.sleep(30)
        # control_box_handler.send_tag_status(345, 123456, "pull")
        # time.sleep(30)
        # control_box_handler.send_tag_status(345, 123456, "pulled")
        # time.sleep(30)

