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
│   ├── Niveau.hpp / .cpp
│   ├── GenerateurDonjon.hpp / .cpp
│   ├── Partie.hpp / .cpp
│   ├── Controleur.hpp / .cpp
│   ├── TileMap.hpp / .cpp
│   ├── Escouade.hpp / .cpp
│   ├── Provocations.hpp / .cpp
│   ├── Agresseurs.hpp / .cpp
│   ├── Chemin.hpp / .cpp
│   ├── Personnage.hpp / .cpp
│   ├── Deplacement.hpp / .cpp
│   ├── Monstre.hpp / .cpp
│   ├── Skill.hpp / .cpp
│   ├── Cible.hpp
│   ├── ActionEngagee.hpp
│   ├── Coup.hpp
│   ├── MonstreVu.hpp / .cpp
│   ├── TypeSkill.hpp
│   ├── Autodefense.hpp / .cpp
│   ├── Mobile.hpp / .cpp
│   ├── Cercle.hpp / .cpp
│   ├── EtatSkill.hpp
│   ├── BarreDeSkills.hpp / .cpp
│   ├── EcranDeDefaite.hpp / .cpp
│   ├── BarreDeVie.hpp / .cpp
│   ├── Defense.hpp / .cpp
│   ├── Eclaircir.hpp / .cpp
│   ├── Aleatoire.hpp
│   └── PointsDeVie.hpp / .cpp
├── assets/
│   └── fonts/ (DejaVuSans.ttf, LICENSE-DejaVu.txt)
├── tests/
│   ├── aides.hpp (aides communes, inline)
│   ├── test_TileMap.cpp, test_Chemin.cpp
│   ├── test_Escouade_deplacement.cpp, test_Escouade_combat.cpp,
│   │   test_Escouade_skills.cpp, test_Escouade_agresseurs.cpp,
│   │   test_Escouade_vue.cpp
│   ├── test_Provocations.cpp, test_Agresseurs.cpp
│   ├── test_Deplacement.cpp, test_Monstre.cpp, test_Personnage.cpp
│   ├── test_Partie.cpp, test_Controleur.cpp, test_GenerateurDonjon.cpp
│   └── test_Skill.cpp, test_Mobile.cpp, test_Cercle.cpp,
│       test_Autodefense.cpp, test_BarreDeSkills.cpp,
│       test_MonstreVu.cpp, test_BarreDeVie.cpp,
│       test_PointsDeVie.cpp, test_Defense.cpp, test_Eclaircir.cpp
├── docs/
│   ├── build.md
│   ├── boucle-et-entrees.md
│   ├── carte-et-collision.md
│   ├── chemin.md
│   ├── formation.md
│   ├── generation.md
│   ├── combat.md
│   ├── monstre.md
│   ├── tests.md
│   ├── audit.md
│   ├── brainstorm.md
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
- `main.cpp` possède la fenêtre, la boucle SFML, la caméra, la police
  (`sf::Font`), une `Partie` et un `Controleur`. Il traduit les événements
  SFML (fermeture, clic, touches 1 à 4, Échap, clic droit) en appels au
  `Controleur` (touche R : recommence la partie si elle est perdue),
  convertit le curseur en coordonnées du monde, centre la caméra sur
  l'escouade et enchaîne le dessin. Il ne connaît ni `Monstre`,
  ni `Coup`, ni `MonstreVu`, ni `Chemin`, ni `Personnage`.
- `Partie` possède la `TileMap`, l'`Escouade`, un `std::vector<Monstre>` et le
  `Generateur` aléatoire (graine reçue au constructeur),
  construits à partir d'un `Niveau`. Elle fait l'intermédiaire entre
  `Escouade` et `Monstre`, qui ne se connaissent pas : elle leur donne la
  liste des monstres (`MonstreVu`) et applique les `Coup` renvoyés.
- `Controleur` porte l'état d'interface : le mode visée et le déplacement
  continu. Il ne garde aucune référence à la `Partie` : celle-ci lui est
  passée à chaque appel. Il ne connaît pas les événements SFML.
- `Escouade` possède un `Mobile` (l'ancre), 4 `Personnage`, des `Provocations` et des `Agresseurs`. Ces deux derniers ne connaissent ni `Personnage`, ni `Monstre`, ni `Escouade`.
- `Personnage` possède un `Mobile`, un `Skill` (son attaque de base) et, en option, un second `Skill` ; `Monstre` possède un `Mobile` et un
  `Skill` (son attaque).
- `Mobile` utilise `Chemin` et `Deplacement`. `Personnage` utilise
  `Mobile` et le test de trajet de `TileMap`. `Skill`, `Personnage`, `Monstre` et `Escouade` échangent
  des `Cercle`. `Personnage` et `Monstre` possèdent chacun un `PointsDeVie` et dessinent leur barre de vie avec `BarreDeVie` ; leur `recevoirDegats` passe par `appliquerArmure` (`Defense`). `Escouade` utilise `choisirMonstreAFrapper` (`Autodefense`) et
`replierVersOrigine` (`Chemin`).

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
| `bool ligneDeVue(sf::Vector2f a, sf::Vector2f b) const` | Le segment de `a` à `b` (pixels du monde) ne traverse-t-il aucun mur ? Faux si `a` ou `b` est dans un mur |

### `Niveau` — les données d'un niveau

`Niveau.hpp` / `.cpp` : structure sans comportement et une fonction libre.

Données membres : `struct Niveau { std::vector<std::string> plan; float
tailleCase; sf::Vector2f departEscouade; std::vector<sf::Vector2f>
positionsMonstres; int armureMonstres; }`.

| Fonction | Rôle |
|---|---|
| `Niveau creerNiveau()` | Le niveau du jeu (carte de 59 × 40 cases de 40 px, départ en `(100, 100)`, six monstres d'armure 20) ; pas de lecture de fichier |

### `GenerateurDonjon` — la carte générée

`GenerateurDonjon.hpp` / `.cpp` : logique pure (sans SFML), fonctions libres
sans état, plus deux structures. Le générateur aléatoire est reçu en
paramètre (`Generateur&`).

`struct Salle { int x, y, largeur, hauteur; }` : l'emprise d'une salle, en
cases. `struct ParametresDonjon` : `largeurCarte` 59, `hauteurCarte` 40,
`tailleCase` 40 px, `tentatives` 30, `coteMin` 6, `coteMax` 12, `marge` 1,
`largeurCouloir` 2. `struct Liaison { std::size_t de, vers; }` (indices de
salles) et `enum class SensCouloir { HorizontalPuisVertical,
VerticalPuisHorizontal }`.

| Fonction | Rôle |
|---|---|
| `sf::Vector2i Salle::centre() const` | Case au centre de l'emprise (`x + largeur / 2`, `y + hauteur / 2`) |
| `bool Salle::chevauche(const Salle& autre, int marge) const` | Les emprises sont-elles à moins de `marge` cases l'une de l'autre, ou se recouvrent-elles ? Deux salles qui se touchent ne se chevauchent pas avec une marge de 0 |
| `std::vector<Salle> placerSalles(Generateur&, const ParametresDonjon&)` | Placement au hasard avec rejet ; au moins une salle ; emprises à l'intérieur de la bordure de mur |
| `void creuserSalle(std::vector<std::string>& plan, const Salle&)` | Transforme l'emprise en sol (aujourd'hui tout le rectangle) |
| `std::vector<Liaison> choisirLiaisons(const std::vector<Salle>&)` | Sans aléatoire : chaque salle i ≥ 1 est reliée à la plus proche (distance au carré entre centres) parmi les salles 0 à i−1 ; égalité : le plus petit indice ; n−1 liaisons |
| `void creuserCouloir(std::vector<std::string>& plan, sf::Vector2i depart, sf::Vector2i arrivee, SensCouloir, int largeur)` | Couloir en L de `largeur` cases (étendue vers le bas et la droite), coude plein, départ et arrivée en sol |
| `Niveau genererNiveau(Generateur&, const ParametresDonjon&)` | Plan tout en mur, salles creusées, puis un couloir par liaison (sens tiré au hasard, un tirage par liaison) entre les centres ; départ de l'escouade au centre en pixels de la première salle, aucun monstre, `armureMonstres` 0 |

### `Partie` — les règles de jeu

Possède par valeur la carte, l'escouade et les monstres. Orchestre
seulement : les règles propres à `Escouade` ou à `Monstre` y restent. Le
joueur agit par des demandes, mémorisées puis exécutées par `mettreAJour` à
leur place dans l'image ; l'ordre est décrit dans `docs/boucle-et-entrees.md`.
Invariant : entre deux appels, tout monstre du vecteur est vivant.
Une partie perdue est figée. Elle se recommence en la remplaçant par une
`Partie` neuve (réaffectation par déplacement : règle du zéro).

Données membres : `m_carte` (`TileMap`), `m_escouade` (`Escouade`),
`m_monstres` (`std::vector<Monstre>`), `m_lancerDeZoneDemande`
(`std::optional<LancerDeZone>`, structure privée : `place`, `point`),
`m_skillDemande` (`std::optional<std::size_t>`), `m_deplacementOrdonne`
(`std::optional<sf::Vector2f>`), `m_generateur` (`Generateur`, le seul de
la partie : il descend en paramètre `Generateur&` jusqu'à `Skill`, que
personne ne copie ni ne garde).

| Fonction | Rôle |
|---|---|
| `Partie(const Niveau& niveau, Generateur::result_type graine)` | Construit la carte, l'escouade et les monstres (identifiants 1, 2, 3…, armure `niveau.armureMonstres`) et initialise le générateur avec `graine` (`main.cpp` la tire avec `std::random_device` et l'affiche ; les tests passent une graine fixe) |
| `ResultatClic cliquer(sf::Vector2f point)` | Appliqué tout de suite. `Monstre` : verrouille le monstre qui contient le point ; `Sol` : lève le verrou et annule les coups en exécution des personnages ; `Ignore` si l'escouade est vaincue, sans rien changer. `enum class ResultatClic { Monstre, Sol, Ignore }` |
| `void demanderSkill(std::size_t place)` | Mémorise la demande de skill (touches 1 à 3) ; la dernière demande de l'image gagne |
| `void lancerSkillDeZone(std::size_t place, sf::Vector2f point)` | Mémorise le lancer du skill de zone |
| `void ordonnerDeplacement(sf::Vector2f point)` | Mémorise l'ordre de déplacement ; exécuté seulement si l'escouade est vivante |
| `bool estPerdue() const` | Vrai quand tous les personnages sont morts (`!m_escouade.estVivante()`) ; `[[nodiscard]]` |
| `void mettreAJour(float dt)` | Sans effet si la partie est perdue (le monde est figé). Sinon exécute, dans l'ordre : lancer de zone, skill demandé, `attaquer(dt, …)` et application des coups, déverrouillage si la cible est morte, retrait des monstres morts, ordre de déplacement, mise à jour (obstacles, escouade, tour de chaque monstre, avec l'engagement et le départ de son coup) ; vide les demandes |
| `void dessiner(sf::RenderTarget& rendu) const` | Carte, monstres (contour jaune sur le monstre verrouillé), escouade |
| `const Escouade& getEscouade() const` | Lecture seule : caméra, barre de skills, requêtes du `Controleur` |
| `const std::vector<Monstre>& getMonstres() const` | Lecture seule (utilisée par les tests) |

### `Controleur` — l'état d'interface

Ne connaît pas les événements SFML : `main.cpp` les traduit en appels
simples. Points en pixels du monde.

Type valeur, constructible par défaut et réaffectable (règle du zéro) : aucune
référence membre (C++ Core Guidelines C.12), la `Partie` est reçue à chaque
appel (`const Partie&` si la fonction ne fait que lire).

Données membres : `m_deplacementContinu` (`bool`), `m_placeEnVisee`
(`std::optional<std::size_t>`, vide = mode normal).

| Fonction | Rôle |
|---|---|
| `Controleur()` (par défaut) | Mode normal, déplacement continu désarmé ; `controleur = Controleur{}` remet l'état d'interface à zéro |
| `void debutImage(const Partie& partie)` | Sort de la visée si le lanceur ne peut plus lancer (`!skillEstPret`) |
| `void appuiGauche(Partie& partie, sf::Vector2f point)` | En visée : `lancerSkillDeZone` puis sortie de la visée (sans verrouiller ni déplacer). Sinon `cliquer` : `Monstre` désarme le déplacement continu, `Sol` l'arme, `Ignore` ne change rien |
| `void appuiDroit()` / `void appuiEchap()` | Annulent la visée sans recharge |
| `void appuiTouche(Partie& partie, std::size_t place)` | 1 à 3 : `demanderSkill`. 4 : sort de la visée si on y est ; sinon y entre si l'escouade est vivante et le skill prêt, et désarme le déplacement continu. Autre place : ignorée |
| `void mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur)` | Si `boutonTenu` (bouton gauche enfoncé ET focus, calculé par `main.cpp`) et déplacement armé : `ordonnerDeplacement(curseur)` |
| `void dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const` | En visée seulement : cercle jaune de la zone sous le curseur, cercle blanc de la portée autour du lanceur |
| `std::optional<std::size_t> getPlaceEnVisee() const` | Place en visée (pour la barre de skills) |

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
`place` du lanceur, `Cible cible`, qui peut être un `CiblePoint`), `m_cibleCombat`
(`std::optional<Cercle>`), `m_places` (`std::vector<sf::Vector2f>`, place
de formation de chaque personnage après repli, recalculée par
`mettreAJour`), `m_monstreAPortee` (`std::array<bool, s_nombrePersonnages>`, par personnage :
monstre à portée de son attaque et en vue, calculé par `attaquer`, lu par `mettreAJour`), `m_agresseurs`
(`Agresseurs`, monstres qui ont frappé l'escouade récemment),
`m_provocations` (`Provocations`, monstres provoqués et leur provocateur).

| Fonction | Rôle |
|---|---|
| `explicit Escouade(sf::Vector2f positionDepart)` | Crée l'escouade et ses 4 personnages en formation (deux mêlées devant, deux tireurs derrière) |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Ordre du joueur : annule les coups en exécution (`annulerAttaquesEngagees`), puis délègue à `m_mobile.allerVers` |
| `void annulerAttaquesEngagees()` | Annule le coup en exécution de chaque personnage, sans recharge consommée ; appelée par `allerVers`, `verrouiller` (autre monstre) et `Partie::cliquer` (sol) |
| `std::vector<Coup> attaquer(float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur)` | Avec un verrou dont le monstre est dans `monstres` : mémorise son cercle et approche tant qu'un personnage vivant n'atteint pas la cible (à portée ET en vue : `peutAtteindre`). Fait entrer en activation le skill de zone en attente dès que son lanceur vivant peut lancer au point, et le skill de dégâts en attente dès que son lanceur atteint la cible. Puis fait avancer l'activation de chaque skill engagé (`avancerSkills`) : l'effet part à la fin de l'activation, évalué à ce moment ; ses coups s'ajoutent à la liste. Verrou ou non : chaque personnage choisit pour lui-même le monstre à frapper parmi ceux qu'il atteint (`choisirMonstreAFrapper` avec `peutAtteindre`) ; s'il est immobile, sans action engagée et son attaque prête, il **engage** un coup ; le coup engagé avance de `dt` et part à la fin de l'exécution (0,15 s), même si le monstre a bougé. Il n'est annulé que par la mort de son auteur ou la disparition de sa cible (et par un ordre du joueur ou un skill, voir ci-dessus). Renvoie un `Coup` par coup parti |
| `std::vector<Coup> declencherSkill(std::size_t place, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur)` | Skill du personnage à la place `place` (1 à 4). Liste vide si la place est invalide, sans skill, si le skill est de type `Zone` (il se lance par `declencherSkillDeZone`), si le lanceur est mort, si le skill est en recharge ou en activation, si un skill est déjà en attente ou si `choisirCible` ne trouve rien. Cible atteinte (à portée ET en vue), ou provocation (sans cible) : l'**activation commence** (`Personnage::engagerSkill`) ; l'effet part à la fin (temps d'activation du skill) dans `attaquer`, ou tout de suite, dans la liste renvoyée, si l'activation est nulle. Provocation : au départ, tous les monstres de `monstres` touchés par sa zone (cercle centré sur le lanceur) ET en vue du lanceur sont provoqués, même s'il n'y en a aucun (la recharge est consommée). Dégâts : un coup (identifiant de la cible, dégâts) ; soin : appliqué sur place à l'allié engagé (liste vide). Non atteinte (seulement avec un verrou pour les dégâts) : le skill est mis en attente (liste vide) |
| `std::optional<EtatSkill> etatDuSkill(std::size_t place) const` | Skill de cette place tel que l'affichage en a besoin (`EtatSkill`) ; vide si la place est invalide ou sans skill |
| `bool skillEstPret(std::size_t place) const` | Le personnage à cette place est-il vivant, avec un skill dont la recharge est écoulée ? Faux si la place est invalide |
| `float rayonZoneDuSkill(std::size_t place) const` | Rayon de zone du skill de cette place ; 0 sans skill ou si la place est invalide |
| `Cercle cercleDePorteeDuSkill(std::size_t place) const` | Cercle centré sur le lanceur, de rayon (rayon du personnage + portée du skill) ; rayon 0 si la place est invalide |
| `std::vector<Coup> declencherSkillDeZone(std::size_t place, sf::Vector2f point, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur)` | Lance le skill de type `Zone` de cette place sur un cercle centré sur `point` (rayon de zone du skill). Point à portée (bord à bord, comme un cercle de rayon 0) et en vue du lanceur : l'activation commence tout de suite ; au départ, un `Coup` (identifiant, dégâts du skill) par monstre touché (bord à bord 0 ET en vue du point d'impact, centre à centre : `ligneDeVue`), zone évaluée à ce moment. Le sort lance la recharge même si la zone est vide. Hors portée ou pas en vue : le skill est mis en attente (`CiblePoint`), liste vide, sans recharge ; le lanceur marche vers le point (`mettreAJour`) et son activation commence dans `attaquer` dès que portée et vue sont réunies. Liste vide, sans recharge ni attente, si le point est dans un mur, si la place est invalide, si son skill n'est pas de type `Zone`, si le lanceur est mort ou en recharge ou en activation, ou si un skill est déjà en attente |
| `std::optional<Cible> choisirCible(TypeSkill type, std::size_t place, const std::vector<MonstreVu>& monstres, const TileMap& carte) const` | `Degats` : avec un verrou, le monstre verrouillé trouvé dans `monstres` (atteint ou non, vide s'il n'y est pas) ; sans verrou, le monstre que le lanceur de la place `place` frapperait en autodéfense, avec la portée de son skill et en vue (vide si aucun). `Soin` : le vivant qui a le moins de points de vie parmi ceux qui ne sont pas à pleine vie (égalité : la plus petite place) |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Avance l'ancre, tourne la formation, calcule la place de chaque personnage (place théorique ramenée vers l'ancre par `replierVersOrigine` si elle est dans un mur ou cachée derrière un coin), déplace chaque personnage vers sa place (ou reste sur place s'il atteint la cible en combat, à portée et en vue; ancre à l'arrêt : vers l'agresseur le plus proche dans une laisse de 100 px autour de sa place, ou sur place s'il a déjà un monstre à portée et en vue) ; fait écouler le délai d'oubli des agresseurs ; le lanceur d'un skill en attente vise sa cible à la place (un soin entre ici en activation à l'arrivée, annulé si le lanceur ou la cible meurt ou si la cible est à pleine vie ; un personnage en exécution ou en activation reste sur place) ; `obstacles` est transmis aux personnages, jamais à l'ancre |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine l'ancre, les 4 personnages, puis un point jaune (repère provisoire) à la place de formation de chaque personnage vivant |
| `sf::Vector2f getCentre() const` | Centre de l'ancre (utilisé par la caméra) |
| `std::optional<Cercle> proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const` | Cercle du provocateur si ce monstre est provoqué et que le provocateur est vivant ; sinon du personnage vivant le plus proche de `positionMonstre` ; vide si aucun n'est vivant |
| `std::optional<std::size_t> placeDeLaProie(int idMonstre, sf::Vector2f positionMonstre) const` | Place (1 à 4) du personnage que ce monstre vise : même règle que `proieDuMonstre` |
| `bool placeEstVivante(std::size_t place) const` | Place valide ET personnage vivant |
| `void recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite)` | Applique les dégâts au personnage que `idMonstre` vise (même règle que `proieDuMonstre`) ; ne fait rien si aucun n'est vivant. Délègue à `recevoirCoup` |
| `void recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite)` | Applique les dégâts au personnage de cette place (le coup engagé d'un monstre) ; sans effet s'il est mort. Si `quantite > 0`, enregistre le monstre comme agresseur (ou remet son temps à 3 s) |
| `bool estVivante() const` | Vrai si au moins un personnage est vivant |
| `std::vector<Cercle> cerclesDesVivants() const` | Cercles des personnages vivants |
| `void verrouiller(int idCible)` | Pose le verrou de combat sur ce monstre, en remplaçant la cible précédente ; annule un skill de dégâts en attente si l'identifiant change |
| `void deverrouiller()` | Retire le verrou de combat et annule un skill de dégâts en attente |
| `std::optional<int> getCibleVerrouillee() const` | Identifiant du monstre engagé en combat, vide si aucun |
| `const std::vector<sf::Vector2f>& getPlacesDeFormation() const` | Places de formation (après repli) calculées par le dernier `mettreAJour` |
| `void provoquer(int idMonstre, std::size_t place, float duree)` | Enregistre ce monstre comme provoqué par le personnage de la place `place` (1 à 4) pour `duree` secondes, ou remet sa durée au maximum s'il l'est déjà. Fin : durée écoulée ou provocateur mort (`mettreAJour`), monstre absent de `monstres` (`attaquer`) |
| `const std::vector<Provocation>& getProvocations() const` | Monstres actuellement provoqués |
| `const std::vector<Agresseur>& getAgresseurs() const` | Monstres qui ont frappé l'escouade récemment (oubliés après 3 s sans coup, ou absents de `monstres` dans `attaquer`) |

### `Provocations` — les monstres provoqués

Possédée par valeur par `Escouade`. Données privées : `m_provocations`
(`std::vector<Provocation>`, structure publique : `id` du monstre, `place` du
provocateur de 1 à 4, `tempsRestant`).

| Fonction | Rôle |
|---|---|
| `void provoquer(int id, std::size_t place, float duree)` | Ajoute la provocation, ou remet la durée au maximum (et change le provocateur) si ce monstre l'est déjà ; pas de doublon. La validation de la place reste dans `Escouade::provoquer` |
| `void mettreAJour(float dt)` | Fait écouler le temps ; retire les provocations échues |
| `void retirerMonstresAbsents(const std::vector<MonstreVu>& monstres)` | Retire les monstres absents de la liste (morts) |
| `void retirerProvocateur(std::size_t place)` | Retire les provocations de ce provocateur (mort) |
| `std::optional<std::size_t> provocateurDe(int id) const` | Place du provocateur de ce monstre, vide s'il n'est pas provoqué |
| `const std::vector<Provocation>& liste() const` | Provocations en cours |

### `Agresseurs` — les monstres qui ont frappé l'escouade

Possédée par valeur par `Escouade`. Données privées : `m_agresseurs`
(`std::vector<Agresseur>`, structure publique : `MonstreVu monstre`,
`tempsRestant`). Mémoire de 3 s sans nouveau coup du même monstre.

| Fonction | Rôle |
|---|---|
| `void enregistrer(int id, sf::Vector2f origine)` | Ajoute l'agresseur (cercle de rayon nul centré sur l'origine du coup), ou remet son temps à 3 s et son centre à l'origine s'il est connu. La règle « coup à 0 = rien » reste dans `Escouade::recevoirDegats` |
| `void mettreAJour(float dt)` | Fait écouler le temps ; oublie les agresseurs échus |
| `void actualiser(const std::vector<MonstreVu>& monstres)` | Oublie les absents (morts) et met à jour le cercle des autres |
| `std::vector<MonstreVu> monstresVus() const` | Les agresseurs en `MonstreVu` ; `Escouade::mettreAJour` l'appelle une fois par image |
| `const std::vector<Agresseur>& liste() const` | Agresseurs retenus |

### `Mobile` — déplacement par chemin

Suit un chemin calculé par le A* puis lissé, avec collision contre les
murs et les cercles obstacles. Possédé par valeur par `Escouade`, `Personnage` et
`Monstre`. Ne connaît ni `Escouade`, ni `Personnage`, ni `Monstre`, ni
`Skill`, et ne dessine rien.

Données membres : `m_position` (coin haut-gauche), `m_taille`, `m_chemin`
(`std::vector<sf::Vector2f>`), `m_indexChemin`, `m_vitesse`, `m_enMouvement` (`bool`).

| Fonction | Rôle |
|---|---|
| `Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)` | Crée le mobile à ce centre, cette taille, cette vitesse ; immobile |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Calcule un chemin vers la destination (A*, puis lissé) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe |
| `void allerToutDroit(sf::Vector2f destination)` | Remplace le chemin par ce seul point, sans A* : le mobile avance en ligne droite (collision et glissement) |
| `void arreter()` | Réduit le chemin au centre actuel, pour cesser d'avancer sans reculer ; `estEnMouvement()` devient faux |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Avance vers le point courant du chemin (collision avec les murs et les cercles obstacles, glissement) |
| `sf::Vector2f getCentre() const` | Centre du mobile |
| `sf::Vector2f getTaille() const` | Taille du mobile |
| `bool estEnMouvement() const` | Définition unique de « en mouvement » : au dernier tic, le point courant du chemin était à plus de 0,01 px, que le mobile ait pu avancer ou non (bloqué = en mouvement ; arrivé à ce tic = encore en mouvement). Faux avant le premier tic et après `arreter()` ; `[[nodiscard]]` |
| `std::optional<sf::Vector2f> directionVoulue() const` | Vecteur du centre vers le point courant du chemin ; vide s'il n'y a plus de point à atteindre ou si la distance est nulle |

### `Chemin` — recherche de chemin (A*)

Trois fonctions libres, sans état. `trouverChemin` raisonne en cases,
`lisserChemin` et `replierVersOrigine` en pixels. Les deux premières sont
utilisées par `Mobile::allerVers`, la troisième par `Escouade::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee)` | Plus court chemin entre deux cases (4 directions, coût 1 par pas, heuristique de Manhattan) ; inclut le départ et l'arrivée ; vecteur vide si départ/arrivée est un mur (ou hors carte) ou si l'arrivée est inaccessible |
| `std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Vector2f taille)` | Retire de `points` (positions du centre d'un rectangle `taille`, en pixels) les points de passage inutiles ; `depart` n'en fait pas partie dans le résultat |
| `sf::Vector2f replierVersOrigine(const TileMap& carte, sf::Vector2f origine, sf::Vector2f cible, sf::Vector2f taille)` | `cible` si le rectangle `taille` centré sur `origine` peut aller en ligne droite jusqu'à elle sans toucher un mur ; sinon le point libre le plus éloigné de `origine` sur le segment (dichotomie, 6 itérations) ; `origine` si aucun point n'est libre |

### `Personnage` — un membre de l'escouade

Un carré coloré de 20 × 20 qui va vers le point que lui donne `Escouade`,
frappe à portée, a des points de vie et peut mourir. Ne connaît ni la
formation, ni `Escouade`, ni `Monstre`.

Données membres : `m_mobile` (`Mobile`, porte la position et la vitesse),
`m_couleur`, `m_attaqueDeBase` (`Skill`), `m_skill`
(`std::optional<Skill>`), `m_cibleChemin` (`std::optional<sf::Vector2f>`,
cible du dernier A*), `m_pointsDeVie` (`PointsDeVie`), `m_armure` (`int`), `m_actionEngagee` (`std::optional<ActionEngagee>`, l'action en exécution — attaque de base ou skill — et sa cible ; lue seulement pendant l'exécution).

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::optional<Skill> skill, int armure)` | Crée le personnage à ce centre, avec cette couleur, cette attaque de base, ce skill éventuel et cette armure (50 pour les mêlées, 0 pour les tireurs, fournie par `Escouade`) |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Si vivant, avance vers la cible (en ligne droite si le trajet est libre, sinon par un chemin A* lissé recalculé seulement si la cible s'éloigne de plus d'une case de sa fin ; collision avec les murs et les cercles obstacles, glissement) puis décrémente la recharge de l'attaque et du skill ; sinon ne fait rien |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage (éclairci de 50 % pendant l'exécution d'un coup ou l'activation d'un skill) et sa barre de vie, plus la barre de recharge du skill s'il en a un ; carré gris sans barre s'il est mort |
| `bool estAPortee(Cercle cible) const` | L'écart bord à bord entre son cercle et `cible` est-il `<=` la portée de `m_attaqueDeBase` ? |
| `bool engagerAttaque(int idCible)` | Engage un coup d'attaque de base sur ce monstre : faux s'il est mort, en mouvement (`Mobile::estEnMouvement`), si une action (attaque ou skill) est déjà engagée, ou si l'attaque n'est pas prête (`Skill::commencer`). La portée et la vue sont vérifiées par `Escouade` |
| `std::optional<int> cibleEngagee() const` | Monstre visé par le coup de base en exécution ; vide hors exécution (et pendant l'activation d'un skill) |
| `int avancerAttaque(float dt, Generateur& generateur)` | Fait avancer l'exécution ; rend les dégâts (puissance de l'attaque) au départ du coup, 0 sinon ; `[[nodiscard]]` |
| `void annulerAttaque()` | Annule le coup de base en exécution, sans recharge consommée ; sans effet sur l'activation d'un skill (`annulerSkill`) |
| `bool estEnExecution() const` | Un coup de base en exécution ou un skill en activation : le personnage reste sur place |
| `std::optional<TypeSkill> typeSkill() const` | Type du skill, vide sans skill |
| `bool skillEstPret() const` | Vivant, avec un skill en phase prête (recharge écoulée, pas en activation) |
| `std::optional<EtatSkill> etatDuSkill() const` | Type, progression de la recharge, temps restant, état de vie et activation en cours ; vide sans skill |
| `bool skillEstAPortee(Cercle cible) const` | Le skill est-il à portée de `cible` (bord à bord) ? Faux sans skill |
| `Cercle zoneDuSkill() const` | Cercle centré sur le personnage, de rayon le rayon de zone de son skill (0 sans skill) |
| `Cercle cercleDePorteeDuSkill() const` | Cercle centré sur le personnage, de rayon son rayon + la portée de son skill (rayon seul sans skill) |
| `bool engagerSkill(std::optional<Cible> cible)` | Engage le skill : faux s'il est mort, sans skill ou si le skill n'est pas prêt (`Skill::commencer`). Ne demande pas d'immobilité. Annule le coup de base en exécution. La portée et la vue sont vérifiées par `Escouade` ; la cible (vide pour la provocation) est stockée sans être interprétée |
| `std::optional<ActionEngagee> skillEngage() const` | L'action `Skill` en activation, avec sa cible ; vide sinon |
| `int avancerSkill(float dt, Generateur& generateur)` | Fait avancer l'activation ; rend la puissance du skill au départ de l'effet (et relance au complet la recharge de `m_attaqueDeBase`), 0 sinon ; `[[nodiscard]]` |
| `void annulerSkill()` | Annule l'activation, sans recharge consommée |
| `void recevoirSoin(int quantite)` | Rend des points de vie, sans dépasser le maximum ; sans effet sur un mort (délègue à `PointsDeVie::soigner`) |
| `int getPointsDeVie() const` / `int getPointsDeVieMax() const` | Points de vie actuels et maximum |
| `void recevoirDegats(int quantite)` | Retire `appliquerArmure(quantite, m_armure)` points de vie, sans descendre sous zéro (délègue à `PointsDeVie::subirDegats`) |
| `bool estVivant() const` | Points de vie `> 0` |
| `sf::Vector2f getCentre() const` | Centre du personnage |
| `Cercle getCercle() const` | Cercle centré sur le personnage, de rayon 10 |

### `Deplacement` — collision et glissement partagés

Deux fonctions libres, sans état, utilisées par `Mobile::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `bool deplacementBloque(const Cercle& avant, const Cercle& apres, const std::vector<Cercle>& obstacles)` | Vrai si, pour au moins un obstacle, `apres` chevauche l'obstacle (distance entre centres `<` somme des rayons) ET la distance entre centres a diminué par rapport à `avant` |
| `sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement, const std::vector<Cercle>& obstacles)` | Essaie le déplacement axe par axe (X puis Y) ; un essai est refusé s'il chevauche un mur ou si `deplacementBloque` est vrai ; renvoie la nouvelle position du coin haut-gauche |

### `Monstre` — un ennemi

Un carré rouge de 30 × 30 qui détecte un personnage, le poursuit, riposte
à portée et retourne à son poste. Possédé par `main.cpp`. Ne connaît ni
`Escouade` ni `Personnage` : il reçoit sa proie et ses obstacles en
paramètres.

Données membres : `m_mobile`, `m_pointsDeVie` (`PointsDeVie`), `m_armure` (`int`), `m_id`,
`m_attaque`, `m_rayonDetection`, `m_rayonTerritoire`, `m_etat` (`Etat`,
`enum class` privé : `EnGarde`, `Poursuite`, `Retour`), `m_poste`,
`m_vientDEtreTouche` (`bool`, posé par `recevoirDegats` si dégâts `> 0`,
consommé par `mettreAJour`), `m_idProieEngagee` (`std::optional<int>`, place du personnage visé par le coup en exécution).

| Fonction | Rôle |
|---|---|
| `Monstre(sf::Vector2f centre, int id, int armure)` | Crée le monstre à ce centre, avec cet identifiant, cette armure (20, donnée par `Niveau`) et 100 points de vie ; son poste est ce centre |
| `void dessiner(sf::RenderTarget& rendu, bool estCible) const` | Dessine le monstre (éclairci de 50 % pendant l'exécution d'un coup) et sa barre de vie ; ajoute un contour jaune si `estCible` |
| `void mettreAJour(float dt, std::optional<Cercle> cible, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Décrémente la recharge. Pendant l'exécution d'un coup : arrête le monstre et le déplace (immobile), sans toucher à l'état. Sinon met à jour l'état et donne ses ordres à `m_mobile` (poursuite, arrêt à portée, retour au poste), puis le déplace ; `obstacles` lui est transmis |
| `bool engagerAttaque(Cercle proie, int idProie)` | Engage un coup (0,3 s d'exécution) : faux si la proie n'est pas à portée, si le monstre est en mouvement ou si l'attaque n'est pas prête. `idProie` (la place du personnage) est retenu jusqu'au départ |
| `std::optional<int> proieEngagee() const` | `idProie` du coup en exécution ; vide hors exécution |
| `int avancerAttaque(float dt, Generateur& generateur)` | Fait avancer l'exécution ; rend les dégâts au départ du coup, 0 sinon ; `[[nodiscard]]` |
| `void annulerAttaque()` | Annule le coup en exécution, sans recharge consommée (appelée par `Partie` quand la proie engagée meurt) |
| `sf::Vector2f getCentre() const` | Centre du monstre |
| `Cercle getCercle() const` | Cercle centré sur le monstre, de rayon 15 |
| `int getId() const` | Identifiant du monstre |
| `bool contient(sf::Vector2f point) const` | Le point (pixels du monde) est-il dans le carré du monstre ? |
| `void recevoirDegats(int quantite)` | Retire `appliquerArmure(quantite, m_armure)` points de vie, sans descendre sous zéro (`PointsDeVie::subirDegats`) ; si `quantite > 0`, mémorise que le monstre vient d'être touché |
| `bool estVivant() const` | Points de vie `> 0` |
| `int getPointsDeVie() const` | Points de vie actuels |

### `Skill` — une capacité spéciale

Un `TypeSkill` (`enum class { Degats, Soin, Provocation, Zone }`), une
portée, une puissance (dégâts, points de vie rendus ou durée en secondes,
dégâts d'un skill de zone), une recharge, ainsi qu'un rayon de zone (la
taille de la zone touchée par un skill de zone ; 0, non lu, pour un skill
à cible unique). Sert aussi d'attaque de base : un `Skill` de type
`Degats`, de rayon de zone 0.
Il suit une machine à états (`enum class PhaseSkill { Pret, EnExecution, EnRecharge }`) : `commencer` (prêt → en exécution), `avancer` (fin de l'exécution : le coup part, la recharge est tirée, → en recharge), `annuler` (en exécution → prêt, sans recharge). Les skills des touches 1 à 4 passent par la même machine à états (étape 58 : activation de 0,3 s, 0,2 s, 0,6 s et 0,8 s pour dégâts, provocation, soin et zone) ; avec un temps nul, l'effet part dans l'image de l'engagement. `declencher` et `declencherSansCible` ne sont plus utilisées par le jeu ; il ne connaît pas la cible du coup engagé, c'est son propriétaire qui la retient.
Possédé par valeur par `Personnage` (attaque de base, plus un skill en
option) et par `Monstre` (son attaque). Ne connaît ni la position de son
propriétaire ni sa cible : il reçoit deux `Cercle`. Ne dessine rien.

Données membres : `m_type` (`TypeSkill`), `m_portee` (bord à bord),
`m_puissance`, `m_rechargeMin`, `m_rechargeMax` (intervalle de recharge,
en secondes), `m_tempsRecharge` (durée de la recharge en cours, dernier
tirage), `m_recharge`, `m_rayonZone` (`float`, en pixels), `m_tempsExecution`, `m_executionRestante` (secondes), `m_phase` (`PhaseSkill`).

| Fonction | Rôle |
|---|---|
| `Skill(TypeSkill type, float portee, int puissance, float rechargeMin, float rechargeMax, float rayonZone, float tempsExecution = 0)` | Crée le skill de ce type, avec cette portée, cette puissance, cet intervalle de recharge (`assert(0 <= min <= max)` ; fixe si `min == max`), ce rayon de zone et ce temps d'exécution (`assert >= 0`) ; prêt à la création |
| `bool commencer()` | Prêt → en exécution ; faux, sans rien changer, dans toute autre phase ; `[[nodiscard]]` |
| `bool avancer(float dt, Generateur& generateur)` | En exécution : fait avancer l'exécution ; vrai une seule fois, au départ du coup (la recharge est alors tirée, phase → en recharge). Autres phases : comme `mettreAJour`, faux ; `[[nodiscard]]` |
| `void annuler()` | En exécution → prêt, sans recharge ; sans effet ailleurs |
| `PhaseSkill getPhase() const` | Phase actuelle |
| `int getPuissance() const` | Puissance du skill |
| `void mettreAJour(float dt)` | Décrémente la recharge, sans descendre sous zéro ; en recharge écoulée, repasse à prêt |
| `bool estAPortee(Cercle origine, Cercle cible) const` | `distanceBordABord(origine, cible)` est-elle `<=` `m_portee` ? |
| `int declencher(Cercle origine, Cercle cible, Generateur& generateur)` | Puissance si la cible est à portée et le skill prêt (relance alors la recharge, tirée dans l'intervalle), sinon 0 sans toucher la recharge ni le générateur |
| `int declencherSansCible(Generateur& generateur)` | Comme `declencher`, sans vérifier la portée ni prendre de cible : puissance si la recharge est écoulée (relance alors la recharge), sinon 0. Pour la provocation et la zone |
| `void relancerRecharge(Generateur& generateur)` | Remet la recharge au complet, sans déclencher : `min` si `min == max` (aucun tirage), sinon un tirage uniforme (`std::uniform_real_distribution<float>`) dans `[min, max]` |
| `float getProgressionRecharge() const` | De 0 (juste après le déclenchement) à 1 (prêt) ; 1 si le temps de recharge est nul |
| `float getRechargeRestante() const` | Temps de recharge restant, en secondes (0 si prêt) |
| `TypeSkill getType() const` | Type du skill |
| `float getRayonZone() const` | Rayon de zone du skill |
| `float getPortee() const` | Portée du skill (bord à bord) |

### `Coup` — ce qui circule entre `main.cpp` et `Escouade`

`Coup.hpp`, sans `.cpp` : `struct Coup { int idMonstre; int degats; }` (des
dégâts à infliger à ce monstre). Sortie d'`Escouade`.

### `ActionEngagee` — l'action qu'un personnage prépare

`ActionEngagee.hpp`, sans `.cpp` : `enum class TypeAction { AttaqueDeBase, Skill }` et `struct ActionEngagee { TypeAction action; std::optional<Cible> cible; }`. La cible est vide pour la provocation ; pour une attaque de base, c'est une `CibleMonstre` dont le `cercle` n'est pas renseigné. Stockée par `Personnage` (`m_actionEngagee`), résolue par `Escouade`. `TypeAction` sert aussi à `Escouade::peutAtteindre` (portée de l'attaque de base ou du skill).

### `MonstreVu` — un monstre tel que l'escouade le voit

`MonstreVu.hpp` / `.cpp` : `struct MonstreVu { int id; Cercle cercle; }`
(entrée d'`Escouade` et d'`Autodefense`) et une fonction libre.

| Fonction | Rôle |
|---|---|
| `const MonstreVu* trouverMonstre(const std::vector<MonstreVu>& monstres, int id)` | Monstre portant cet identifiant, `nullptr` s'il est absent (`[[nodiscard]]`) |

`main.cpp` a son équivalent pour `std::vector<Monstre>`, `trouverMonstreParId`
(namespace anonyme).

### `TypeSkill` — le type d'un skill

`TypeSkill.hpp`, sans `.cpp` : `enum class TypeSkill { Degats, Soin,
Provocation, Zone }`. Inclus par `Skill`, `EtatSkill`, `BarreDeSkills` et
`Escouade` sans qu'ils dépendent de la classe `Skill`.

### `Autodefense` — quel monstre un combattant frappe

Deux fonctions libres, sans état. `choisirMonstreAFrapper` est utilisée par
`Escouade::attaquer` et `Escouade::choisirCible`, `choisirAgresseurAViser`
par `Escouade::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `std::optional<int> choisirMonstreAFrapper(Cercle origine, const std::function<bool(Cercle)>& estAPortee, const std::vector<MonstreVu>& monstres, std::optional<int> idVerrouille)` | Identifiant du monstre verrouillé s'il est à portée ; sinon du plus proche (bord à bord) parmi ceux à portée (égalité : le premier de la liste) ; sinon vide. `estAPortee` dit si un cercle est à portée (celle de l'attaque ou celle d'un skill) |
| `std::optional<Cercle> choisirAgresseurAViser(Cercle personnage, sf::Vector2f place, float laisse, const std::vector<MonstreVu>& agresseurs)` | Cercle de l'agresseur le plus proche du personnage (bord à bord ; égalité : le premier de la liste) parmi ceux dont le centre est à `laisse` ou moins de `place` ; sinon vide |

### `Cible` — ce que vise un skill

`Cible.hpp`, sans `.cpp` : `struct CibleAllie { std::size_t place; }`
(de 1 à 4), `struct CibleMonstre { int id; Cercle cercle; }`,
`struct CiblePoint { sf::Vector2f point; }` (point fixé au clic, pour un
skill de zone en attente) et
`using Cible = std::variant<CibleAllie, CibleMonstre, CiblePoint>`. L'absence de
cible est un `std::optional<Cible>` vide. `Cible.hpp` définit aussi
`template<class... Ts> struct Surcharges : Ts... { using Ts::operator()...; }`
pour `std::visit` : une lambda par alternative, et un cas oublié est une
erreur de compilation.

### `EtatSkill` — le skill d'une place, pour l'affichage

`EtatSkill.hpp`, sans `.cpp` : `struct EtatSkill { TypeSkill type; float progression; float tempsRestant; bool lanceurVivant; bool enActivation; }`. `progression` va de 0 (juste lancé) à 1 (prêt), `tempsRestant` est en secondes, `enActivation` est vrai pendant l'activation du skill. Produite par `Escouade::etatDuSkill`, consommée par `BarreDeSkills`.

### `BarreDeSkills` — la barre de skills à l'écran

Fonctions libres, sans état : tout est recalculé à chaque image. Ne connaît ni `Escouade` ni `Personnage` : elle reçoit des `EtatSkill`.

| Fonction | Rôle |
|---|---|
| `char lettreDuSkill(TypeSkill type)` | D (dégâts), P (provocation), S (soin), Z (zone) |
| `sf::Color couleurDuContour(bool enVisee, bool enActivation)` | Jaune en visée (prioritaire), blanc en activation, gris sinon |
| `int secondesAffichees(float tempsRestant)` | Secondes entières, arrondies vers le haut |
| `std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre)` | Sommets d'un `TriangleFan` : centre, aiguille, coins du carré dans l'intervalle, midi ; vide si la fraction est nulle |
| `void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police, const std::array<std::optional<EtatSkill>, 4>& etats, std::optional<std::size_t> placeEnVisee)` | Dessine 4 icônes de 56 px en bas de la vue par défaut (à appeler avec elle) : fond, lettre, numéro de touche, quartier sombre et chiffre en recharge, grisée si le lanceur est mort, contour jaune si `placeEnVisee`, blanc si `enActivation` |

### `EcranDeDefaite` — l'écran de défaite

Une fonction libre, sans état, sur le modèle de `BarreDeSkills`. Ne connaît ni `Partie` ni `Escouade`.

| Fonction | Rôle |
|---|---|
| `void dessinerEcranDeDefaite(sf::RenderTarget& rendu, const sf::Font& police)` | Voile noir semi-transparent sur toute la vue par défaut, puis « Défaite » et « Appuyez sur R pour recommencer » au centre ; à appeler avec la vue par défaut, après la barre de skills |

### `BarreDeVie` — la barre de vie d'une entité

Deux fonctions libres, sans état, utilisées par `Personnage::dessiner` et
`Monstre::dessiner`, et une constante `s_hauteurBarreDeVie` (5 px).

| Fonction | Rôle |
|---|---|
| `sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille)` | Coin haut-gauche de la barre d'une entité de ce centre et de cette taille : alignée sur son bord gauche, 6 px au-dessus de son carré |
| `void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio)` | Fond gris, puis rectangle vert proportionnel à `ratio` (0 à 1) |

### `Eclaircir` — l'indice visuel d'un coup en préparation

Une fonction libre, sans état, utilisée par `Personnage::dessiner` et `Monstre::dessiner`.

| Fonction | Rôle |
|---|---|
| `sf::Color eclaircir(sf::Color couleur, float fraction)` | La couleur mélangée au blanc à raison de `fraction` (0 = inchangée, 1 = blanc) ; l'opacité est conservée ; `[[nodiscard]]` |

### `Aleatoire` — le générateur aléatoire

`Aleatoire.hpp`, sans `.cpp` : `using Generateur = std::mt19937;`. Le reste
du code écrit `Generateur` et ne dépend pas de l'algorithme. Possédé par
`Partie` (un seul, initialisé par une graine), passé PAR RÉFÉRENCE le long
de la chaîne `Partie` → `Escouade` → `Personnage` → `Skill` et `Partie` →
`Monstre` → `Skill` ; jamais copié, jamais gardé, jamais global.

### `Defense` — l'armure

Une fonction libre pure, sans état, utilisée par `Personnage::recevoirDegats`
et `Monstre::recevoirDegats`. Ne dépend d'aucun autre module ni de SFML.

| Fonction | Rôle |
|---|---|
| `int appliquerArmure(int montant, int armure)` | Dégâts réellement subis : `montant × K / (armure + K)` avec K = 100 (rendement décroissant, jamais 100 %), arrondi au plus proche, au moins 1 si `montant > 0`, 0 si `montant == 0` ; `assert(montant >= 0)` et `assert(armure >= 0)` ; `[[nodiscard]]` |

### `PointsDeVie` — les points de vie d'une entité

Classe à sémantique de valeur, possédée par valeur par `Personnage` et
`Monstre`, qui lui délèguent leurs façades (`recevoirDegats`, `estVivant`…).
Ne dépend d'aucun autre module ni de SFML. Invariants : `0 <= valeur <=
maximum`, `maximum > 0`, une valeur à 0 ne remonte plus.

Données membres : `m_valeur`, `m_maximum`.

| Fonction | Rôle |
|---|---|
| `explicit PointsDeVie(int maximum)` | Commence plein ; `assert(maximum > 0)` |
| `void subirDegats(int montant)` | `assert(montant >= 0)` ; valeur bornée à 0 ; un montant de 0 est permis |
| `void soigner(int montant)` | `assert(montant >= 0)` ; valeur bornée au maximum ; sans effet si la valeur est à 0 |
| `bool estVivant() const` | Valeur `> 0` (`[[nodiscard]]`) |
| `int getValeur() const` / `int getMaximum() const` | Valeur actuelle et maximum |
| `float getRatio() const` | Valeur / maximum |

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
| `docs/boucle-et-entrees.md` | Qui fait quoi entre `main.cpp`, `Controleur` et `Partie`, ordre d'une image, entrée souris et clavier, mode visée, combat engagé, défaite, caméra | Avant de modifier `main.cpp`, `Partie`, `Controleur` ou `Niveau` |
| `docs/carte-et-collision.md` | Carte, collision et dessin, déplacement avec glissement et obstacles, plan texte et validation | Avant de modifier `TileMap` ou `Deplacement` |
| `docs/generation.md` | Génération de donjon : placement avec rejet, emprise / forme creusée, paramètres, graine unique | Avant de modifier `GenerateurDonjon` |
| `docs/chemin.md` | Recherche de chemin (A*), suivi de chemin (`Mobile`), lissage de chemin | Avant de modifier `Chemin` ou `Mobile` |
| `docs/formation.md` | `Escouade` et `Personnage` : ancre, formation, orientation, verrou de cible, placement en combat, mort et défaite | Avant de modifier `Escouade` ou `Personnage` |
| `docs/combat.md` | `Skill` (dont l'attaque de base), `Cercle`, portée bord à bord, autodéfense, `Cible`, skill en attente, circuit des dégâts, qui bloque qui | Avant de modifier `Cercle`, `Autodefense`, `Coup`, `Skill`, `Cible`, le skill ou les listes d'obstacles |
| `docs/monstre.md` | `Monstre` : identifiant, états, détection, territoire, poursuite, retour au poste, riposte | Avant de modifier `Monstre` |
| `docs/tests.md` | Tests (Catch2, conventions) | Avant d'ajouter ou de modifier un test |
| `docs/audit.md` | Audit du code avant l'étape 49 : constats (R, D, P, F, C, T) et ce qui a été traité | Avant de planifier une refactorisation |
| `docs/brainstorm.md` | Idées à explorer par thème (rythme du combat, défense, gambits, génération procédurale), leur statut et leurs prérequis ; rien n'y est une décision | Avant de proposer une nouvelle étape |
| `docs/historique.md` | Historique détaillé, tel quel | Jamais nécessaire avant de modifier du code |
