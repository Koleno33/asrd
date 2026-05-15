from __future__ import annotations
from typing import TYPE_CHECKING
from typing import List, Dict
from enum import Enum
from objects_module import Vector3, Color, Room, Wall, SurfaceType, Object
from dataclasses import dataclass, field
import random
import math


@dataclass
class ConstraintEvaluation:
    """
    - feasible       (bool) : все ли hard-ограничения выполнены
    - violations     (list) : список словарей с деталями нарушений
    - total_violation(float): суммарная мера нарушения (для сравнения недопустимых особей)
    """
    feasible: bool = True
    violations: List[Dict] = field(default_factory=list)
    total_violation: float = 0.0


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
    NUMBER_TOLERANCE = 0.02  # for `min_distance` rule
    EPSILON = 1e-5           # for `distance`     rule

    def __init__(self, objects: List, room):
        self.__rules = DocumentParser.parse_document("assets/docs/doc.txt")
        self.__objects = objects
        self.__walls = room.get_walls()


    def _get_surface_type_from_str(self, s):
        wall_mapping = {
            "FLOOR": SurfaceType.FLOOR,
            "CEILING": SurfaceType.CEILING,
            "WALL_FRONT": SurfaceType.WALL_FRONT,
            "WALL_BACK": SurfaceType.WALL_BACK,
            "WALL_LEFT": SurfaceType.WALL_LEFT,
            "WALL_RIGHT": SurfaceType.WALL_RIGHT
        }
        return wall_mapping.get(s)


    def _get_objects_by_type_from_list(self, obj_list, type_str):
        """Фильтрует список объектов по строке типа или UserObject.internal_name"""
        # Известные типы (включая UserObject)
        if type_str in ("any", "cube", "sphere", "userobject"):
            if type_str == "any":
                return obj_list
            return [o for o in obj_list if o.get_type().lower() == type_str]
        # Если не совпало – ищем по internal_name
        return [o for o in obj_list 
                if hasattr(o, 'internal_name') and o.internal_name == type_str]

    
    def validate(self):
        result = list()
        for i, rule in enumerate(self.__rules):
            vr = self.validate_rule(rule)
            result += vr

        for i in range(len(self.__objects)):
            for j in range(i + 1, len(self.__objects)):
                obja = self.__objects[i]
                objb = self.__objects[j]
                if obja.calculate_distance(objb) < self.EPSILON:   # касание или пересечение
                    result.append(ValidationResult(
                        ValidationValue.INVALID,
                        {"type": "collision", "obja": obja.id, "objb": objb.id}
                    ))

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
                        wall = self._get_surface_type_from_str(typee)
                        continue
                    objs = self.get_objects(typee)
                    print(f"DEBUG: Found {len(objs)} objects for type/name '{typee}'")

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
                    if abs(dist - rule.value) < self.EPSILON:
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

    def evaluate_constraints(self, objects, room):
        """
        Быстрая проверка ограничений без изменения объектов.
        Возвращает ConstraintEvaluation с полями:
        """
        evaluation = ConstraintEvaluation()
        walls = self.__walls  # уже кэшированы в __init__

        # === Проверка min_distance ===
        for rule in self.__rules:
            if rule.name == "min_distance":
                target_objects = self._get_objects_by_type_from_list(objects, rule.object)
                req = rule.value
                for i in range(len(target_objects)):
                    for j in range(i + 1, len(target_objects)):
                        obja, objb = target_objects[i], target_objects[j]
                        dist = obja.calculate_distance(objb)
                        if dist < req - self.EPSILON:   # с учётом численной погрешности
                            evaluation.feasible = False
                            violation = {
                                "rule": "min_distance",
                                "obj_a": obja.id,
                                "obj_b": objb.id,
                                "required": req,
                                "current": dist
                            }
                            evaluation.violations.append(violation)
                            # Штраф пропорционален относительному нарушению
                            evaluation.total_violation += (req - dist) / req if req > 0 else 1.0

        # === Проверка distance до стен ===
        for rule in self.__rules:
            if rule.name == "distance":
                # Парсим строку типа "cube,WALL_LEFT" или "any,FLOOR"
                parts = rule.object.split(',')
                if len(parts) != 2:
                    continue
                obj_type_str, wall_str = parts[0], parts[1]

                # Получаем стену
                wall_type = self._get_surface_type_from_str(wall_str)
                if wall_type is None:
                    continue
                wall = None
                for w in walls:
                    if w.get_type() == wall_type:
                        wall = w
                        break
                if wall is None:
                    continue

                # Объекты указанного типа
                target_objects = self._get_objects_by_type_from_list(objects, obj_type_str)
                req = rule.value

                for obj in target_objects:
                    dist = obj.calculate_distance_to_wall(wall)
                    if abs(dist - req) > 1e-5:
                        evaluation.feasible = False
                        violation = {
                            "rule": "distance",
                            "obj": obj.id,
                            "wall": wall_str,
                            "required": req,
                            "current": dist
                        }
                        evaluation.violations.append(violation)
                        evaluation.total_violation += abs(dist - req)

        # === Проверка нахождения внутри комнаты ===
        for obj in objects:
            if not room.is_obj_inside(obj):
                evaluation.feasible = False
                evaluation.violations.append({
                    "rule": "inside_room",
                    "obj": obj.id
                })
                # Штраф — большое число
                evaluation.total_violation += 1000.0

        # Проверка на коллизии для всех пар (расстояние < 0)
        n = len(objects)
        for i in range(n):
            for j in range(i + 1, n):
                obja = objects[i]
                objb = objects[j]
                dist = obja.calculate_distance(objb)
                if dist < -self.EPSILON:   # отрицательное = пересечение
                    evaluation.feasible = False
                    evaluation.violations.append({
                        "rule": "collision",
                        "obj_a": obja.id,
                        "obj_b": objb.id,
                        "current": dist
                    })
                    evaluation.total_violation += abs(dist)

        return evaluation

def validate(objs: List[Object], room: Room):
    v = Validator(objs, room)
    # list of dicts
    vrs = v.validate()
    return vrs

def arrange_objects(objs: List[Object], room: Room, method="genetic"):
    from arrangers.monte_carlo import MonteCarloArranger
    from arrangers.genetic import GeneticArranger

    if method == "monte_carlo":
        arranger = MonteCarloArranger(objs, room)
    elif method == "genetic":
        arranger = GeneticArranger(objs, room)
    else:
        raise ValueError(f"Unknown arrangement method: {method}")
    return arranger.arrange()

