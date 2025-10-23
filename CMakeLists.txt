cmake_minimum_required(VERSION 3.15)
project(ShipRoomDesigner LANGUAGES C CXX)

# Пути
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include)
set(SRC_DIR ${PROJECT_ROOT}/src)

# Добавляем подмодуль pybind11
add_subdirectory(${PROJECT_ROOT}/external/pybind11)

# Поиск библиотек
find_package(raylib REQUIRED)
find_package(Python REQUIRED COMPONENTS Development Interpreter)

# Настройка компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Сбор исходников
file(GLOB_RECURSE APP_SOURCES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.cxx
    ${SRC_DIR}/*.cc
)

# Исполняемый файл
add_executable(ShipRoomDesigner ${APP_SOURCES})

# Добавляем include/ в пути поиска заголовков
target_include_directories(ShipRoomDesigner
    PRIVATE
        ${INCLUDE_DIR}
        ${Python_INCLUDE_DIRS}
)

# Подключение библиотек
target_link_libraries(ShipRoomDesigner
    PRIVATE
        raylib
        pybind11::embed
)

# Модуль cpp для Python
pybind11_add_module(objects_module
  ${SRC_DIR}/pybind/bindings.cpp
  ${SRC_DIR}/graphics/cube.cpp
  ${SRC_DIR}/graphics/sphere.cpp
  ${SRC_DIR}/graphics/object.cpp
  ${SRC_DIR}/graphics/wall.cpp
  ${SRC_DIR}/graphics/room.cpp
)
target_include_directories(objects_module PRIVATE ${INCLUDE_DIR})
target_link_libraries(objects_module PRIVATE pybind11::module raylib)

# add_custom_target(copy_assets ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#             ${PROJECT_ROOT}/assets ${CMAKE_BINARY_DIR}/assets
#     COMMENT "Copying assets to build directory"
# )
# Копирование ассетов и скриптов
add_custom_target(copy_python_files ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy
            ${SRC_DIR}/logic/validator.py ${CMAKE_BINARY_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_ROOT}/assets ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying Python files and assets to build directory"
)

# Убедимся, что модуль компилируется в нужную директорию
set_target_properties(objects_module PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    PREFIX ""
    SUFFIX ""
)

if (WIN32)
    set_target_properties(objects_module PROPERTIES SUFFIX ".pyd")
else()
    set_target_properties(objects_module PROPERTIES SUFFIX ".so")
endif()

# Опции для релокации исполняемого и поиска модулей
set_target_properties(ShipRoomDesigner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

include_directories(${INCLUDE_DIR})

# Добавляем зависимость от копирования Python файлов
add_dependencies(ShipRoomDesigner copy_python_files)
add_dependencies(objects_module copy_python_files)

