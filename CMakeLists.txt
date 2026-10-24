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
    src/monde/TileMap.cpp
    src/escouade/Escouade.cpp
    src/escouade/Composition.cpp
    src/escouade/Specialite.cpp
    src/escouade/SelectionDEscouade.cpp
    src/escouade/Provocations.cpp
    src/escouade/Agresseurs.cpp
    src/escouade/Separation.cpp
    src/escouade/Formation.cpp
    src/escouade/PlacesDAttaque.cpp
    src/escouade/Intention.cpp
    src/monde/Chemin.cpp
    src/escouade/Personnage.cpp
    src/monde/Deplacement.cpp
    src/monstres/Monstre.cpp
    src/combat/Skill.cpp
    src/monde/Mobile.cpp
    src/base/Cercle.cpp
    src/rendu/TraceDeChemin.cpp
    src/combat/Autodefense.cpp
    src/monde/Encerclement.cpp
    src/combat/Engagement.cpp
    src/interface/BarreDeRaccourcis.cpp
    src/interface/BarreDeSkills.cpp
    src/interface/ClicInterface.cpp
    src/interface/EcranDeFin.cpp
    src/interface/CommandeDeTouche.cpp
    src/interface/AffichageObjets.cpp
    src/interface/AffichageSkills.cpp
    src/interface/DispositionInventaire.cpp
    src/interface/EcranInventaire.cpp
    src/interface/EntreeDInterface.cpp
    src/interface/PileDEcrans.cpp
    src/combat/EntiteVue.cpp
    src/rendu/BarreDeVie.cpp
    src/base/Eclaircir.cpp
    src/combat/PointsDeVie.cpp
    src/combat/Vitalite.cpp
    src/combat/Defense.cpp
    src/combat/ResolutionDuCoup.cpp
    src/objets/Emplacement.cpp
    src/objets/CatalogueObjets.cpp
    src/objets/Equipement.cpp
    src/objets/Inventaire.cpp
    src/objets/PositionDObjet.cpp
    src/monde/Niveau.cpp
    src/monde/ObjetInteractif.cpp
    src/monde/ChampDeVision.cpp
    src/monde/Vision.cpp
    src/monde/GenerateurDonjon.cpp
    src/jeu/Partie.cpp
    src/interface/TextesFlottants.cpp
    src/interface/Controleur.cpp
    src/interface/Reglages.cpp
    src/interface/TailleDeVue.cpp
    src/interface/EcranJeu.cpp
    src/interface/ListeDeBoutons.cpp
    src/interface/EcranChoixEscouade.cpp
    src/interface/EcranMenu.cpp
    src/interface/EcranOptions.cpp
)
# Les #include donnent le chemin depuis src/ ("combat/Skill.hpp"), y compris
# à l'intérieur d'un dossier.
target_include_directories(squad_core PUBLIC src)
target_link_libraries(squad_core PUBLIC SFML::Graphics)
target_compile_options(squad_core PRIVATE ${AVERTISSEMENTS})

add_executable(main
    src/app/main.cpp
    src/app/Application.cpp
)
target_link_libraries(main PRIVATE squad_core)
target_compile_options(main PRIVATE ${AVERTISSEMENTS})

# Copie assets/ à côté de main.exe (le jeu charge ses fichiers par chemin relatif).
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>/assets)

add_executable(tests
    tests/monde/test_TileMap.cpp
    tests/monde/test_Chemin.cpp
    tests/escouade/test_Escouade_deplacement.cpp
    tests/escouade/test_Escouade_combat.cpp
    tests/escouade/test_Escouade_skills.cpp
    tests/escouade/test_Escouade_skills_degats.cpp
    tests/escouade/test_Escouade_skills_soin.cpp
    tests/escouade/test_Escouade_skills_provocation.cpp
    tests/escouade/test_Escouade_skills_zone.cpp
    tests/escouade/test_Escouade_agresseurs.cpp
    tests/escouade/test_Escouade_vue.cpp
    tests/escouade/test_Escouade_places.cpp
    tests/escouade/test_Escouade_degagement.cpp
    tests/escouade/test_Escouade_equipement.cpp
    tests/escouade/test_Composition.cpp
    tests/escouade/test_Specialite.cpp
    tests/escouade/test_SelectionDEscouade.cpp
    tests/escouade/test_Provocations.cpp
    tests/escouade/test_Agresseurs.cpp
    tests/escouade/test_Separation.cpp
    tests/escouade/test_Formation.cpp
    tests/escouade/test_PlacesDAttaque.cpp
    tests/escouade/test_Intention.cpp
    tests/monde/test_Deplacement.cpp
    tests/monstres/test_Monstre.cpp
    tests/monstres/test_Monstre_etats.cpp
    tests/monstres/test_Monstre_retour.cpp
    tests/monstres/test_Monstre_engagement.cpp
    tests/escouade/test_Personnage.cpp
    tests/combat/test_Skill.cpp
    tests/monde/test_Mobile.cpp
    tests/base/test_Cercle.cpp
    tests/combat/test_Autodefense.cpp
    tests/monde/test_Encerclement.cpp
    tests/combat/test_Engagement.cpp
    tests/interface/test_AffichageObjets.cpp
    tests/interface/test_AffichageSkills.cpp
    tests/interface/test_BarreDeRaccourcis.cpp
    tests/interface/test_BarreDeSkills.cpp
    tests/interface/test_ClicInterface.cpp
    tests/interface/test_CommandeDeTouche.cpp
    tests/interface/test_DispositionInventaire.cpp
    tests/interface/test_EcranInventaire.cpp
    tests/interface/test_PileDEcrans.cpp
    tests/combat/test_EntiteVue.cpp
    tests/rendu/test_BarreDeVie.cpp
    tests/base/test_Eclaircir.cpp
    tests/combat/test_PointsDeVie.cpp
    tests/combat/test_Vitalite.cpp
    tests/combat/test_Defense.cpp
    tests/combat/test_ResolutionDuCoup.cpp
    tests/objets/test_Emplacement.cpp
    tests/objets/test_CatalogueObjets.cpp
    tests/objets/test_Equipement.cpp
    tests/objets/test_Inventaire.cpp
    tests/objets/test_PositionDObjet.cpp
    tests/monde/test_GenerateurDonjon.cpp
    tests/jeu/test_Partie_clics.cpp
    tests/jeu/test_Partie_skills.cpp
    tests/jeu/test_Partie_monstres.cpp
    tests/jeu/test_Partie_fin.cpp
    tests/jeu/test_Partie_portes.cpp
    tests/jeu/test_Partie_zonesDeControle.cpp
    tests/jeu/test_Partie_evenements.cpp
    tests/interface/test_TextesFlottants.cpp
    tests/monde/test_ObjetInteractif.cpp
    tests/monde/test_ChampDeVision.cpp
    tests/monde/test_Vision.cpp
    tests/interface/test_Controleur.cpp
    tests/interface/test_Reglages.cpp
    tests/interface/test_TailleDeVue.cpp
    tests/interface/test_ListeDeBoutons.cpp
    tests/interface/test_EcranChoixEscouade.cpp
    tests/interface/test_EcranMenu.cpp
    tests/interface/test_EcranOptions.cpp
)
# Les tests incluent "aides.hpp" (etc.) depuis la racine de tests/.
target_include_directories(tests PRIVATE tests)
target_link_libraries(tests PRIVATE squad_core Catch2::Catch2WithMain)
target_compile_options(tests PRIVATE ${AVERTISSEMENTS})

# CTest : chaque TEST_CASE devient un test CTest (ctest --preset debug).
# Découverte au lancement de ctest (PRE_TEST) plutôt qu'à la fin de la
# compilation : le build ne lance jamais tests.exe.
enable_testing()
include(Catch)
catch_discover_tests(tests DISCOVERY_MODE PRE_TEST)
