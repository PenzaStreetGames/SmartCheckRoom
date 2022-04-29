import time

from sqlalchemy import select
from sqlalchemy.orm import Session
from sqlalchemy import create_engine

from database.tables import Base, Hook, Hanger, Nfc, ControlBox, Tag, TagState, ControlQueue


class CreateService:

    @staticmethod
    def create(entity):
        with Session(engine) as session:
            session.add(entity)
            session.commit()

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


class HookService:

    @staticmethod
    def create(id=0, hanger_id=0):
        hook = Hook(id=id, hanger_id=hanger_id)
        CreateService.create(hook)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Hook)

    @staticmethod
    def delete_by_id(id=0):
        DeleteService.delete(HookService.select_by_id(id))


class HangerService:

    @staticmethod
    def create(id=0, control_id=0):
        hanger = Hanger(id=id, control_id=control_id)
        CreateService.create(hanger)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Hanger)


class NfcService:

    @staticmethod
    def create(id=0, control_id=0):
        nfc = Nfc(id=id, control_id=control_id)
        CreateService.create(nfc)

    @staticmethod
    def select_by_id(id=0):
        return SimpleSelectService.select_by_id(id, Nfc)


class ControlBoxService:

    @staticmethod
    def create(id=0):
        control_box = ControlBox(id=id)
        CreateService.create(control_box)

    @staticmethod
    def select_by_id(id=0) -> ControlBox:
        return SimpleSelectService.select_by_id(id, ControlBox)


class TagService:

    @staticmethod
    def create(id=0, control_id=0):
        tag = Tag(id=id, control_id=control_id)
        CreateService.create(tag)

    @staticmethod
    def select_by_id(id=0) -> Tag:
        return SimpleSelectService.select_by_id(id, Tag)


class TagStateService:

    @staticmethod
    def create(id=0, tag_id=0, status="push", event_time=time.localtime()):
        tag_state = TagState(id=id, tag_id=tag_id, status=status, event_time=event_time)
        CreateService.create(tag_state)

    @staticmethod
    def select_by_id(id=0) -> TagState:
        return SimpleSelectService.select_by_id(id, TagState)


class ControlQueueService:

    @staticmethod
    def create(id=0, control_id=0, direction="in", position=0, tag_id=0):
        control_queue = ControlQueue(id=id, control_id=control_id,
                                     direction=direction, position=position, tag_id=tag_id)
        CreateService.create(control_queue)

    @staticmethod
    def select_by_id(id=0) -> ControlQueue:
        return SimpleSelectService.select_by_id(id, ControlQueue)


if __name__ == '__main__':
    engine = create_engine("sqlite:///db.db", echo=True, future=True)
    Base.metadata.create_all(engine)
    with Session(engine) as session:
        control_box = ControlBoxService.select_by_id(0)
        print(control_box)
        hanger = HangerService.select_by_id(0)
        if hanger is None:
            DeleteService.delete_by_id(0, Hanger)
            HangerService.create(id=0)
            hanger = HangerService.select_by_id(0)
        hanger.control_box = control_box
        session.commit()
        print(hanger)
    # HookService.create(id=0, hanger_id=0)
    # hook = HookService.select_by_id(id=0)
    # print(hook)
