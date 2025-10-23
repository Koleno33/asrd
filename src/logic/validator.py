from __future__ import annotations
from typing import TYPE_CHECKING
from typing import List
from enum import Enum
from objects_module import Vector3, Color, Room, Wall, SurfaceType


class Rule:
    def __init__(self, name: str, object: str, value: str):
        self.name = name
        self.object = object

        try:
            self.value = float(value)
        except:
            self.value = value


class DocumentParser:
    # words count per line in document
    WORDS_COUNT = 3

    @classmethod
    def parse_document(cls, filename):
        res = list()
        with open(filename, "r") as file:
            lines = file.readlines()
        for line in lines:
            words = line.split()
            if len(words) == cls.WORDS_COUNT:
                rule = Rule(
                    words[cls.WORDS_COUNT - 3],
                    words[cls.WORDS_COUNT - 2],
                    words[cls.WORDS_COUNT - 1],
                )
                res.append(rule)
        return res


class ValidationValue(Enum):
    OK = "ok"
    VIOLATED = "violated"
    INVALID = "invalid"


class ValidationResult:
    def __init__(self, value: ValidationValue, attrs: dict = dict()):
        self.value = value
        self.attrs = attrs

    def to_dict(self):
        return {
            'value': self.value.value,
            'attrs': self.attrs,
        }


class Validator:
    NUMBER_TOLERANCE = 0.02

    def __init__(self, objects: List, room):
        self.__rules = DocumentParser.parse_document("assets/docs/doc.txt")
        self.__objects = objects
        self.__walls = room.get_walls()
    
    def validate(self):
        result = list()
        for i, rule in enumerate(self.__rules):
            vr = self.validate_rule(rule)
            result += vr

        # sorting ValidationResults from good to bad 
        order = list(ValidationValue)
        index_map = { v: i for i, v in enumerate(order) }
        result.sort(key=lambda it: index_map[it.value])

        return [vr.to_dict() for vr in result]
    
    def get_objects(self, type: str):
        match type:
            case "any":
                return self.__objects
            case "cube":
                return [ o for o in self.__objects if o.get_type() == "Cube" ]
            case "sphere":
                return [ o for o in self.__objects if o.get_type() == "Sphere" ]
        return list()

    def validate_rule(self, rule):
        def is_wall(s):
            wall_names = [e.name for e in [SurfaceType.FLOOR, SurfaceType.CEILING, 
                                          SurfaceType.WALL_FRONT, SurfaceType.WALL_BACK,
                                          SurfaceType.WALL_LEFT, SurfaceType.WALL_RIGHT]]
            return s in wall_names

        def get_wall(surface_type):
            for w in self.__walls:
                if w.get_type() == surface_type:
                    return w
            return None

        def get_surface_type(s):
            wall_mapping = {
                "FLOOR": SurfaceType.FLOOR,
                "CEILING": SurfaceType.CEILING,
                "WALL_FRONT": SurfaceType.WALL_FRONT,
                "WALL_BACK": SurfaceType.WALL_BACK,
                "WALL_LEFT": SurfaceType.WALL_LEFT,
                "WALL_RIGHT": SurfaceType.WALL_RIGHT
            }
            return wall_mapping.get(s)

        objs = self.get_objects(rule.object)

        # Validation Results
        vrs = list()

        match rule.name:
            case "min_distance":
                for i in range(len(objs)):
                    for j in range(i, len(objs)):
                        if i == j: continue
                        obja = objs[i]
                        objb = objs[j]
                        dist = obja.calculate_distance(objb)
                        # TODO: make rule.value str comparison
                        assert(float(rule.value))
                        if dist < rule.value:
                            current_status = ValidationValue.INVALID
                        elif dist < rule.value * (1 + self.NUMBER_TOLERANCE):
                            current_status = ValidationValue.VIOLATED
                        else:
                            continue
                        current_attrs = {"type": rule.name, "obja": obja.id, "objb": objb.id}
                        vrs.append(
                            ValidationResult(current_status, current_attrs)
                        )
                        print(f"Status {current_status} for entry \'{rule.name}\': excepted "
                              f"\'{rule.name} {rule.value:.2f}\', got {dist:.2f}. "
                              f"Objects: ({obja.get_type()}{obja.id}, {objb.get_type()}{objb.id})")
            case "distance":
                wall = None 
                objs_types = rule.object.strip().split(',')

                for typee in objs_types:
                    if is_wall(typee):
                        wall = get_surface_type(typee)
                        continue
                    objs = self.get_objects(typee)

                if wall is None:
                    print(f"ERROR: No wall type found in rule: {rule.object}")

                # TODO: make pretty "Unknown Wall" error printing
                assert(wall is not None)
                wall_obj = get_wall(wall)

                if wall_obj is None:
                    return []

                if wall_obj is None:
                    print(f"ERROR: Room doesn't have wall of type: {wall}")

                for obj in objs:
                    dist = obj.calculate_distance_to_wall(wall_obj)
                    if dist == rule.value:
                        continue
                    else:
                        current_status = ValidationValue.INVALID

                    current_attrs = {"type": rule.name, "obja": obj.id, "objb": wall_obj.get_type().name}
                    vrs.append(
                        ValidationResult(current_status, current_attrs)
                    )
                    print(f"Status {current_status} for entry \'{rule.name}\': excepted "
                          f"\'{rule.name} {rule.value:.2f}\', got {dist:.2f}. "
                          f"Objects: ({obj.get_type()}{obj.id}, {wall_obj.get_type().name})")

        return vrs


def validate(objs: List, room: Room):
    v = Validator(objs, room)
    # list of dicts
    vrs = v.validate()
    return vrs

