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
│   ├── test_Monstre.cpp
│   └── test_Personnage.cpp
├── docs/
│   ├── build.md
│   ├── boucle-et-entrees.md
│   ├── carte-et-collision.md
│   ├── chemin.md
│   ├── formation.md
│   └── tests.md
├── .gitignore
├── CLAUDE.md
├── ARCHITECTURE.md
└── ROADMAP.md
```

Six modules : `TileMap` (l'environnement de jeu), `Escouade` (le point
de contrôle, l'ancre, qui suit un chemin calculé par le A*, possède les
personnages et les fait attaquer), `Chemin` (recherche de chemin A*,
utilisé uniquement par `Escouade`), `Personnage` (un membre de
l'escouade, qui sait aller vers un point et frapper une cible à
portée), `Deplacement` (collision/glissement partagée entre `Escouade`
et `Personnage`) et `Monstre` (un ennemi immobile, avec un identifiant,
des points de vie et une riposte à portée, possédé directement par
`main.cpp` dans un `std::vector<Monstre>`, sans lien direct avec
`Escouade` ni `Personnage` — `main.cpp` fait l'intermédiaire).
`main.cpp` garde la fenêtre, la boucle de jeu, les événements
(fermeture, et le verrouillage de cible sur un monstre), l'état de la
souris (déplacement), la caméra, la carte (son plan) et les monstres ;
il ne connaît ni `Chemin` ni `Personnage`, et ne gère plus la recharge
d'attaque lui-même depuis l'étape 17 (`Escouade::attaquer` s'en charge
via les personnages). La structure continue d'évoluer par nécessité
réelle, pas par anticipation (voir principes ci-dessous).

Deux exécutables : `main` (le jeu) et `tests` (les tests Catch2). Les deux
se lient à la bibliothèque statique `squad_core`, qui contient toute la
logique de jeu (`TileMap`, `Escouade`, `Chemin`, `Personnage`,
`Deplacement`, `Monstre`) indépendamment de la fenêtre et de la boucle de
jeu.

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
- **Fonction privée `indicePersonnageLePlusProche(point)`** (étape 20) :
  seul endroit du code qui cherche, parmi les 4 personnages, celui dont
  le centre est le plus proche d'un point donné (boucle simple,
  comparaison de distances) ; utilisée par `personnageLePlusProche` et
  par `recevoirDegats`, pour que les deux désignent toujours le même
  personnage par le même critère.

Interface publique (inchangée pour `main.cpp`, qui ne connaît pas
`Personnage`) :

| Fonction | Rôle |
|---|---|
| `explicit Escouade(sf::Vector2f positionDepart)` | Crée l'escouade (et ses 4 personnages en formation) à la position donnée |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Calcule un chemin vers la destination (A*, puis lissé) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe |
| `int attaquer(sf::Vector2f cible, const TileMap& carte)` | Approche la cible (`allerVers`) tant qu'un personnage n'est pas à portée, s'arrête sur place quand les 4 y sont, puis fait frapper chacun ; renvoie la somme des dégâts |
| `void mettreAJour(float dt, const TileMap& carte)` | Avance l'ancre vers le point courant du chemin (collision, glissement, orientation), puis chaque personnage vers sa place |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine l'ancre, puis les 4 personnages |
| `sf::Vector2f getCentre() const` | Centre de l'escouade (utilisé par la caméra, par `allerVers` et par `placePersonnage`) |
| `sf::Vector2f personnageLePlusProche(sf::Vector2f point) const` | Centre du personnage le plus proche de `point` (utilisé par `main.cpp` pour désigner la cible d'un monstre) |
| `void recevoirDegats(sf::Vector2f origine, int quantite)` | Applique `quantite` de dégâts au personnage le plus proche d'`origine` |

### `Chemin` — recherche de chemin (A*)

Une fonction libre, pas une classe : l'algorithme n'a aucun état à
conserver entre deux appels.

- **Travaille en cases** (coordonnées de grille, `sf::Vector2i`), jamais
  en pixels — contrairement à `Escouade`, qui travaille en pixels
  (`sf::Vector2f`).
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
  puisse rattraper sa place), `m_portee` (80 px), `m_degats` (5),
  `m_tempsRecharge` (0,5 s, ne change jamais) et `m_recharge` (secondes
  avant le prochain coup possible, 0 à la création, décrémentée dans
  `mettreAJour` sans descendre sous zéro) — valeurs fixes identiques
  pour les 4, comme `m_vitesse`, en attendant de dépendre d'une arme ou
  de capacités propres à chacun. Depuis l'étape 20 : `m_pointsDeVie` et
  `m_pointsDeVieMax` (`100` chacun à la création), dupliqués
  volontairement de `Monstre` (même logique de points de vie et de
  barre de vie) — aucune factorisation entre les deux classes pour
  l'instant.
- **Déclaration anticipée** : `Personnage.hpp` contient `class TileMap;`
  (une référence ne demande pas la définition complète) ; aucun include
  de `TileMap.hpp` n'est nécessaire dans `Personnage.cpp` non plus,
  puisque la carte n'est que transmise à `deplacerAvecGlissement`, jamais
  utilisée directement.
- **Pas de A* ni de lissage** : juste une ligne droite vers la cible, pas
  fixe (`vitesse * dt`), arrêt exact si la distance restante est `≤` ce
  pas — même logique que l'ancre avant l'étape 11.
- **Barre de vie dans `dessiner`** (étape 20) : même apparence que celle
  de `Monstre` (fond sombre + rectangle vert proportionnel aux points de
  vie restants, au-dessus du personnage).
- **Un personnage à 0 point de vie ne meurt pas encore** : il reste en
  formation, continue de frapper et reste ciblable par les monstres
  (limite connue, voir état actuel de ROADMAP.md).

Interface publique :

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, sf::Color couleur)` | Crée le personnage à ce centre, avec cette couleur |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte)` | Avance vers la cible (collision, glissement), puis décrémente la recharge d'attaque |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage et sa barre de vie |
| `bool estAPortee(sf::Vector2f cible) const` | La distance entre le centre et `cible` est-elle `<=` `m_portee` ? |
| `int frapper(sf::Vector2f cible)` | Si `cible` est à portée et la recharge écoulée, relance la recharge et renvoie `m_degats` ; sinon renvoie 0 |
| `void recevoirDegats(int quantite)` | Retire des points de vie, sans descendre sous zéro |
| `bool estVivant() const` | Points de vie `> 0` |
| `sf::Vector2f getCentre() const` | Centre du personnage (utilisé par `Escouade::indicePersonnageLePlusProche`) |

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
  la création), `m_id`, et depuis l'étape 20 : `m_portee` (80 px),
  `m_degats` (10), `m_tempsRecharge` (1 s) et `m_recharge` (secondes
  avant le prochain coup possible, 0 à la création) — même structure
  que la portée/recharge de `Personnage`, dupliquée volontairement (pas
  de factorisation pour l'instant). Pas de couleur stockée : toujours
  rouge, donc une valeur fixe directement dans `dessiner`, pas un membre
  (contrairement à `Personnage`, qui a besoin d'une couleur différente
  par instance).
- **`Monstre(sf::Vector2f centre, int id)`** : centre et identifiant en
  paramètres ; `explicit` retiré puisque le constructeur en prend deux
  (une conversion implicite à un seul argument n'est plus possible).
  C'est `main.cpp` qui attribue les identifiants (variable locale
  `prochainId`, incrémentée à chaque création) — pas de compteur
  `static` dans `Monstre`.
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
- **Possédé par `main.cpp`**, pas par `Escouade` : un `std::vector<Monstre>`
  (trois monstres pour l'instant), pas une classe dédiée (`Horde`,
  `Monstres`...) pour les porter. Un monstre mort est retiré du vecteur
  (`std::erase_if`, à chaque image) ; l'objet lui-même cesse donc
  d'exister, contrairement à l'étape 17 où il restait en mémoire sans
  être dessiné ni attaquable.
- **Pas de collision avec lui** : l'escouade et les personnages lui
  passent dessus ; le A* l'ignore complètement (il n'existe pas du
  point de vue de `TileMap`).
- **Contour jaune sur la cible verrouillée** (étape 19) : `dessiner`
  reçoit un paramètre `bool estCible`, calculé par `main.cpp` par
  comparaison directe (`idCible == monstre.getId()`) — `Monstre` ne
  stocke pas lui-même le fait d'être visé, `idCible` reste l'unique
  source de vérité. Contour de 3 px vers l'intérieur
  (`setOutlineThickness(-3.0f)`), pour ne pas toucher la barre de vie
  ni changer l'empreinte du monstre.
- **Riposte** (étape 20) : `mettreAJour(dt)` décrémente seulement
  `m_recharge` (le monstre reste immobile) ; `frapper(cible)` a le même
  contrat que `Personnage::frapper` (dégâts si `cible` est à portée et
  la recharge écoulée, sinon 0 sans toucher la recharge). `Monstre` ne
  connaît toujours pas `Escouade` ni `Personnage` : il frappe un simple
  `sf::Vector2f`, c'est `main.cpp` qui lui donne le centre du
  personnage le plus proche (`Escouade::personnageLePlusProche`) et qui
  transmet le résultat à `Escouade::recevoirDegats`.

Interface publique :

| Fonction | Rôle |
|---|---|
| `Monstre(sf::Vector2f centre, int id)` | Crée le monstre à ce centre, avec cet identifiant, 30 × 30, 100 points de vie |
| `void dessiner(sf::RenderTarget& rendu, bool estCible) const` | Dessine le monstre et sa barre de vie ; ajoute un contour jaune si `estCible` |
| `void mettreAJour(float dt)` | Décrémente la recharge d'attaque, sans descendre sous zéro |
| `int frapper(sf::Vector2f cible)` | Si `cible` est à portée et la recharge écoulée, relance la recharge et renvoie `m_degats` ; sinon renvoie 0 |
| `sf::Vector2f getCentre() const` | Centre du monstre (utilisé par `main.cpp` pour le donner à `Escouade::attaquer` et `personnageLePlusProche`) |
| `int getId() const` | Identifiant du monstre (utilisé par `main.cpp` pour désigner la cible verrouillée) |
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

## Documentation détaillée (`docs/`)

Les décisions techniques détaillées (le pourquoi, pas seulement le quoi)
vivent dans `docs/`, un fichier par sujet, pour que `CLAUDE.md` n'ait pas
à importer tout ce détail à chaque session. Avant de modifier un module,
lire le fichier correspondant :

| Fichier | Contenu | Quand le lire |
|---|---|---|
| `docs/build.md` | Build (CMake, `squad_core`, exécutables) | Avant de modifier `CMakeLists.txt` ou d'ajouter un fichier source |
| `docs/boucle-et-entrees.md` | Boucle de jeu, déplacement, entrée souris et attaque, caméra | Avant de modifier `main.cpp` |
| `docs/carte-et-collision.md` | Carte, collision et dessin, plan texte et validation | Avant de modifier `TileMap` ou `Deplacement` |
| `docs/chemin.md` | Recherche de chemin (A*), suivi de chemin par l'escouade, lissage de chemin | Avant de modifier `Chemin` ou le suivi de chemin dans `Escouade` |
| `docs/formation.md` | `Escouade`, formation de l'escouade | Avant de modifier `Escouade` ou `Personnage` |
| `docs/tests.md` | Tests (Catch2, conventions) | Avant d'ajouter ou de modifier un test |
