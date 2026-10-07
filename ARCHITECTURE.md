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
│   ├── Monstre.cpp
│   ├── Attaque.hpp
│   ├── Attaque.cpp
│   ├── Mobile.hpp
│   └── Mobile.cpp
├── tests/
│   ├── test_TileMap.cpp
│   ├── test_Chemin.cpp
│   ├── test_Escouade.cpp
│   ├── test_Deplacement.cpp
│   ├── test_Monstre.cpp
│   ├── test_Personnage.cpp
│   ├── test_Attaque.cpp
│   └── test_Mobile.cpp
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

Huit modules : `TileMap` (l'environnement de jeu), `Escouade` (le point
de contrôle, l'ancre, qui possède les personnages et les fait
attaquer), `Mobile` (déplacement par chemin calculé par le A*, possédé
par valeur par `Escouade` et, depuis l'étape 24, par `Monstre`),
`Chemin` (recherche de chemin A*, utilisé uniquement par `Mobile`),
`Personnage` (un membre de l'escouade, qui sait aller vers un point et
frapper une cible à portée), `Deplacement` (collision/glissement
partagée entre `Mobile` et `Personnage`), `Monstre` (un ennemi qui
détecte, poursuit et abandonne une cible via son propre `Mobile`, avec
un identifiant, des points de vie et une riposte à portée, possédé
directement par `main.cpp` dans un `std::vector<Monstre>`, sans lien
direct avec `Escouade` ni `Personnage` — `main.cpp` fait
l'intermédiaire) et `Attaque` (portée, dégâts et recharge, possédée par
valeur par `Personnage` et par `Monstre`).
`main.cpp` garde la fenêtre, la boucle de jeu, les événements
(fermeture, et le verrouillage de cible sur un monstre), l'état de la
souris (déplacement), la caméra, la carte (son plan) et les monstres ;
il ne connaît ni `Chemin` ni `Personnage`, et ne gère plus la recharge
d'attaque lui-même depuis l'étape 17 (`Escouade::attaquer` s'en charge
via les personnages). Depuis l'étape 24, il donne aussi à chaque
monstre le personnage le plus proche de lui comme cible de poursuite
(`Escouade::personnageLePlusProche`), avant de relire son centre à jour
pour la riposte. La structure continue d'évoluer par nécessité réelle,
pas par anticipation (voir principes ci-dessous).

Deux exécutables : `main` (le jeu) et `tests` (les tests Catch2). Les deux
se lient à la bibliothèque statique `squad_core`, qui contient toute la
logique de jeu (`TileMap`, `Escouade`, `Mobile`, `Chemin`, `Personnage`,
`Deplacement`, `Monstre`, `Attaque`) indépendamment de la fenêtre et de la boucle de
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

- **Données membres** : `m_mobile` (`Mobile`, depuis l'étape 23 — porte
  le déplacement par chemin, voir plus bas), `m_decalages` (les 4
  décalages de formation, pour une escouade orientée vers `+x`),
  `m_orientation` (`sf::Angle`, `0` = vers `+x`, ce qu'utilise
  `placePersonnage`), `m_orientationVoulue` (`sf::Angle`, la direction
  vers laquelle `m_orientation` tourne progressivement),
  `m_vitesseRotation` (degrés par seconde), `m_personnages`
  (`std::vector<Personnage>`, inclut `Personnage.hpp` en entier — un
  `std::vector` a besoin de la définition complète de son type
  d'élément).
- **Valeurs fixes** passées au constructeur de `m_mobile` : taille
  20 × 20, vitesse 180 px/s. Orientation `0` à la création. Elles
  deviendront des paramètres quand il le faudra.
- **Fonction privée `placePersonnage(indice)`** : seul endroit du code
  qui calcule la place d'un personnage (`getCentre() + m_decalages[indice]
  .rotatedBy(m_orientation)`) — utilisée à la fois pour créer les
  personnages dans le constructeur et pour leur donner leur cible à
  chaque `mettreAJour`.
- **Fonction privée `indicePersonnageLePlusProche(point)`** (étape 20,
  filtrage des morts à l'étape 21) : seul endroit du code qui cherche,
  parmi les personnages **vivants**, celui dont le centre est le plus
  proche d'un point donné (boucle simple, comparaison de distances,
  `std::optional<std::size_t>` vide si aucun n'est vivant) ; utilisée
  par `personnageLePlusProche` et par `recevoirDegats`, pour que les
  deux désignent toujours le même personnage par le même critère.

Interface publique (inchangée pour `main.cpp`, qui ne connaît pas
`Personnage`) :

| Fonction | Rôle |
|---|---|
| `explicit Escouade(sf::Vector2f positionDepart)` | Crée l'escouade (et ses 4 personnages en formation) à la position donnée |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Délègue à `m_mobile.allerVers` |
| `int attaquer(sf::Vector2f cible, const TileMap& carte)` | Approche la cible (`allerVers`) tant qu'un personnage n'est pas à portée, s'arrête sur place (`m_mobile.arreter()`) quand les 4 y sont, puis fait frapper chacun ; renvoie la somme des dégâts |
| `void mettreAJour(float dt, const TileMap& carte)` | Met à jour `m_orientationVoulue` à partir de `m_mobile.directionVoulue()`, avance l'ancre (`m_mobile.mettreAJour`), met à jour l'orientation, puis chaque personnage vers sa place |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine l'ancre (position reconstruite à partir de `getCentre()` et `m_mobile.getTaille()`), puis les 4 personnages |
| `sf::Vector2f getCentre() const` | Délègue à `m_mobile.getCentre()` (utilisé par la caméra, par `allerVers` et par `placePersonnage`) |
| `std::optional<sf::Vector2f> personnageLePlusProche(sf::Vector2f point) const` | Centre du personnage **vivant** le plus proche de `point` (vide si aucun n'est vivant) ; utilisé par `main.cpp` pour désigner la cible d'un monstre |
| `void recevoirDegats(sf::Vector2f origine, int quantite)` | Applique `quantite` de dégâts au personnage vivant le plus proche d'`origine` ; ne fait rien si aucun n'est vivant |

### `Mobile` — déplacement par chemin (étape 23)

Possédée par valeur par `Escouade` (`m_mobile`) et, depuis l'étape 24,
par `Monstre` également. Porte tout ce qu'`Escouade` faisait elle-même
avant l'étape 23 pour suivre un chemin calculé par le A* : position,
chemin, vitesse, collision/glissement.

- **Composition, pas héritage** : `Escouade` a un membre `Mobile
  m_mobile` ; aucune classe de base commune, aucune fonction virtuelle.
- **Données membres** : `m_position` (coin haut-gauche, pixels du
  monde), `m_taille`, `m_chemin` (`std::vector<sf::Vector2f>`, les
  points — son centre — à atteindre dans l'ordre), `m_indexChemin`
  (indice du prochain point visé), `m_vitesse` (pixels par seconde) —
  les mêmes données qu'`Escouade` portait directement avant cette
  étape.
- **Interface en centres** : le constructeur et `getCentre` raisonnent
  en centre, pas en coin haut-gauche (`Escouade::placePersonnage` et le
  reste du code de formation ont toujours raisonné en centre). En
  interne, `m_position` reste le coin haut-gauche, comme avant, pour
  que le code d'`allerVers` et de `mettreAJour` ait pu être déplacé sans
  modification.
- **`allerVers` et la partie « ancre » de `mettreAJour`** sont le code
  d'`Escouade` déplacé tel quel (A* + lissage pour le premier,
  déplacement à pas fixe + collision/glissement pour le second), sans
  aucune réécriture.
- **`arreter()`** extrait l'arrêt sur place qu'`Escouade::attaquer`
  faisait en ligne (`m_chemin = {getCentre()}`) quand les 4 personnages
  sont à portée.
- **`directionVoulue()`** : vecteur du centre vers le point courant du
  chemin, vide s'il n'y a plus de point à atteindre ou si la distance
  est nulle — remplace l'écriture directe de `m_orientationVoulue` dans
  l'ancien `Escouade::mettreAJour`. `Escouade` l'appelle avant
  `m_mobile.mettreAJour(dt, carte)`, pour lire la direction au même
  moment du tic qu'avant (avant l'essai de collision), et en déduit
  elle-même son `m_orientationVoulue` (`direction.angle()`).
- **Ne connaît ni `Escouade`, ni `Personnage`, ni `Monstre`, ni
  `Attaque`**, et ne dessine rien ; connaît `TileMap`, `Chemin` et
  `Deplacement`. `Escouade.cpp` n'inclut plus directement ni
  `TileMap.hpp`, ni `Chemin.hpp`, ni `Deplacement.hpp`.

Interface publique :

| Fonction | Rôle |
|---|---|
| `Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)` | Crée le mobile à ce centre, cette taille, cette vitesse ; chemin initial réduit au centre (immobile) |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Calcule un chemin vers la destination (A*, puis lissé) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe |
| `void arreter()` | Réduit le chemin au centre actuel, pour cesser d'avancer sans reculer |
| `void mettreAJour(float dt, const TileMap& carte)` | Avance vers le point courant du chemin (collision, glissement) |
| `sf::Vector2f getCentre() const` | Centre du mobile |
| `sf::Vector2f getTaille() const` | Taille du mobile |
| `std::optional<sf::Vector2f> directionVoulue() const` | Vecteur du centre vers le point courant du chemin ; vide s'il n'y a plus de point à atteindre ou si la distance est nulle |

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
pixels. Utilisé uniquement par `Mobile::allerVers` (depuis l'étape 23 ;
auparavant `Escouade::allerVers`) ; `main.cpp` ne connaît pas ce module.

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
  puisse rattraper sa place) et, depuis l'étape 22, `m_attaque`
  (`Attaque`, portée 80 px / dégâts 5 / recharge 0,5 s — valeurs fixes
  identiques pour les 4, comme `m_vitesse`, en attendant de dépendre
  d'une arme ou de capacités propres à chacun). Depuis l'étape 20 :
  `m_pointsDeVie` et `m_pointsDeVieMax` (`100` chacun à la création),
  dupliqués volontairement de `Monstre` (même logique de points de vie
  et de barre de vie) — aucune factorisation entre les deux classes
  pour l'instant.
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
  formation et continue de se déplacer (limite connue, voir état actuel
  de ROADMAP.md). Depuis l'étape 21, il ne frappe plus
  (`Personnage::frapper`) et n'est plus ciblé par les monstres
  (`Escouade::indicePersonnageLePlusProche` l'ignore).
- **`estAPortee` et `frapper` délèguent à `m_attaque`** (étape 22),
  avec `m_centre` comme origine ; `mettreAJour` délègue la
  décrémentation de la recharge à `m_attaque.mettreAJour(dt)`. La règle
  « un personnage mort ne frappe pas » reste dans `Personnage::frapper`,
  avant la délégation.

Interface publique :

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, sf::Color couleur)` | Crée le personnage à ce centre, avec cette couleur |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte)` | Avance vers la cible (collision, glissement), puis décrémente la recharge d'attaque |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage et sa barre de vie |
| `bool estAPortee(sf::Vector2f cible) const` | La distance entre le centre et `cible` est-elle `<=` la portée de `m_attaque` ? |
| `int frapper(sf::Vector2f cible)` | Si le personnage est vivant, délègue à `m_attaque.frapper` avec `m_centre` comme origine ; sinon renvoie 0 |
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

Utilisée par `Mobile::mettreAJour` (pour l'ancre, depuis l'étape 23 ;
auparavant `Escouade::mettreAJour`) et par `Personnage::mettreAJour`
(qui convertit entre son centre et le coin haut-gauche attendu par la
fonction, comme `Mobile::getCentre()` le fait dans l'autre sens).

### `Monstre` — un ennemi qui détecte, poursuit et retourne à son poste

Un carré rouge de 30 × 30, avec des points de vie et une barre de vie.
Pas de classe de base commune avec `Personnage`, pas d'héritage : les
deux classes restent indépendantes, même si toutes les deux stockent un
centre (et, depuis l'étape 24, se déplacent par un `Mobile`) — la
ressemblance n'implique pas de dépendance commune tant qu'aucun
comportement partagé ne le justifie.

- **Position et taille portées par `m_mobile`** (`Mobile`, depuis
  l'étape 24 ; taille 30 × 30, vitesse 120 px/s) : `m_centre` et
  `m_taille` ont quitté `Monstre`, une seule copie de ces données
  existe désormais. `getCentre`, `contient`, `dessiner` et `frapper`
  lisent `m_mobile.getCentre()`/`getTaille()`.
- **Données membres** : `m_mobile`, `m_pointsDeVie` et
  `m_pointsDeVieMax` (`100` chacun à la création), `m_id`, `m_attaque`
  (`Attaque`, portée 80 px / dégâts 10 / recharge 1 s — même structure
  que celle de `Personnage`, valeurs différentes, dupliquée
  volontairement comme objet, pas de factorisation des points de vie
  pour l'instant), `m_rayonDetection` (200 px, depuis l'étape 24), et
  depuis l'étape 25 : `m_rayonTerritoire` (400 px, renommé et resenti
  depuis `m_rayonAbandon` de l'étape 24), `m_etat` (`Etat`, un
  `enum class` **privé** à trois valeurs — `EnGarde`, `Poursuite`,
  `Retour` —, `EnGarde` à la création) et `m_poste` (le centre donné au
  constructeur, fixe). Pas de couleur stockée : toujours rouge, donc
  une valeur fixe directement dans `dessiner`, pas un membre
  (contrairement à `Personnage`, qui a besoin d'une couleur différente
  par instance).
- **Statut à trois valeurs plutôt que des booléens** (étape 25) :
  `enum class Etat { EnGarde, Poursuite, Retour }`, déclaré privé dans
  `Monstre.hpp` (comme `Tile` dans `TileMap`, mais à portée encore plus
  restreinte) — un `enum class` rend impossible l'état incohérent
  « en poursuite et en retour à la fois » que deux `bool` indépendants
  auraient permis. « À portée, en train de frapper » n'est pas un
  statut : recalculé à chaque image à partir de la distance
  (`m_attaque.estAPortee`), comme avant l'étape 25.
- **Détection limitée au territoire** (étape 25), dans une fonction
  privée `const`, `proieDetectee(cible)` : une proie est détectée si
  elle existe, si elle est à `<=` `m_rayonDetection` **du monstre** ET
  à `<=` `m_rayonTerritoire` **du poste** — la seconde condition évite
  qu'un monstre en retour ne redétecte aussitôt une proie restée à la
  frontière et ne tremble sur place. Utilisée à l'identique en
  `EnGarde` et en `Retour`, pour que les deux appliquent exactement la
  même règle.
- **Transitions, une seule par appel de `mettreAJour`** : `EnGarde` →
  `Poursuite` si `proieDetectee` ; `Poursuite` → `Retour` si plus de
  proie, ou si le monstre lui-même est à `>` `m_rayonTerritoire` de
  `m_poste` (mesuré sur le monstre, pas sur la proie, depuis
  l'étape 25 — avant, la règle portait sur la distance à la proie) ;
  `Retour` → `Poursuite` si `proieDetectee` ; `Retour` → `EnGarde` une
  fois arrivé (`m_mobile.directionVoulue()` vide).
- **Ordres donnés à `m_mobile`, séparés des transitions** : à l'entrée
  en `Retour` **seulement** (détectée en comparant l'état avant et
  après le `switch` des transitions, pour agir une seule fois et non à
  chaque image), `arreter()` puis `allerVers(m_poste, carte)` ; en
  `Poursuite`, à chaque image, `arreter()` si la proie est à portée de
  `m_attaque`, sinon `allerVers(proie, carte)` ; en `EnGarde` et
  pendant le reste du `Retour`, aucun ordre (le chemin donné à l'entrée
  suffit à `Mobile`). La riposte (`frapper`) ne dépend pas du statut :
  un monstre en retour frappe toujours ce qui passe à sa portée.
- **Les distances (détection, territoire, portée) sont mesurées à vol
  d'oiseau**, centre à centre, en ignorant les murs — comme la portée
  d'attaque (étape 17, limite connue).
- **Limites connues** (étape 25) : un monstre qui rentre ignore une
  proie dès qu'elle sort du territoire, même si elle reste proche de
  lui (le filtre `proieDetectee` l'exige) ; pas de patrouille ni de
  récupération de points de vie au retour ; plusieurs monstres
  gardent chacun leur propre poste et territoire, sans les partager ni
  se coordonner.
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
  (six monstres depuis l'étape 24, répartis sur la carte), pas une
  classe dédiée (`Horde`, `Monstres`...) pour les porter. Un monstre
  mort est retiré du vecteur (`std::erase_if`, à chaque image) ;
  l'objet lui-même cesse donc d'exister, contrairement à l'étape 17 où
  il restait en mémoire sans être dessiné ni attaquable.
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
- **Riposte** (étape 20) : `frapper(cible)` délègue entièrement à
  `m_attaque.frapper`, avec `m_mobile.getCentre()` comme origine — sans
  règle supplémentaire, contrairement à `Personnage::frapper` qui
  vérifie d'abord `estVivant()`. `Monstre` ne connaît toujours pas
  `Escouade` ni `Personnage` : il frappe un simple `sf::Vector2f`,
  c'est `main.cpp` qui lui donne le centre du personnage le plus proche
  (`Escouade::personnageLePlusProche`) et qui transmet le résultat à
  `Escouade::recevoirDegats`.
- **`main.cpp` donne la cible, relit le centre après coup** (étape 24) :
  pour chaque monstre, `escouade.personnageLePlusProche(m.getCentre())`
  est calculé *avant* `m.mettreAJour(dt, cible, carte)` (sur la position
  précédente du monstre), puis `m.getCentre()` est relu *après* pour la
  riposte (`frapper`/`recevoirDegats`), sur la position à jour.

Interface publique :

| Fonction | Rôle |
|---|---|
| `Monstre(sf::Vector2f centre, int id)` | Crée le monstre à ce centre, avec cet identifiant, 30 × 30, 100 points de vie |
| `void dessiner(sf::RenderTarget& rendu, bool estCible) const` | Dessine le monstre et sa barre de vie ; ajoute un contour jaune si `estCible` |
| `void mettreAJour(float dt, std::optional<sf::Vector2f> cible, const TileMap& carte)` | Décrémente la recharge d'attaque, met à jour l'état de poursuite (détection/abandon par hystérésis) et avance vers `cible` en poursuite (s'arrête à portée d'attaque), reste sur place sinon |
| `int frapper(sf::Vector2f cible)` | Délègue à `m_attaque.frapper`, avec `m_mobile.getCentre()` comme origine |
| `sf::Vector2f getCentre() const` | Centre du monstre (utilisé par `main.cpp` pour le donner à `Escouade::attaquer` et `personnageLePlusProche`) |
| `int getId() const` | Identifiant du monstre (utilisé par `main.cpp` pour désigner la cible verrouillée) |
| `bool contient(sf::Vector2f point) const` | Le point (pixels du monde) est-il dans le carré du monstre ? |
| `void recevoirDegats(int quantite)` | Retire des points de vie, sans descendre sous zéro |
| `bool estVivant() const` | Points de vie `> 0` |

### `Attaque` — portée, dégâts et recharge (étape 22)

Possédée par valeur par `Personnage` et par `Monstre`, qui portaient
chacun la même structure (portée, dégâts, temps de recharge, recharge
courante) depuis l'étape 20, avec des valeurs différentes. Composition,
pas héritage : aucune classe de base commune, aucune fonction virtuelle.

- **Ne connaît pas la position de son propriétaire** : `origine` et
  `cible` sont reçus en paramètre de `estAPortee` et `frapper`, plutôt
  que stockés. Ne connaît ni `TileMap`, ni `Personnage`, ni `Monstre`,
  ni `Escouade`, et ne dessine rien.
- **Données membres** : `m_portee`, `m_degats`, `m_tempsRecharge`
  (ces trois valeurs sont des paramètres du constructeur, puisqu'elles
  diffèrent déjà entre `Personnage` et `Monstre`) et `m_recharge`
  (0 à la création).

Interface publique :

| Fonction | Rôle |
|---|---|
| `Attaque(float portee, int degats, float tempsRecharge)` | Crée l'attaque avec cette portée, ces dégâts et ce temps de recharge |
| `void mettreAJour(float dt)` | Décrémente la recharge, sans descendre sous zéro |
| `bool estAPortee(sf::Vector2f origine, sf::Vector2f cible) const` | La distance entre `origine` et `cible` est-elle `<=` `m_portee` ? |
| `int frapper(sf::Vector2f origine, sf::Vector2f cible)` | Si `cible` est à portée de `origine` et la recharge écoulée, relance la recharge et renvoie `m_degats` ; sinon renvoie 0 |

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
| `docs/chemin.md` | Recherche de chemin (A*), lissage de chemin, déplacement par chemin (`Mobile`) | Avant de modifier `Chemin` ou `Mobile` |
| `docs/formation.md` | `Escouade`, formation de l'escouade, attaque (`Attaque`, `Personnage`, `Monstre`) | Avant de modifier `Escouade`, `Personnage`, `Monstre` ou `Attaque` |
| `docs/tests.md` | Tests (Catch2, conventions) | Avant d'ajouter ou de modifier un test |
