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

### Étape 7 — Tests unitaires (Catch2) ✅ TERMINÉE

1. [x] `CMakeLists.txt` réorganisé : `TileMap.cpp` et `Escouade.cpp`
       regroupés dans une bibliothèque statique `squad_core` (SFML en
       `PUBLIC`, dossier `src` en `PUBLIC` include) ; `main` ne compile
       plus que `src/main.cpp` et se lie à `squad_core` en `PRIVATE`
2. [x] `find_package(Catch2 3 REQUIRED)` ; exécutable `tests` (lié à
       `squad_core` et `Catch2::Catch2WithMain`) avec un test trivial pour
       valider la chaîne d'outils
3. [x] Tests du constructeur et de `TileMap::getTile` : sol/mur,
       dimensions déduites du plan, coordonnée négative, plan vide
       (`std::out_of_range`), lignes de longueurs différentes
       (`std::invalid_argument`)
4. [x] Tests de `TileMap::chevaucheUnMur` : sol, chevauchement, contact
       exact sans chevauchement (convention `ceil - 1`), décalage de 1 px,
       hors carte en négatif

### Étape 8 — Module de recherche de chemin (A*) ✅ TERMINÉE

Module `Chemin` autonome, testé, mais pas encore branché sur l'escouade :
elle continue d'aller en ligne droite (branchement prévu à l'étape 9).

1. [x] Squelette : `Chemin.hpp`/`Chemin.cpp`, fonction libre
       `trouverChemin(const TileMap&, sf::Vector2i depart, sf::Vector2i
       arrivee)` (cas départ == arrivée seulement) ; ajout à `squad_core`
       et à l'exécutable `tests` (`test_Chemin.cpp`)
2. [x] Chemin vide si départ ou arrivée est un mur ou hors carte (y
       compris quand départ == arrivée sur un mur)
3. [x] `TileMap::getLargeur`/`getHauteur` (en cases), avec un test sur un
       plan non carré
4. [x] Algorithme A* complet (file de priorité, coûts, parents,
       reconstruction du chemin) ; tests : ligne droite, contournement
       d'un mur, arrivée inaccessible

### Étape 9 — L'escouade suit le chemin du A* ✅ TERMINÉE

1. [x] `TileMap::caseContenant(sf::Vector2f)` : conversion pixels → case
       (`std::floor`, comme `chevaucheUnMur`) ; tests (milieu de case,
       limite exacte entre deux cases, coordonnée négative)
2. [x] `TileMap::centreDeCase(sf::Vector2i)` : conversion case → pixels ;
       tests (case `(0,0)`, une autre case, aller-retour avec
       `caseContenant`)
3. [x] Le centre de l'escouade (et non plus son coin haut-gauche) vise le
       point visé dans `mettreAJour`, via un simple vecteur de
       déplacement appliqué à `m_position`
4. [x] `m_cible` remplacée par `m_chemin` (`std::vector<sf::Vector2f>`) et
       un indice de progression ; `allerVers` y place un seul point,
       `mettreAJour` avance l'indice quand un point est atteint
5. [x] Branchement du A* : nouvelle signature
       `allerVers(destination, const TileMap&)`, appel de
       `trouverChemin` depuis `Escouade` (`main.cpp` ne connaît pas le
       module `Chemin`) ; case de départ ignorée ; chemin vide = ordre
       ignoré
6. [x] Le dernier point du chemin est le point exact cliqué, pas le
       centre de la case d'arrivée

**Limite connue** : en cliquant près du coin d'une case alignée avec
l'escouade sur un seul axe, le centre de case visé peut être presque sur
la même ligne/colonne que l'escouade, ce qui produit parfois un léger
zigzag (l'escouade descend sur l'axe avant de remonter). Comportement
accepté pour l'instant, à revisiter plus tard. **Résolu à l'étape 11**
par le lissage du chemin.

### Étape 10 — Tests unitaires de l'escouade ✅ TERMINÉE

Couvre `Escouade` par des tests Catch2, sans modifier `src/` — ces tests
serviront de filet de sécurité pour le lissage de chemin à venir.

1. [x] `test_Escouade.cpp` créé et ajouté au CMake ; état initial (centre
       = position de départ + `(10, 10)`) et immobilité sans ordre
2. [x] Test de vitesse : un seul tic vers une destination lointaine en
       terrain dégagé avance le centre de `vitesse * dt`, quelle que soit
       la direction exacte
3. [x] Fonction d'aide de simulation (tics de 1/60 s pendant une durée
       donnée) ; tests d'arrivée : point exact hors centre de case en
       terrain dégagé, et contournement d'un mur
4. [x] Gestion des ordres : ordre vers un mur à l'arrêt (ignoré), ordre
       vers un mur en cours de route (trajet d'origine conservé), second
       ordre valide en cours de route (remplace la destination)

Aucun comportement inattendu révélé : les 8 tests passent sans qu'aucune
modification de `src/` n'ait été nécessaire.

### Étape 11 — Lissage du chemin ✅ TERMINÉE

Retire du chemin calculé par le A* les points de passage inutiles :
l'escouade va en ligne droite vers le point le plus lointain qu'elle peut
atteindre sans toucher un mur. Corrige le zigzag de l'étape 9.

1. [x] `TileMap::trajetLibre(sf::FloatRect, sf::Vector2f)`, cas
       horizontal et vertical (enveloppe rectangulaire + `chevaucheUnMur`) ;
       tests (trajet dégagé, mur sur le trajet, contact exact, décalage
       de 1 px, déplacement nul, sortie de carte)
2. [x] `trajetLibre`, cas oblique : balayage par colonnes de cases,
       paramétré par `t` entre 0 (départ) et 1 (arrivée) ; tests (trajet
       oblique dégagé, mur en plein milieu dans les 4 sens obliques, coin
       qui accroche malgré une ligne des centres dégagée, trajet voisin
       libre, arrivée et départ dans un mur)
3. [x] `Chemin::lisserChemin` : algorithme glouton (recherche du point le
       plus lointain atteignable, en partant de la fin) ; tests (liste
       vide, un seul point, terrain dégagé → seul le dernier point
       reste, mur à contourner → moins de points, segments tous libres)
4. [x] Branchement dans `Escouade::allerVers`, entre la construction du
       chemin en pixels et son stockage ; les 8 tests d'`Escouade` de
       l'étape 10 passent sans modification, plus un test de vitesse
       (ligne droite lissée assez rapide pour un trajet diagonal, alors
       qu'un escalier non lissé aurait été trop lent pour la même durée
       simulée)

**Limite connue** : `lisserChemin` ne fait que retirer des points de la
liste donnée par le A* (centres de case) ; il ne peut pas en ajouter ni
en déplacer. Autour d'un pilier, dès qu'un saut direct est bloqué par le
mur, l'escouade retombe sur le centre de la case suivante tel quel,
plutôt que sur un point plus proche du coin du pilier qui raccourcirait
encore le trajet. Elle frôle donc moins les murs qu'un lissage « idéal »
(type *funnel algorithm* ou Theta*) ne le ferait, mais elle ne reste
jamais coincée. Accepté pour l'instant (cf. ARCHITECTURE.md).

### Étape 12 — Membres de l'escouade (`Personnage`, formation en carré) ✅ TERMINÉE

L'escouade possède 4 personnages qui se déplacent eux-mêmes pour tenir
une formation en carré autour d'elle, avec collision contre les murs, et
la formation s'oriente selon la direction de déplacement. Principe
retenu pour l'avenir (combat, actions) : un personnage sait seulement
« aller vers un point » ; c'est l'escouade qui décide de ce point.

1. [x] `Personnage.hpp`/`.cpp` : structure de données (`m_centre`,
       `m_taille`, `m_couleur`) et constructeur ; ajout à `squad_core`
2. [x] `Personnage::dessiner`
3. [x] `Escouade` : `m_decalages` (4 décalages de formation, orientation
       0 = vers +x) et `m_personnages`, créés à leur place dans le
       constructeur via la fonction privée `placePersonnage` (point de
       calcul unique) ; dessinés après l'ancre
4. [x] `Personnage::mettreAJour(dt, cible)` : ligne droite, pas fixe,
       sans collision à ce stade
5. [x] `Escouade::mettreAJour` met à jour l'ancre puis appelle celle de
       chaque personnage avec sa place courante
6. [x] Module `Deplacement` : extraction de `deplacerAvecGlissement`
       (collision/glissement axe par axe) depuis `Escouade::mettreAJour`,
       sans modification de comportement ; les 9 tests d'`Escouade`
       passent sans changement
7. [x] `tests/test_Deplacement.cpp` : terrain dégagé, mur qui bloque un
       axe, glissement oblique (composante parallèle conservée),
       déplacement nul
8. [x] `Personnage::mettreAJour` utilise `deplacerAvecGlissement` ; les
       personnages ne traversent plus les murs
9. [x] `Escouade::m_orientation` (`sf::Angle`, 0 au départ), utilisée
       dans `placePersonnage` via `rotatedBy`
10. [x] `Escouade::mettreAJour` met à jour `m_orientation` selon la
        direction **voulue** de l'ancre (avant collision), seulement si
        elle a un point à atteindre et que la distance est `> 0` ;
        rotation instantanée, dernière orientation conservée à l'arrêt

**Limite connue** : rotation instantanée de la formation (pas de
transition progressive) — un personnage peut donc traverser brusquement
sa place à chaque virage du chemin. Accepté pour l'instant (cf.
ARCHITECTURE.md). Autre limite héritée des décisions de conception : pas
de collision entre personnages ni entre un personnage et l'ancre ; un
personnage peut rester accroché derrière un coin de mur tant que
l'escouade ne repasse pas par là.

### Étape 13 — À définir

Pistes discutées :
- Rotation progressive de la formation plutôt qu'instantanée
- Un lissage plus serré autour des coins (*funnel algorithm* ou Theta*),
  si le contournement actuel des piliers s'avère insuffisant
- Finitions : caméra bloquée aux bords de la carte, texte qui suit
  l'escouade ou affichage fixe à l'écran, police committée dans le dépôt
- Types de personnages, statistiques, combat, actions (hors de portée
  explicite de l'étape 12)

## État actuel (à mettre à jour à chaque session)

- Dépôt Git : `squad-arpg`, poussé sur GitHub
- Outils : conception dans Claude.ai, implémentation avec Claude Code dans
  VSCode (règles dans `CLAUDE.md`)
- Bibliothèque graphique : SFML 3.0.2, intégrée à CMake
- Fenêtre 1200 × 800 ; carte de 59 × 40 cases de 40 px (2360 × 1600 px),
  plusieurs pièces et couloirs, définie en texte dans `main.cpp`
- L'escouade (classe `Escouade`, carré vert de 20 × 20, l'ancre) démarre
  en `(100, 100)`, se déplace à 180 px/s en suivant le chemin calculé par
  le A* puis lissé (trajets directs en terrain dégagé, plus de zigzag,
  coins de murs contournés au plus court sans les frôler exactement),
  s'arrête exactement sur le point cliqué, reste bloquée par les murs et
  glisse le long de ceux-ci (filet de sécurité conservé)
- L'escouade possède 4 personnages (`Personnage`, carrés colorés de
  20 × 20, 240 px/s) qui tiennent une formation en carré autour de
  l'ancre, bloqués par les murs et glissant le long d'eux comme elle
  (fonction partagée `deplacerAvecGlissement`, module `Deplacement`) ;
  la formation pivote selon la direction de marche (rotation
  instantanée)
- Caméra (`sf::View`) toujours centrée sur l'escouade ; du noir apparaît
  au-delà des bords de la carte
- Police : `arial.ttf` du système Windows, chargée via chemin absolu — à
  revoir plus tard pour une police committée dans le projet
- Le texte de position s'affiche au point cliqué (coordonnées du monde)
- Logique de jeu (`TileMap`, `Escouade`, `Chemin`, `Personnage`,
  `Deplacement`) dans une bibliothèque `squad_core`, testée avec Catch2 3
  (`./build/tests.exe`, 58 tests : 32 sur `TileMap`, 13 sur `Chemin`,
  9 sur `Escouade`, 4 sur `Deplacement`)
- Près des piliers, le contournement n'est pas toujours le plus court
  possible (voir étape 11, limite connue) — accepté pour l'instant
- Rotation de la formation instantanée, pas progressive (voir étape 12,
  limite connue) — accepté pour l'instant
- Étape en cours : **Étape 13** — à choisir
