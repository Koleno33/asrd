#include <raylib.h>
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
  camera.position = (Vector3){ 10.0f, 5.0f, 0.0f };    // Позиция камеры
  camera.target = (Vector3){ -1.5f, 0.0f, -1.5f };      // Куда смотрит камера
  camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Вверх (обычно вектор (0,1,0))
  camera.fovy = 45.0f;                                // Поле зрения
  camera.projection = CAMERA_PERSPECTIVE;             // Тип проекции
 
  SetTargetFPS(60);

  std::vector<Cube*> cubes;

  Cube* cube1 = new Cube();
  cube1->setPosition((Vector3){ 0.0f, 0.0f, 0.0f });
  cube1->setSize((Vector3){ 2.0f, 2.0f, 2.0f });
  cube1->setId(1);
  
  Cube* cube2 = new Cube();
  cube2->setPosition((Vector3){ -3.0f, 0.0f, -3.0f });
  cube2->setSize((Vector3){ 2.0f, 2.0f, 2.0f });
  cube2->setId(2);

  cubes.push_back(cube1);
  cubes.push_back(cube2);

  CubeValidator validator;

  while (!WindowShouldClose()) {
      // Обновление
      //UpdateCamera(&camera, CAMERA_ORBITAL); // Вращение камеры вокруг объекта

      Color validationColor = validator.validate_cubes(cubes);

      BeginDrawing();
      ClearBackground(RAYWHITE);

      BeginMode3D(camera);

      // Отрисовка кубов
      for (const auto& cube : cubes) {
          DrawCubeV(cube->getPosition(), cube->getSize(), validationColor);
          DrawCubeWiresV(cube->getPosition(), cube->getSize(), DARKGRAY);
      }

      // Оси
      DrawGrid(10, 1.0f);

      EndMode3D(); // Конец 3D-режима
                   
      // Отображаем статус валидации
      const char* statusText;
      int colnum = ColorToInt(validationColor);
      if (colnum == ColorToInt(GREEN)) {
          statusText = "OK";
      } else if (colnum == ColorToInt(ORANGE)) {
          statusText = "VIOLATED";
      } else if (colnum == ColorToInt(RED)) {
          statusText = "INVALID";
      } else {
          statusText = "UNKNOWN";
      }

      DrawText("AI Ship Room Designer", 10, 10, 20, DARKGRAY);
      DrawText(TextFormat("Status: %s", statusText), 10, 40, 20, validationColor);
      DrawFPS(10, 70);

      EndDrawing(); // Конец рисования
  }

  for (auto cube : cubes) {
    delete cube;
  }

  CloseWindow();
  return 0;
}

