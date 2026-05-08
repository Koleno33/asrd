from __future__ import annotations
from typing import TYPE_CHECKING
from typing import List, Dict
from enum import Enum
from objects_module import Vector3, Color, Room, Wall, SurfaceType, Object
import random
import math


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


class ObjectArrangerPython:
    def __init__(self, objects: List[Object], room: Room):
        self.__room = room
        self.__rules = DocumentParser.parse_document("assets/docs/doc.txt")
        self.__objects = objects
        self.__walls = room.get_walls()
        self.validator = Validator(objects, room)
        
        # cache for effective access to walls
        self.__wall_cache = {}
        for wall in self.__walls:
            self.__wall_cache[wall.get_type()] = wall
            
        # params
        self.MAX_ATTEMPTS = 250
        
    def arrange_objects(self) -> Dict[int, Vector3]:
        """Main method of object arrangement"""
        print("The beggining of objects arrangement...")
        
        # main alg for unsatisfied rules
        for attempt in range(self.MAX_ATTEMPTS):
            if attempt % 50 == 0:
                print(f"Attempt {attempt + 1}/{self.MAX_ATTEMPTS}")
                
            initial_positions = self._save_positions()

            self._aggressive_rearrangement()
            
            validation_results = self.validator.validate()
            if self._all_rules_satisfied(validation_results):
                print(f"✓ Successful arrangement on attempt {attempt + 1}")
                return self._get_current_positions()
                
            # rollback if not success
            self._restore_positions(initial_positions)
            
        
        print("✗ Failed to find any correct arrangement.")
        return self._get_current_positions()

    def _phase_apply_all_rules(self) -> bool:
        """Applies all the rules"""
        max_iterations = 100
        
        for iteration in range(max_iterations):
            improvements = 0
            
            # apply "distance" rules to walls
            for obj in self.__objects:
                if self._apply_distance_rules_for_object(obj):
                    improvements += 1
            
            # apply "min_distance" rules
            if self._apply_min_distance_rules():
                improvements += 1
            
            # check results
            validation_results = self.validator.validate()
            if self._all_rules_satisfied(validation_results):
                return True
                
            # didn't do any improvements
            if improvements == 0:
                break
                
        return False

    def _apply_distance_rules_for_object(self, obj) -> bool:
        """Apply 'distance' rule for object"""
        improvements = 0
        
        for rule in self.__rules:
            if rule.name != "distance":
                continue
                
            if not self._does_rule_apply_to_object(rule, obj):
                continue
                
            parts = rule.object.split(',')
            if len(parts) != 2:
                continue
                
            wall_type_str = parts[1]
            wall = self._get_wall_by_string(wall_type_str)

            if wall and self._enforce_wall_distance(obj, wall, rule.value):
                improvements += 1
                
        return improvements > 0

    def _enforce_wall_distance(self, obj, wall, required_distance) -> bool:
        """Ensures precies distance to wall"""
        current_distance = obj.calculate_distance_to_wall(wall)
        
        if current_distance == required_distance:
            return False  # satisfied
            
        normal = wall.get_normal()
        
        # calculate necessary shift

        # calculate sign
        if current_distance < required_distance:
            # direction FROM wall
            move_direction = 1
        else:
            # direction TO wall
            move_direction = -1
            
        # calculate distance to move
        move_distance = abs(required_distance - current_distance)

        move_vector = Vector3(
            normal.x * move_distance * move_direction,
            normal.y * move_distance * move_direction,
            normal.z * move_distance * move_direction
        )
        
        new_position = Vector3(
            obj.position.x + move_vector.x,
            obj.position.y + move_vector.y,
            obj.position.z + move_vector.z
        )
        
        # check validity
        if self._is_position_valid_for_object(obj, new_position):
            obj.position = new_position
        else:
            #print(f"Placed {obj} into invalid position {new_position}")
            pass
                    
        return False

    def _apply_min_distance_rules(self) -> bool:
        """Applies 'min_distance' rule to objects"""
        max_iterations = 50
        improvements = 0
        
        for iteration in range(max_iterations):
            violations = self._find_all_min_distance_violations()
            if not violations:
                return improvements > 0
                
            # correct the most invalid violation
            worst_violation = max(violations, key=lambda v: v["severity"])
            if self._fix_min_distance_violation(worst_violation):
                improvements += 1
                
        return improvements > 0

    def _find_all_min_distance_violations(self):
        """Find all violations of 'min_distance' rules"""
        violations = []
        
        for rule in self.__rules:
            if rule.name != "min_distance":
                continue
                
            target_objects = self._get_objects_by_type(rule.object)
            
            for i in range(len(target_objects)):
                for j in range(i + 1, len(target_objects)):
                    obj1, obj2 = target_objects[i], target_objects[j]
                    current_distance = obj1.calculate_distance(obj2)
                    
                    if current_distance < rule.value:
                        severity = (rule.value - current_distance) / rule.value
                        violations.append({
                            "obj1": obj1,
                            "obj2": obj2,
                            "required": rule.value,
                            "current": current_distance,
                            "severity": severity
                        })
        
        return violations

    def _fix_min_distance_violation(self, violation) -> bool:
        """Corrects one violation of 'min_distance' """
        obj1, obj2 = violation["obj1"], violation["obj2"]
        required_distance = violation["required"]
        current_distance = violation["current"]
        
        direction = Vector3(
            obj2.position.x - obj1.position.x,
            obj2.position.y - obj1.position.y,
            obj2.position.z - obj1.position.z
        )
        
        # nornalize the direction
        length = math.sqrt(direction.x**2 + direction.y**2 + direction.z**2)
        if length < 0.001:
            # if objects in one position then random direction
            direction = Vector3(random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1))
            length = 1.0
            
        direction = Vector3(
            direction.x / length,
            direction.y / length,
            direction.z / length
        )
        
        # calculate shift
        move_distance = (required_distance - current_distance) * 0.6  # ???
        
        # trying to move objects
        new_pos1 = Vector3(
            obj1.position.x - direction.x * move_distance,
            obj1.position.y - direction.y * move_distance,
            obj1.position.z - direction.z * move_distance
        )
        
        new_pos2 = Vector3(
            obj2.position.x + direction.x * move_distance,
            obj2.position.y + direction.y * move_distance,
            obj2.position.z + direction.z * move_distance
        )
        
        # saving old positions
        old_pos1 = Vector3(obj1.position.x, obj1.position.y, obj1.position.z)
        old_pos2 = Vector3(obj2.position.x, obj2.position.y, obj2.position.z)
        
        # moving objects temporarily
        obj1.position = new_pos1
        obj2.position = new_pos2
        
        # check if new positions are valid
        pos1_valid = self._is_position_valid_for_object(obj1, new_pos1)
        pos2_valid = self._is_position_valid_for_object(obj2, new_pos2)
        
        if pos1_valid and pos2_valid:
            # check if new distance is better
            new_distance = obj1.calculate_distance(obj2)
            if new_distance > current_distance:
                return True
            else:
                # rollback if not
                obj1.position = old_pos1
                obj2.position = old_pos2
                return False
        else:
            # rollback if all positions are invalid
            obj1.position = old_pos1
            obj2.position = old_pos2
            return False

    def _aggressive_rearrangement(self):
        """Aggressive rearrangement if case is difficult"""
        print("Aggressive arrangement...")
        
        # fully random rearrangement of random objects
        for obj in self.__objects:
            new_pos = self._generate_random_position_for_object(obj)
            if new_pos:
                obj.position = new_pos

    def _generate_random_position_for_object(self, obj):
        """Generates random position for object"""
        room_center = self.__room.get_center()
        room_dims = self.__room.get_dimensions()

        wall_distances = dict()
        for rule in self.__rules:
            if rule.name != "distance":
                continue
                
            if not self._does_rule_apply_to_object(rule, obj):
                continue
                
            parts = rule.object.split(',')
            if len(parts) != 2:
                continue
                
            wall_type_str = parts[1]
            wall_distances[wall_type_str] = rule.value

        # walk along the walls and check if obj is outside
        for wall_type_str in self.__wall_cache:
            if obj.calculate_distance_to_wall(self.__wall_cache[wall_type_str]) < 0.0:
                wall_distances[wall_type_str] = 0.0

        # calculating safe borders for object with its size
        if obj.get_type() == "Sphere":
            padding = obj.radius
        else:  # Cube
            padding = max(obj.size.x, obj.size.y, obj.size.z) / 2
            
        safe_center = Vector3(
            room_center.x + padding - room_dims.x / 2,
            room_center.y + padding - room_dims.y / 2,
            room_center.z + padding - room_dims.z / 2
        )
        
        safe_dims = Vector3(
            room_dims.x / 2 - padding,
            room_dims.y / 2 - padding,
            room_dims.z / 2 - padding
        )
        
        # generating random position
        new_x = random.uniform(safe_center.x, safe_center.x + safe_dims.x) * random.choice([-1, 1])
        new_y = random.uniform(safe_center.y, safe_center.y + safe_dims.y) 
        new_z = random.uniform(safe_center.z, safe_center.z + safe_dims.z) * random.choice([-1, 1])

        new_position = Vector3(new_x, new_y, new_z)

        print(f"Placed {obj} position from random {new_position} ", end='')
        for wall_str, need_dist in wall_distances.items():
            wall_obj = self._get_wall_by_string(wall_str)
            if not wall_obj:
                continue
            normal = wall_obj.get_normal()
            # Вычисляем полупроекцию объекта на нормаль стены (уже с учётом поворота куба)
            half_proj = obj.get_projection_on_axis(normal)
            target_center_dist = need_dist + half_proj  # требуемое расстояние от центра до плоскости стены

            origin = self.__room.get_origin()
            dims = self.__room.get_dimensions()

            match wall_str:
                case "FLOOR":
                    # нормаль (0,1,0), плоскость y = origin.y
                    new_y = origin.y + target_center_dist
                case "CEILING":
                    # нормаль (0,-1,0), плоскость y = origin.y + dimensions.y
                    new_y = origin.y + dims.y - target_center_dist
                case "WALL_FRONT":
                    # нормаль (0,0,1), плоскость z = origin.z - dims.z/2
                    new_z = origin.z - dims.z/2 + target_center_dist
                case "WALL_BACK":
                    # нормаль (0,0,-1), плоскость z = origin.z + dims.z/2
                    new_z = origin.z + dims.z/2 - target_center_dist
                case "WALL_LEFT":
                    # нормаль (1,0,0), плоскость x = origin.x - dims.x/2
                    new_x = origin.x - dims.x/2 + target_center_dist
                case "WALL_RIGHT":
                    # нормаль (-1,0,0), плоскость x = origin.x + dims.x/2
                    new_x = origin.x + dims.x/2 - target_center_dist
        
        new_position = Vector3(new_x, new_y, new_z)

        print(f"to {new_position}")
        
        if self._is_position_valid_for_object(obj, new_position):
            return new_position
                
        return None

    def _is_position_valid_for_object(self, obj, position):
        """Checks if position is valid"""
        if not self.__room.is_obj_inside(obj):
            return False
        
        # saving current position
        old_position = Vector3(obj.position.x, obj.position.y, obj.position.z)
        obj.position = position
        
        # checking collisions
        is_valid = True
        for other_obj in self.__objects:
            if other_obj.id != obj.id:
                if obj.check_collision(other_obj):
                    is_valid = False
                    break
        
        # restoring position
        obj.position = old_position
        return is_valid

    def _does_rule_apply_to_object(self, rule, obj):
        """Checks if the rule is applied to object"""
        if rule.name == "min_distance":
            return rule.object == "any" or rule.object == obj.get_type().lower()
        elif rule.name == "distance":
            parts = rule.object.split(',')
            if len(parts) == 2:
                obj_type = parts[0]
                return obj_type == "any" or obj_type == obj.get_type().lower()
        return False

    def _get_wall_by_string(self, wall_type_str):
        """Returns the wall by string representation"""
        wall_mapping = {
            "FLOOR": SurfaceType.FLOOR,
            "CEILING": SurfaceType.CEILING,
            "WALL_FRONT": SurfaceType.WALL_FRONT,
            "WALL_BACK": SurfaceType.WALL_BACK,
            "WALL_LEFT": SurfaceType.WALL_LEFT,
            "WALL_RIGHT": SurfaceType.WALL_RIGHT
        }
        
        wall_type = wall_mapping.get(wall_type_str)
        return self.__wall_cache.get(wall_type)

    def _get_objects_by_type(self, obj_type_str):
        """Returns objects by their type"""
        if obj_type_str == "any":
            return self.__objects
        elif obj_type_str == "cube":
            return [obj for obj in self.__objects if obj.get_type() == "Cube"]
        elif obj_type_str == "sphere":
            return [obj for obj in self.__objects if obj.get_type() == "Sphere"]
        else:
            return []

    def _all_rules_satisfied(self, validation_results):
        """Checks if al rules are satisfied"""
        return all(result['value'] == 'ok' for result in validation_results)

    def _get_current_positions(self):
        """Returns current positions of all objects"""
        return {obj.id: Vector3(obj.position.x, obj.position.y, obj.position.z) 
                for obj in self.__objects}

    def _save_positions(self):
        """Saves current positions of all objects"""
        return self._get_current_positions()

    def _restore_positions(self, positions):
        """Restores positions to all of objects"""
        for obj in self.__objects:
            if obj.id in positions:
                obj.position = positions[obj.id]


def validate(objs: List[Object], room: Room):
    v = Validator(objs, room)
    # list of dicts
    vrs = v.validate()
    return vrs

def arrange_objects(objs: List[Object], room: Room):
    arranger = ObjectArrangerPython(objs, room)
    return arranger.arrange_objects()

