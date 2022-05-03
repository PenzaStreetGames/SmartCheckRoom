import datetime
import time

from sqlalchemy import select
from sqlalchemy.orm import Session
from sqlalchemy import create_engine

from server.database.tables import Base, Hook, Hanger, Nfc, ControlBox, Tag, TagState, ControlQueue


class CreateService:

    @staticmethod
    def create(entity, classname):
        # with Session(engine) as session:
        session.add(entity)
        session.commit()
        return SimpleSelectService.select_by_id(entity.id, classname)


class DeleteService:

    @staticmethod
    def delete(entity):
        # with Session(engine) as session:
        session.delete(entity)
        session.commit()

    @staticmethod
    def delete_by_id(id, classname):
        entity = SimpleSelectService.select_by_id(id, classname)
        DeleteService.delete(entity)


class SimpleSelectService:

    @staticmethod
    def select_by_id(id, classname):
        # with Session(engine) as session:
        request = select(classname).where(classname.id == id)
        entity = session.scalar(request)
        session.commit()
        return entity


class HookRepository:

    @staticmethod
    def create(id=0, hanger_id=0):
        hook = Hook(id=id, hanger_id=hanger_id)
        return CreateService.create(hook, Hook)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Hook)

    @staticmethod
    def delete_by_id(id=0):
        DeleteService.delete(HookRepository.select_by_id(id))


class HangerRepository:

    @staticmethod
    def create(id=0, control_id=0):
        hanger = Hanger(id=id, control_id=control_id)
        return CreateService.create(hanger, Hanger)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Hanger)


class NfcRepository:

    @staticmethod
    def create(id=0, control_id=0):
        nfc = Nfc(id=id, control_id=control_id)
        return CreateService.create(nfc, Nfc)

    @staticmethod
    def select_by_id(id=0) -> Nfc:
        return SimpleSelectService.select_by_id(id, Nfc)


class ControlBoxRepository:

    @staticmethod
    def create(id=0):
        control_box = ControlBox(id=id)
        return CreateService.create(control_box, ControlBox)

    @staticmethod
    def select_by_id(id=0) -> ControlBox:
        return SimpleSelectService.select_by_id(id, ControlBox)


class TagRepository:

    @staticmethod
    def create(id=0, state="") -> Tag:
        tag = Tag(id=id, state=state)
        return CreateService.create(tag, Tag)

    @staticmethod
    def select_by_id(id=0) -> Tag:
        return SimpleSelectService.select_by_id(id, Tag)


class TagStateRepository:

    @staticmethod
    def create(id=0, tag_id=0, state="push", event_time=datetime.datetime.utcnow()) -> TagState:
        tag_state = TagState(tag_id=tag_id, state=state, event_time=event_time)
        return CreateService.create(tag_state, TagState)

    @staticmethod
    def select_by_id(id=0) -> TagState:
        return SimpleSelectService.select_by_id(id, TagState)


class ControlQueueRepository:

    @staticmethod
    def create(id=0, control_id=0, direction="in", position=0, tag_id=0) -> ControlQueue:
        control_queue = ControlQueue(id=id, control_id=control_id,
                                     direction=direction, position=position, tag_id=tag_id)
        return CreateService.create(control_queue, ControlQueue)

    @staticmethod
    def select_by_id(id=0) -> ControlQueue:
        return SimpleSelectService.select_by_id(id, ControlQueue)


def session_commit():
    session.commit()


engine = create_engine("sqlite:///database/db.db", echo=True, future=True)
Base.metadata.create_all(engine)
session = Session(engine)

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
