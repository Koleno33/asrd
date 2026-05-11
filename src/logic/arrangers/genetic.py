import random
import math
from typing import List, Dict, Optional
from objects_module import Vector3, Object, Cube, Sphere, Room
from validator import Validator, ConstraintEvaluation, DocumentParser
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

    def set_ideal_preferences(self, prefs: Dict[int, dict]):
        """Установка идеальных позиций/углов для объектов."""
        self.ideal_prefs = prefs

    def arrange(self) -> Dict[int, Vector3]:
        """Главный метод генетического алгоритма."""
        print(f"Starting genetic arrangement... (pop={self.pop_size}, gens={self.max_generations})")
        
        # 1. Составляем список свободных объектов (не locked)
        self.free_objects = [obj for obj in self._objects if not obj.is_locked()]
        self.locked_objects = [obj for obj in self._objects if obj.is_locked()]
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

        return self._get_current_positions()

    def _create_random_individual(self) -> List[float]:
        """Создаёт случайный генотип, соблюдая границы комнаты."""
        genotype = []
        origin = self._room.get_origin()
        dims = self._room.get_dimensions()

        for obj in self.free_objects:
            # Генерируем позицию внутри комнаты (наивно, без учёта других объектов)
            if isinstance(obj, Cube):
                half_sizes = [obj.get_size().x / 2, obj.get_size().y / 2, obj.get_size().z / 2]
            elif isinstance(obj, Sphere):
                r = obj.get_radius()
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
            new_obj.set_angle(angle)
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
        objects_list = self._decode_genotype(genotype)

        # Проверяем ограничения
        eval_result: ConstraintEvaluation = self.validator.evaluate_constraints(objects_list, self._room)

        if eval_result.feasible:
            return self._objective_function(objects_list)
        else:
            # Штраф за нарушения: большое базовое число + суммарное нарушение
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
                diff = abs(obj.get_angle() - ideal_angle)
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
            obj.set_angle(genotype[idx+3])
            idx += 4

    def _get_current_positions(self) -> Dict[int, Vector3]:
        """Словарь текущих позиций всех объектов."""
        return {obj.id: obj.position for obj in self._objects}

