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

## Structure du dépôt

Le chemin de chaque fichier est indiqué en commentaire sur sa première
ligne (par exemple `// ./src/main.cpp` ou `# ./CMakeLists.txt`).

```
squad-arpg/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── TileMap.hpp / .cpp
│   ├── Escouade.hpp / .cpp
│   ├── Chemin.hpp / .cpp
│   ├── Personnage.hpp / .cpp
│   ├── Deplacement.hpp / .cpp
│   ├── Monstre.hpp / .cpp
│   ├── Attaque.hpp / .cpp
│   ├── Skill.hpp / .cpp
│   ├── Cible.hpp
│   ├── Mobile.hpp / .cpp
│   └── Cercle.hpp / .cpp
├── tests/
│   ├── test_TileMap.cpp, test_Chemin.cpp, test_Escouade.cpp
│   ├── test_Deplacement.cpp, test_Monstre.cpp, test_Personnage.cpp
│   └── test_Attaque.cpp, test_Skill.cpp, test_Mobile.cpp, test_Cercle.cpp
├── docs/
│   ├── build.md
│   ├── boucle-et-entrees.md
│   ├── carte-et-collision.md
│   ├── chemin.md
│   ├── formation.md
│   ├── combat.md
│   ├── monstre.md
│   ├── tests.md
│   └── historique.md
├── .gitignore
├── CLAUDE.md
├── ARCHITECTURE.md
└── ROADMAP.md
```

Deux exécutables : `main` (le jeu) et `tests` (les tests Catch2). Les deux
se lient à la bibliothèque statique `squad_core`, qui contient toute la
logique de jeu (les modules ci-dessous) indépendamment de la fenêtre
et de la boucle de jeu.

Qui possède quoi :
- `main.cpp` possède la fenêtre, la boucle de jeu, la caméra, la carte
  (`TileMap`), l'`Escouade` et un `std::vector<Monstre>`. Il traite les
  événements (fermeture, clic, touches 1 à 4) et fait l'intermédiaire entre
  `Escouade` et `Monstre`, qui ne se connaissent pas. Il ne connaît ni
  `Chemin` ni `Personnage`.
- `Escouade` possède un `Mobile` (l'ancre) et 4 `Personnage`.
- `Personnage` possède une `Attaque` et, en option, un `Skill` ; `Skill` possède une `Attaque` ; `Monstre` possède un `Mobile` et une
  `Attaque`.
- `Mobile` utilise `Chemin` et `Deplacement` ; `Personnage` utilise
  `Deplacement`. `Attaque`, `Personnage`, `Monstre` et `Escouade` échangent
  des `Cercle`.

## Modules

### `TileMap` — l'environnement de jeu

Grille de cases de taille fixe, chaque case étant du sol ou un mur. Un
point hors de la carte est un mur. Construite à partir d'un plan texte
(`'#'` = mur, tout autre caractère = sol).

Données membres : `m_largeur`, `m_hauteur` (cases), `m_tailleCase`
(pixels), `m_cases` (`std::vector<Tile>`, `enum class Tile { Floor, Wall }`).

| Fonction | Rôle |
|---|---|
| `TileMap(const std::vector<std::string>& plan, float tailleCase)` | Construit la carte à partir d'un plan texte ; `std::out_of_range` si le plan est vide, `std::invalid_argument` si les lignes n'ont pas toutes la même longueur |
| `Tile getTile(int x, int y) const` | Type de la case `(x, y)` ; hors de la carte = `Wall` |
| `int getLargeur() const` | Largeur de la carte, en cases |
| `int getHauteur() const` | Hauteur de la carte, en cases |
| `sf::Vector2i caseContenant(sf::Vector2f point) const` | Case (coordonnées de grille) contenant ce point, en pixels du monde |
| `sf::Vector2f centreDeCase(sf::Vector2i coordCase) const` | Centre de cette case, en pixels du monde |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine toutes les cases |
| `bool chevaucheUnMur(const sf::FloatRect& rect) const` | Le rectangle (en pixels du monde) chevauche-t-il un mur ? |
| `bool trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const` | Le rectangle, translaté de sa position jusqu'à `position + deplacement`, ne chevauche-t-il un mur à aucun moment du trajet ? |

### `Escouade` — le point de contrôle (l'ancre)

Un carré vert de 20 × 20 (l'ancre) qui suit le chemin calculé par le A*
jusqu'au point cliqué, et qui possède 4 `Personnage` en formation en carré
autour d'elle. Elle porte le verrou de combat et fait attaquer ses
personnages. Elle ne connaît pas `Monstre`.

Données membres : `m_mobile` (`Mobile`), `m_decalages` (4
`sf::Vector2f`), `m_orientation`, `m_orientationVoulue` (`sf::Angle`),
`m_vitesseRotation` (`float`), `m_personnages`
(`std::vector<Personnage>`), `m_cibleVerrouillee` (`std::optional<int>`),
`m_skillEnAttente` (`std::optional<SkillEnAttente>`, structure privée :
`place` du lanceur, `Cible cible`), `m_cibleCombat`
(`std::optional<Cercle>`).

| Fonction | Rôle |
|---|---|
| `explicit Escouade(sf::Vector2f positionDepart)` | Crée l'escouade et ses 4 personnages en formation (deux mêlées devant, deux tireurs derrière) |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Délègue à `m_mobile.allerVers` |
| `int attaquer(Cercle cible, const TileMap& carte)` | Mémorise `cible`, approche tant qu'un personnage vivant n'est pas à portée, s'arrête sinon ; fait partir le skill de dégâts en attente si son lanceur est à portée, puis fait frapper chaque personnage ; renvoie la somme des dégâts |
| `int declencherSkill(std::size_t place, std::optional<Cercle> monstreVerrouille)` | Skill du personnage à la place `place` (1 à 4). 0 si la place est invalide, sans skill, si le lanceur est mort, si le skill est en recharge, si un skill est déjà en attente ou si `choisirCible` ne trouve rien. Cible à portée : un skill de dégâts renvoie ses dégâts, un soin est appliqué sur place (renvoie 0). Hors de portée : le skill est mis en attente (renvoie 0) |
| `std::optional<Cible> choisirCible(TypeSkill type, std::optional<Cercle> monstreVerrouille) const` | `Degats` : le monstre verrouillé (vide sans verrou ou sans cercle) ; `Soin` : le vivant qui a le moins de points de vie parmi ceux qui ne sont pas à pleine vie (égalité : la plus petite place) |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Avance l'ancre, tourne la formation, déplace chaque personnage vers sa place (ou reste sur place s'il est à portée de la cible en combat) ; le lanceur d'un skill en attente vise sa cible à la place (un soin part ici, annulé si le lanceur ou la cible meurt ou si la cible est à pleine vie) ; `obstacles` est transmis aux personnages, jamais à l'ancre |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine l'ancre, puis les 4 personnages |
| `sf::Vector2f getCentre() const` | Centre de l'ancre (utilisé par la caméra) |
| `std::optional<Cercle> personnageLePlusProche(sf::Vector2f point) const` | Cercle du personnage vivant dont le centre est le plus proche de `point` ; vide si aucun n'est vivant |
| `void recevoirDegats(sf::Vector2f origine, int quantite)` | Applique les dégâts au personnage vivant le plus proche d'`origine` ; ne fait rien si aucun n'est vivant |
| `bool estVivante() const` | Vrai si au moins un personnage est vivant |
| `std::vector<Cercle> cerclesDesVivants() const` | Cercles des personnages vivants |
| `void verrouiller(int idCible)` | Pose le verrou de combat sur ce monstre, en remplaçant la cible précédente ; annule un skill de dégâts en attente si l'identifiant change |
| `void deverrouiller()` | Retire le verrou de combat et annule un skill de dégâts en attente |
| `std::optional<int> getCibleVerrouillee() const` | Identifiant du monstre engagé en combat, vide si aucun |

### `Mobile` — déplacement par chemin

Suit un chemin calculé par le A* puis lissé, avec collision contre les
murs et les cercles obstacles. Possédé par valeur par `Escouade` et par
`Monstre`. Ne connaît ni `Escouade`, ni `Personnage`, ni `Monstre`, ni
`Attaque`, et ne dessine rien.

Données membres : `m_position` (coin haut-gauche), `m_taille`, `m_chemin`
(`std::vector<sf::Vector2f>`), `m_indexChemin`, `m_vitesse`.

| Fonction | Rôle |
|---|---|
| `Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)` | Crée le mobile à ce centre, cette taille, cette vitesse ; immobile |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Calcule un chemin vers la destination (A*, puis lissé) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe |
| `void arreter()` | Réduit le chemin au centre actuel, pour cesser d'avancer sans reculer |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Avance vers le point courant du chemin (collision avec les murs et les cercles obstacles, glissement) |
| `sf::Vector2f getCentre() const` | Centre du mobile |
| `sf::Vector2f getTaille() const` | Taille du mobile |
| `std::optional<sf::Vector2f> directionVoulue() const` | Vecteur du centre vers le point courant du chemin ; vide s'il n'y a plus de point à atteindre ou si la distance est nulle |

### `Chemin` — recherche de chemin (A*)

Deux fonctions libres, sans état. `trouverChemin` raisonne en cases,
`lisserChemin` en pixels. Utilisées uniquement par `Mobile::allerVers`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee)` | Plus court chemin entre deux cases (4 directions, coût 1 par pas, heuristique de Manhattan) ; inclut le départ et l'arrivée ; vecteur vide si départ/arrivée est un mur (ou hors carte) ou si l'arrivée est inaccessible |
| `std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Vector2f taille)` | Retire de `points` (positions du centre d'un rectangle `taille`, en pixels) les points de passage inutiles ; `depart` n'en fait pas partie dans le résultat |

### `Personnage` — un membre de l'escouade

Un carré coloré de 20 × 20 qui va vers le point que lui donne `Escouade`,
frappe à portée, a des points de vie et peut mourir. Ne connaît ni la
formation, ni `Escouade`, ni `Monstre`.

Données membres : `m_centre`, `m_taille`, `m_couleur`, `m_vitesse`,
`m_attaque` (`Attaque`), `m_skill` (`std::optional<Skill>`),
`m_pointsDeVie`, `m_pointsDeVieMax`.

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, sf::Color couleur, Attaque attaque, std::optional<Skill> skill)` | Crée le personnage à ce centre, avec cette couleur, cette attaque et ce skill éventuel |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Si vivant, avance vers la cible (collision avec les murs et les cercles obstacles, glissement) puis décrémente la recharge de l'attaque et du skill ; sinon ne fait rien |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage et sa barre de vie, plus la barre de recharge du skill s'il en a un ; carré gris sans barre s'il est mort |
| `bool estAPortee(Cercle cible) const` | L'écart bord à bord entre son cercle et `cible` est-il `<=` la portée de `m_attaque` ? |
| `int frapper(Cercle cible)` | 0 s'il est mort ; sinon délègue à `m_attaque.frapper` avec `getCercle()` comme origine |
| `std::optional<TypeSkill> typeSkill() const` | Type du skill, vide sans skill |
| `bool skillEstPret() const` | Vivant, avec un skill dont la recharge est écoulée |
| `bool skillEstAPortee(Cercle cible) const` | Le skill est-il à portée de `cible` (bord à bord) ? Faux sans skill |
| `int declencherSkill(Cercle cible)` | 0 sans skill ou s'il est mort ; sinon délègue à `m_skill->declencher` (dégâts ou points de vie rendus) et, si la puissance est `> 0`, relance au complet la recharge de `m_attaque` |
| `void recevoirSoin(int quantite)` | Rend des points de vie, sans dépasser le maximum ; sans effet sur un mort |
| `int getPointsDeVie() const` / `int getPointsDeVieMax() const` | Points de vie actuels et maximum |
| `void recevoirDegats(int quantite)` | Retire des points de vie, sans descendre sous zéro |
| `bool estVivant() const` | Points de vie `> 0` |
| `sf::Vector2f getCentre() const` | Centre du personnage |
| `Cercle getCercle() const` | Cercle centré sur le personnage, de rayon 10 |

### `Deplacement` — collision et glissement partagés

Deux fonctions libres, sans état, utilisées par `Mobile::mettreAJour` et
`Personnage::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `bool deplacementBloque(const Cercle& avant, const Cercle& apres, const std::vector<Cercle>& obstacles)` | Vrai si, pour au moins un obstacle, `apres` chevauche l'obstacle (distance entre centres `<` somme des rayons) ET la distance entre centres a diminué par rapport à `avant` |
| `sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement, const std::vector<Cercle>& obstacles)` | Essaie le déplacement axe par axe (X puis Y) ; un essai est refusé s'il chevauche un mur ou si `deplacementBloque` est vrai ; renvoie la nouvelle position du coin haut-gauche |

### `Monstre` — un ennemi

Un carré rouge de 30 × 30 qui détecte un personnage, le poursuit, riposte
à portée et retourne à son poste. Possédé par `main.cpp`. Ne connaît ni
`Escouade` ni `Personnage` : il reçoit sa proie et ses obstacles en
paramètres.

Données membres : `m_mobile`, `m_pointsDeVie`, `m_pointsDeVieMax`, `m_id`,
`m_attaque`, `m_rayonDetection`, `m_rayonTerritoire`, `m_etat` (`Etat`,
`enum class` privé : `EnGarde`, `Poursuite`, `Retour`), `m_poste`.

| Fonction | Rôle |
|---|---|
| `Monstre(sf::Vector2f centre, int id)` | Crée le monstre à ce centre, avec cet identifiant, 100 points de vie ; son poste est ce centre |
| `void dessiner(sf::RenderTarget& rendu, bool estCible) const` | Dessine le monstre et sa barre de vie ; ajoute un contour jaune si `estCible` |
| `void mettreAJour(float dt, std::optional<Cercle> cible, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Décrémente la recharge, met à jour l'état et donne ses ordres à `m_mobile` (poursuite, arrêt à portée, retour au poste), puis le déplace ; `obstacles` lui est transmis |
| `int frapper(Cercle cible)` | Délègue à `m_attaque.frapper`, avec `getCercle()` comme origine |
| `sf::Vector2f getCentre() const` | Centre du monstre |
| `Cercle getCercle() const` | Cercle centré sur le monstre, de rayon 15 |
| `int getId() const` | Identifiant du monstre |
| `bool contient(sf::Vector2f point) const` | Le point (pixels du monde) est-il dans le carré du monstre ? |
| `void recevoirDegats(int quantite)` | Retire des points de vie, sans descendre sous zéro |
| `bool estVivant() const` | Points de vie `> 0` |

### `Attaque` — portée, dégâts et recharge

Possédée par valeur par `Personnage` (attaque de base), par `Skill` et par
`Monstre`. Ne connaît pas la position de son propriétaire : elle reçoit
deux `Cercle` en paramètre. Ne dessine rien.

Données membres : `m_portee` (bord à bord), `m_degats`, `m_tempsRecharge`,
`m_recharge`.

| Fonction | Rôle |
|---|---|
| `Attaque(float portee, int degats, float tempsRecharge)` | Crée l'attaque avec cette portée, ces dégâts et ce temps de recharge |
| `void mettreAJour(float dt)` | Décrémente la recharge, sans descendre sous zéro |
| `bool estAPortee(Cercle origine, Cercle cible) const` | `distanceBordABord(origine, cible)` est-elle `<=` `m_portee` ? |
| `int frapper(Cercle origine, Cercle cible)` | Si `cible` est à portée de `origine` et la recharge écoulée, relance la recharge et renvoie `m_degats` ; sinon renvoie 0 |
| `void relancerRecharge()` | Remet la recharge à `m_tempsRecharge`, sans frapper |
| `float getProgressionRecharge() const` | Avancement de la recharge, de 0 (juste après un coup) à 1 (prête) ; 1 si le temps de recharge est nul |

### `Skill` — une capacité spéciale

Un `TypeSkill` (`enum class { Degats, Soin }`) et une `Attaque`, qui
fournit portée, puissance (dégâts ou points de vie rendus) et recharge.
Possédé par valeur, en option, par `Personnage`. Ne connaît ni la
position de son lanceur ni sa cible : il reçoit deux `Cercle`. Ne dessine
rien.

Données membres : `m_type` (`TypeSkill`), `m_attaque` (`Attaque`).

| Fonction | Rôle |
|---|---|
| `Skill(TypeSkill type, Attaque attaque)` | Crée le skill de ce type, autour de cette attaque |
| `void mettreAJour(float dt)` | Fait avancer la recharge |
| `bool estAPortee(Cercle origine, Cercle cible) const` | Délègue à l'`Attaque` |
| `int declencher(Cercle origine, Cercle cible)` | Puissance si la cible est à portée et la recharge écoulée (relance alors la recharge), sinon 0 |
| `float getProgressionRecharge() const` | De 0 (juste après le déclenchement) à 1 (prêt) |
| `TypeSkill getType() const` | Type du skill |

### `Cible` — ce que vise un skill

`Cible.hpp`, sans `.cpp` : `struct CibleAllie { std::size_t place; }`
(de 1 à 4), `struct CibleMonstre { Cercle cercle; }` et
`using Cible = std::variant<CibleAllie, CibleMonstre>`. L'absence de
cible est un `std::optional<Cible>` vide.

### `Cercle` — la forme d'une entité

Une structure et une fonction libre, sans comportement.

Données membres : `struct Cercle { sf::Vector2f centre; float rayon; }`.

| Fonction | Rôle |
|---|---|
| `float distanceBordABord(const Cercle& a, const Cercle& b)` | Distance entre les centres moins les deux rayons ; 0 si les cercles se touchent ou se chevauchent (jamais négative) |

## Principes de conception à respecter

1. **Ne jamais anticiper.** On ne construit que ce qui est nécessaire pour
   le travail en cours. Pas de fonction ou dépendance "pour plus tard" — on y
   revient explicitement quand le besoin devient réel. À l'inverse, le
   code devenu inutile est supprimé (Git le conserve dans l'historique).
2. **Avancer par petits pas.** D'abord la structure de données (les
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

Les décisions techniques détaillées (le pourquoi, les limites connues)
vivent dans `docs/`, un fichier par sujet, pour que `CLAUDE.md` n'ait pas
à importer tout ce détail à chaque session. Avant de modifier un module,
lire le fichier correspondant :

| Fichier | Contenu | Quand le lire |
|---|---|---|
| `docs/build.md` | Build (CMake, `squad_core`, exécutables), choix de SFML | Avant de modifier `CMakeLists.txt` ou d'ajouter un fichier source |
| `docs/boucle-et-entrees.md` | Boucle de jeu, entrée souris et clavier, combat engagé, défaite, monstres dans la boucle, caméra | Avant de modifier `main.cpp` |
| `docs/carte-et-collision.md` | Carte, collision et dessin, déplacement avec glissement et obstacles, plan texte et validation | Avant de modifier `TileMap` ou `Deplacement` |
| `docs/chemin.md` | Recherche de chemin (A*), suivi de chemin (`Mobile`), lissage de chemin | Avant de modifier `Chemin` ou `Mobile` |
| `docs/formation.md` | `Escouade` et `Personnage` : ancre, formation, orientation, verrou de cible, placement en combat, mort et défaite | Avant de modifier `Escouade` ou `Personnage` |
| `docs/combat.md` | `Attaque`, `Cercle`, portée bord à bord, `Skill`, `Cible`, skill en attente, circuit des dégâts, qui bloque qui | Avant de modifier `Attaque`, `Cercle`, `Skill`, `Cible`, le skill ou les listes d'obstacles |
| `docs/monstre.md` | `Monstre` : identifiant, états, détection, territoire, poursuite, retour au poste, riposte | Avant de modifier `Monstre` |
| `docs/tests.md` | Tests (Catch2, conventions) | Avant d'ajouter ou de modifier un test |
| `docs/historique.md` | Historique détaillé, tel quel | Jamais nécessaire avant de modifier du code |
