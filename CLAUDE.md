# Squad-ARPG — Instructions pour Claude Code

Ce fichier est lu automatiquement par Claude Code au début de chaque
session. Il décrit comment travailler sur ce projet.

Documents de référence (à lire avant toute modification), en version
courte : dossiers, une ligne par module et règles de dépendance ; état
résumé et étapes à venir. Le détail d'un module (données membres,
fonctions publiques, décisions, limites) est dans le fichier de `docs/`
de son sujet (tableau à la fin d'`ARCHITECTURE.md`), à lire avant de le
modifier.

@ARCHITECTURE.md
@ROADMAP.md

## Contexte et répartition des rôles

- Squad-ARPG est d'abord un **projet d'apprentissage du C++ et du génie
  logiciel** pour Olivier, pas un jeu à livrer vite. Comprendre le code
  compte plus qu'aller vite.
- Les discussions de conception (génie logiciel et design de jeu) ont lieu
  dans Claude.ai. Elles se terminent par un **prompt** qu'Olivier colle ici.
- Claude Code **implémente ce prompt, et seulement ce prompt**. Si quelque
  chose hors de sa portée semble nécessaire, le signaler et le proposer,
  sans le faire.

## Façon de travailler

1. **Une sous-étape à la fois.** D'abord la structure de données (les
   membres), puis les fonctions une par une. Déclaration dans le `.hpp`
   avant l'implémentation dans le `.cpp`.
2. **Compiler après chaque sous-étape** (les deux presets, `debug` et `jeu`),
   lancer les tests (`ctest --preset debug`) et corriger les erreurs et
   avertissements avant de continuer.
3. **S'arrêter après chaque sous-étape** et donner à Olivier un résumé
   court :
   - ce qui a changé et **pourquoi**, en quelques lignes ;
   - l'explication des notions de C++ ou de conception **nouvelles**
     dans cette sous-étape (les notions déjà rencontrées dans le projet
     ne sont pas réexpliquées, sauf si Olivier le demande) ;
   - ce qu'il doit **tester visuellement** en lançant le jeu (Claude Code
     ne voit pas la fenêtre du jeu) ;
   - le message de commit proposé.
   Puis attendre sa confirmation avant la sous-étape suivante.
4. **Sous-étapes enchaînables.** Si un prompt marque une sous-étape
   « (enchaînable) » parce qu'elle n'a rien à tester visuellement, la
   réaliser, compiler, lancer les tests, puis passer directement à la
   sous-étape suivante sans s'arrêter. Au prochain arrêt, le résumé
   couvre tout le groupe de sous-étapes enchaînées, et un seul commit
   est proposé pour l'ensemble. S'arrêter quand même si la compilation
   ou un test échoue, ou si quelque chose d'imprévu se présente. Sans
   cette mention, la règle précédente (un arrêt par sous-étape)
   s'applique. Ne jamais décider soi-même qu'une sous-étape est
   enchaînable : seule la mention dans le prompt compte.
5. **Anticiper la structure, pas les fonctionnalités.** Rien « pour plus
   tard » côté fonctionnalités : pas de fonction, paramètre, membre ou
   dépendance dont l'étape en cours n'a pas besoin. Mais quand une extension
   est connue (prompt, ROADMAP.md, `docs/brainstorm.md`), on utilise la
   structure qui l'accueillera sans réécriture (voir le principe 1 d'
   `ARCHITECTURE.md`), à condition qu'elle serve dès l'étape présente. Le
   code devenu inutile est supprimé (Git le garde).
6. **Migrations sans casse** : quand on déplace du code, on le déplace sans
   le modifier, et les améliorations se font dans un commit séparé.
7. **Git** : ne pas faire de commit soi-même. Proposer la commande
   `git add` / `git commit -m "..."` ; Olivier la lance après avoir testé.
8. **Fin d'une étape complète** : quand Olivier le demande, mettre à
   jour :
   - `ROADMAP.md` (version courte) : étape cochée, « État actuel » résumé,
     étapes à venir ;
   - `ARCHITECTURE.md` (version courte) : arbre des dossiers, une ligne
     par module, règles de dépendance et leurs exceptions ;
   - les fichiers de `docs/` correspondant aux modules modifiés
     pendant l'étape (tableau à la fin d'`ARCHITECTURE.md`) : le
     **quoi** détaillé (section « Référence des modules » : données
     membres, interfaces publiques) et le **pourquoi** — décisions,
     compromis, limites connues ;
   - `docs/historique.md` : l'étape dans la liste et son résumé.
   Ne pas dupliquer le détail d'un fichier de `docs/` dans
   `ARCHITECTURE.md` ni dans `ROADMAP.md`. Un nouveau fichier dans `docs/` seulement si
   aucun fichier existant ne couvre le sujet ; l'ajouter alors au
   tableau. Terminer en listant les fichiers de documentation
   modifiés.

## Conventions de code

- C++20, noms en **français** (classes, fonctions, variables), membres
  préfixés par `m_`.
- Première ligne de chaque fichier : son chemin en commentaire
  (`// ./src/escouade/Escouade.hpp`, `# ./CMakeLists.txt`).
- `#include` du projet à chemin complet depuis `src/`, même à l'intérieur
  d'un dossier (`#include "combat/Skill.hpp"`) ; les tests incluent leurs
  aides depuis la racine de `tests/` (`#include "aides.hpp"`).
- `#pragma once` dans les en-têtes.
- Fonctions membres de lecture `const` ; objets lourds en lecture passés
  par `const T&` ; petits types (`sf::Vector2f`, `float`) par valeur.
- `[[nodiscard]]` : tout retour non `void` d'une fonction sans effet de bord,
  ou dont le retour signale un échec (`engagerSkill`, `Skill::commencer`), est
  `[[nodiscard]]`. Un échec normal qu'on ignore exprès s'écrit
  `static_cast<void>(...)` avec un commentaire qui dit pourquoi ; un échec
  impossible s'écrit `[[maybe_unused]] const bool ok = ...; assert(ok);`.
  `clang-tidy` (`modernize-use-nodiscard`) vérifie les fonctions membres.
- `Cercle` (12 octets) est toujours passé **par valeur**, comme un petit
  type, y compris dans `Cercle.hpp` et `Deplacement.hpp`. Seul un
  `std::vector<Cercle>` se passe par `const&`.
- Constantes nommées : préfixe `s_` (`s_porteeMelee`), `constexpr` dans un
  namespace anonyme du `.cpp` (`static constexpr` pour un membre de
  classe). Pas de majuscules (`COTE_LIGNE_DE_VUE`) ni de littéral local
  répété : une valeur qui sert deux fois a un nom.
- Constructeur à un seul paramètre : `explicit`.
- Noms de `TEST_CASE` **sans accents ni cédilles** (« a portee », « degats »,
  « deja »), pour pouvoir les filtrer en ligne de commande dans n'importe
  quel terminal ; les commentaires gardent leurs accents.
- Chaque fichier inclut lui-même les en-têtes des types qu'il utilise ;
  déclaration anticipée (`class X;`) dans un `.hpp` quand une référence
  suffit, include complet dans le `.cpp`.
- État de jeu stocké dans des données (positions, tailles…), pas dans des
  objets graphiques SFML ; les formes SFML sont créées dans `dessiner`.
- Coordonnées en pixels du **monde**, jamais de l'écran.
- Chaque nouveau fichier naît dans le dossier de sa famille (`src/base/`,
  `src/monde/`, `src/combat/`… ; règles de dépendance dans
  `ARCHITECTURE.md`) et s'ajoute à `add_library(squad_core ...)` avec son
  chemin (`src/combat/Skill.cpp`). Un nouveau fichier de test va dans le
  dossier miroir de `tests/` et s'ajoute à `add_executable(tests ...)`.

## Commandes

Environnement : Windows, MSYS2 UCRT64, MinGW-w64, CMake + Ninja, SFML 3.0.2,
clang-tools-extra (clang-tidy, clang-format). Deux presets (`CMakePresets.json`,
voir `docs/build.md`) : `debug` (tests, `assert`, assertions de la bibliothèque
standard, UBSan) dans `build/debug`, `jeu` (Release) dans `build/jeu`.

```bash
cmake --preset debug && cmake --build --preset debug   # configuration (une fois) et compilation
cmake --preset jeu && cmake --build --preset jeu
ctest --preset debug                                    # toute la suite, en parallèle
./build/debug/tests.exe "*a portee*"                    # quelques tests, par filtre
./build/jeu/main.exe                                    # exécution (c'est Olivier qui lance le jeu pour tester)
clang-tidy -p build/jeu src/escouade/Escouade.cpp       # analyse statique d'une source (règles : .clang-tidy)
```

Le dossier `build/` n'est jamais modifié à la main ni commité. Les deux presets
doivent compiler sans aucun avertissement, tests compris.
