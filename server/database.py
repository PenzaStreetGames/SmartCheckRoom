from sqlalchemy import Column, Integer, ForeignKey, String, Time
from sqlalchemy.orm import declarative_base, relationship
from sqlalchemy import create_engine


Base = declarative_base()


class Hook(Base):
    __tablename__ = "hook"

    id = Column(Integer, primary_key=True)
    hanger_id = Column(Integer, ForeignKey("hanger.id"))

    hanger = relationship("Hanger", back_populates="hooks")
    tag = relationship("Tag", back_populates="hook")


class Hanger(Base):
    __tablename__ = "hanger"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))

    hooks = relationship("Hook", back_populates="hanger")
    control_box = relationship("ControlBox", back_populates="hangers")


class ControlBox(Base):
    __tablename__ = "control_box"

    id = Column(Integer, primary_key=True)

    hangers = relationship("Hook", back_populates="control_box")
    nfcs = relationship("Nfc", back_populates="control_box")


class Nfc(Base):
    __tablename__ = "nfc"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))

    control_box = relationship("ControlBox", back_populates="nfcs")


class Tag(Base):
    __tablename__ = "tag"

    id = Column(Integer, primary_key=True)
    state = String(10)
    hook_id = Column(Integer, ForeignKey("hook.id"))

    hook = relationship("Hook", back_populates="tag")


class TagState(Base):
    __tablename__ = "tag_state"

    id = Column(Integer, primary_key=True)
    tag_id = Column(Integer, ForeignKey("tag_state.id"))
    state = String(10)
    event_time = Time()

    tag = relationship("Tag")


class ControlQueue(Base):
    __tablename__ = "control_queue"

    id = Column(Integer, primary_key=True)
    control_id = Column(Integer, ForeignKey("control_box.id"))
    direction = Column(String(4))
    position = Column(Integer)
    tag_id = Column(Integer, ForeignKey("tag.id"))

    control_box = relationship("Tag")
    tag = relationship("Tag")


if __name__ == '__main__':
    engine = create_engine("sqlite:///db.db", echo=True, future=True)
    Base.metadata.create_all(engine)
