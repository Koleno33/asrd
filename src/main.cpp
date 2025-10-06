#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <memory>
#include <graphics/cube.h>
#include <logic/objvalidator.h>

int main(void) 
{
  constexpr int screenWidth  = 800;
  constexpr int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "AI Ship Room Designer");

  Camera camera = { 0 };
  camera.position = (Vector3){ 10.0f, 5.0f, 0.0f };   // Позиция камеры
  camera.target = (Vector3){ 0.0f, 5.0f, 10.0f };      // Куда смотрит камера
  camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Вверх (обычно вектор (0,1,0))
  camera.fovy = 45.0f;                                // Поле зрения
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

  std::vector<Cube*> cubes;

  Cube* cube1 = new Cube(
    (Vector3){ 0.0f, 0.0f, 0.0f },
    (Vector3){ 2.0f, 2.0f, 2.0f }
  );

  Cube* cube2 = new Cube(
    (Vector3){ -3.0f, 0.0f, -3.0f },
    (Vector3){ 2.0f, 2.0f, 2.0f }
  );
  
  cubes.push_back(cube1);
  cubes.push_back(cube2);

  CubeValidator validator { };

  while (!WindowShouldClose()) {
    Vector2 cur_mouse_pos = GetMousePosition();
    Vector2 mouse_delta = {
      (cur_mouse_pos.x - prev_mouse_pos.x) * mousesens,
      (cur_mouse_pos.y - prev_mouse_pos.y) * mousesens
    };

    camera_yaw += mouse_delta.x;
    camera_pitch -= mouse_delta.y;

    // Чтобы камера на перевернулась
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

    if (IsKeyDown(KEY_W)) camera.position = Vector3Add(camera.position, Vector3Scale(forward, movspeed));
    if (IsKeyDown(KEY_S)) camera.position = Vector3Subtract(camera.position, Vector3Scale(forward, movspeed));
    if (IsKeyDown(KEY_D)) camera.position = Vector3Add(camera.position, Vector3Scale(right, movspeed));
    if (IsKeyDown(KEY_A)) camera.position = Vector3Subtract(camera.position, Vector3Scale(right, movspeed));
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 
        camera.position.y -= movspeed;
    if (IsKeyDown(KEY_SPACE)) 
        camera.position.y += movspeed;

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

    // Обработка выхода из программы
    if (IsKeyPressed(KEY_Q)) break;

    Color valcolor = validator.validate_cubes(cubes);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    // Отрисовка кубов
    for (const auto& cube : cubes) {
      DrawCubeV(cube->get_position(), cube->get_size(), valcolor);
      DrawCubeWiresV(cube->get_position(), cube->get_size(), DARKGRAY);
    }

    // Оси
    DrawGrid(10, 1.0f);

    EndMode3D(); // Конец 3D-режима
                 
    // Отображаем статус валидации
    const char* status;
    int colnum = ColorToInt(valcolor);
    if (colnum == ColorToInt(GREEN)) {
      status = "OK";
    }
    else if (colnum == ColorToInt(ORANGE)) {
      status = "VIOLATED";
    }
    else if (colnum == ColorToInt(RED)) {
      status = "INVALID";
    }
    else {
      status = "UNKNOWN";
    }

    DrawText("AI Ship Room Designer", 10, 10, 20, DARKGRAY);
    DrawText(TextFormat("Status: %s", status), 10, 40, 20, valcolor);
    DrawText(TextFormat("Distance: %.2f", cube1->calculate_distance(*cube2)), 10, 70, 20, valcolor);
    DrawFPS(10, 100);

    EndDrawing(); // Конец рисования
  }

  for (auto cube : cubes) {
    delete cube;
  }

  CloseWindow();
  return 0;
}

