import datetime
import traceback
from typing import List

from sqlalchemy import select
from sqlalchemy.orm import Session
from sqlalchemy import create_engine

from database.tables import Base, Hook, Hanger, Nfc, ControlBox, Tag, TagState, ControlQueue
from database.session import *


class CreateService:
    """
    Сервис, создающий сущности в базе данных
    """

    @staticmethod
    def create(entity, classname):
        """Метод создания сущностей"""
        # with Session(engine) as session:
        session.add(entity)
        session.commit()
        return SimpleSelectService.select_by_id(entity.id, classname)


class DeleteService:
    """
    Сервис, удаляющий сущности из базы данных
    """

    @staticmethod
    def delete(entity):
        """Метод удаления одной сущности"""
        # with Session(engine) as session:
        session.delete(entity)
        session.commit()

    @staticmethod
    def delete_by_id(id, classname):
        """Метод удаления сущности по id"""
        entity = SimpleSelectService.select_by_id(id, classname)
        DeleteService.delete(entity)

    @staticmethod
    def delete_all(entities):
        """Метод удаления всех сущностей, переданных в качестве параметра"""
        for entity in entities:
            session.delete(entity)
        session.commit()


class SimpleSelectService:
    """
    Сервис, занимающийся выборкой сущностей из базы данных
    """

    @staticmethod
    def select_by_id(id, classname):
        """Поиск сущности по id"""
        # with Session(engine) as session:
        request = select(classname).where(classname.id == id)
        entity = session.scalar(request)
        # session.commit()
        return entity

    @staticmethod
    def select_all(classname):
        """Метод получения всех сущностей данного класса"""
        request = select(classname)
        entities = session.scalars(request)
        return entities


class HookRepository:
    """
    Сервис, работающий с сущностями класса Крючок

    Методы сервиса обращаются к методам сервисов создания, удаления, выборки
    """

    @staticmethod
    def create(id=0, hanger_id=0):
        hook = Hook(id=id, hanger_id=hanger_id)
        return CreateService.create(hook, Hook)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Hook)

    @staticmethod
    def select_all():
        return SimpleSelectService.select_all(Hook)

    @staticmethod
    def delete_by_id(id=0):
        DeleteService.delete(HookRepository.select_by_id(id))


class HangerRepository:
    """
    Сервис, работающий с сущностями класса Вешалка

    Методы сервиса обращаются к методам сервисов создания, выборки
    """

    @staticmethod
    def create(id=0, control_id=0):
        hanger = Hanger(id=id, control_id=control_id)
        return CreateService.create(hanger, Hanger)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Hanger)

    @staticmethod
    def select_all():
        return SimpleSelectService.select_all(Hanger)


class NfcRepository:
    """
    Сервис, работающий с сущностями класса NFC-датчик

    Методы сервиса обращаются к методам сервисов создания, выборки
    """

    @staticmethod
    def create(id=0, control_id=0):
        nfc = Nfc(id=id, control_id=control_id)
        return CreateService.create(nfc, Nfc)

    @staticmethod
    def select_by_id(id=0) -> Nfc:
        return SimpleSelectService.select_by_id(id, Nfc)

    @staticmethod
    def select_all():
        return SimpleSelectService.select_all(Nfc)


class ControlBoxRepository:
    """
    Сервис, работающий с сущностями класса Пульт управления

    Методы сервиса обращаются к методам сервисов создания, выборки
    """

    @staticmethod
    def create(id=0):
        control_box = ControlBox(id=id)
        return CreateService.create(control_box, ControlBox)

    @staticmethod
    def select_by_id(id=0) -> ControlBox:
        return SimpleSelectService.select_by_id(id, ControlBox)

    @staticmethod
    def select_all():
        return SimpleSelectService.select_all(ControlBox)


class TagRepository:
    """
    Сервис, работающий с сущностями класса Тег

    Методы сервиса обращаются к методам сервисов создания, выборки
    """

    @staticmethod
    def create(id=0, state="") -> Tag:
        tag = Tag(id=id, state=state)
        return CreateService.create(tag, Tag)

    @staticmethod
    def select_by_id(id=0) -> Tag:
        return SimpleSelectService.select_by_id(id, Tag)

    @staticmethod
    def select_all() -> List[Tag]:
        return SimpleSelectService.select_all(Tag)


class TagStateRepository:
    """
    Сервис, работающий с сущностями класса Состояние тега

    Методы сервиса обращаются к методам сервисов создания, удаления, выборки
    """

    @staticmethod
    def create(tag_id=0, state="push", event_time=datetime.datetime.utcnow()) -> TagState:
        tag_state = TagState(tag_id=tag_id, state=state, event_time=event_time)
        return CreateService.create(tag_state, TagState)

    @staticmethod
    def select_by_id(id=0) -> TagState:
        return SimpleSelectService.select_by_id(id, TagState)

    @staticmethod
    def select_all() -> List[TagState]:
        return SimpleSelectService.select_all(TagState)


class ControlQueueRepository:
    """
    Сервис, работающий с сущностями класса Очередь на пульте

    Методы сервиса обращаются к методам сервисов создания, выборки
    """

    @staticmethod
    def create(id=0, control_id=0, direction="in", position=0, tag_id=0) -> ControlQueue:
        control_queue = ControlQueue(id=id, control_id=control_id,
                                     direction=direction, position=position, tag_id=tag_id)
        return CreateService.create(control_queue, ControlQueue)

    @staticmethod
    def select_by_id(id=0) -> ControlQueue:
        return SimpleSelectService.select_by_id(id, ControlQueue)

    @staticmethod
    def select_all() -> List[ControlQueue]:
        return SimpleSelectService.select_all(ControlQueue)


def session_commit():
    """Метод сохранения изменений в базе данных. Вызывается после завершения работы с сущностями в коде"""
    session.commit()


if __name__ == '__main__':
    pass
    # control_box = ControlBoxRepository.select_by_id(0)
    # print(control_box)
    # hanger = HangerRepository.select_by_id(0)
    # if hanger is not None:
    #     DeleteService.delete_by_id(0, Hanger)
    # hanger = HangerRepository.create(id=0)
    # hanger.control_box = control_box
    # session.commit()
    # print(hanger)
    # HookService.create(id=0, hanger_id=0)
    # hook = HookService.select_by_id(id=0)
    # print(hook)
