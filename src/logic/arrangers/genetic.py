import random
import math
from typing import List, Dict, Optional
from objects_module import Vector3, Object, Cube, Sphere, Room
from validator import Validator, ConstraintEvaluation, DocumentParser, SurfaceType
from arrangers.base import BaseArranger


class GeneticArranger(BaseArranger):
    def __init__(self, objects: List[Object], room: Room):
        super().__init__(objects, room)
        # Параметры алгоритма
        self.pop_size = 50
        self.max_generations = 100
        self.crossover_rate = 0.9
        self.mutation_rate = 0.2
        self.elite_count = 2
        self.tournament_size = 3

        # Идеальные позиции и углы (пользовательские предпочтения)
        # Формат: { obj.id: {"position": Vector3, "angle": float} }
        self.ideal_prefs: Dict[int, dict] = {}

        # Собираем правила distance для быстрого доступа при repair
        # Формат: self.wall_dist_rules[obj_type_str][wall_type_str] = required_distance
        self.wall_dist_rules = {}  # ключ: "cube", "sphere", "any"
        for rule in self._rules:
            if rule.name == "distance":
                parts = rule.object.split(',')
                if len(parts) == 2 and self._is_wall_str(parts[1]):
                    obj_type, wall_str = parts[0], parts[1]
                    self.wall_dist_rules.setdefault(obj_type, {})[wall_str] = rule.value

        print("WALL DIST RULES")
        print(self.wall_dist_rules)

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

    def _is_wall_str(self, s):
        return s in ["FLOOR", "CEILING", "WALL_FRONT", "WALL_BACK", "WALL_LEFT", "WALL_RIGHT"]

    def _get_object_half_sizes(self, obj):
        """Возвращает половинные размеры объекта (x, y, z) для ограничения его центра внутри комнаты."""
        if obj.get_type() == "Cube":
            # У куба есть свойство size (Vector3)
            sz = obj.size
            return (sz.x / 2.0, sz.y / 2.0, sz.z / 2.0)
        elif obj.get_type() == "Sphere":
            r = obj.radius
            return (r, r, r)
        else:
            # Для неизвестных типов — нулевые половинные размеры
            return (0.0, 0.0, 0.0)

    def _repair_clones(self, clones):
        """Принудительно выставляет точные расстояния до стен (использует wall.distance)."""
        origin = self._room.get_origin()
        dims   = self._room.get_dimensions()
        walls  = {w.get_type(): w for w in self._walls}

        for obj in clones:
            if obj.locked:
                continue

            obj_type = obj.get_type().lower()
            applicable_rules = {}
            if "any" in self.wall_dist_rules:
                applicable_rules.update(self.wall_dist_rules["any"])
            if obj_type in self.wall_dist_rules:
                applicable_rules.update(self.wall_dist_rules[obj_type])

            for wall_str, req_dist in applicable_rules.items():
                wall_type = self._get_surface_type_from_str(wall_str)
                wall = walls.get(wall_type)
                if wall is None:
                    continue

                normal = wall.get_normal()
                # Знаковое расстояние от центра до плоскости:
                #   signed_dist = dot(pos, normal) - wall.distance
                signed_center_dist = (normal.x * obj.position.x +
                                      normal.y * obj.position.y +
                                      normal.z * obj.position.z) - wall.distance

                half = obj.get_projection_on_axis(normal)
                # требуемое знаковое расстояние от центра
                target_signed = half + req_dist

                delta = target_signed - signed_center_dist
                obj.position = Vector3(
                    obj.position.x + normal.x * delta,
                    obj.position.y + normal.y * delta,
                    obj.position.z + normal.z * delta
                )

                # отладка
                if obj.id == self._objects[0].id:  # для первого объекта
                    print(f"Repair {obj.get_type()}{obj.id}: before {obj.position}, rule {wall_str}={req_dist}, half={half}, signed_center={signed_center_dist}, target={target_signed}, delta={delta}")

            # Проверка, остался ли объект внутри комнаты
            if not self._room.is_obj_inside(obj):
                half_sizes = self._get_object_half_sizes(obj)
                x_min = origin.x - dims.x/2 + half_sizes[0]
                x_max = origin.x + dims.x/2 - half_sizes[0]
                y_min = origin.y + half_sizes[1]
                y_max = origin.y + dims.y - half_sizes[1]
                z_min = origin.z - dims.z/2 + half_sizes[2]
                z_max = origin.z + dims.z/2 - half_sizes[2]
                obj.position = Vector3(
                    max(x_min, min(obj.position.x, x_max)),
                    max(y_min, min(obj.position.y, y_max)),
                    max(z_min, min(obj.position.z, z_max))
                )

    def set_ideal_preferences(self, prefs: Dict[int, dict]):
        """Установка идеальных позиций/углов для объектов."""
        self.ideal_prefs = prefs

    def arrange(self) -> Dict[int, Vector3]:
        """Главный метод генетического алгоритма."""
        print(f"Starting genetic arrangement... (pop={self.pop_size}, gens={self.max_generations})")
        
        # 1. Составляем список свободных объектов (не locked)
        self.free_objects = [obj for obj in self._objects if not obj.locked]
        self.locked_objects = [obj for obj in self._objects if obj.locked]
        self.obj_index_map = {obj.id: obj for obj in self._objects}

        # 2. Определяем размер генотипа: 4 гена на каждый свободный объект (x,y,z,angle)
        self.genotype_length = len(self.free_objects) * 4

        # 3. Создаём начальную популяцию
        population = [self._create_random_individual() for _ in range(self.pop_size)]

        # 4. Оцениваем начальную популяцию
        fitness_values = [self._evaluate(ind) for ind in population]

        best_feasible_fitness = float('inf')
        best_individual = None
        generations_without_improvement = 0

        for gen in range(self.max_generations):
            # Сортировка по фитнесу (чем меньше, тем лучше)
            combined = list(zip(population, fitness_values))
            combined.sort(key=lambda x: x[1])
            population, fitness_values = zip(*combined) if combined else ([], [])
            population = list(population)
            fitness_values = list(fitness_values)

            # Проверяем наличие допустимых решений
            feasible_pop = [(ind, fit) for ind, fit in zip(population, fitness_values) if fit < 1e6]
            if feasible_pop:
                best_feasible = min(feasible_pop, key=lambda x: x[1])
                if best_feasible[1] < best_feasible_fitness:
                    best_feasible_fitness = best_feasible[1]
                    best_individual = best_feasible[0][:]
                print(f"Gen {gen}: best feasible fitness = {best_feasible[1]:.4f}, "
                      f"feasible count = {len(feasible_pop)}")
                if best_feasible_fitness == 0.0:
                    print("Perfect solution found!")
                    break
            else:
                # Если допустимых нет, лучшая по штрафам
                best_penalty = min(fitness_values)
                print(f"Gen {gen}: best penalty = {best_penalty:.4f}, "
                      f"feasible count = 0")

            # Формируем новое поколение
            new_population = []

            # Элитизм: сохраняем лучшие (допустимые или с лучшим фитнесом)
            for i in range(self.elite_count):
                if i < len(population):
                    new_population.append(population[i][:])

            # Заполняем оставшуюся часть
            while len(new_population) < self.pop_size:
                # Турнирный отбор
                parent1 = self._tournament_selection(population, fitness_values)
                parent2 = self._tournament_selection(population, fitness_values)

                # Кроссовер
                if random.random() < self.crossover_rate:
                    child1, child2 = self._crossover(parent1, parent2)
                else:
                    child1, child2 = parent1[:], parent2[:]

                # Мутация
                child1 = self._mutate(child1)
                child2 = self._mutate(child2)

                new_population.append(child1)
                if len(new_population) < self.pop_size:
                    new_population.append(child2)

            population = new_population
            fitness_values = [self._evaluate(ind) for ind in population]

        # Возвращаем лучшую найденную особь (или последнюю популяцию)
        if best_individual is not None:
            self._apply_genotype(best_individual)
        else:
            # Если не нашли допустимых, можно попробовать применить лучшую по штрафу
            best_idx = min(range(len(fitness_values)), key=lambda i: fitness_values[i])
            self._apply_genotype(population[best_idx])

        self._repair_clones(self._objects)

        return self._get_current_positions()

    def _create_random_individual(self) -> List[float]:
        """Создаёт случайный генотип, соблюдая границы комнаты."""
        genotype = []
        origin = self._room.get_origin()
        dims = self._room.get_dimensions()

        for obj in self.free_objects:
            # Генерируем позицию внутри комнаты (наивно, без учёта других объектов)
            if isinstance(obj, Cube):
                half_sizes = [obj.size.x / 2, obj.size.y / 2, obj.size.z / 2]
            elif isinstance(obj, Sphere):
                r = obj.radius
                half_sizes = [r, r, r]
            else:
                half_sizes = [0, 0, 0]

            x_min = origin.x - dims.x / 2 + half_sizes[0]
            x_max = origin.x + dims.x / 2 - half_sizes[0]
            y_min = origin.y + half_sizes[1]
            y_max = origin.y + dims.y - half_sizes[1]
            z_min = origin.z - dims.z / 2 + half_sizes[2]
            z_max = origin.z + dims.z / 2 - half_sizes[2]

            x = random.uniform(x_min, x_max)
            y = random.uniform(y_min, y_max)
            z = random.uniform(z_min, z_max)
            angle = random.uniform(0.0, 360.0)

            genotype.extend([x, y, z, angle])
        return genotype

    def _decode_genotype(self, genotype: List[float]) -> List[Object]:
        """
        Создаёт копии свободных объектов с параметрами из генотипа.
        Возвращает список всех объектов (копии свободных + оригиналы locked) для проверки.
        """
        clones = []
        idx = 0
        for obj in self.free_objects:
            x = genotype[idx]
            y = genotype[idx + 1]
            z = genotype[idx + 2]
            angle = genotype[idx + 3]
            idx += 4

            new_obj = obj.clone()  # Возвращает shared_ptr<Object>
            new_obj.position = Vector3(x, y, z)
            new_obj.angle = angle
            clones.append(new_obj)

        # Добавляем locked объекты (их позиции/углы не меняются)
        locked_clones = [obj.clone() for obj in self.locked_objects]  # или можно не клонировать, но для чистоты клонируем
        return clones + locked_clones

    def _evaluate(self, genotype: List[float]) -> float:
        """
        Вычисляет фитнес особи.
        Допустимые особи получают значение objective_function (0 пока),
        недопустимые — большое число + штраф из evaluate_constraints.
        """
        clones = self._decode_genotype(genotype)

        # Проверка, что это именно клоны, а не оригиналы
        if clones[0].id == self._objects[0].id:
            raise RuntimeError("Clones share same ID as original!")

        # Принудительно удовлетворяем правилам distance до стен
        self._repair_clones(clones)

        eval_result = self.validator.evaluate_constraints(clones, self._room)
        if eval_result.feasible:
            return self._objective_function(clones)
        else:
            return 1e6 + eval_result.total_violation

    def _objective_function(self, objects_list: List[Object]) -> float:
        """
        Сумма квадратов отклонений от идеальных позиций/углов.
        Пока ideal_prefs пуст, возвращает 0.
        """
        total_error = 0.0
        for obj in objects_list:
            prefs = self.ideal_prefs.get(obj.id, None)
            if prefs is None:
                continue
            ideal_pos = prefs.get("position")
            if ideal_pos is not None:
                dx = obj.position.x - ideal_pos.x
                dy = obj.position.y - ideal_pos.y
                dz = obj.position.z - ideal_pos.z
                total_error += dx*dx + dy*dy + dz*dz
            ideal_angle = prefs.get("angle")
            if ideal_angle is not None:
                diff = abs(obj.angle - ideal_angle)
                # Угловое отклонение с учётом цикличности
                if diff > 180:
                    diff = 360 - diff
                total_error += diff * diff  # можно градусы в квадрате
        return total_error

    def _tournament_selection(self, population, fitnesses):
        """Турнирный отбор: выбираем лучшего из случайных t особей."""
        candidates = random.sample(range(len(population)), self.tournament_size)
        best_idx = min(candidates, key=lambda i: fitnesses[i])
        return population[best_idx][:]

    def _crossover(self, parent1, parent2):
        """
        Арифметический кроссовер (BLX-α) для всего вектора.
        Для углов учитываем цикличность.
        """
        child1, child2 = [], []
        for i in range(0, len(parent1), 4):
            # x,y,z координаты
            for j in range(3):
                a, b = parent1[i+j], parent2[i+j]
                if random.random() < 0.5:
                    a, b = b, a
                beta = random.random()
                child1.append(beta * a + (1 - beta) * b)
                child2.append((1 - beta) * a + beta * b)
            # угол (индекс i+3)
            ang1 = parent1[i+3]
            ang2 = parent2[i+3]
            # Разворачиваем по кратчайшей дуге
            diff = (ang2 - ang1) % 360
            if diff > 180:
                diff -= 360
            # Теперь diff в [-180, 180)
            mid = ang1 + diff / 2
            range_ang = abs(diff) * 0.5  # половина дуги
            child_ang1 = (mid + random.uniform(-range_ang, range_ang)) % 360
            child_ang2 = (mid + random.uniform(-range_ang, range_ang)) % 360
            child1.append(child_ang1)
            child2.append(child_ang2)
        return child1, child2

    def _mutate(self, genotype):
        """Мутация: добавляем гауссов шум к отдельным генам."""
        new_gen = genotype[:]
        room_dims = self._room.get_dimensions()
        sigma_pos = min(room_dims.x, room_dims.y, room_dims.z) / 10.0
        sigma_ang = 15.0
        for i in range(len(new_gen)):
            if random.random() < self.mutation_rate:
                # Определяем, это координата или угол
                gene_type = i % 4
                if gene_type < 3:  # позиция
                    noise = random.gauss(0, sigma_pos)
                    new_gen[i] += noise
                else:  # угол
                    noise = random.gauss(0, sigma_ang)
                    new_gen[i] = (new_gen[i] + noise) % 360
        return new_gen

    def _apply_genotype(self, genotype):
        """Применяет генотип к реальным объектам (меняет их позиции и углы)."""
        idx = 0
        for obj in self.free_objects:
            obj.position = Vector3(genotype[idx], genotype[idx+1], genotype[idx+2])
            obj.angle = genotype[idx+3]
            idx += 4

    def _get_current_positions(self) -> Dict[int, Vector3]:
        """Словарь текущих позиций всех объектов."""
        return {obj.id: obj.position for obj in self._objects}

