#include <logic/objvalidator.h>
#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <graphics/room.h>
#include <memory>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

std::map<SurfaceType, std::string> surfacetype_str_map = {
  {SurfaceType::FLOOR, "FLOOR"},
  {SurfaceType::CEILING, "CEILING"},
  {SurfaceType::WALL_FRONT, "WALL_FRONT"},
  {SurfaceType::WALL_BACK, "WALL_BACK"},
  {SurfaceType::WALL_LEFT, "WALL_LEFT"},
  {SurfaceType::WALL_RIGHT, "WALL_RIGHT"}
};

std::string surfacetype_to_str(SurfaceType type) {
  auto it = surfacetype_str_map.find(type);
  if (it != surfacetype_str_map.end()) {
      return it->second;
  }
  return "Unknown Surface Type"; 
}

int main(void) 
{
  constexpr int screen_width  = 800;
  constexpr int screen_height = 450;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE); 
  InitWindow(screen_width, screen_height, "AI Ship Room Designer");

  Camera camera = { 0 };
  camera.position = (Vector3){ 10.0f, 5.0f, 0.0f };   // Позиция камеры
  camera.target = (Vector3){ 0.0f, 5.0f, 10.0f };     // Куда смотрит камера
  camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Вверх (обычно вектор (0,1,0))
  camera.fovy = 60.0f;                                // Поле зрения
  camera.projection = CAMERA_PERSPECTIVE;             // Тип проекции

  // Начальные углы
  Vector3 direction = Vector3Subtract(camera.target, camera.position);
  direction = Vector3Normalize(direction);

  // Управление камерой
  Vector2 prev_mouse_pos = { 0 };
  float camera_yaw = atan2f(direction.z, direction.x);
  float camera_pitch = asinf(direction.y);
  float mousesens = 0.003f;
  float movspeed = 0.09f;

  bool cursor_enabled { false };

  HideCursor();
  DisableCursor();
 
  SetTargetFPS(60);

  std::vector<std::shared_ptr<Object>> objects;

  auto cube1 = std::make_shared<Cube> (
    (Vector3){ 0.0f, 1.0f, 0.0f },
    (Vector3){ 2.0f, 2.0f, 2.0f }
  );

  auto cube2 = std::make_shared<Cube> (
    (Vector3){ -3.0f, 1.0f, -1.5f },
    (Vector3){ 2.0f, 2.0f, 2.0f }
  );

  auto cube3 = std::make_shared<Cube> (
    (Vector3){ -9.5f, 0.5f, 5.0f },
    (Vector3){ 1.0f, 1.0f, 1.0f }
  );

  auto sphere1 = std::make_shared<Sphere>(
    (Vector3){ -9.0f, 1.0f, -3.0f },
    1.0f
  );

  auto sphere2 = std::make_shared<Sphere>(
    (Vector3){ -9.0f, 4.1f, -3.0f },
    1.0f
  );
  
  objects.push_back(cube1);
  objects.push_back(cube2);
  objects.push_back(cube3);
  objects.push_back(sphere1);
  objects.push_back(sphere2);

  auto room = std::make_shared<Room>(
    (Vector3){ 0.0f, 0.0f, 0.0f },       // origin
    (Vector3){ 20.0f, 10.0f, 20.0f },    // dimensions
    BLUE                                 // wireframe color
  );

  Vector3 room_center = room->get_center();
  Vector3 room_origin = room->get_origin();
  std::cout << "Room center is at " << room_center.x << " " << room_center.y << " " << room_center.z << '\n';
  std::cout << "Room origin is at " << room_origin.x << " " << room_origin.y << " " << room_origin.z << '\n';
  // Walls
  std::cout << "*******************************************************\n";
  for (int i = 0; i < objects.size(); ++i) {
    for (int j = 0; j < room->get_walls().size(); ++j) {
      std::shared_ptr<Object> obja = objects[i];
      std::shared_ptr<const Wall> wall = room->get_walls()[j];
      std::cout << "Distance between " << obja->get_type() << obja->get_id() << " and " << surfacetype_to_str(wall->get_type()) 
                << "\t is \t" << obja->calculate_distance_to_wall(*wall) << "\t (Object is " << (room->is_obj_inside(*obja)
                ? "inside room)" : "not inside room)");
      std::cout << '\n';
    }
  }
  std::cout << "-------------------------------------------------------\n\n";

  // Выводим все объекты и расстояния между ними в stdout
  std::cout << "*******************************************************\n";
  for (int i = 0; i < objects.size(); ++i) {
    for (int j = i + 1; j < objects.size(); ++j) {
      std::shared_ptr<Object> obja = objects[i];
      std::shared_ptr<Object> objb = objects[j];
      std::cout << "Distance between " << obja->get_type() << obja->get_id() << " and " << objb->get_type() << objb->get_id()
                << "\t is \t" << obja->calculate_distance(*objb) << '\n';
    }
  }
  std::cout << "-------------------------------------------------------\n";

  ObjValidator validator { };

  // Временно единоразовая проверка
  validator.validate(objects, room);

  // Кнопка компоновки
  Rectangle arrange_btn { screen_width - 150, 10, 140, 40 };
  bool arrange_btn_pressed { false };

  while (!WindowShouldClose()) {
    Vector2 cur_mouse_pos = GetMousePosition();
    Vector2 mouse_delta = {
      (cur_mouse_pos.x - prev_mouse_pos.x) * mousesens,
      (cur_mouse_pos.y - prev_mouse_pos.y) * mousesens
    };

    camera_yaw += mouse_delta.x;
    camera_pitch -= mouse_delta.y;

    // Чтобы камера не перевернулась
    if (camera_pitch > 89.0f * DEG2RAD) camera_pitch = 89.0f * DEG2RAD;
    if (camera_pitch < -89.0f * DEG2RAD) camera_pitch = -89.0f * DEG2RAD;

    Vector3 forward = {
        cosf(camera_yaw) * cosf(camera_pitch),
        0,  // Игнорируем вертикальную составляющую для горизонтального движения
        sinf(camera_yaw) * cosf(camera_pitch)
    };
    forward = Vector3Normalize(forward);
    
    Vector3 right = {
        -forward.z,
        0.0f,
        forward.x
    };
    right = Vector3Normalize(right);

    if (IsKeyPressed(KEY_TAB)) {
      cursor_enabled = !cursor_enabled;
      if (cursor_enabled) {
        EnableCursor();
        ShowCursor();
      } 
      else {
        DisableCursor();
        HideCursor();
      }
    }

    if (IsKeyDown(KEY_W)) {
      camera.position = Vector3Add(camera.position, Vector3Scale(forward, movspeed));
    }
    if (IsKeyDown(KEY_S)) {
      camera.position = Vector3Subtract(camera.position, Vector3Scale(forward, movspeed));
    }
    if (IsKeyDown(KEY_D)) {
      camera.position = Vector3Add(camera.position, Vector3Scale(right, movspeed));
    }
    if (IsKeyDown(KEY_A)) {
      camera.position = Vector3Subtract(camera.position, Vector3Scale(right, movspeed));
    }
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
      camera.position.y -= movspeed;
    }
    if (IsKeyDown(KEY_SPACE)) {
      camera.position.y += movspeed;
    }

    if (IsKeyPressed(KEY_ENTER)) {
      arrange_btn_pressed = true;
    }

    // Рассчитываем новый target на основе углов
    Vector3 new_direction = {
      cosf(camera_yaw) * cosf(camera_pitch),
      sinf(camera_pitch),
      sinf(camera_yaw) * cosf(camera_pitch)
    };
    new_direction = Vector3Normalize(new_direction);
    camera.target = Vector3Add(camera.position, new_direction);

    // Обновляем позицию мыши для следующего кадра
    prev_mouse_pos = cur_mouse_pos;

    // Обработка нажатия на кнопку компоновки
    if (CheckCollisionPointRec(GetMousePosition(), arrange_btn) && 
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      arrange_btn_pressed = true;
    }

    if (arrange_btn_pressed) {
      validator.arrange_objs(objects, room);
      validator.validate(objects, room);
      arrange_btn_pressed = false;
    }

    if (IsKeyPressed(KEY_Q)) break;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Отрисовка кнопки
    DrawRectangleRec(arrange_btn, arrange_btn_pressed ? DARKGRAY : LIGHTGRAY);
    DrawText("Arrange", arrange_btn.x + 10, arrange_btn.y + 10, 20, DARKGRAY);

    BeginMode3D(camera);

    // Отрисовка объектов
    for (std::shared_ptr<Object> obj : objects) {
      obj->draw();
    }

    // Отрисовка помещения
    room->draw(camera.position);

    // Оси
    DrawGrid(20, 1.0f);

    EndMode3D(); // Конец 3D-режима
                 
    // Отображаем статус валидации

    DrawText("AI Ship Room Designer", 10, 10, 20, DARKGRAY);
    DrawFPS(10, 40);

    EndDrawing(); // Конец рисования
  }

  CloseWindow();
  return 0;
}

