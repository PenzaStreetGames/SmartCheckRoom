import datetime

from sqlalchemy import Column, Integer, ForeignKey, String, Time, DateTime
from sqlalchemy.orm import declarative_base, relationship, Session

Base = declarative_base()


class Hook(Base):
    __tablename__ = "hook"

    id = Column(Integer, primary_key=True)
    hanger_id = Column(Integer, ForeignKey("hanger.id"))

    hanger = relationship("Hanger", back_populates="hooks")
    tag = relationship("Tag", back_populates="hook")

    def __repr__(self) -> str:
        return f"Hook(id={self.id!r}, hanger_id={self.hanger_id!r})"


class Hanger(Base):
    __tablename__ = "hanger"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))

    hooks = relationship("Hook", back_populates="hanger")
    control_box = relationship("ControlBox", back_populates="hangers")

    def __repr__(self) -> str:
        return f"Hanger(id={self.id!r}, control_id={self.control_id!r})"


class ControlBox(Base):
    __tablename__ = "control_box"

    id = Column(Integer, primary_key=True)

    hangers = relationship("Hanger", back_populates="control_box")
    nfcs = relationship("Nfc", back_populates="control_box")
    queues = relationship("ControlQueue", back_populates="control_box")

    def __repr__(self) -> str:
        return f"ControlBox(id={self.id!r})"


class Nfc(Base):
    __tablename__ = "nfc"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))

    control_box = relationship("ControlBox", back_populates="nfcs")

    def __repr__(self) -> str:
        return f"Nfc(id={self.id!r}, control_id={self.control_id!r})"


class Tag(Base):
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
    __tablename__ = "tag_state"

    id = Column(Integer, primary_key=True, autoincrement=True)
    tag_id = Column(Integer, ForeignKey("tag.id"))
    state = Column(String(10))
    event_time = Column(DateTime(), default=datetime.datetime.utcnow)

    tag = relationship("Tag", back_populates="states")

    def __repr__(self) -> str:
        return f"Tag(id={self.id!r}, tag_id={self.tag_id!r}, state={self.state!r}, event_time={self.event_time!r})"


class ControlQueue(Base):
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
