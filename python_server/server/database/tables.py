import datetime

from sqlalchemy import Column, Integer, ForeignKey, String, Time, DateTime
from sqlalchemy.orm import declarative_base, relationship, Session

Base = declarative_base()


class Hook(Base):
    """
    Крючок - место в гардеробе, куда вешается одежда.

    Имеет номер, жёстко привязан к вешалке, на которой расположен
    """

    __tablename__ = "hook"

    id = Column(Integer, primary_key=True)
    hanger_id = Column(Integer, ForeignKey("hanger.id"))

    hanger = relationship("Hanger", back_populates="hooks")
    tag = relationship("Tag", back_populates="hook")

    def __repr__(self) -> str:
        return f"Hook(id={self.id!r}, hanger_id={self.hanger_id!r})"


class Hanger(Base):
    """
    Вешалка - пролёт в гардеробе, ряд крючков, обслуживаемых одной RGB-лентой

    Имеет номер, жёстко привязана к пульту гардеробщика, который её обслуживает
    """
    __tablename__ = "hanger"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))

    hooks = relationship("Hook", back_populates="hanger")
    control_box = relationship("ControlBox", back_populates="hangers")

    def __repr__(self) -> str:
        return f"Hanger(id={self.id!r}, control_id={self.control_id!r})"


class ControlBox(Base):
    """
    Пульт гардеробщика - устройство с подсвеченной клавиатурой, с помощью которого гардеробщик подтверждает факт
    приёма и выдачи одежды.

    Имеет только номер
    """
    __tablename__ = "control_box"

    id = Column(Integer, primary_key=True)

    hangers = relationship("Hanger", back_populates="control_box")
    nfcs = relationship("Nfc", back_populates="control_box")
    queues = relationship("ControlQueue", back_populates="control_box")

    def __repr__(self) -> str:
        return f"ControlBox(id={self.id!r})"


class Nfc(Base):
    """
    Nfc-датчик - устройство, считывающее nfc-метки пользователей и передающее данные о метках на сервер

    Имеет номер, жёстко привязана к пульту гардеробщика, который его обслуживает
    """
    __tablename__ = "nfc"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))

    control_box = relationship("ControlBox", back_populates="nfcs")

    def __repr__(self) -> str:
        return f"Nfc(id={self.id!r}, control_id={self.control_id!r})"


class Tag(Base):
    """
    Тег - значение, считанное с nfc-метки пользователя

    Имеет само значение тега и состояние, описывающее текущее положение вещи в гардеробе
    (запрошена сдача, сдана, запрошена выдача, выдана)

    Имеет привязку к крючку, на который эта вещь вешается.
    """
    __tablename__ = "tag"

    id = Column(Integer, primary_key=True)
    state = Column(String(10))
    hook_id = Column(Integer, ForeignKey("hook.id"))

    hook = relationship("Hook", back_populates="tag")
    states = relationship("TagState", back_populates="tag")
    queues = relationship("ControlQueue", back_populates="tag")

    def __repr__(self) -> str:
        return f"Tag(id={self.id!r}, state={self.state!r}, hook_id={self.hook_id})"


class TagState(Base):
    """
    Состояние тега - запись в базе данных, отражающая состояние тега в конкретный момент времени (прошлого, настоящего)

    Используется для логирования и отслеживания вещей в гардеробе

    Имеет поля: номер тега, его состояние и время, в которое это состояние было зафиксировано
    """
    __tablename__ = "tag_state"

    id = Column(Integer, primary_key=True, autoincrement=True)
    tag_id = Column(Integer, ForeignKey("tag.id"))
    state = Column(String(10))
    event_time = Column(DateTime(), default=datetime.datetime.utcnow)

    tag = relationship("Tag", back_populates="states")

    def __repr__(self) -> str:
        return f"Tag(id={self.id!r}, tag_id={self.tag_id!r}, state={self.state!r}, event_time={self.event_time!r})"


class ControlQueue(Base):
    """
    Очередь - поток вещей на сдачу или на выдачу, проходящий через пульт гардеробщика

    Имеет номер, направление (сдача, выдача), позицию.

    Имеет жёсткую привязку к пульту, в котором она существует. Также хранит ссылку на тег, который в ней обрабатывается
    """
    __tablename__ = "control_queue"

    id = Column(Integer, primary_key=True, autoincrement=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))
    direction = Column(String(4))
    position = Column(Integer)
    tag_id = Column(Integer, ForeignKey("tag.id"))

    control_box = relationship("ControlBox", back_populates="queues")
    tag = relationship("Tag", back_populates="queues")

    def __repr__(self) -> str:
        return f"ControlQueue(id={self.id!r}, control_id={self.control_id!r}, direction={self.direction}, " \
               f"position={self.position}, tag_id={self.tag_id})"
