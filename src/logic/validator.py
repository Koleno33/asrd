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
        result = None
        for i, rule in enumerate(self.__rules):
            vr = self.validate_rule(rule)
            if vr.value != ValidationValue.OK:
                return vr
        return ValidationResult(ValidationValue.OK)
    
    def get_objects(self, type: str):
        match type:
            case "cube":
                return self.__objects
        return list()

    def validate_rule(self, rule):
        match rule.object:
            case "cube":
                objs = self.get_objects("cube")
                worst_status = ValidationValue.OK
                worst_attrs = {}
                for i in range(len(objs)):
                    for j in range(len(objs)):
                        if i == j: continue
                        cubea = objs[i]
                        cubeb = objs[j]
                        dist = cubea.calculate_distance(cubeb)
                        # TODO: make rule.value str comparison
                        assert(float(rule.value))
                        if dist < rule.value:
                            current_status = ValidationValue.INVALID
                            current_attrs = {"type": rule.name, "cubea": cubea.id, "cubeb": cubeb.id}
                        elif dist < rule.value * (1 + self.NUMBER_TOLERANCE):
                            current_status = ValidationValue.VIOLATED
                            current_attrs = {"type": rule.name, "cubea": cubea.id, "cubeb": cubeb.id}
                        else:
                            current_status = ValidationValue.OK
                            current_attrs = {}

                        if current_status == ValidationValue.INVALID:
                            worst_status = current_status
                            worst_attrs = current_attrs
                            # Нет смысла проверять дальше, нашли максимальное нарушение
                            return ValidationResult(worst_status, worst_attrs)
                        elif current_status == ValidationValue.VIOLATED and worst_status != ValidationValue.INVALID:
                            worst_status = current_status
                            worst_attrs = current_attrs

                return ValidationResult(worst_status, worst_attrs)


        return ValidationResult(ValidationValue.OK)


def validate(cubes):
    v = Validator(cubes)
    vr = v.validate()
    return vr.to_dict()


