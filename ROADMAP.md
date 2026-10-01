# Squad-ARPG — Roadmap

## Méthode de travail

- **Conception dans Claude.ai, implémentation dans Claude Code.** On
  discute des options (génie logiciel et design de jeu) dans Claude.ai ;
  quand Olivier dit « fait moi un prompt », Claude.ai rédige un prompt qu'il
  colle dans Claude Code (VSCode). Les règles de travail de Claude Code
  sont dans `CLAUDE.md`.
- On avance une pièce à la fois : d'abord la structure de données (les
  paramètres/membres), puis les fonctions une par une. Déclaration dans le
  `.hpp` avant implémentation dans le `.cpp`.
- On ne construit que ce dont on a besoin maintenant, jamais en prévision
  d'une étape future.
- Après chaque petit ajout (même une seule fonction), on compile et on
  teste avant de continuer.
- Olivier teste visuellement et fait les commits lui-même.
- On confirme toujours où en est Olivier avant de proposer la suite.

## Découpage des étapes

### Étape 0 — Squelette de projet ✅ TERMINÉE

- [x] `main.cpp` minimal (`std::cout`, compile et s'exécute)
- [x] Validation manuelle de la chaîne d'outils (`g++` direct)
- [x] `CMakeLists.txt` minimal (C++20, cible `add_executable(main ...)`)
- [x] Configuration CMake + génération Ninja
- [x] Build et exécution via `cmake --build build`
- [x] `.gitignore` (`build/`)
- [x] Commits Git propres (main.cpp, puis CMakeLists.txt séparément)

### Étape 1 — Carré qui bouge au clic ✅ TERMINÉE

1. [x] Squelette de projet (fait à l'étape 0)
2. [x] Ouvrir une fenêtre SFML vide
3. [x] Dessiner un carré statique dedans
4. [x] Détecter un clic de souris (afficher les coordonnées en console,
       rien ne bouge encore)
5. [x] Déplacer le carré à la position du clic
6. [x] Afficher la position sous le carré (texte SFML)

### Étape 2 — Déplacement progressif ("tic" par "tic") ✅ TERMINÉE

1. [x] Position cible (`cible`) stockée séparément de la position du carré
2. [x] Déplacement d'un pas fixe par tic vers la cible (vecteur direction
       normalisé × vitesse)
3. [x] Arrêt exact sur la cible : si la distance restante ≤ pas, on pose
       le carré sur la cible (pas d'oscillation)
4. [x] Vitesse indépendante de la cadence : delta time via `sf::Clock`,
       vitesse en pixels par seconde

### Étape 3 — Salle avec murs ✅ TERMINÉE (remplacée en partie par l'étape 4)

1. [x] Dessiner un mur : un seul rectangle statique au milieu de la fenêtre
2. [x] Détecter le chevauchement avec le mur
3. [x] Bloquer le carré : on calcule la position suivante et on n'avance
       pas si elle entre dans le mur
4. [~] Glisser le long du mur → réalisé à l'étape 4.5, sur la grille
5. [~] Plusieurs murs (`std::vector`) → remplacé par la `TileMap` de
       l'étape 4

### Étape 4 — Carte en grille (`TileMap`) ✅ TERMINÉE

1. [x] `TileMap.hpp` : structure de données seule (`enum class Tile`,
       dimensions, taille de case, `std::vector<Tile>` 1D)
2. [x] Constructeur : sol partout, murs sur les bords ; ajout de
       `TileMap.cpp` au CMake
3. [x] `getTile(x, y)` (hors carte = mur) et fonction privée `indice(x, y)`
4. [x] `dessiner(sf::RenderTarget&)` : grille visible, sol brun, murs gris
5. [x] `chevaucheUnMur(sf::FloatRect)` : remplace le mur unique ;
       glissement le long des murs (axes X et Y testés séparément)
6. [x] Construction à partir d'un plan texte (`'#'` = mur), avec
       validation (plan vide, lignes de longueurs différentes) ; l'ancien
       constructeur "bordure" est supprimé

### Étape 5 — Caméra centrée sur le personnage ✅ TERMINÉE

1. [x] Carte agrandie au-delà de la fenêtre (59 × 40 cases), sans caméra
2. [x] `sf::View` centrée sur le carré à chaque image (après la mise à
       jour, avant le dessin)
3. [x] Clic converti en coordonnées du monde (`window.mapPixelToCoords`) ;
       le texte de position reste attaché au point cliqué sur la carte

### Étape 6 — Classe `Escouade` ✅ TERMINÉE

Le carré de `main.cpp` devient une classe, construite à côté de l'ancien
carré puis substituée à lui une fois validée.

1. [x] `Escouade.hpp` : structure de données seule (position, taille,
       cible, vitesse)
2. [x] Constructeur `explicit` (position de départ) ; `Escouade.cpp`
       ajouté au CMake
3. [x] `dessiner` : forme SFML créée à partir des données
4. [x] `allerVers(destination)` : le clic donne la cible
5. [x] `mettreAJour(dt, carte)` : déplacement et collision transférés
       sans modification (déclaration anticipée de `TileMap`) ; validé en
       comparant les deux carrés
6. [x] `getCentre()` pour la caméra ; suppression de l'ancien carré

### Étape 7 — À définir

Pistes discutées :
- Premiers tests unitaires avec Catch2 (`TileMap::getTile`,
  `TileMap::chevaucheUnMur`, cas limites)
- Recherche de chemin (A*) sur la grille, pour contourner les murs
- Finitions : caméra bloquée aux bords de la carte, texte qui suit
  l'escouade ou affichage fixe à l'écran, police committée dans le dépôt

## État actuel (à mettre à jour à chaque session)

- Dépôt Git : `squad-arpg`, poussé sur GitHub
- Outils : conception dans Claude.ai, implémentation avec Claude Code dans
  VSCode (règles dans `CLAUDE.md`)
- Bibliothèque graphique : SFML 3.0.2, intégrée à CMake
- Fenêtre 1200 × 800 ; carte de 59 × 40 cases de 40 px (2360 × 1600 px),
  plusieurs pièces et couloirs, définie en texte dans `main.cpp`
- L'escouade (classe `Escouade`, carré vert de 20 × 20) démarre en
  `(100, 100)`, se déplace à 180 px/s vers le point cliqué, s'arrête
  exactement dessus, est bloquée par les murs et glisse le long de ceux-ci
- Caméra (`sf::View`) toujours centrée sur l'escouade ; du noir apparaît
  au-delà des bords de la carte
- Police : `arial.ttf` du système Windows, chargée via chemin absolu — à
  revoir plus tard pour une police committée dans le projet
- Le texte de position s'affiche au point cliqué (coordonnées du monde)
- Étape en cours : **Étape 7** — à choisir
