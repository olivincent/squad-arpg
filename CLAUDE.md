# Squad-ARPG — Instructions pour Claude Code

Ce fichier est lu automatiquement par Claude Code au début de chaque
session. Il décrit comment travailler sur ce projet.

Documents de référence (à lire avant toute modification) :

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
2. **Compiler après chaque sous-étape** (`cmake --build build`) et corriger
   les erreurs et avertissements avant de continuer.
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
5. **Ne jamais anticiper.** Rien « pour plus tard » : pas de fonction,
   paramètre, membre ou dépendance dont l'étape en cours n'a pas besoin.
   Le code devenu inutile est supprimé (Git le garde).
6. **Migrations sans casse** : quand on déplace du code, on le déplace sans
   le modifier, et les améliorations se font dans un commit séparé.
7. **Git** : ne pas faire de commit soi-même. Proposer la commande
   `git add` / `git commit -m "..."` ; Olivier la lance après avoir testé.
8. **Fin d'une étape complète** : quand Olivier le demande, mettre à
   jour :
   - `ROADMAP.md` : étape cochée, section « État actuel » ;
   - `ARCHITECTURE.md` : le **quoi** — structure du dépôt, modules,
     données membres, interfaces publiques ;
   - les fichiers de `docs/` correspondant aux modules modifiés
     pendant l'étape (tableau à la fin d'`ARCHITECTURE.md`) : le
     **pourquoi** — décisions, compromis, limites connues.
   Ne pas dupliquer le détail d'un fichier de `docs/` dans
   `ARCHITECTURE.md`. Un nouveau fichier dans `docs/` seulement si
   aucun fichier existant ne couvre le sujet ; l'ajouter alors au
   tableau. Terminer en listant les fichiers de documentation
   modifiés.

## Conventions de code

- C++20, noms en **français** (classes, fonctions, variables), membres
  préfixés par `m_`.
- Première ligne de chaque fichier : son chemin en commentaire
  (`// ./src/Escouade.hpp`, `# ./CMakeLists.txt`).
- `#pragma once` dans les en-têtes.
- Fonctions membres de lecture `const` ; objets lourds en lecture passés
  par `const T&` ; petits types (`sf::Vector2f`, `float`) par valeur.
- Constructeur à un seul paramètre : `explicit`.
- Chaque fichier inclut lui-même les en-têtes des types qu'il utilise ;
  déclaration anticipée (`class X;`) dans un `.hpp` quand une référence
  suffit, include complet dans le `.cpp`.
- État de jeu stocké dans des données (positions, tailles…), pas dans des
  objets graphiques SFML ; les formes SFML sont créées dans `dessiner`.
- Coordonnées en pixels du **monde**, jamais de l'écran.
- Tout nouveau `.cpp` de logique de jeu est ajouté à `add_library(squad_core
  ...)` dans `CMakeLists.txt` ; un nouveau fichier de test est ajouté à
  `add_executable(tests ...)`.

## Commandes

Environnement : Windows, MSYS2 UCRT64, MinGW-w64, CMake + Ninja, SFML 3.0.2.

```bash
cmake -S . -B build -G Ninja   # configuration (une seule fois, ou si build/ est supprimé)
cmake --build build            # compilation
./build/main.exe               # exécution (c'est Olivier qui lance le jeu pour tester)
```

Le dossier `build/` n'est jamais modifié à la main ni commité.
