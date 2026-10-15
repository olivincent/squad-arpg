# ./CMakeLists.txt
cmake_minimum_required(VERSION 4.4.3)

project(SquadArpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)
find_package(Catch2 3 REQUIRED)

# Avertissements du compilateur (pas -Werror). Non appliqués à la cible
# tests, pour ne pas hériter des avertissements des macros de Catch2.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(AVERTISSEMENTS -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
elseif(MSVC)
    set(AVERTISSEMENTS /W4)
else()
    set(AVERTISSEMENTS)
endif()

add_library(squad_core STATIC
    src/TileMap.cpp
    src/Escouade.cpp
    src/Provocations.cpp
    src/Agresseurs.cpp
    src/Chemin.cpp
    src/Personnage.cpp
    src/Deplacement.cpp
    src/Monstre.cpp
    src/Skill.cpp
    src/Mobile.cpp
    src/Cercle.cpp
    src/Autodefense.cpp
    src/BarreDeSkills.cpp
    src/EcranDeDefaite.cpp
    src/MonstreVu.cpp
    src/BarreDeVie.cpp
    src/Eclaircir.cpp
    src/PointsDeVie.cpp
    src/Defense.cpp
    src/Niveau.cpp
    src/GenerateurDonjon.cpp
    src/Partie.cpp
    src/Controleur.cpp
)
target_include_directories(squad_core PUBLIC src)
target_link_libraries(squad_core PUBLIC SFML::Graphics)
target_compile_options(squad_core PRIVATE ${AVERTISSEMENTS})

add_executable(main
    src/main.cpp
)
target_link_libraries(main PRIVATE squad_core)
target_compile_options(main PRIVATE ${AVERTISSEMENTS})

# Copie assets/ à côté de main.exe (le jeu charge ses fichiers par chemin relatif).
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>/assets)

add_executable(tests
    tests/test_TileMap.cpp
    tests/test_Chemin.cpp
    tests/test_Escouade_deplacement.cpp
    tests/test_Escouade_combat.cpp
    tests/test_Escouade_skills.cpp
    tests/test_Escouade_agresseurs.cpp
    tests/test_Escouade_vue.cpp
    tests/test_Provocations.cpp
    tests/test_Agresseurs.cpp
    tests/test_Deplacement.cpp
    tests/test_Monstre.cpp
    tests/test_Personnage.cpp
    tests/test_Skill.cpp
    tests/test_Mobile.cpp
    tests/test_Cercle.cpp
    tests/test_Autodefense.cpp
    tests/test_BarreDeSkills.cpp
    tests/test_MonstreVu.cpp
    tests/test_BarreDeVie.cpp
    tests/test_Eclaircir.cpp
    tests/test_PointsDeVie.cpp
    tests/test_Defense.cpp
    tests/test_GenerateurDonjon.cpp
    tests/test_Partie.cpp
    tests/test_Controleur.cpp
)
target_link_libraries(tests PRIVATE squad_core Catch2::Catch2WithMain)
