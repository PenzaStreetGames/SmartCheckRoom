from database.repositories import NfcRepository, TagRepository, TagStateRepository, ControlBoxRepository, \
    session_commit
from mqtt_handlers.server_handlers.handlers import NfcHandler, ControlBoxHandler, HooksHandler
from database.tables import *
from json import dumps, loads
from typing import Optional


class Service:
    """
    Сервис - набор операций над сущностями на уровне бизнес-логики
    """

    colors = {
        0: "black",
        1: "blue",
        2: "red"
    }

    @staticmethod
    def check_tag_exists(tag_id) -> Optional[Tag]:
        """Проверка тега на существование. Возвращает тег или None"""
        tag_entity = TagRepository.select_by_id(tag_id)
        return tag_entity

    @staticmethod
    def get_free_queue(control_box: ControlBox, direction="in") -> ControlQueue:
        """Получение свободного места в очереди. Возвращает место в очереди или None"""
        queues = control_box.queues
        res = [queue for queue in queues if queue.direction == direction and queue.tag is None]
        return res[0] if res else None

    @staticmethod
    def get_free_hook(control_box: ControlBox) -> Optional[Hook]:
        """Получение свободного крючка на вешалке. Возвращает крючок или None"""
        hangers = control_box.hangers
        for hanger in hangers:
            hooks = hanger.hooks
            for hook in hooks:
                tag = hook.tag
                if not tag:
                    return hook
        return None

    @staticmethod
    def get_control_box_by_hook(hook: Hook) -> ControlBox:
        """Получение пульта, обслуживающего крючок"""
        hanger = hook.hanger
        control_box = hanger.control_box
        return control_box

    @staticmethod
    def log_tag_state(tag: Tag) -> None:
        """Запись в базу данных состояния тега в определённый момент времени"""
        tag_state = TagStateRepository.create(state=tag.state)
        tag_state.tag = tag

    @staticmethod
    def get_queue_with_tag(control_box: ControlBox, tag: Tag) -> Optional[ControlQueue]:
        """Получение очереди на пульте, которая содержит указанный тег. Возвращает очередь или None"""
        queues = control_box.queues
        for queue in queues:
            tag_in_queue: Tag = queue.tag
            if tag_in_queue is None:
                continue
            if tag_in_queue.id == tag.id:
                return queue
        return None


class NfcService(Service):
    """
    Сервис, обрабатывающий сообщения с NFC-датчика
    """

    def __init__(self):
        self.nfc_handler = NfcHandler()
        self.control_box_handler = ControlBoxHandler()
        self.hooks_handler = HooksHandler()

    def handle_request(self, msg):
        """Обработка сообщения, пришедшего от nfc-датчика"""
        topic, payload = msg.topic, msg.payload
        payload_parsed = loads(payload)
        parsed_body = payload_parsed["body"]
        tag = parsed_body["tag"]
        nfc_id = payload_parsed["from"]["id"]

        nfc_entity: Nfc = NfcRepository.select_by_id(nfc_id)
        control_box_entity: ControlBox = nfc_entity.control_box
        tag_entity = self.check_tag_exists(tag)
        if tag_entity is None or tag_entity.state == "pulled":
            free_queue = self.get_free_queue(control_box_entity, direction="in")
            if free_queue is None:
                self.nfc_handler.send_tag_status(nfc_id, tag, "blocked", reason="stuck_queue")
                return

            hook_entity = self.get_free_hook(control_box_entity)
            if hook_entity is None:
                self.nfc_handler.send_tag_status(nfc_id, tag, "blocked", reason="stuck hooks")
                return

            if tag_entity is None:
                tag_entity = TagRepository.create(tag)
            tag_entity.state = "push"
            tag_entity.hook = hook_entity
            self.log_tag_state(tag_entity)
            free_queue.tag = tag_entity
            session_commit()

            color = self.colors[free_queue.position]
            self.send_success(control_id=control_box_entity.id, hanger_id=hook_entity.hanger.id,
                              nfc_id=nfc_entity.id, hook_id=hook_entity.id, tag=tag, status="push", color=color)
        else:
            if tag_entity.state != "pushed":
                self.nfc_handler.send_tag_status(nfc_id, tag, "blocked", reason="tag not pushed")
                return

            hook_entity = tag_entity.hook
            also_control_box_entity = self.get_control_box_by_hook(hook_entity)
            if control_box_entity != also_control_box_entity:
                self.nfc_handler.send_tag_status(nfc_id, tag, "blocked", reason="wrong control box")
                return

            free_queue = self.get_free_queue(control_box_entity, direction="out")
            if free_queue is None:
                self.nfc_handler.send_tag_status(nfc_id, tag, "blocked", reason="stuck queue")
                return

            tag_entity.state = "pull"
            self.log_tag_state(tag_entity)
            free_queue.tag = tag_entity
            session_commit()

            color = self.colors[free_queue.position]
            self.send_success(control_id=control_box_entity.id, hanger_id=hook_entity.hanger.id,
                              nfc_id=nfc_entity.id, hook_id=hook_entity.id, tag=tag, status="pull", color=color)

    def send_success(self, control_id, hanger_id, nfc_id, hook_id, tag, status, color):
        """Отправка сообщений устройствам об успешной обработке метки"""
        self.control_box_handler.send_tag_status(id=control_id, tag=tag, status=status)
        self.hooks_handler.send_hook_id_status(id=hanger_id, hook_id=hook_id, status=status, color=color)
        self.nfc_handler.send_tag_status(id=nfc_id, tag=tag, status=status)


class ControlBoxService(Service):
    """
    Сервис, обрабатывающий сообщения с пульта гардеробщика
    """

    def __init__(self):
        self.nfc_handler = NfcHandler()
        self.hooks_handler = HooksHandler()
        self.control_box_handler = ControlBoxHandler()

    def handle_request(self, msg):
        """Обработка сообщения, пришедшего от пульта гардеробщика"""
        topic, payload = msg.topic, msg.payload
        payload_parsed = loads(payload)
        parsed_body = payload_parsed["body"]
        tag = parsed_body["tag"]
        status = parsed_body["status"]
        tag_entity = TagRepository.select_by_id(id=tag)
        hook_entity = tag_entity.hook
        control_box_entity = self.get_control_box_by_hook(hook_entity)
        if status == "pushed":
            if tag_entity.state != "push":
                return

            tag_entity.state = "pushed"
            hook_entity.tag = [tag_entity]
            queue_entity = self.get_queue_with_tag(control_box_entity, tag_entity)
            queue_entity.tag = None
            NfcService.log_tag_state(tag=tag_entity)
            session_commit()

            color = self.colors[0]
            self.send_success(control_id=control_box_entity.id, hanger_id=hook_entity.hanger.id, hook_id=hook_entity.id,
                              tag=tag, status="pushed", color=color)
        elif status == "pulled":
            if tag_entity.state != "pull":
                print("state not pull")
                return

            tag_entity.state = "pulled"
            queue_entity = self.get_queue_with_tag(control_box_entity, tag_entity)
            queue_entity.tag = None
            hook_entity.tag = []
            tag_entity.hook = None
            NfcService.log_tag_state(tag=tag_entity)
            session_commit()

            color = self.colors[0]
            self.send_success(control_id=control_box_entity.id, hanger_id=hook_entity.hanger.id, hook_id=hook_entity.id,
                              tag=tag, status="pulled", color=color)
        else:
            print(f"unknown status {status}")

    def send_success(self, control_id, hanger_id, hook_id, tag, status, color):
        """Отправка сообщений об успешной обработке сигнала с пульта"""
        self.control_box_handler.send_tag_status(id=control_id, tag=tag, status=status)
        self.hooks_handler.send_hook_id_status(id=hanger_id, hook_id=hook_id, status=status, color=color)
