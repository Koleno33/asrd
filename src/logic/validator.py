from typing import List
from enum import Enum


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
    NUMBER_TOLERANCE = 0.15

    def __init__(self, objects: List):
        self.__rules = DocumentParser.parse_document("assets/docs/doc.txt")
        self.__objects = objects
    
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
        objs = self.get_objects(rule.object)

        # TODO: make rule.name match construction
        assert(rule.name == "min_distance")

        # Validation Results
        vrs = list()
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
                    current_status = ValidationValue.OK
                    continue
                current_attrs = {"type": rule.name, "obja": obja.id, "objb": objb.id}
                vrs.append(
                    ValidationResult(current_status, current_attrs)
                )
                print(f"Status {current_status} for entry \'{rule.name}\': excepted \'{rule.name} {rule.value:.2f}\', got {dist:.2f}. "
                      f"Objects: ({obja.get_type()}{obja.id}, {objb.get_type()}{objb.id})")

        return vrs


def validate(objs):
    v = Validator(objs)
    # list of dicts
    vrs = v.validate()
    return vrs

