# Squad-ARPG — Build

## Build

- **CMake minimal** : `cmake_minimum_required`, `project()`,
  `CMAKE_CXX_STANDARD` = 20 (`CMAKE_CXX_STANDARD_REQUIRED` = ON).
  - **`cmake_minimum_required(VERSION 3.20...4.4)`** (étape 80) : 3.20 est la plus petite version
    qui suffit (`CMakePresets.json` en version 2, pour les presets de build et de test ;
    `catch_discover_tests` demande 3.19). La borne haute (4.4, la version installée) garde le
    comportement des règles de CMake (*policies*) de cette version. Avant, la version exacte
    installée (4.4.3) était exigée sans raison.
  - **Type de build par défaut : Release** (`if(NOT CMAKE_BUILD_TYPE)`), pour une configuration
    sans preset. Jusqu'à l'étape 79, `build/` n'avait aucun type : ni optimisation ni `NDEBUG`
    (24 monstres en poursuite coûtaient 31 ms par image au lieu de 2 ms).
  - **`CMAKE_CXX_SCAN_FOR_MODULES OFF`** (étape 80) : le projet n'utilise pas les modules C++20.
    La recherche de modules ajoutait des étapes au build et, dans `compile_commands.json`, des
    options propres à GCC (`-fmodules-ts`, `-fmodule-mapper=…`, `-fdeps-format=…`) que clang
    (donc `clang-tidy`) refuse.
- **`find_package`** regroupés en haut du fichier : SFML
  (`find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)`) et Catch2
  (`find_package(Catch2 3 REQUIRED)`).
- **Bibliothèque `squad_core`** (`add_library(squad_core STATIC ...)`,
  une source par ligne pour que chaque ajout ne change qu'une ligne du
  diff) : regroupe la logique de jeu, réutilisée par `main` et par `tests`.
  Depuis l'étape 81, les sources sont rangées en dossiers par domaine (`src/combat/Skill.cpp`,
  voir `ARCHITECTURE.md`) et listées avec leur dossier ; il reste **une seule** bibliothèque. Une
  bibliothèque par dossier, qui rendrait une inclusion montante impossible à compiler, attend la
  fin des exceptions transitoires de M1 (`docs/audit.md`, « Organisation en dossiers »).
  - `target_link_libraries(squad_core PUBLIC SFML::Graphics)` : `PUBLIC`
    parce que les en-têtes de `squad_core` (`monde/TileMap.hpp`, `escouade/Escouade.hpp`)
    incluent des en-têtes SFML — cette dépendance fait partie de son
    interface et doit se propager à qui la lie.
  - `target_include_directories(squad_core PUBLIC src)` : même raison,
    pour que `tests/` puisse faire `#include "monde/TileMap.hpp"` sans connaître
    le chemin relatif vers `src/`. C'est le seul dossier d'inclusion : chaque `#include` donne le
    chemin depuis `src/`, y compris à l'intérieur d'un même dossier, pour que le dossier se lise
    dans le code et que les règles de dépendance se vérifient par `grep`. (Pendant le déplacement
    de l'étape 81, les sous-dossiers ont été ajoutés un temps à la liste, pour un commit `git mv`
    sans changement de code, puis retirés.)
- **`main`** (`add_executable(main src/app/main.cpp src/app/Application.cpp)`) se lie à `squad_core`
  en `PRIVATE` (`target_link_libraries(main PRIVATE squad_core)`) :
  `PRIVATE` parce que rien d'autre ne relie jamais `main`, un exécutable
  final.
  - **Assets** : une commande `POST_BUILD` (`add_custom_command(TARGET
    main ...)`, `cmake -E copy_directory`) copie `assets/` à côté de
    `main.exe` (`$<TARGET_FILE_DIR:main>`). Le jeu charge la police par
    chemin relatif (`assets/fonts/DejaVuSans.ttf`) : il doit être lancé
    depuis le dossier de `main.exe`. La copie n'est refaite qu'à la
    recompilation de `main`.
- **Avertissements** (étape 49) : `-Wall -Wextra -Wpedantic -Wshadow
  -Wconversion` (GCC/Clang ; `/W4` pour MSVC) par `target_compile_options`
  sur `squad_core`, `main` et, depuis l'étape 80, `tests`, regroupés dans la variable
  `AVERTISSEMENTS`. Pas de `-Werror` : un avertissement ne bloque pas le build, mais le build
  doit en rester sans, **dans les deux presets** (le preset `jeu` en a révélé un : une fonction
  utilisée seulement par un `assert`, marquée `[[maybe_unused]]`). Les en-têtes de Catch2 et de
  SFML ne produisent rien : ce sont déjà des en-têtes système (dossier par défaut du compilateur,
  `/ucrt64/include`, et cibles importées par `find_package`, `SYSTEM` par défaut), sans ligne à
  ajouter. Pas de `-Wsign-conversion` (il viendra avec une classe `Grille`, voir l'audit).
- **`tests`** (`add_executable(tests ...)`, un fichier par ligne) se lie à `squad_core` et à
  `Catch2::Catch2WithMain` (qui fournit le `main()` des tests). Les fichiers de tests sont rangés
  en miroir de `src/` (`tests/escouade/test_Escouade_combat.cpp`) ; les aides (`aides.hpp`,
  `aides_escouade.hpp`, `aides_monstre.hpp`) restent à la racine de `tests/`, ajoutée par
  `target_include_directories(tests PRIVATE tests)` : un test de n'importe quel dossier écrit
  `#include "aides.hpp"`.
- **Générateur Ninja**, choisi par les presets. Ninja relance la configuration automatiquement
  quand `CMakeLists.txt` change.

## Presets (étape 80)

`CMakePresets.json` (version 2) remplace les options à retaper. Un preset caché `commun`
(Ninja, dossier `build/<nom du preset>`, `CMAKE_EXPORT_COMPILE_COMMANDS` pour les outils clang),
dont héritent :

| Preset | Dossier | Type | En plus |
|---|---|---|---|
| `debug` | `build/debug` | Debug (`-O0 -g`, `assert` actifs) | `-D_GLIBCXX_ASSERTIONS` (bornes des conteneurs de la bibliothèque standard vérifiées), UBSan en mode piège (`-fsanitize=undefined -fsanitize-undefined-trap-on-error`) |
| `jeu` | `build/jeu` | Release (`-O3`, `NDEBUG`) | — |

- **UBSan en mode piège** : le compilateur ajoute des vérifications de comportement indéfini
  (débordement d'entier signé, décalage invalide…) ; au premier cas, le programme s'arrête sur une
  instruction illégale. Ce mode n'a pas besoin de `libubsan`, absente de MinGW. Il vit dans le preset
  `debug`, sans option CMake à part.
- **Presets de build** (`debug`, `jeu`) et **de test** (`debug`). VSCode (CMake Tools) les détecte
  et propose de choisir un preset de configuration.
- Commandes :

  ```bash
  cmake --preset jeu && cmake --build --preset jeu     # le jeu : ./build/jeu/main.exe
  cmake --preset debug && cmake --build --preset debug # les tests
  ctest --preset debug                                  # toute la suite
  ```

  Le jeu charge la police et `reglages.ini` depuis le dossier courant : le lancer depuis le dossier
  de son `main.exe`, ou depuis la racine du dépôt (qui a aussi `assets/`).
- **Fichiers JSON sans commentaire** : `CMakePresets.json` n'a pas sa ligne de chemin (le JSON
  n'accepte pas de commentaire). Le nombre de tâches de ctest (12) y est écrit en dur : c'est le
  nombre de cœurs de la machine de développement.

## CTest (étape 80)

`enable_testing()`, `include(Catch)` et `catch_discover_tests(tests DISCOVERY_MODE PRE_TEST)` :
chaque `TEST_CASE` devient un test CTest (893 à l'étape 80), lancé dans son propre processus.
- **`PRE_TEST`** plutôt que le mode par défaut (`POST_BUILD`) : la liste des tests est lue au
  lancement de ctest, pas à la fin de la compilation. Le build ne lance donc jamais `tests.exe`, qui
  échouerait si les DLL d'UCRT64 (SFML, Catch2, libstdc++) n'étaient pas dans le `PATH` du build.
- **Durée** : un processus par test coûte environ 90 ms de démarrage ; en série, la suite prend
  97 s, contre 18 s pour `tests.exe` seul. Avec 12 tâches en parallèle (`execution.jobs` du preset
  de test), 9 à 10 s.
- **Noms sans accents** (règle de l'étape 50) : ctest transmet le nom du test à `tests.exe` ; un nom
  accentué arrive mal encodé, Catch2 ne trouve aucun test et ctest compte un échec. C'est ainsi que
  le dernier nom accentué (« posé ») a été trouvé.
- `tests.exe` reste utilisable seul, avec ses filtres : `./build/debug/tests.exe "*a portee*"`.

## clang-format et clang-tidy (étape 80)

Paquet MSYS2 `mingw-w64-ucrt-x86_64-clang-tools-extra` (LLVM 23).
- **`.clang-format`** : base Google, 4 espaces, 120 colonnes, en-têtes regroupés et triés
  (`IncludeBlocks: Regroup`), `public:` en colonne 0 comme dans le code existant. **Le code existant
  n'a pas été reformaté** : l'outil sert au code nouveau ou retouché ; un reformatage complet serait
  un commit à part, de formatage pur.
- **`.clang-tidy`** : `bugprone-*`, `cppcoreguidelines-*`, `modernize-use-nodiscard`,
  `performance-*`, moins les règles trop bavardes ou sans objet ici, chacune avec sa raison en
  commentaire (paramètres voisins de même type, nombres magiques, `gsl::at`, alias en double,
  `noexcept`, constantes globales de type classe, taille des `enum`). Les en-têtes de `src/` sont
  analysés (`HeaderFilterRegex` qui accepte `/` et `\`), sous-dossiers compris.
- **Commande**, une source à la fois, depuis la racine : `clang-tidy -p build/jeu src/escouade/Escouade.cpp`
  (base de compilation du preset `jeu`). Le décompte par règle est dans `docs/audit.md`
  (« clang-tidy, premier passage »).
- **Limite connue** : l'`assert` de MinGW appelle `_assert`, qui n'est pas déclaré `noreturn` ;
  `bugprone-unchecked-optional-access` ne voit donc pas un `assert` comme une vérification et signale
  l'accès qui le suit (7 cas, tous des invariants vérifiés par un `assert`).

## Choix de SFML plutôt que raylib

SFML a été choisie plutôt que raylib parce qu'elle est orientée objet
(`sf::RenderWindow`, `sf::RectangleShape`, `sf::Text`, `sf::Event`), ce qui
correspond à l'objectif d'apprendre le C++ et le génie logiciel (classes,
encapsulation). raylib est plus proche du C, avec des fonctions libres.
SFML gère aussi nativement le texte (police + rendu).

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Qualité du code** (étapes 49 et 80) : `squad_core`, `main` et `tests` se compilent avec
  `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` sans aucun avertissement, dans les deux presets
  (pas de `-Werror`) ; `[[nodiscard]]` sur tout retour d'une fonction sans effet de bord ou qui
  signale un échec (règle écrite dans `CLAUDE.md`) ; `assert` devant les déréférencements qui
  reposent sur un invariant ; `Cercle` passé par valeur ; constantes en `s_`. **Build** (étape 80) :
  presets `debug` (assertions de la bibliothèque standard, UBSan) et `jeu` (Release), CTest,
  `.clang-format` (sans reformatage du code existant) et `.clang-tidy` (59 avertissements restants,
  aucun corrigé hors de `[[nodiscard]]` et des accès à un `optional` ; décompte dans `docs/audit.md`).
  `dt` borné à 1/20 s dans `Application` : après un gel, le jeu ralentit au lieu de faire traverser
  les murs. `coutDesObstacles` ne visite plus que le rectangle englobant de chaque obstacle (même
  résultat ; 24 monstres en poursuite : 50 ms → 3,9 ms par image en debug, 2,35 → 0,34 ms en jeu).
  Voir `docs/build.md`. Barre de vie
  des personnages et des monstres dessinée par le module `BarreDeVie`.
  Points de vie portés par la classe `PointsDeVie` (étape 53), possédée par
  `Personnage` et `Monstre`. `Skill` n'a plus que la machine à états
  (`commencer` / `avancer` / `annuler`) : `declencher` et
  `declencherSansCible` ont été supprimées à l'étape 62.
