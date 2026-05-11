from abc import ABC, abstractmethod
from typing import Dict
from objects_module import Vector3
from validator import DocumentParser, Validator

class BaseArranger(ABC):
    def __init__(self, objects, room):
        self._objects = objects
        self._room = room
        self._walls = room.get_walls()
        self._wall_cache = {w.get_type(): w for w in self._walls}
        self._rules = DocumentParser.parse_document("assets/docs/doc.txt")
        self.validator = Validator(objects, room)

    @abstractmethod
    def arrange(self) -> Dict[int, Vector3]:
        """Запускает компоновку и возвращает словарь {id: позиция}"""
        pass
