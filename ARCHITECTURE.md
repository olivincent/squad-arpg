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
| IDE | VSCode, avec Claude Code pour l'implémentation |
| Compilateur | MinGW-w64 (via MSYS2 UCRT64) |
| Build | CMake + Ninja |
| Langage | C++20 |
| Bibliothèque graphique | SFML 3.0.2 |
| Tests | Catch2 3 (paquet MSYS2 `mingw-w64-ucrt-x86_64-catch`) |

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
│   ├── TileMap.cpp
│   ├── Escouade.hpp
│   ├── Escouade.cpp
│   ├── Chemin.hpp
│   ├── Chemin.cpp
│   ├── Personnage.hpp
│   ├── Personnage.cpp
│   ├── Deplacement.hpp
│   ├── Deplacement.cpp
│   ├── Monstre.hpp
│   └── Monstre.cpp
├── tests/
│   ├── test_TileMap.cpp
│   ├── test_Chemin.cpp
│   ├── test_Escouade.cpp
│   ├── test_Deplacement.cpp
│   └── test_Monstre.cpp
├── .gitignore
├── CLAUDE.md
├── ARCHITECTURE.md
└── ROADMAP.md
```

Six modules : `TileMap` (l'environnement de jeu), `Escouade` (le point
de contrôle, l'ancre, qui suit un chemin calculé par le A* et possède
les personnages), `Chemin` (recherche de chemin A*, utilisé uniquement
par `Escouade`), `Personnage` (un membre de l'escouade, qui sait aller
vers un point), `Deplacement` (collision/glissement partagée entre
`Escouade` et `Personnage`) et `Monstre` (un ennemi immobile, avec des
points de vie, possédé directement par `main.cpp`, sans lien avec
`Escouade` ni `Personnage`). `main.cpp` garde la fenêtre, la boucle de
jeu, les événements (fermeture, et maintenant l'attaque au clic), l'état
de la souris (déplacement), la caméra, la carte (son plan) et le
monstre ; il ne connaît ni `Chemin` ni `Personnage`. La structure
continue d'évoluer par nécessité réelle, pas par anticipation (voir
principes ci-dessous).

Deux exécutables : `main` (le jeu) et `tests` (les tests Catch2). Les deux
se lient à la bibliothèque statique `squad_core`, qui contient la logique
de jeu (`TileMap`, `Escouade`) indépendamment de la fenêtre et de la
boucle de jeu.

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
| `int getLargeur() const` | Largeur de la carte, en cases |
| `int getHauteur() const` | Hauteur de la carte, en cases |
| `sf::Vector2i caseContenant(sf::Vector2f point) const` | Case (coordonnées de grille) contenant ce point, en pixels du monde |
| `sf::Vector2f centreDeCase(sf::Vector2i coordCase) const` | Centre de cette case, en pixels du monde |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine toutes les cases |
| `bool chevaucheUnMur(const sf::FloatRect& rect) const` | Le rectangle (en pixels du monde) chevauche-t-il un mur ? |
| `bool trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const` | Le rectangle, translaté de sa position jusqu'à `position + deplacement`, ne chevauche-t-il un mur à aucun moment du trajet ? |

### `Escouade` — le point de contrôle (l'ancre)

Un carré vert de 20 × 20 qui suit le chemin calculé par le A* jusqu'au
point cliqué, en contournant les murs, et qui possède 4 `Personnage` en
formation en carré autour d'elle.

- **Données membres** : `m_position` (coin haut-gauche, pixels du monde),
  `m_taille`, `m_chemin` (`std::vector<sf::Vector2f>`, les points — centre
  de l'escouade — à atteindre dans l'ordre), `m_indexChemin` (indice du
  prochain point visé), `m_vitesse` (pixels par seconde), `m_decalages`
  (les 4 décalages de formation, pour une escouade orientée vers `+x`),
  `m_orientation` (`sf::Angle`, `0` = vers `+x`, ce qu'utilise
  `placePersonnage`), `m_orientationVoulue` (`sf::Angle`, la direction
  vers laquelle `m_orientation` tourne progressivement),
  `m_vitesseRotation` (degrés par seconde), `m_personnages`
  (`std::vector<Personnage>`, inclut `Personnage.hpp` en entier — un
  `std::vector` a besoin de la définition complète de son type
  d'élément).
- **Valeurs fixes** dans le constructeur : taille 20 × 20, vitesse
  180 px/s, chemin = un seul point (le centre de départ, donc immobile
  tant qu'on n'a pas cliqué), orientation `0`. Elles deviendront des
  paramètres quand il le faudra.
- **Fonction privée `placePersonnage(indice)`** : seul endroit du code
  qui calcule la place d'un personnage (`getCentre() + m_decalages[indice]
  .rotatedBy(m_orientation)`) — utilisée à la fois pour créer les
  personnages dans le constructeur et pour leur donner leur cible à
  chaque `mettreAJour`.

Interface publique (inchangée pour `main.cpp`, qui ne connaît pas
`Personnage`) :

| Fonction | Rôle |
|---|---|
| `explicit Escouade(sf::Vector2f positionDepart)` | Crée l'escouade (et ses 4 personnages en formation) à la position donnée |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Calcule un chemin vers la destination (A*, puis lissé) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe |
| `void mettreAJour(float dt, const TileMap& carte)` | Avance l'ancre vers le point courant du chemin (collision, glissement, orientation), puis chaque personnage vers sa place |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine l'ancre, puis les 4 personnages |
| `sf::Vector2f getCentre() const` | Centre de l'escouade (utilisé par la caméra, par `allerVers` et par `placePersonnage`) |

### `Chemin` — recherche de chemin (A*)

Une fonction libre, pas une classe : l'algorithme n'a aucun état à
conserver entre deux appels.

- **Travaille en cases** (coordonnées de grille, `sf::Vector2i`), jamais
  en pixels — contrairement à `Escouade`, qui travaille en pixels
  (`sf::Vector2f`). Pas encore de conversion pixels ↔ cases dans le
  projet.
- **Déclaration anticipée** : `Chemin.hpp` contient `class TileMap;` ;
  l'include de `TileMap.hpp` est dans `Chemin.cpp`.
- **Tout l'interne de l'algorithme** (structure de nœud, comparateur de
  la file de priorité, heuristique) est dans un espace de noms anonyme
  de `Chemin.cpp` — rien n'apparaît dans `Chemin.hpp`.

Interface publique :

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee)` | Plus court chemin entre deux cases (4 directions, coût 1 par pas, heuristique de Manhattan) ; inclut le départ et l'arrivée ; vecteur vide si départ/arrivée est un mur (ou hors carte) ou si l'arrivée est inaccessible |
| `std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Vector2f taille)` | Retire de `points` (positions du centre d'un rectangle `taille`, en pixels) les points de passage inutiles ; `depart` n'en fait pas partie dans le résultat |

Deux niveaux : `trouverChemin` raisonne en cases, `lisserChemin` en
pixels. Utilisé uniquement par `Escouade::allerVers` ; `main.cpp` ne
connaît pas ce module.

### `Personnage` — un membre de l'escouade

Un carré coloré de 20 × 20 qui sait seulement « aller vers un point » :
il ne connaît ni la formation, ni son décalage, ni l'escouade qui le
possède. C'est elle qui décide du point à atteindre et le lui donne en
paramètre de `mettreAJour` — principe retenu pour permettre, plus tard,
qu'un personnage reçoive un point différent de sa place en formation
(combat, action sur un coffre ou une porte) sans changer son interface.

- **Stocke son centre** (`m_centre`), pas son coin haut-gauche : le point
  qu'on lui demande d'atteindre est lui-même un centre (une place de
  formation), pas un coin.
- **Données membres** : `m_centre`, `m_taille` (20 × 20, fixée dans le
  constructeur), `m_couleur` (reçue au constructeur, pour distinguer les
  4 personnages à l'écran), `m_vitesse` (240 px/s — plus rapide que
  l'escouade à 180 px/s, pour qu'un personnage retardé par un mur
  puisse rattraper sa place).
- **Déclaration anticipée** : `Personnage.hpp` contient `class TileMap;`
  (une référence ne demande pas la définition complète) ; aucun include
  de `TileMap.hpp` n'est nécessaire dans `Personnage.cpp` non plus,
  puisque la carte n'est que transmise à `deplacerAvecGlissement`, jamais
  utilisée directement.
- **Pas de A* ni de lissage** : juste une ligne droite vers la cible, pas
  fixe (`vitesse * dt`), arrêt exact si la distance restante est `≤` ce
  pas — même logique que l'ancre avant l'étape 11.

Interface publique :

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, sf::Color couleur)` | Crée le personnage à ce centre, avec cette couleur |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte)` | Avance vers la cible, avec collision et glissement le long des murs |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage |

### `Deplacement` — collision et glissement partagés

Une fonction libre, pas une classe : extraite sans modification du code
qui était auparavant dans `Escouade::mettreAJour`, pour que `Personnage`
puisse l'utiliser aussi, sans dépendre d'`Escouade` ni dupliquer le code.

| Fonction | Rôle |
|---|---|
| `sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement)` | Essaie le déplacement axe par axe (X puis Y, l'essai Y partant du résultat du X) ; renvoie la nouvelle position du coin haut-gauche |

Utilisée par `Escouade::mettreAJour` (pour l'ancre) et par
`Personnage::mettreAJour` (qui convertit entre son centre et le coin
haut-gauche attendu par la fonction, comme `Escouade::getCentre()` le
fait dans l'autre sens).

### `Monstre` — un ennemi immobile

Un carré rouge de 30 × 30, avec des points de vie et une barre de vie.
Pas de classe de base commune avec `Personnage`, pas d'héritage : les
deux classes restent indépendantes, même si toutes les deux stockent un
centre et une taille — la ressemblance n'implique pas de dépendance
commune tant qu'aucun comportement partagé ne le justifie.

- **Stocke son centre** (`m_centre`), comme `Personnage`.
- **Données membres** : `m_centre`, `m_taille` (30 × 30, fixée dans le
  constructeur), `m_pointsDeVie` et `m_pointsDeVieMax` (`100` chacun à
  la création). Pas de couleur stockée : toujours rouge, donc une
  valeur fixe directement dans `dessiner`, pas un membre (contrairement
  à `Personnage`, qui a besoin d'une couleur différente par instance).
- **`explicit Monstre(sf::Vector2f centre)`** : seul le centre est un
  paramètre.
- **`contient`** réutilise `sf::FloatRect::contains` plutôt que de
  réimplémenter sa convention à la main (bords gauche/haut inclus,
  droit/bas exclus).
- **`recevoirDegats` ne descend jamais sous zéro** : `m_pointsDeVie` est
  ramené à `0` s'il devient négatif, pour qu'un coup qui dépasse le
  reste ne laisse pas un nombre négatif stocké.
- **Pas d'accesseur pour les points de vie** : les tests passent par
  `estVivant()`, et la barre de vie est dessinée par la classe
  elle-même — aucun autre code n'a besoin de lire `m_pointsDeVie`
  directement.
- **Barre de vie dans `dessiner`** : un rectangle de fond sombre (largeur
  du monstre, quelques pixels au-dessus de lui), recouvert d'un
  rectangle vert dont la largeur est `m_taille.x * (m_pointsDeVie /
  m_pointsDeVieMax)`.
- **Possédé par `main.cpp`**, pas par `Escouade` : une simple variable
  `Monstre`, pas un `std::vector` ni un `std::optional` (un seul monstre
  pour l'instant, qui ne meurt jamais au sens de disparaître de la
  mémoire — seulement de ne plus être dessiné ni attaquable).
- **Pas de collision avec lui** : l'escouade et les personnages lui
  passent dessus ; le A* l'ignore complètement (il n'existe pas du
  point de vue de `TileMap`).

Interface publique :

| Fonction | Rôle |
|---|---|
| `explicit Monstre(sf::Vector2f centre)` | Crée le monstre à ce centre, 30 × 30, 100 points de vie |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le monstre et sa barre de vie |
| `bool contient(sf::Vector2f point) const` | Le point (pixels du monde) est-il dans le carré du monstre ? |
| `void recevoirDegats(int quantite)` | Retire des points de vie, sans descendre sous zéro |
| `bool estVivant() const` | Points de vie `> 0` |

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
   référence constante (`const T&`) ; petits types (`sf::Vector2f`,
   `float`) passés par valeur.
6. **Chaque fichier inclut lui-même les en-têtes des types qu'il utilise
   directement**, sans dépendre des inclusions indirectes.
7. **Migrer sans casser.** Une nouvelle classe est construite à côté de
   l'ancien code, validée par comparaison, puis substituée. Le code
   déplacé l'est sans modification ; les améliorations viennent dans un
   commit séparé.

## Décisions techniques prises

### Build

- **CMake minimal** : `cmake_minimum_required`, `project()`,
  `CMAKE_CXX_STANDARD` = 20 (`CMAKE_CXX_STANDARD_REQUIRED` = ON).
- **`find_package`** regroupés en haut du fichier : SFML
  (`find_package(SFML 3.0.2 COMPONENTS Graphics REQUIRED)`) et Catch2
  (`find_package(Catch2 3 REQUIRED)`).
- **Bibliothèque `squad_core`** (`add_library(squad_core STATIC
  src/TileMap.cpp src/Escouade.cpp src/Chemin.cpp src/Personnage.cpp
  src/Deplacement.cpp src/Monstre.cpp)`) : regroupe la logique de jeu,
  réutilisée par `main` et par `tests`.
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
- **`tests`** (`add_executable(tests tests/test_TileMap.cpp
  tests/test_Chemin.cpp tests/test_Escouade.cpp
  tests/test_Deplacement.cpp tests/test_Monstre.cpp)`) se lie à
  `squad_core` et à `Catch2::Catch2WithMain` (qui fournit le `main()`
  des tests) ; lancé directement (`./build/tests.exe`), sans CTest.
- **Générateur Ninja** (`cmake -S . -B build -G Ninja`). Ninja relance la
  configuration automatiquement quand `CMakeLists.txt` change.

### Boucle de jeu et déplacement

- **Boucle de jeu en quatre phases** : événements (fermeture de la
  fenêtre), entrée souris (`escouade.allerVers` si le bouton gauche est
  enfoncé — voir « Entrée souris » ci-dessous), mise à jour
  (`escouade.mettreAJour`), dessin.
- **Déplacement à pas fixe** plutôt que proportionnel à la distance :
  vitesse constante, comme un personnage d'ARPG.
- **Delta time** (`sf::Clock::restart()`) : vitesse exprimée en pixels
  par seconde, pour qu'elle ne dépende pas de la cadence réelle.
  `setFramerateLimit(60)` est conservé pour limiter l'usage du CPU, mais
  la logique n'en dépend plus. L'horloge reste dans `main.cpp` ; `dt` est
  passé en paramètre à l'escouade.

### Entrée souris

- **Déplacement : lecture d'état** (étape 13) : `main.cpp` traite
  `sf::Event::MouseButtonPressed` uniquement pour l'attaque (voir
  ci-dessous) ; pour le déplacement, `sf::Mouse::isButtonPressed(
  sf::Mouse::Button::Left)` est interrogé à chaque image, après la
  boucle d'événements et avant `escouade.mettreAJour`. Tant que le
  bouton reste enfoncé, `escouade.allerVers` est rappelée à chaque image
  avec la position courante du curseur — pas de second mode de
  déplacement « en direction du curseur », c'est le même A* + lissage
  qu'un clic ponctuel, simplement redonné en continu.
- **`window.hasFocus()`** : condition nécessaire, parce que
  `isButtonPressed` lit l'état global de la souris au niveau du système,
  même quand la fenêtre du jeu n'a pas le focus.
- **Pas de limitation de fréquence** : un appel au A* par image, jugé
  négligeable sur la carte actuelle ; à revisiter seulement si un
  problème de performance est mesuré.
- **Attaque : événement** (étape 15) : l'événement
  `MouseButtonPressed` (bouton gauche) est redevenu nécessaire pour le
  monstre, parce que l'attaque doit être un coup par clic, pas un coup
  par image — contrairement au déplacement, qui veut justement se
  répéter tant que le bouton reste enfoncé. Les deux coexistent sans
  conflit : l'événement gère l'attaque (si le monstre est vivant et que
  le clic le touche, `monstre.recevoirDegats(degatsParClic)`), la
  lecture d'état gère le déplacement ; un même clic sur le monstre
  déclenche les deux indépendamment, sans cas particulier à écrire.
  `degatsParClic` (20) est une constante locale à `main.cpp` — une
  propriété de l'attaque, pas du monstre.
- **Texte de position retiré** (ainsi que la police `arial.ttf` qui ne
  servait qu'à lui) : plus rien n'affiche la position du curseur à
  l'écran.

### Carte, collision et dessin

- **Coordonnées du monde** : la carte et l'escouade sont positionnées en
  pixels de la carte, jamais en pixels de l'écran. C'est ce qui a permis
  d'ajouter la caméra sans modifier `TileMap`.
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
  Conséquence acceptée pour l'instant : en glissant, l'escouade ne garde
  que la composante du déplacement parallèle au mur et avance donc plus
  lentement.
- **Dessin de la carte** : un seul `sf::RectangleShape` repositionné et
  recoloré pour chaque case, avec un contour de -1 px pour rendre la
  grille visible. Environ 2400 appels à `draw` par image avec la carte
  actuelle (`sf::VertexArray` envisageable si ça devient un problème).
  Les fonctions `dessiner` prennent un `sf::RenderTarget&` (classe de base
  de `sf::RenderWindow`) pour ne pas être couplées à la fenêtre.

### Plan texte et validation

- **Format** : `std::vector<std::string>`, une chaîne par ligne de cases.
  Largeur = longueur des lignes, hauteur = nombre de lignes.
- **Validation** : `plan.at(0)` lance `std::out_of_range` si le plan est
  vide ; une ligne de longueur différente lance `std::invalid_argument`
  avec le numéro de la ligne. Ces exceptions ne sont pas attrapées : le
  programme s'arrête avec un message clair, ce qui est voulu pour détecter
  une erreur de saisie dans le plan.
- Le plan est pour l'instant défini directement dans `main.cpp`.

### Escouade

- **État de jeu séparé de l'affichage** : la position, la taille et la
  cible sont des données de la classe ; le `sf::RectangleShape` n'existe
  que dans `dessiner`. Même modèle que `TileMap`.
- **Dépendance à `TileMap` le temps d'un appel** : la carte est passée en
  `const TileMap&` à `mettreAJour` plutôt que gardée en membre — pas de
  question de durée de vie, et l'escouade ne peut pas modifier la carte.
- **Déclaration anticipée** : `Escouade.hpp` contient `class TileMap;`
  (une référence ne demande pas la définition complète) ; l'include de
  `TileMap.hpp` est dans `Escouade.cpp`. Les fichiers qui incluent
  `Escouade.hpp` ne dépendent donc pas de `TileMap.hpp`.
- **En-têtes SFML précis** dans `Escouade` (`Vector2.hpp`,
  `RenderTarget.hpp`, `RectangleShape.hpp`…), alors que `TileMap.hpp`
  inclut tout `<SFML/Graphics.hpp>`. Non harmonisé pour l'instant.
- **`explicit`** sur le constructeur à un paramètre, pour interdire la
  conversion implicite `sf::Vector2f` → `Escouade`.
- **Noms d'intention** : `allerVers` plutôt que `setCible` ; `getCentre`
  encapsule le calcul « coin haut-gauche + moitié de la taille », pour que
  `main.cpp` ne dépende pas de la convention de position.

### Suivi de chemin par l'escouade

- **Conversions pixels ↔ cases regroupées dans `TileMap`** (seule à
  connaître `m_tailleCase`) : `caseContenant` (comme `chevaucheUnMur`,
  avec `std::floor` pour rester correct en négatif) et `centreDeCase`.
- **Les points de passage sont des positions du centre de l'escouade**,
  pas de son coin haut-gauche. `mettreAJour` calcule donc la direction et
  la distance depuis `getCentre()`, mais applique le déplacement obtenu
  (un simple vecteur de translation) à `m_position` — une translation
  déplace un rectangle rigide tout entier, inutile de la recalculer
  séparément pour le centre et le coin.
- **`m_chemin` + `m_indexChemin`** plutôt qu'un seul point cible : avancer
  l'indice évite un `erase(begin())` à chaque point atteint (coûteux, car
  il décale tous les éléments suivants en mémoire). Le chemin déjà
  parcouru reste en mémoire jusqu'au prochain `allerVers`.
- **L'appel au A* est dans `Escouade::allerVers`**, pas dans `main.cpp`,
  avec la carte passée en paramètre comme pour `mettreAJour` (jamais
  gardée en membre).
- **Construction du chemin suivi** à partir du résultat de
  `trouverChemin` (qui travaille en cases) : la case de départ est
  ignorée (l'escouade est déjà dessus, la viser d'abord la ferait
  reculer vers le centre de sa propre case) ; les cases intermédiaires
  sont converties en centres de case ; le dernier point est remplacé par
  le point exact cliqué, pour que l'escouade s'arrête précisément dessus
  plutôt qu'au centre de la case d'arrivée.
- **Chemin vide = ordre ignoré** : `allerVers` ne touche à `m_chemin`
  qu'après avoir confirmé que `trouverChemin` a renvoyé un résultat non
  vide ; un clic sur un mur ou une zone inaccessible n'interrompt donc
  pas le trajet en cours.
- **Collision et glissement conservés tels quels**, comme filet de
  sécurité : l'escouade peut partir d'une position décentrée dans sa case
  ou viser un point proche d'un mur (notamment le point exact cliqué).
- **Chemin lissé avant d'être stocké** (étape 11) : entre la
  construction du chemin en pixels et son affectation à `m_chemin`,
  `allerVers` le passe par `Chemin::lisserChemin`, avec `getCentre()`
  comme point de départ et `m_taille` comme taille de rectangle. Voir
  « Lissage de chemin » ci-dessous. L'ancien zigzag près des coins de
  case (viser un centre de case presque aligné avec l'escouade sur un
  seul axe) est résolu par ce lissage.

### Formation de l'escouade

- **Un personnage sait seulement « aller vers un point »** (décision de
  conception retenue en vue de l'avenir — combat, actions) : il reçoit
  sa cible en paramètre de `mettreAJour`, sans connaître la formation ni
  son décalage. C'est `Escouade` qui décide de ce point et le lui donne.
- **`Escouade` possède les personnages** (`std::vector<Personnage>`
  stocké par valeur) et les décalages de formation ; elle reste le
  point de contrôle dessiné (le carré vert), dont le comportement
  (chemin, vitesse, collision, `getCentre`) n'a pas changé.
- **`placePersonnage(indice)` : un seul point de calcul** pour la place
  d'un personnage (`getCentre() + m_decalages[indice].rotatedBy(
  m_orientation)`), utilisé à la fois dans le constructeur (pour créer
  les personnages à leur place) et dans `mettreAJour` (pour leur donner
  leur cible à chaque tic) — éviter de dupliquer cette formule à deux
  endroits qui pourraient diverger.
- **Ordre dans `mettreAJour` et `dessiner`** : l'ancre d'abord, les
  personnages ensuite. Les personnages visent la place de l'image
  *courante* (calculée après que l'ancre a bougé), pas celle d'avant.
- **Module `Deplacement` extrait** (voir ci-dessus) pour que
  `Personnage` réutilise la même collision/glissement que l'ancre, sans
  dépendre d'`Escouade`.
- **Orientation voulue et orientation réelle séparées** : `m_orientationVoulue`
  reçoit la direction voulue de l'ancre (vecteur vers le point courant du
  chemin, *avant* l'essai de collision — si l'escouade est bloquée par un
  mur et glisse, la formation vise quand même la direction qu'elle
  essayait de suivre) ; mise à jour seulement si l'ancre a un point à
  atteindre et que la distance est `> 0` (`sf::Vector2f::angle()` sur un
  vecteur nul n'a pas de sens), sinon la dernière valeur est conservée.
  `m_orientation`, elle, est ce qu'utilise réellement `placePersonnage`.
- **Rotation progressive, par le plus court côté** (étape 14) :
  `m_orientation` se rapproche de `m_orientationVoulue` à chaque
  `mettreAJour`, que l'ancre bouge ou non (si elle s'arrête en pleine
  rotation, la formation finit quand même de pivoter sur place).
  L'écart, `(m_orientationVoulue - m_orientation).wrapSigned()`, est
  ramené dans `[-180°, 180°]` pour que passer de 350° à 10° tourne de
  +20° et non de -340°. La rotation par image est bornée à
  `m_vitesseRotation * dt` (`m_vitesseRotation` = 360°/s, fixe, comme
  `m_vitesse`) ; si l'écart restant est inférieur ou égal à ce pas,
  `m_orientation` prend exactement la valeur voulue (même logique que
  le déplacement à pas fixe de l'ancre).
- **Limites connues** : pas de collision entre personnages ni entre un
  personnage et l'ancre ; un personnage peut rester accroché derrière un
  coin de mur tant que l'escouade ne repasse pas par là.
  - **Chevauchement résiduel lors d'un demi-tour rapide** : les 4
    décalages forment deux paires diagonales exactement opposées
    (`avant-gauche`/`arrière-droite`, `avant-droite`/`arrière-gauche`),
    donc les places *idéales* de deux personnages quelconques ne se
    chevauchent jamais pendant la rotation. Mais chaque personnage
    poursuit sa cible en ligne droite (`Personnage::mettreAJour`), pas
    en suivant l'arc de la rotation : lors d'un demi-tour très rapide,
    la cible d'un personnage peut passer presque instantanément de
    l'avant à l'arrière (ou l'inverse), et il coupe alors tout droit à
    travers le centre de la formation — pendant qu'un autre personnage
    fait le trajet inverse au même moment. La rotation progressive
    réduit nettement ce phénomène (elle a remplacé un saut instantané
    par un mouvement continu), sans l'éliminer. Le corriger demanderait
    une vraie collision personnage-personnage, ou un déplacement qui
    suit l'arc plutôt qu'une ligne droite — les deux explicitement hors
    de portée de l'étape 14.

### Lissage de chemin

- **`TileMap::trajetLibre(rect, deplacement)`** : vrai si le rectangle,
  translaté de sa position jusqu'à `position + deplacement`, ne chevauche
  un mur à aucun moment du trajet. Même convention que `chevaucheUnMur`
  (toucher sans chevaucher n'est pas une collision ; hors carte = mur).
  Calcul exact, pas d'échantillonnage.
  - **Cas horizontal, vertical ou nul** (une composante de `deplacement`
    nulle) : la zone balayée est le rectangle qui englobe la position de
    départ et celle d'arrivée ; un seul appel à `chevaucheUnMur` suffit.
  - **Cas oblique** : balayage par colonnes de cases, paramétré par `t`
    entre 0 (départ) et 1 (arrivée). Pour chaque colonne couverte par le
    mouvement, deux instants (`t1`, `t2`) marquent quand le bord arrière
    du rectangle atteint le bord gauche de la colonne, et quand son bord
    avant atteint son bord droit. `tEntree = min(t1, t2)` et
    `tSortie = max(t1, t2)` (bornés à `[0, 1]`) donnent la bonne paire
    entrée/sortie quel que soit le sens du déplacement, puisque diviser
    par une composante négative de `deplacement` inverse l'ordre de `t1`
    et `t2` — sans `min`/`max`, il faudrait une branche selon le signe.
    Une composante nulle est exclue de ce cas parce que les formules de
    `t1`/`t2` divisent par elle (division par zéro indéfinie), d'où la
    séparation avec le cas horizontal/vertical.
  - Pour chaque colonne avec une fenêtre `[tEntree, tSortie]` non vide,
    la plage de hauteur balayée pendant cette fenêtre (`haut`/`bas`) donne
    les rangées à vérifier avec `getTile`.
  - Un rectangle peut « accrocher » le coin d'un mur même quand la ligne
    reliant les CENTRES du rectangle à chaque instant ne traverse jamais
    la case du mur — vérifié par un test dédié (voir Tests ci-dessous) :
    le rectangle a une taille, pas seulement un centre.
- **`Chemin::lisserChemin(carte, depart, points, taille)`** : algorithme
  glouton. L'ancre part de `depart` ; à chaque tour, on cherche, **en
  partant de la fin** de `points`, le plus lointain atteignable depuis
  l'ancre (`trajetLibre` avec le rectangle `taille` centré sur l'ancre) —
  s'arrêter au premier trouvé donne directement le plus lointain. Ce
  point devient la nouvelle ancre, et on continue avec les points qui le
  suivent. Si aucun point restant n'est atteignable, le prochain point
  de la liste est gardé tel quel, sans être testé : la collision dans
  `mettreAJour` reste le filet de sécurité.
- **Branchement dans `Escouade::allerVers`** : le chemin en pixels est
  construit comme avant l'étape 11 (centres de case, puis point exact
  cliqué), puis lissé avant d'être stocké dans `m_chemin`. Le reste
  d'`Escouade` (`mettreAJour`, collision, glissement, chemin vide = ordre
  ignoré) n'a pas changé.
- **Limite connue** : `lisserChemin` ne fait que retirer des points de la
  liste donnée par le A* (centres de case) ; il ne peut pas en ajouter ni
  en déplacer. Autour d'un pilier, dès qu'un saut direct est bloqué par
  le mur, l'escouade retombe sur le centre de la case suivante tel quel,
  plutôt que sur un point plus proche du coin du pilier qui
  raccourcirait encore le trajet — elle frôle donc moins les murs qu'un
  lissage « idéal » (type *funnel algorithm* ou Theta*) ne le ferait,
  mais elle ne reste jamais coincée. Accepté pour l'instant.

### Recherche de chemin (A*)

- **Fonction libre `trouverChemin`**, pas de classe : aucun état à
  conserver entre deux appels.
- **File de priorité** : `std::priority_queue<Noeud, std::vector<Noeud>,
  ComparateurNoeud>`. `std::priority_queue` sort toujours le plus **grand**
  élément selon son comparateur ; `ComparateurNoeud` inverse l'ordre
  (`a.coutTotal > b.coutTotal`) pour que ce soit le plus **petit** coût
  total qui sorte en premier.
- **Heuristique** : distance de Manhattan entre la case et l'arrivée,
  ignorant les murs. Elle ne surestime jamais la distance réelle
  (admissible), ce qui garantit que l'A* trouve le plus court chemin ; son
  rôle est de diriger l'exploration vers l'arrivée plutôt que dans toutes
  les directions comme le ferait un Dijkstra sans heuristique.
- **Entrées périmées** : `std::priority_queue` ne permet pas de mettre à
  jour une entrée existante. Quand un meilleur chemin est trouvé vers une
  case déjà dans la file, une nouvelle entrée est ajoutée sans retirer
  l'ancienne. Le tableau `cout` (coût réel actuellement connu par case)
  fait autorité : à la sortie de la file, une entrée dont le coût
  parcouru dépasse `cout[case]` est ignorée (suppression paresseuse).
- **Tableaux 1D** `cout` et `parent`, indexés `y * largeur + x` comme
  `TileMap::m_cases`, dimensionnés via `getLargeur()`/`getHauteur()`.
- **Reconstruction** : remontée des `parent` depuis l'arrivée jusqu'au
  départ, puis `std::reverse`.
- **Chemin vide** si départ ou arrivée est un mur (ou hors carte, que
  `getTile` traite comme un mur) ou si l'arrivée est inaccessible ; ce
  test précède le raccourci départ == arrivée, pour qu'une case murée ne
  renvoie jamais un chemin d'une case.

### Caméra

- **`sf::View` dans `main.cpp`** : une simple variable `camera`, copiée de
  `window.getDefaultView()` (taille de la fenêtre). Pas de classe `Camera`
  tant qu'il n'y a pas de comportement à y encapsuler.
- **Toujours centrée sur l'escouade** : à chaque image, après la mise à
  jour et avant le dessin, `camera.setCenter(escouade.getCentre())` puis
  `window.setView(camera)`.
- **Conséquence acceptée** : près des bords de la carte, la caméra montre
  du noir au-delà des murs. Bloquer la caméra aux bords de la carte reste
  une option pour plus tard.
- **Position du curseur** : convertie avec `window.mapPixelToCoords(...)`
  après la boucle d'événements, donc avec la vue de l'image précédente —
  celle effectivement affichée à l'écran au moment de la lecture — ce
  qui est voulu (même raisonnement que pour l'ancien clic, voir
  « Entrée souris » ci-dessous).

### Tests

- **Catch2 3 adopté**, installé via MSYS2
  (`mingw-w64-ucrt-x86_64-catch`), trouvé par `find_package(Catch2 3
  REQUIRED)` comme SFML (pas de `FetchContent`).
- **Un seul exécutable `tests`**, cinq fichiers (`test_TileMap.cpp`,
  `test_Chemin.cpp`, `test_Escouade.cpp`, `test_Deplacement.cpp`,
  `test_Monstre.cpp`), lancé directement (`./build/tests.exe`) — pas de
  CTest ni de `catch_discover_tests`.
- **Petits plans dédiés** : chaque test construit son propre plan texte
  (3×3, 4×4, 5×5…), jamais la carte de `main.cpp`, pour isoler exactement
  le cas testé.
- **Teste uniquement l'interface publique** de `TileMap`, `Chemin` et
  `Escouade` (`getTile`, `chevaucheUnMur`, `trajetLibre`,
  `getLargeur`/`getHauteur`, `caseContenant`/`centreDeCase`, le
  constructeur, `trouverChemin`, `lisserChemin`, `allerVers`,
  `mettreAJour`, `getCentre`) — aucune fonction n'a été ajoutée
  seulement pour faciliter les tests.
- **Propriétés plutôt que chemin exact** : pour `trouverChemin`, comme
  plusieurs plus courts chemins peuvent exister, les tests vérifient des
  propriétés (premier élément = départ, dernier = arrivée, cases
  consécutives voisines, aucun mur, longueur attendue) via une fonction
  d'aide partagée (`verifierCheminValide`), plutôt que de comparer à une
  liste de cases exacte. Même principe pour `lisserChemin` (moins de
  points, dernier point conservé, chaque segment validé par
  `trajetLibre`).
- **Chaque test `trajetLibre` porte un commentaire qui justifie ses
  chiffres** : les tests obliques en particulier (coin qui accroche un
  mur malgré une ligne des centres dégagée, même obstacle abordé dans
  les 4 sens obliques) ont été vérifiés à la main avant d'être écrits,
  pour s'assurer que le résultat attendu correspond bien à la géométrie
  décrite, pas à une intuition approximative.
- **`Escouade` : des résultats, pas des trajectoires**. Les tests
  vérifient où l'escouade finit et ce qu'elle fait d'un ordre, jamais sa
  position à un tic intermédiaire précis ni la forme du chemin suivi —
  pour rester valides même après un futur lissage du chemin. Pas de
  doublure (mock) : `dt` étant déjà un paramètre de `mettreAJour`, et
  `TileMap` se construisant à partir d'un plan texte, les tests utilisent
  les vraies classes.
  - **Comparaison avec tolérance** (`REQUIRE_THAT` + `Catch::Matchers::
    WithinAbs`, jamais `==`) : les positions accumulent des calculs en
    virgule flottante (vitesse × dt, racines carrées), donc deux calculs
    mathématiquement égaux peuvent différer d'une poussière d'arrondi.
    Fonction d'aide `verifierPointProche`.
  - **Fonction d'aide de simulation** `simuler(escouade, carte, durée)` :
    appelle `mettreAJour` en boucle avec un `dt` de 1/60 s (comme
    `setFramerateLimit(60)` dans `main.cpp`) pendant la durée donnée. Les
    tests simulent une durée généreuse par rapport au trajet attendu et
    ne vérifient que l'état final.
  - **`dt` en paramètre rend la classe testable** : si `mettreAJour`
    lisait le temps lui-même (horloge interne), un test n'aurait aucun
    moyen de contrôler le temps simulé à chaque tic ; en le recevant en
    paramètre, le comportement devient déterministe et reproductible
    sans horloge factice.
- **`Personnage` et la formation ne sont pas encore testés** : seule la
  fonction partagée `deplacerAvecGlissement` (module `Deplacement`) est
  couverte. À discuter séparément si le besoin se présente.
- **`Monstre` testé par son interface publique** (`contient`,
  `recevoirDegats`, `estVivant`) : bords d'inclusion/exclusion de
  `contient` vérifiés séparément sur chaque côté (gauche/haut inclus,
  droit/bas exclus) ; `recevoirDegats`/`estVivant` couvrent le cas d'un
  coup qui dépasse le reste des points de vie et celui d'un coup
  supplémentaire une fois déjà mort. Le branchement dans `main.cpp`
  (événement de clic, constante `degatsParClic`) n'est pas testé — tout
  se passe dans `main.cpp`, hors de portée de `tests/`.
