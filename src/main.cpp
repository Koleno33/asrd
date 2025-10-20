#include <raylib.h>
#include <raymath.h>
#include <graphics/cube.h>
#include <graphics/sphere.h>
#include <graphics/room.h>
#include <logic/objvalidator.h>
#include <iostream>

int main(void) 
{
  constexpr int screen_width  = 800;
  constexpr int screen_height = 450;

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

  HideCursor();
  DisableCursor();
 
  SetTargetFPS(60);

  std::vector<Object*> objects;

  Cube* cube1 = new Cube(
    (Vector3){ 0.0f, 1.0f, 0.0f },
    (Vector3){ 2.0f, 2.0f, 2.0f }
  );

  Cube* cube2 = new Cube(
    (Vector3){ -3.0f, 1.0f, -3.0f },
    (Vector3){ 2.0f, 2.0f, 2.0f }
  );

  Cube* cube3 = new Cube(
    (Vector3){ 5.0f, 0.5f, 5.0f },
    (Vector3){ 1.0f, 1.0f, 1.0f }
  );

  Sphere* sphere1 = new Sphere(
    (Vector3){ -9.0f, 1.0f, -3.0f },
    1.0f
  );

  Sphere* sphere2 = new Sphere(
    (Vector3){ -9.0f, 4.1f, -3.0f },
    1.0f
  );
  
  objects.push_back(cube1);
  objects.push_back(cube2);
  objects.push_back(cube3);
  objects.push_back(sphere1);
  objects.push_back(sphere2);

  Room *room = new Room(
    (Vector3){ 0.0f, 0.0f, 0.0f },       // origin
    (Vector3){ 20.0f, 10.0f, 20.0f },    // dimensions
    BLUE                                 // wireframe color
  );

  // Выводим все объекты и расстояния между ними в stdout
  std::cout << "*******************************************************\n";
  for (int i = 0; i < objects.size(); ++i) {
    for (int j = i + 1; j < objects.size(); ++j) {
      Object* obja = objects[i];
      Object* objb = objects[j];
      std::cout << "Distance between " << obja->get_type() << obja->get_id() << " and " << objb->get_type() << objb->get_id()
                << "\t is \t" << obja->calculate_distance(*objb) << '\n';
    }
  }
  std::cout << "-------------------------------------------------------\n";

  ObjValidator validator { };

  // Временно единоразовая проверка
  validator.validate(objects);

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

    if (IsKeyPressed(KEY_Q)) break;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    // Отрисовка объектов
    for (Object* obj : objects) {
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

  for (auto obj : objects) {
    delete obj;
  }
  delete room;

  CloseWindow();
  return 0;
}

