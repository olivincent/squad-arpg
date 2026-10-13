# Squad-ARPG — Build

## Build

- **CMake minimal** : `cmake_minimum_required`, `project()`,
  `CMAKE_CXX_STANDARD` = 20 (`CMAKE_CXX_STANDARD_REQUIRED` = ON).
- **`find_package`** regroupés en haut du fichier : SFML
  (`find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)`) et Catch2
  (`find_package(Catch2 3 REQUIRED)`).
- **Bibliothèque `squad_core`** (`add_library(squad_core STATIC
  src/TileMap.cpp src/Escouade.cpp src/Chemin.cpp src/Personnage.cpp
  src/Deplacement.cpp src/Monstre.cpp src/Skill.cpp
  src/Mobile.cpp src/Cercle.cpp src/Autodefense.cpp
  src/BarreDeSkills.cpp)`) :
  regroupe la logique de jeu, réutilisée par `main` et par `tests`.
  - `target_link_libraries(squad_core PUBLIC SFML::Graphics)` : `PUBLIC`
    parce que les en-têtes de `squad_core` (`TileMap.hpp`, `Escouade.hpp`)
    incluent des en-têtes SFML — cette dépendance fait partie de son
    interface et doit se propager à qui la lie.
  - `target_include_directories(squad_core PUBLIC src)` : même raison,
    pour que `tests/` puisse faire `#include "TileMap.hpp"` sans connaître
    le chemin relatif vers `src/`.
- **`main`** (`add_executable(main src/main.cpp)`) se lie à `squad_core`
  en `PRIVATE` (`target_link_libraries(main PRIVATE squad_core)`) :
  `PRIVATE` parce que rien d'autre ne relie jamais `main`, un exécutable
  final.
  - **Assets** : une commande `POST_BUILD` (`add_custom_command(TARGET
    main ...)`, `cmake -E copy_directory`) copie `assets/` à côté de
    `main.exe` (`$<TARGET_FILE_DIR:main>`). Le jeu charge la police par
    chemin relatif (`assets/fonts/DejaVuSans.ttf`) : il doit être lancé
    depuis le dossier de `main.exe`. La copie n'est refaite qu'à la
    recompilation de `main`.
- **`tests`** (`add_executable(tests tests/test_TileMap.cpp
  tests/test_Chemin.cpp tests/test_Escouade.cpp
  tests/test_Deplacement.cpp tests/test_Monstre.cpp
  tests/test_Personnage.cpp
  tests/test_Skill.cpp tests/test_Mobile.cpp tests/test_Cercle.cpp
  tests/test_Autodefense.cpp tests/test_BarreDeSkills.cpp)`) se lie à `squad_core` et à
  `Catch2::Catch2WithMain` (qui fournit le `main()` des tests) ; lancé
  directement (`./build/tests.exe`), sans CTest.
- **Générateur Ninja** (`cmake -S . -B build -G Ninja`). Ninja relance la
  configuration automatiquement quand `CMakeLists.txt` change.

## Choix de SFML plutôt que raylib

SFML a été choisie plutôt que raylib parce qu'elle est orientée objet
(`sf::RenderWindow`, `sf::RectangleShape`, `sf::Text`, `sf::Event`), ce qui
correspond à l'objectif d'apprendre le C++ et le génie logiciel (classes,
encapsulation). raylib est plus proche du C, avec des fonctions libres.
SFML gère aussi nativement le texte (police + rendu).
