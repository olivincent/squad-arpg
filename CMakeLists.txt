# ./CMakeLists.txt
cmake_minimum_required(VERSION 4.4.3)

project(SquadArpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main src/main.cpp src/TileMap.cpp src/Escouade.cpp)

find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)
target_link_libraries(main PRIVATE SFML::Graphics)