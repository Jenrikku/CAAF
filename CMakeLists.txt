cmake_minimum_required(VERSION 3.5)
project(caaf)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(caafeditor src/main.cpp
                          src/engine/io.cpp
                          src/engine/caaf.cpp
                          src/engine/lzma.cpp
                          src/engine/model.cpp
                          src/imgui/imgui.cpp
                          src/imgui/imgui_demo.cpp
                          src/imgui/imgui_draw.cpp
                          src/imgui/imgui_tables.cpp
                          src/imgui/imgui_widgets.cpp
                          src/imgui/imgui_impl_sdl3.cpp
                          src/imgui/imgui_impl_sdlgpu3.cpp)

target_include_directories(caafeditor PRIVATE include)
target_include_directories(caafeditor PRIVATE include/imgui)
target_link_libraries(caafeditor PRIVATE SDL3::SDL3 assimp lzma)
