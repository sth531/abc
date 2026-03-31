cmake_minimum_required(VERSION 3.16)

project(SnakeDoubleBuffer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(snake
    main_new.cpp
    src/Common.cpp
    src/Snake.cpp
    src/Food.cpp
    src/Input.cpp
    src/SaveSystem.cpp
    src/Renderer.cpp
    src/Game.cpp
)

target_include_directories(snake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_compile_definitions(snake PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
