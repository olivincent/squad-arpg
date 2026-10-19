# ./CMakeLists.txt
# 3.20 : la plus petite version qui suffit (CMakePresets.json en version 2,
# pour les presets de build et de test ; catch_discover_tests demande 3.19).
# 4.4 : la dernière version essayée, dont on garde le comportement (policies).
cmake_minimum_required(VERSION 3.20...4.4)

project(SquadArpg LANGUAGES CXX)

# Type de build par défaut quand aucun n'est donné (configuration sans
# preset) : Release. Les presets « debug » et « jeu » le fixent eux-mêmes.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Type de build" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Le projet n'utilise pas les modules C++20 : pas de recherche de modules. Elle
# ajoute des étapes au build et des options propres à GCC (-fmodules-ts…) dans
# compile_commands.json, que clang-tidy refuse.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)
find_package(Catch2 3 REQUIRED)

# Avertissements du compilateur (pas -Werror), sur les trois cibles. Ceux des
# en-têtes de Catch2 et de SFML ne sortent pas : ce sont des en-têtes système
# (dossier par défaut du compilateur, cibles importées par find_package).
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
    src/Composition.cpp
    src/Specialite.cpp
    src/SelectionDEscouade.cpp
    src/Provocations.cpp
    src/Agresseurs.cpp
    src/Chemin.cpp
    src/Personnage.cpp
    src/Deplacement.cpp
    src/Monstre.cpp
    src/Skill.cpp
    src/Mobile.cpp
    src/Cercle.cpp
    src/TraceDeChemin.cpp
    src/Autodefense.cpp
    src/Encerclement.cpp
    src/Engagement.cpp
    src/BarreDeRaccourcis.cpp
    src/BarreDeSkills.cpp
    src/ClicInterface.cpp
    src/EcranDeFin.cpp
    src/EntiteVue.cpp
    src/BarreDeVie.cpp
    src/Eclaircir.cpp
    src/PointsDeVie.cpp
    src/Defense.cpp
    src/Niveau.cpp
    src/ObjetInteractif.cpp
    src/ChampDeVision.cpp
    src/Vision.cpp
    src/GenerateurDonjon.cpp
    src/Partie.cpp
    src/TextesFlottants.cpp
    src/Controleur.cpp
    src/Reglages.cpp
    src/TailleDeVue.cpp
    src/EcranJeu.cpp
    src/ListeDeBoutons.cpp
    src/EcranChoixEscouade.cpp
    src/EcranMenu.cpp
    src/EcranOptions.cpp
)
target_include_directories(squad_core PUBLIC src)
target_link_libraries(squad_core PUBLIC SFML::Graphics)
target_compile_options(squad_core PRIVATE ${AVERTISSEMENTS})

add_executable(main
    src/main.cpp
    src/Application.cpp
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
    tests/test_Escouade_skills_degats.cpp
    tests/test_Escouade_skills_soin.cpp
    tests/test_Escouade_skills_provocation.cpp
    tests/test_Escouade_skills_zone.cpp
    tests/test_Escouade_agresseurs.cpp
    tests/test_Escouade_vue.cpp
    tests/test_Escouade_places.cpp
    tests/test_Escouade_degagement.cpp
    tests/test_Composition.cpp
    tests/test_Specialite.cpp
    tests/test_SelectionDEscouade.cpp
    tests/test_Provocations.cpp
    tests/test_Agresseurs.cpp
    tests/test_Deplacement.cpp
    tests/test_Monstre.cpp
    tests/test_Monstre_etats.cpp
    tests/test_Monstre_retour.cpp
    tests/test_Monstre_engagement.cpp
    tests/test_Personnage.cpp
    tests/test_Skill.cpp
    tests/test_Mobile.cpp
    tests/test_Cercle.cpp
    tests/test_Autodefense.cpp
    tests/test_Encerclement.cpp
    tests/test_Engagement.cpp
    tests/test_BarreDeRaccourcis.cpp
    tests/test_BarreDeSkills.cpp
    tests/test_ClicInterface.cpp
    tests/test_EntiteVue.cpp
    tests/test_BarreDeVie.cpp
    tests/test_Eclaircir.cpp
    tests/test_PointsDeVie.cpp
    tests/test_Defense.cpp
    tests/test_GenerateurDonjon.cpp
    tests/test_Partie_clics.cpp
    tests/test_Partie_skills.cpp
    tests/test_Partie_monstres.cpp
    tests/test_Partie_fin.cpp
    tests/test_Partie_portes.cpp
    tests/test_Partie_zonesDeControle.cpp
    tests/test_Partie_evenements.cpp
    tests/test_TextesFlottants.cpp
    tests/test_ObjetInteractif.cpp
    tests/test_ChampDeVision.cpp
    tests/test_Vision.cpp
    tests/test_Controleur.cpp
    tests/test_Reglages.cpp
    tests/test_TailleDeVue.cpp
    tests/test_ListeDeBoutons.cpp
    tests/test_EcranChoixEscouade.cpp
    tests/test_EcranMenu.cpp
    tests/test_EcranOptions.cpp
)
target_link_libraries(tests PRIVATE squad_core Catch2::Catch2WithMain)
target_compile_options(tests PRIVATE ${AVERTISSEMENTS})

# CTest : chaque TEST_CASE devient un test CTest (ctest --preset debug).
# Découverte au lancement de ctest (PRE_TEST) plutôt qu'à la fin de la
# compilation : le build ne lance jamais tests.exe.
enable_testing()
include(Catch)
catch_discover_tests(tests DISCOVERY_MODE PRE_TEST)
