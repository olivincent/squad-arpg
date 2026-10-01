# ./CMakeLists.txt
cmake_minimum_required(VERSION 4.4.3)

project(SquadArpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)
find_package(Catch2 3 REQUIRED)

add_library(squad_core STATIC src/TileMap.cpp src/Escouade.cpp)
target_include_directories(squad_core PUBLIC src)
target_link_libraries(squad_core PUBLIC SFML::Graphics)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE squad_core)

add_executable(tests tests/test_TileMap.cpp)
target_link_libraries(tests PRIVATE squad_core Catch2::Catch2WithMain)