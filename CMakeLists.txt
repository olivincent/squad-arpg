# ./CMakeLists.txt
cmake_minimum_required(VERSION 4.4.3)

project(SquadArpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)
find_package(Catch2 3 REQUIRED)

add_library(squad_core STATIC src/TileMap.cpp src/Escouade.cpp src/Chemin.cpp src/Personnage.cpp src/Deplacement.cpp src/Monstre.cpp src/Skill.cpp src/Mobile.cpp src/Cercle.cpp src/Autodefense.cpp src/BarreDeSkills.cpp)
target_include_directories(squad_core PUBLIC src)
target_link_libraries(squad_core PUBLIC SFML::Graphics)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE squad_core)

# Copie assets/ à côté de main.exe (le jeu charge ses fichiers par chemin relatif).
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>/assets)

add_executable(tests tests/test_TileMap.cpp tests/test_Chemin.cpp tests/test_Escouade.cpp tests/test_Deplacement.cpp tests/test_Monstre.cpp tests/test_Personnage.cpp tests/test_Skill.cpp tests/test_Mobile.cpp tests/test_Cercle.cpp tests/test_Autodefense.cpp tests/test_BarreDeSkills.cpp)
target_link_libraries(tests PRIVATE squad_core Catch2::Catch2WithMain)