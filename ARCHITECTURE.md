# Squad-ARPG — Architecture

## Concept du jeu

Un ARPG/RTS où le joueur contrôle une seule entité "escouade" représentant
plusieurs personnages, sans micro-gestion individuelle des membres.
Inspiration :
- **Aliens: Dark Descent** pour l'aspect escouade (un point de contrôle
  unique, l'escouade se déplace et agit comme un seul bloc)
- **Titan Quest** pour l'aspect ARPG

## Objectif du projet

Ce projet est avant tout un exercice d'apprentissage du C++ et du génie
logiciel — pas un jeu à livrer rapidement. Le concept d'escouade se prête
bien à l'apprentissage : il amène naturellement vers de l'encapsulation
propre (l'escouade comme classe, les membres comme composants internes)
sans complexité excessive pour démarrer.

## Environnement technique

| Élément | Choix |
|---|---|
| OS | Windows, console MSYS2 UCRT64 |
| IDE | VSCode |
| Compilateur | MinGW-w64 (via MSYS2 UCRT64) |
| Build | CMake + Ninja |
| Langage | C++20 |
| Bibliothèque graphique | SFML 3.0.2 |
| Tests | À introduire plus tard (Catch2 pressenti, pas encore décidé formellement) |

### Pourquoi SFML plutôt que raylib

SFML a été choisie plutôt que raylib parce qu'elle est orientée objet
(`sf::RenderWindow`, `sf::RectangleShape`, `sf::Text`, `sf::Event`), ce qui
correspond à l'objectif d'apprendre le C++ et le génie logiciel (classes,
encapsulation). raylib est plus proche du C, avec des fonctions libres.
SFML gère aussi nativement le texte (police + rendu), utile pour afficher
des informations à l'écran (ex. position du curseur/de l'escouade).

## Structure du dépôt (état actuel)

Le chemin de chaque fichier est indiqué en commentaire sur sa première
ligne (par exemple `// ./src/main.cpp` ou `# ./CMakeLists.txt`).

```
squad-arpg/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── TileMap.hpp
│   └── TileMap.cpp
├── .gitignore
├── ARCHITECTURE.md
└── ROADMAP.md
```

Un seul exécutable `main`. Premier module : `TileMap` (l'environnement de
jeu). La structure continue d'évoluer par nécessité réelle, pas par
anticipation (voir principes ci-dessous).

## Modules

### `TileMap` — l'environnement de jeu

Grille de cases de taille fixe, chaque case étant soit du sol, soit un mur.

- **`enum class Tile { Floor, Wall }`** : `enum class` plutôt que `enum`
  pour la portée (`Tile::Wall`) et l'absence de conversion implicite en
  `int`.
- **Données membres** : `m_largeur`, `m_hauteur` (en cases),
  `m_tailleCase` (en pixels), `m_cases` (`std::vector<Tile>`).
- **Stockage 1D** : la case `(x, y)` est à l'indice `y * largeur + x`,
  calculé par la fonction privée `indice(x, y)`. Plus simple et contigu en
  mémoire qu'un vecteur de vecteurs.
- **Ordre de déclaration des membres** : `m_cases` est déclaré après
  `m_largeur` et `m_hauteur`, car il est initialisé à partir d'eux dans la
  liste d'initialisation du constructeur.

Interface publique :

| Fonction | Rôle |
|---|---|
| `TileMap(const std::vector<std::string>& plan, float tailleCase)` | Construit la carte à partir d'un plan texte (`'#'` = mur, tout autre caractère = sol) |
| `Tile getTile(int x, int y) const` | Type de la case `(x, y)` ; **hors de la carte = `Wall`** |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine toutes les cases |
| `bool chevaucheUnMur(const sf::FloatRect& rect) const` | Le rectangle (en pixels du monde) chevauche-t-il un mur ? |

## Principes de conception à respecter

1. **Ne jamais anticiper.** On ne construit que ce qui est nécessaire pour
   l'étape en cours. Pas de fonction ou dépendance "pour plus tard" — on y
   revient explicitement quand le besoin devient réel. À l'inverse, le
   code devenu inutile est supprimé (Git le conserve dans l'historique).
2. **Avancer par petites étapes.** D'abord la structure de données (les
   paramètres/membres d'une classe), puis les fonctions une par une.
   Déclaration dans le `.hpp` avant implémentation dans le `.cpp`.
3. **Compiler et tester après chaque ajout**, même une seule fonction —
   garde le cycle de débogage court.
4. **Build out-of-source.** Le dossier `build/` (généré par CMake/Ninja)
   n'est jamais commité — voir `.gitignore`.
5. **Fonctions membres de lecture `const`**, objets en lecture passés par
   référence constante (`const T&`).
6. **Chaque fichier inclut lui-même les en-têtes des types qu'il utilise
   directement**, sans dépendre des inclusions indirectes.

## Décisions techniques prises

### Build

- **CMake minimal** : `cmake_minimum_required`, `project()`,
  `CMAKE_CXX_STANDARD` = 20 (`CMAKE_CXX_STANDARD_REQUIRED` = ON), une seule
  cible `add_executable(main src/main.cpp src/TileMap.cpp)`.
- **SFML** : `find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)` +
  `target_link_libraries(main PRIVATE SFML::Graphics)`.
- **Générateur Ninja** (`cmake -S . -B build -G Ninja`). Ninja relance la
  configuration automatiquement quand `CMakeLists.txt` change.

### Boucle de jeu et déplacement

- **Boucle de jeu en trois phases** : événements (le clic modifie la
  cible), mise à jour (le carré avance), dessin.
- **Déplacement à pas fixe** plutôt que proportionnel à la distance :
  vitesse constante, comme un personnage d'ARPG.
- **Delta time** (`sf::Clock::restart()`) : vitesse exprimée en pixels
  par seconde, pour qu'elle ne dépende pas de la cadence réelle.
  `setFramerateLimit(60)` est conservé pour limiter l'usage du CPU, mais
  la logique n'en dépend plus.

### Carte, collision et dessin

- **Coordonnées du monde** : la carte et le carré sont positionnés en
  pixels de la carte, jamais en pixels de l'écran. C'est ce qui permettra
  d'ajouter une caméra (`sf::View`) sans modifier `TileMap`.
- **Hors de la carte = mur** : `getTile` vérifie les bornes et renvoie
  `Tile::Wall` pour toute case hors limites (pas d'accès indéfini au
  vecteur, et la carte est infranchissable à ses bords).
- **Collision rectangle/grille** : on ne parcourt que les cases couvertes
  par le rectangle. Bord gauche/haut : `std::floor` (correct aussi pour
  les coordonnées négatives, contrairement à la troncature d'un
  `static_cast<int>`). Bord droit/bas : `std::ceil(...) - 1`, pour qu'un
  rectangle qui *touche* une case sans la *chevaucher* ne soit pas en
  collision (même convention que `sf::Rect::findIntersection`).
- **Glissement le long des murs** : le déplacement est testé axe par axe,
  X puis Y, l'essai Y partant de la position obtenue après l'essai X.
  Conséquence acceptée pour l'instant : en glissant, le carré ne garde que
  la composante du déplacement parallèle au mur et avance donc plus
  lentement.
- **Dessin** : un seul `sf::RectangleShape` repositionné et recoloré pour
  chaque case, avec un contour de -1 px pour rendre la grille visible.
  Environ 600 appels à `draw` par image, acceptable à cette taille
  (`sf::VertexArray` envisageable si la carte grandit beaucoup).
  `dessiner` prend un `sf::RenderTarget&` (classe de base de
  `sf::RenderWindow`) pour ne pas être couplée à la fenêtre.

### Plan texte et validation

- **Format** : `std::vector<std::string>`, une chaîne par ligne de cases.
  Largeur = longueur des lignes, hauteur = nombre de lignes.
- **Validation** : `plan.at(0)` lance `std::out_of_range` si le plan est
  vide ; une ligne de longueur différente lance `std::invalid_argument`
  avec le numéro de la ligne. Ces exceptions ne sont pas attrapées : le
  programme s'arrête avec un message clair, ce qui est voulu pour détecter
  une erreur de saisie dans le plan.
- Le plan est pour l'instant défini directement dans `main.cpp`.

### Caméra

- **`sf::View` dans `main.cpp`** : une simple variable `camera`, copiée de
  `window.getDefaultView()` (taille de la fenêtre). Pas de classe `Camera`
  tant qu'il n'y a pas de comportement à y encapsuler.
- **Toujours centrée sur le carré** : à chaque image, après la mise à jour
  et avant le dessin, `camera.setCenter(position + taille / 2)` puis
  `window.setView(camera)`. La position du carré est son coin haut-gauche
  (l'origine n'est pas modifiée, car la collision suppose
  `sf::FloatRect(position, taille)`).
- **Conséquence acceptée** : près des bords de la carte, la caméra montre
  du noir au-delà des murs. Bloquer la caméra aux bords de la carte reste
  une option pour plus tard.
- **Aucun changement à `TileMap`** : grâce aux coordonnées du monde, la
  caméra ne concerne que ce que la fenêtre affiche.
- **Clic** : converti avec `window.mapPixelToCoords(...)`. Les événements
  étant traités en début d'image, la vue utilisée est celle de l'image
  précédente, c'est-à-dire celle affichée au moment du clic — ce qui est
  voulu.
- **Texte de position** : placé en coordonnées du monde au point cliqué,
  il reste attaché à cet endroit de la carte quand elle défile.
