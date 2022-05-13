from sqlalchemy.orm import Session
from sqlalchemy import create_engine
from database.tables import Base

engine = create_engine(f"sqlite:///database/db.db", echo=False, future=True)
Base.metadata.create_all(engine)
session = Session(engine)
