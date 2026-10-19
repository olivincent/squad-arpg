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
│   ├── Application.hpp / .cpp
│   ├── Ecran.hpp
│   ├── Transition.hpp
│   ├── Contexte.hpp
│   ├── EcranMenu.hpp / .cpp
│   ├── EcranOptions.hpp / .cpp
│   ├── EcranChoixEscouade.hpp / .cpp
│   ├── EcranJeu.hpp / .cpp
│   ├── ListeDeBoutons.hpp / .cpp
│   ├── Reglages.hpp / .cpp
│   ├── TailleDeVue.hpp / .cpp
│   ├── Niveau.hpp / .cpp
│   ├── ObjetInteractif.hpp / .cpp
│   ├── ChampDeVision.hpp / .cpp
│   ├── Vision.hpp / .cpp
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
│   ├── IdSkill.hpp
│   ├── SkillConnu.hpp
│   ├── AdresseSkill.hpp
│   ├── Composition.hpp / .cpp
│   ├── Specialite.hpp / .cpp
│   ├── SelectionDEscouade.hpp / .cpp
│   ├── Coup.hpp
│   ├── EntiteVue.hpp / .cpp
│   ├── TypeSkill.hpp
│   ├── Autodefense.hpp / .cpp
│   ├── Encerclement.hpp / .cpp
│   ├── Engagement.hpp / .cpp
│   ├── Mobile.hpp / .cpp
│   ├── Cercle.hpp / .cpp
│   ├── TraceDeChemin.hpp / .cpp
│   ├── EtatSkill.hpp
│   ├── BarreDeRaccourcis.hpp / .cpp
│   ├── ClicInterface.hpp / .cpp
│   ├── BarreDeSkills.hpp / .cpp
│   ├── EcranDeFin.hpp / .cpp
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
│   │   test_Escouade_vue.cpp, test_Escouade_places.cpp,
│   │   test_Escouade_degagement.cpp,
│   │   test_Composition.cpp, test_Specialite.cpp,
│   │   test_SelectionDEscouade.cpp
│   ├── test_Provocations.cpp, test_Agresseurs.cpp
│   ├── test_Deplacement.cpp, test_Monstre.cpp, test_Personnage.cpp
│   ├── test_Partie.cpp, test_Controleur.cpp, test_GenerateurDonjon.cpp,
│   │   test_ObjetInteractif.cpp
│   ├── test_ChampDeVision.cpp, test_Vision.cpp
│   └── test_Skill.cpp, test_Mobile.cpp, test_Cercle.cpp,
│       test_Autodefense.cpp, test_Encerclement.cpp, test_Engagement.cpp, test_BarreDeSkills.cpp,
│       test_BarreDeRaccourcis.cpp, test_ClicInterface.cpp,
│       test_EntiteVue.cpp, test_BarreDeVie.cpp,
│       test_PointsDeVie.cpp, test_Defense.cpp, test_Eclaircir.cpp,
│       test_Reglages.cpp, test_TailleDeVue.cpp,
│       test_ListeDeBoutons.cpp, test_EcranMenu.cpp,
│       test_EcranOptions.cpp, test_EcranChoixEscouade.cpp
├── docs/
│   ├── build.md
│   ├── boucle-et-entrees.md
│   ├── ecrans.md
│   ├── carte-et-collision.md
│   ├── chemin.md
│   ├── formation.md
│   ├── generation.md
│   ├── vision.md
│   ├── combat.md
│   ├── monstre.md
│   ├── tests.md
│   ├── audit.md
│   ├── brainstorm.md
│   └── historique.md
├── reglages.ini (créé par le jeu à côté de main.exe, ignoré par Git)
├── .gitignore
├── CLAUDE.md
├── ARCHITECTURE.md
└── ROADMAP.md
```

Deux exécutables : `main` (le jeu) et `tests` (les tests Catch2). Les deux
se lient à la bibliothèque statique `squad_core`, qui contient toute la
logique de jeu et les écrans (les modules ci-dessous) indépendamment de la
fenêtre et de la boucle. Seuls `main.cpp` et `Application` restent dans
l'exécutable `main`.

Qui possède quoi :
- `main.cpp` construit une `Application`, appelle `executer()` et affiche
  l'erreur (code 1) si la police manque.
- `Application` possède la fenêtre (`sf::RenderWindow`), la police
  (`sf::Font`), les `Reglages` et la pile d'écrans
  (`std::vector<std::unique_ptr<Ecran>>`). Elle charge `reglages.ini`, ouvre
  la fenêtre, fait tourner la boucle SFML (horloge, événements, mise à jour,
  dessin, `display`) et applique les `Transition` que renvoient les écrans.
  La fermeture de la fenêtre quitte, quel que soit l'écran. Seul l'écran du
  dessus reçoit les événements, est mis à jour et est dessiné.
- `EcranMenu`, `EcranOptions`, `EcranChoixEscouade` et `EcranJeu` sont les quatre `Ecran`. Ils
  reçoivent la fenêtre, la police et les réglages dans un `Contexte` à chaque
  appel et ne les gardent pas. `EcranChoixEscouade` possède la
  `SelectionDEscouade` ; `EcranJeu` reçoit la liste des `IdSpecialite` choisies, la garde, et possède une `Partie`, un
  `Controleur`, une `BarreDeRaccourcis`, la caméra et le `std::random_device` (il appelle `composer` pour construire la `Partie`) ; il traduit les
  événements SFML (clic, touches de la barre de raccourcis, Échap, clic droit, F3, F4 et F6 de débogage) en appels au
  `Controleur`. Un clic gauche est d'abord testé sur l'interface (`clicSurInterface`, vue d'interface) : s'il touche la barre, il est consommé et n'atteint pas le monde. Une touche est traduite en emplacement par `emplacementPourTouche` (table des `Reglages`). (Touche R : retour à l'écran de choix de l'escouade si la partie est perdue ou gagnée, sinon c'est un raccourci ;
  F5 : nouvelle carte à tout moment ; F5, comme l'entrée dans l'écran,
  passe par `nouvellePartie`, qui tire la graine et génère le niveau),
  convertit le curseur en coordonnées du monde, centre la caméra sur
  l'escouade et enchaîne le dessin. Il ne connaît ni `Monstre`,
  ni `Coup`, ni `EntiteVue`, ni `Chemin`, ni `Personnage`.
- `Partie` possède la `TileMap`, l'`Escouade`, un `std::vector<Monstre>` et le
  `Generateur` aléatoire (graine reçue au constructeur),
  construits à partir d'un `Niveau`, ainsi que les `ObjetInteractif` (les portes) et la `Vision` (ce que le joueur sait de la carte). Elle fait l'intermédiaire entre
  `Escouade` et `Monstre`, qui ne se connaissent pas : elle leur donne la
  liste des monstres (`EntiteVue`, deux listes : les cibles, sans ceux qui rentrent à leur poste, et tous les monstres comme obstacles) ; pour chaque monstre, elle lui donne aussi sa proie et ses candidats de frappe (`EntiteVue` de personnages) et applique les `Coup` renvoyés. Elle prend deux photos des camps par image (`EntiteDeControle`) et applique les coups gratuits que `Engagement` en déduit, dans les deux sens.
- `Controleur` porte l'état d'interface : le mode visée et le déplacement
  continu. Il ne garde aucune référence à la `Partie` : celle-ci lui est
  passée à chaque appel. Il ne connaît pas les événements SFML.
- `Escouade` possède un `Mobile` (l'ancre), N `Personnage` (un par `DefinitionPersonnage` de la composition reçue ; 3 par défaut), des `Provocations` et des `Agresseurs`, et utilise `Encerclement` (géométrie des places d'attaque et de formation). Ces deux derniers ne connaissent ni `Personnage`, ni `Monstre`, ni `Escouade`.
- `Personnage` possède un `Mobile`, un `Skill` (son attaque de base) et une liste de `SkillConnu` (ses skills, chacun avec son `IdSkill`) ; `Monstre` possède un `Mobile` et un
  `Skill` (son attaque).
- `Mobile` utilise `Chemin` et `Deplacement` ; il reçoit en plus, pour chaque ordre, la liste des cercles que son chemin doit contourner (`aContourner`), distincte de ses obstacles bloquants (`docs/chemin.md`). `Personnage` utilise
  `Mobile` et le test de trajet de `TileMap`. `Skill`, `Personnage`, `Monstre` et `Escouade` échangent
  des `Cercle`. `Personnage` et `Monstre` possèdent chacun un `PointsDeVie` et dessinent leur barre de vie avec `BarreDeVie` ; leur `recevoirDegats` passe par `appliquerArmure` (`Defense`). `Escouade` et `Monstre` utilisent `choisirCibleAFrapper` (`Autodefense`) et
`replierVersOrigine` (`Chemin`).

## Modules

### `Application` — la boucle et la pile d'écrans

`Application.hpp` / `.cpp`, dans l'exécutable `main` (pas dans `squad_core` :
elle possède la fenêtre). Voir `docs/ecrans.md`.

Données membres : `m_fenetre` (`sf::RenderWindow`), `m_police` (`sf::Font`),
`m_reglages` (`Reglages`), `m_pile` (`std::vector<std::unique_ptr<Ecran>>`).

| Fonction | Rôle |
|---|---|
| `Application()` | Charge `reglages.ini` (absent ou illisible = réglages par défaut), ouvre la fenêtre, charge la police (`std::runtime_error` si elle manque), empile `EcranMenu` |
| `int executer()` | Boucle : `dt`, événements (la croix ferme ; sinon `traiterEvenement` de l'écran du dessus puis `appliquer` de sa `Transition`), `mettreAJour`, `dessiner`, `display`. S'arrête quand la fenêtre est fermée ou la pile vide |
| `void ouvrirFenetre()` (privée) | Crée ou recrée la fenêtre à partir des réglages : plein écran à la résolution du bureau, ou fenêtré (`Titlebar` et `Close`) à celle des réglages ; réapplique 60 images par seconde et `setKeyRepeatEnabled(false)` |
| `void appliquer(Transition)` (privée) | `std::visit` : `Empiler` pousse l'écran, `Depiler` retire celui du dessus, `Quitter` ferme la fenêtre, `AppliquerReglages` copie les réglages, rouvre la fenêtre puis écrit `reglages.ini` |

### `Ecran`, `Transition`, `Contexte` — le patron State

`Ecran.hpp`, `Transition.hpp` et `Contexte.hpp`, sans `.cpp`.

`class Ecran` : base abstraite (copie interdite, destructeur virtuel) ;
fonctions virtuelles pures `Transition traiterEvenement(const sf::Event&,
const Contexte&)`, `void mettreAJour(float dt, const Contexte&)` et `void
dessiner(const Contexte&) const`. Les écrans ne sont manipulés que par
`std::unique_ptr<Ecran>`.

`Transition` : `std::variant<Aucune, Empiler, Depiler, Quitter,
AppliquerReglages>` ; `Empiler` porte un `std::unique_ptr<Ecran>`,
`AppliquerReglages` des `Reglages`. Un écran ne modifie jamais la pile lui-même :
l'`Application` applique la transition une fois l'appel terminé.

`struct Contexte { sf::RenderWindow& fenetre; const sf::Font& police; const
Reglages& reglages; }` : construit par l'`Application` à chaque appel, jamais
stocké par un écran.

### `EcranJeu` — la partie

`EcranJeu.hpp` / `.cpp`. Reprend ce que faisait `main()` pour le jeu.

Données membres : `m_tirageDeGraine` (`std::random_device`), `m_specialites` (`std::vector<IdSpecialite>`, les classes de l'escouade, dans l'ordre des places), `m_partie`
(`Partie`), `m_controleur` (`Controleur`), `m_barre` (`BarreDeRaccourcis`, reconstruite par `barreParDefaut` à chaque `nouvelleCarte`), `m_camera` (`sf::View`),
`m_debogage` (`OptionsDeDebogage { bool pointsVises = false; bool ancre = true; bool brouillard = true; }`,
état d'interface remis à sa valeur de départ par `nouvelleCarte`).

| Fonction | Rôle |
|---|---|
| `explicit EcranJeu(std::vector<IdSpecialite> specialites)` | Garde les classes (`m_specialites`, déclaré avant `m_partie`), tire une graine, la génère et construit la partie (`nouvellePartie`, qui appelle `composer(specialites)`) ; F5 réutilise les mêmes classes |
| `Transition traiterEvenement(...)` | Clic gauche (d'abord `clicSurInterface` : sur la barre, `Controleur::appuiInterface`, sauf partie perdue ou gagnée ; sinon `appuiGauche`), touches de la barre (`emplacementPourTouche` puis `activerEmplacement`, ignorées sur l'écran de fin), Échap, F5 (nouvelle carte), F3 (bascule les points visés et les chemins restants des personnages et des monstres, les zones de contrôle, et la semi-transparence des monstres qui rentrent à leur poste), F4 (bascule le carré vert de l'ancre), F6 (bascule le brouillard de guerre, visuel seulement) ; R sur l'écran de fin (partie perdue ou gagnée) renvoie `Depiler` (retour à l'écran de choix) ; sinon `Aucune` |
| `void mettreAJour(float, const Contexte&)` | Taille de la caméra (`tailleDeVue`), déplacement continu, `Partie::mettreAJour`, `Controleur::debutImage` (sortie de la visée), caméra centrée sur l'escouade |
| `void dessiner(const Contexte&) const` | Monde avec la caméra (ancre cachée si `!m_debogage.ancre`, brouillard selon `m_debogage.brouillard`), aperçu de visée, points visés et chemins restants si `m_debogage.pointsVises`, puis, avec `vueDInterface` : barre de raccourcis (une `IconeDeRaccourci` par emplacement), compteur « Monstres : N » et écran de fin |

### `EcranMenu` — le menu de départ

`EcranMenu.hpp` / `.cpp`. Donnée membre : `m_boutons` (`ListeDeBoutons`) :
« Commencer partie », « Options », « Quitter ».

| Fonction | Rôle |
|---|---|
| `Transition cliquer(sf::Vector2f point, sf::Vector2f centre, const Reglages&) const` | Point en coordonnées de la vue d'interface. Commencer : `Empiler(EcranChoixEscouade)` ; Options : `Empiler(EcranOptions)` avec les réglages en cours et `resolutionsDisponibles` du bureau ; Quitter : `Quitter` ; ailleurs : `Aucune` |

### `EcranOptions` — les options d'affichage

`EcranOptions.hpp` / `.cpp`. Données membres : `m_reglages` (copie), `m_resolutions`
(`std::vector<Resolution>` disponibles), `m_indiceResolution`, `m_boutons`
(trois lignes : mode, résolution, Retour).

| Fonction | Rôle |
|---|---|
| `EcranOptions(const Reglages&, std::vector<Resolution>)` | Part des réglages en cours ; l'indice est celui de la résolution courante dans la liste (0 si absente) |
| `Transition cliquer(sf::Vector2f point, sf::Vector2f centre)` | Mode : bascule, `AppliquerReglages`. Résolution : tiers gauche = précédente, tiers droit = suivante, `AppliquerReglages` (`Aucune` aux bouts de la liste, au milieu, ou ligne inactive : plein écran ou liste vide). Retour : `Depiler` |

### `EcranChoixEscouade` — le choix des 3 classes

`EcranChoixEscouade.hpp` / `.cpp`. Entre le menu et la partie. Données membres : `m_selection` (`SelectionDEscouade`), `m_classes` (`ListeDeBoutons`, un bouton par entrée du `catalogue()`, dans son ordre), `m_actions` (`ListeDeBoutons` : « Commencer », « Retour »). Trois colonnes sans position stockée, recalculées à partir du `centre` de la vue d'interface : les classes à gauche, les cases de sélection puis les actions à droite. Voir `docs/ecrans.md`.

| Fonction | Rôle |
|---|---|
| `sf::Vector2f centreDesClasses(sf::Vector2f centre)` / `centreDesCases(...)` / `centreDesActions(...)` | Centre de chaque colonne (décalages de −300, +300 et +300 px en x ; 0, −80 et +140 en y) ; une seule source pour le dessin, le clic et les tests ; `[[nodiscard]]` |
| `EcranChoixEscouade()` | Sélection vide, « Commencer » inactif |
| `Transition cliquer(sf::Vector2f point, sf::Vector2f centre)` | Classe : l'ajoute à la sélection (sans effet si elle est complète). Case remplie : la retire. « Commencer » (actif seulement si la sélection est complète) : `Empiler(EcranJeu(sélection))`, la sélection restant dans cet écran. « Retour » : `Depiler`. Case vide ou ailleurs : `Aucune` ; `[[nodiscard]]` |
| `const SelectionDEscouade& getSelection() const` | La sélection en cours (tests) ; `[[nodiscard]]` |
| `void dessiner(const Contexte&) const` | Titre, boutons des classes, trois cases « 1. Guerrier » ou « 1. — » (bande de la couleur que le personnage aura en jeu, doublons assombris), actions |

### `SelectionDEscouade` — les classes choisies

`SelectionDEscouade.hpp` / `.cpp`. Logique pure : une classe et deux fonctions libres. Donnée membre : `m_specialites` (`std::vector<IdSpecialite>`) ; constante `static constexpr std::size_t s_taille = 3`.

| Fonction | Rôle |
|---|---|
| `void ajouter(IdSpecialite id)` | À la fin s'il reste de la place, sans effet sinon ; les doublons sont permis |
| `void retirer(std::size_t indice)` | Retire cette case, les suivantes remontent ; `assert(indice < liste().size())` |
| `bool estComplete() const` | Trois classes choisies ; `[[nodiscard]]` |
| `const std::vector<IdSpecialite>& liste() const` | Dans l'ordre de sélection, c'est l'ordre des places 1 à 3 ; `[[nodiscard]]` |
| `sf::FloatRect rectangleDeLaCase(std::size_t indice, sf::Vector2f centre)` | Case de sélection `indice` : colonne de 3 cases de 520 × 60 px, 20 px d'écart, centrée sur `centre` ; `assert` ; `[[nodiscard]]` |
| `std::optional<std::size_t> caseSous(sf::Vector2f point, sf::Vector2f centre)` | Case qui contient le point, vide dans un écart ou hors des cases ; construite sur `rectangleDeLaCase` ; `[[nodiscard]]` |

### `ListeDeBoutons` — une colonne de boutons

`ListeDeBoutons.hpp` / `.cpp`. Données membres : `m_libelles`, `m_actifs`. Aucune
position stockée : la colonne (boutons de 520 × 60 px, 20 px d'écart) est
recalculée à partir du `centre` reçu.

| Fonction | Rôle |
|---|---|
| `explicit ListeDeBoutons(std::vector<std::string> libelles)` | Tous les boutons sont actifs |
| `sf::FloatRect rectangleDuBouton(std::size_t indice, sf::Vector2f centre) const` | Rectangle du bouton, la colonne centrée sur `centre` |
| `void definirActif(std::size_t indice, bool actif)` | Un bouton inactif est grisé et jamais « sous » le curseur |
| `std::optional<std::size_t> boutonSous(sf::Vector2f point, sf::Vector2f centre) const` | Indice du bouton actif qui contient le point |
| `void dessiner(sf::RenderTarget&, const sf::Font&, sf::Vector2f centre, std::optional<std::size_t> survole) const` | Dessine fond, contour et libellé (UTF-8) ; le bouton survolé est plus clair |

### `Reglages` — les réglages d'affichage

`Reglages.hpp` / `.cpp` : logique pure, fonctions libres sans état.
`enum class ModeAffichage { Fenetre, PleinEcran }`, `struct Resolution {
unsigned largeur, hauteur; }` (comparable), `struct Reglages { unsigned largeur =
1200; unsigned hauteur = 800; ModeAffichage mode = Fenetre; TouchesDesRaccourcis touchesDesRaccourcis; }`, où `using TouchesDesRaccourcis = std::array<sf::Keyboard::Scancode, s_emplacementsPrincipaux>` (index = emplacement ; touches physiques ; par défaut Q W E R / A S D F / Z X C V). Les touches ne sont ni lues ni écrites dans `reglages.ini` (l'écran de configuration viendra plus tard).

| Fonction | Rôle |
|---|---|
| `const std::vector<Resolution>& resolutionsProposees()` | 1200 × 800, 1280 × 720, 1600 × 900, 1920 × 1080 |
| `std::vector<Resolution> resolutionsDisponibles(sf::Vector2u tailleBureau)` | Celles qui tiennent dans le bureau |
| `Reglages lireReglages(std::istream&)` | Format `cle=valeur` (`largeur`, `hauteur`, `mode` = `fenetre` ou `plein_ecran`). Jamais d'exception : clé inconnue ou ligne mal formée ignorée, valeur invalide = défaut, résolution hors liste = 1200 × 800 |
| `void ecrireReglages(std::ostream&, const Reglages&)` | Écrit les trois lignes lues par `lireReglages` |
| `std::optional<std::size_t> emplacementPourTouche(const TouchesDesRaccourcis&, sf::Keyboard::Scancode)` | Emplacement de cette touche dans la table, vide si elle n'y est pas ; `[[nodiscard]]` |

### `TailleDeVue` — la vue à hauteur fixe

`TailleDeVue.hpp` / `.cpp`, constante `s_hauteurDeVue` (800 px).

| Fonction | Rôle |
|---|---|
| `sf::Vector2f tailleDeVue(sf::Vector2u tailleFenetre, float hauteur)` | `hauteur` en hauteur, largeur = `hauteur × largeur / hauteur de la fenêtre` |
| `sf::View vueDInterface(sf::Vector2u tailleFenetre)` | Vue de 800 px de haut, coin haut gauche en (0, 0) : barre de skills, compteur, écran de fin, menus |

### `TileMap` — l'environnement de jeu

Grille de cases de taille fixe, chaque case étant du sol ou un mur. Un
point hors de la carte est un mur. Construite à partir d'un plan texte
(`'#'` = mur, tout autre caractère = sol).

Données membres : `m_largeur`, `m_hauteur` (cases), `m_tailleCase`
(pixels), `m_cases` (`std::vector<Tile>`, `enum class Tile { Floor, Wall, PorteFermee, PorteOuverte }`). `bool estBloquante(Tile)` (fonction libre, vrai pour `Wall` et `PorteFermee`) est la seule façon de demander « passe-t-on, voit-on » : collision, A*, lissage, repli, champ de vision (`ChampDeVision`, donc aussi `ligneDeVue`) et skill de zone l'utilisent, aucun ne teste `Tile::Wall` directement.

| Fonction | Rôle |
|---|---|
| `TileMap(const std::vector<std::string>& plan, float tailleCase)` | Construit la carte à partir d'un plan texte ; `std::out_of_range` si le plan est vide, `std::invalid_argument` si les lignes n'ont pas toutes la même longueur |
| `Tile getTile(int x, int y) const` | Type de la case `(x, y)` ; hors de la carte = `Wall` |
| `int getLargeur() const` | Largeur de la carte, en cases |
| `int getHauteur() const` | Hauteur de la carte, en cases |
| `float getTailleCase() const` | Côté d'une case, en pixels (`coutDesObstacles` construit les rectangles des cases) ; `[[nodiscard]]` |
| `sf::Vector2i caseContenant(sf::Vector2f point) const` | Case (coordonnées de grille) contenant ce point, en pixels du monde |
| `sf::Vector2f centreDeCase(sf::Vector2i coordCase) const` | Centre de cette case, en pixels du monde |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine toutes les cases (porte fermée : case brune pleine ; porte ouverte : sol avec un cadre brun de 4 px) |
| `void placerPorte(sf::Vector2i coordCase)` | `Floor` → `PorteFermee` (`assert`) |
| `void ouvrirPorte(sf::Vector2i coordCase)` | `PorteFermee` → `PorteOuverte` (`assert`) ; une porte ouverte le reste |
| `bool chevaucheUnMur(const sf::FloatRect& rect) const` | Le rectangle (en pixels du monde) chevauche-t-il une case bloquante (mur ou porte fermée) ? |
| `bool trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const` | Le rectangle, translaté de sa position jusqu'à `position + deplacement`, ne chevauche-t-il un mur à aucun moment du trajet ? |

### `Niveau` — les données d'un niveau

`Niveau.hpp` / `.cpp` : structure sans comportement et une fonction libre.

Données membres : `struct Niveau { std::vector<std::string> plan; float
tailleCase; sf::Vector2f departEscouade; std::vector<sf::Vector2f>
monstres; int armureMonstres; }`, où `monstres` est un `std::vector<MonstreDuNiveau>` (`struct MonstreDuNiveau { sf::Vector2f position; int groupe; }`, groupe : identifiant opaque, même valeur = « vont ensemble »), et `struct PorteDuNiveau { std::vector<sf::Vector2i> cases; }` (cases du plan, du sol, qu'une porte ferme ; `Niveau::portes`, vide par défaut, vide dans `creerNiveau()`), et la constante
`s_armureMonstresParDefaut` (20), partagée par `creerNiveau()` et
`genererNiveau()`.

| Fonction | Rôle |
|---|---|
| `Niveau creerNiveau()` | Le niveau des tests (carte de 59 × 40 cases de 40 px, départ en `(100, 100)`, six monstres d'armure `s_armureMonstresParDefaut`) ; pas de lecture de fichier |

### `GenerateurDonjon` — la carte générée

`GenerateurDonjon.hpp` / `.cpp` : logique pure (sans SFML), fonctions libres
sans état, plus deux structures. Le générateur aléatoire est reçu en
paramètre (`Generateur&`).

`struct Salle { int x, y, largeur, hauteur; }` : l'emprise d'une salle, en
cases. `struct ParametresDonjon` : `largeurCarte` 59, `hauteurCarte` 40,
`tailleCase` 40 px, `tentatives` 30, `coteMin` 6, `coteMax` 12, `marge` 1,
`largeurCouloir` 2, `monstresParSalleMin` 1, `monstresParSalleMax` 3,
`distanceMinDepart` 400 px, `tentativesParMonstre` 10. `struct Liaison { std::size_t de, vers; }` (indices de
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
| `std::vector<MonstreDuNiveau> placerMonstres(Generateur&, const std::vector<Salle>&, sf::Vector2f depart, const ParametresDonjon&)` | Groupe : compteur qui augmente de 1 pour chaque salle qui reçoit au moins un monstre, partagé par les monstres de la salle. Aucun monstre dans la salle 0 ; dans chaque autre, un nombre tiré dans [min, max] de monstres, au centre d'une case intérieure (une case de marge). Rejet, `tentativesParMonstre` essais, monstre abandonné sinon : position à moins de 2 cases (Tchebychev) d'un monstre placé, ou à moins de `distanceMinDepart` px de `depart` |
| `std::vector<PorteDuNiveau> placerPortes(const std::vector<std::string>& plan, const std::vector<Salle>&, int largeurCouloir)` | Sans aléatoire, sur le plan creusé. Pour chaque salle, l'anneau de cases juste à l'extérieur de l'emprise, côté par côté, sans les coins : une porte est une suite maximale de cases de sol qui fait exactement `largeurCouloir` cases, dont les deux cases qui la prolongent (coins compris) sont des murs et dont les cases au-delà de l'anneau sont du sol. Autre largeur : pas de porte. Doublons supprimés |
| `bool contientUnCoinDiagonal(const std::vector<std::string>& plan)` | Le plan contient-il un damier : un carré de 2 × 2 cases (bords compris) dont deux cases en diagonale sont du sol et les deux autres des murs, dans l'une ou l'autre orientation ? Pure, sans aléatoire ; `genererNiveau` ne l'appelle pas, seul un test de propriété (graines 1 à 50) la lit ; `[[nodiscard]]` |
| `Niveau genererNiveau(Generateur&, const ParametresDonjon&)` | Plan tout en mur, salles creusées, puis un couloir par liaison (sens tiré au hasard, un tirage par liaison) entre les centres ; `portes` de `placerPortes` (après les couloirs, sans tirage) ; départ de l'escouade au centre en pixels de la première salle, monstres de `placerMonstres` (appelée en dernier), `armureMonstres` = `s_armureMonstresParDefaut` |

### `Partie` — les règles de jeu

Possède par valeur la carte, l'escouade et les monstres. Orchestre
seulement : les règles propres à `Escouade` ou à `Monstre` y restent. Le
joueur agit par des demandes, mémorisées puis exécutées par `mettreAJour` à
leur place dans l'image ; l'ordre est décrit dans `docs/boucle-et-entrees.md`.
Invariant : entre deux appels, tout monstre du vecteur est vivant.
Une partie perdue ou gagnée est figée. Elle se recommence en la remplaçant par une
`Partie` neuve (réaffectation par déplacement : règle du zéro).

Données membres : `m_carte` (`TileMap`), `m_escouade` (`Escouade`),
`m_monstres` (`std::vector<Monstre>`), `m_objets` (`std::vector<ObjetInteractif>`), `m_interactionEnCours` (`std::optional<int>`, identifiant de l'objet que l'escouade va activer), `m_lancerDeZoneDemande`
(`std::optional<LancerDeZone>`, structure privée : `adresse`, `point`),
`m_skillDemande` (`std::optional<AdresseSkill>`), `m_deplacementOrdonne`
(`std::optional<sf::Vector2f>`), `m_generateur` (`Generateur`, le seul de
la partie : il descend en paramètre `Generateur&` jusqu'à `Skill`, que
personne ne copie ni ne garde), `m_vision` (`Vision`, mise à jour à la fin du
constructeur et de chaque `mettreAJour`, voir `docs/vision.md`).

Deux listes d'`EntiteVue` sont recalculées à chaque usage (fonctions privées, rien de
stocké) : `entitesCiblables()` (les monstres pour lesquels `estCiblable()` est vrai) pour
tout ce qui choisit un monstre à frapper, provoquer, viser ou verrouiller ; `entitesPhysiques()`
(tous) pour les obstacles bloquants et à contourner. Un monstre qui rentre à son poste
bloque donc comme avant mais n'est plus une cible, et le verrou posé sur lui est levé.

| Fonction | Rôle |
|---|---|
| `Partie(const Niveau& niveau, std::vector<DefinitionPersonnage> composition, Generateur::result_type graine)` | Construit la carte, l'escouade (avec cette composition : elle n'appartient pas au niveau) et les monstres (identifiants 1, 2, 3…, armure `niveau.armureMonstres`), ferme chaque case de `niveau.portes` (`placerPorte`) et crée un `ObjetInteractif` par porte (identifiants 1, 2, 3…, compteur distinct de celui des monstres) et initialise le générateur avec `graine` (`EcranJeu` la tire avec `std::random_device` et l'affiche ; les tests passent une graine fixe) |
| `ResultatClic cliquer(sf::Vector2f point)` | Appliqué tout de suite. `Monstre` : verrouille le monstre ciblable et **visible** (`m_vision.estVisible` de son centre) qui contient le point (un monstre qui rentre à son poste ou hors de vue compte comme le sol) ; `Objet` : un point dans la zone d'une porte fermée dont une case au moins n'est pas `Inconnue` (et pas sur un monstre cliquable) lève le verrou et annule les coups en exécution ; si un personnage vivant est déjà à portée (`unVivantPresDe`), la porte s'ouvre tout de suite (`ouvrir`), l'ordre de déplacement en attente est effacé, aucune interaction n'est mémorisée et la formation regarde la porte (`regarderVers`) ; sinon mémorise l'interaction et l'ordre d'aller au `pointDApproche` (`mettreAJour` appelle `regarderVers` juste après l'`allerVers` de cet ordre) ; sans point d'approche, c'est `Sol`. `Sol` : lève le verrou et annule les coups en exécution des personnages ; `Ignore` si l'escouade est vaincue, sans rien changer. Tout clic (sauf `Ignore`) annule l'interaction en cours. `enum class ResultatClic { Monstre, Objet, Sol, Ignore }` |
| `void demanderSkill(AdresseSkill adresse)` | Mémorise la demande de skill ; la dernière demande de l'image gagne |
| `void lancerSkillDeZone(AdresseSkill adresse, sf::Vector2f point)` | Mémorise le lancer du skill de zone |
| `void ordonnerDeplacement(sf::Vector2f point)` | Mémorise l'ordre de déplacement ; exécuté seulement si l'escouade est vivante ; annule l'interaction en cours |
| `bool estPerdue() const` | Vrai quand tous les personnages sont morts (`!m_escouade.estVivante()`) ; `[[nodiscard]]` |
| `bool estGagnee() const` | Vrai quand `m_monstres` est vide ET que la partie n'est pas perdue (la défaite l'emporte) ; `[[nodiscard]]` |
| `int monstresRestants() const` | Nombre de monstres vivants (`m_monstres.size()`), pour le compteur d'`EcranJeu` ; `[[nodiscard]]` |
| `void mettreAJour(float dt)` | Sans effet si la partie est perdue ou gagnée (le monde est figé). Sinon exécute, dans l'ordre : lancer de zone, skill demandé, `attaquer(dt, …)` et application des coups, déverrouillage si la cible est morte, retrait des monstres morts, ordre de déplacement, mise à jour (`EntiteVue` de tous les monstres, **photo des deux camps** pour les zones de contrôle, escouade, puis ouverture de la porte en cours d'interaction si `unVivantPresDe(zone, 20 px)`, tour de chaque monstre avec ses obstacles bloquants — personnages vivants et autres monstres — et ses cercles à contourner — les autres monstres seulement —, avec l'engagement et le départ de son coup, puis **seconde photo** et coups gratuits des deux sens : `coupsDOpportunite` ; dégâts de base de l'attaquant, `Escouade::recevoirCoup` côté personnage, `Monstre::recevoirDegats` côté monstre ; monstres tués retirés, verrou levé) ; vide les demandes |
| `void dessiner(sf::RenderTarget& rendu, bool afficherAncre, bool marquerLeRetour, bool brouillard) const` | Carte, monstres (contour jaune sur le monstre verrouillé ; `marquerLeRetour` est transmis à `Monstre::dessiner` ; un monstre non visible n'est pas dessiné si `brouillard`), escouade (`afficherAncre` est transmis à `Escouade::dessiner`), puis le voile de la `Vision` si `brouillard` |
| `void dessinerCheminsDesMonstres(sf::RenderTarget& rendu) const` | Débogage (F3) : le chemin restant de chaque monstre (`Monstre::dessinerChemin`) |
| `void dessinerZonesDeControle(sf::RenderTarget& rendu) const` | Débogage (F3) : un cercle fin magenta (rayon de l'entité + zone) autour de chaque personnage vivant et de chaque monstre qui ne rentre pas à son poste, qui a une zone > 0 |
| `const Escouade& getEscouade() const` | Lecture seule : caméra, barre de skills, requêtes du `Controleur` |
| `const std::vector<Monstre>& getMonstres() const` | Lecture seule (utilisée par les tests) |

### `Controleur` — l'état d'interface

Ne connaît pas les événements SFML : `EcranJeu` les traduit en appels
simples. Points en pixels du monde.

Type valeur, constructible par défaut et réaffectable (règle du zéro) : aucune
référence membre (C++ Core Guidelines C.12), la `Partie` est reçue à chaque
appel (`const Partie&` si la fonction ne fait que lire).

Données membres : `m_deplacementContinu` (`bool`), `m_adresseEnVisee`
(`std::optional<AdresseSkill>`, vide = mode normal).

| Fonction | Rôle |
|---|---|
| `Controleur()` (par défaut) | Mode normal, déplacement continu désarmé ; `controleur = Controleur{}` remet l'état d'interface à zéro |
| `void debutImage(const Partie& partie)` | Sort de la visée si le lanceur ne peut plus lancer (`!skillEstPret`) |
| `void appuiGauche(Partie& partie, sf::Vector2f point)` | En visée : `lancerSkillDeZone` puis sortie de la visée (sans verrouiller ni déplacer). Sinon `cliquer` : `Monstre` et `Objet` désarment le déplacement continu, `Sol` l'arme, `Ignore` ne change rien |
| `void appuiDroit()` / `void appuiEchap()` | Annulent la visée sans recharge |
| `void activerEmplacement(Partie& partie, const BarreDeRaccourcis& barre, std::size_t emplacement)` | Une seule action pour la touche et le clic. Emplacement vide : rien. Skill de type `Zone` (`typeDuSkill`) : sort de la visée si cette même adresse y est ; sinon entre en visée de cette adresse (remplaçant la visée d'un autre skill de zone) si l'escouade est vivante et le skill prêt, et désarme le déplacement continu. Autre type : `demanderSkill(adresse)`, la visée en cours continue. `assert(emplacement < barre.taille())` |
| `void appuiInterface(Partie& partie, const BarreDeRaccourcis& barre, ClicInterface clic)` | Clic gauche consommé par l'interface : désarme le déplacement continu (un appui qui commence sur la barre ne le lance pas, et ne laisse pas un ancien clic au sol le relancer), puis `activerEmplacement` pour la barre `Raccourcis`. Jamais de verrou, de porte, de déplacement ni de point d'impact de visée |
| `void mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur)` | Si `boutonTenu` (bouton gauche enfoncé ET focus, calculé par `EcranJeu`) et déplacement armé : `ordonnerDeplacement(curseur)` |
| `void dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const` | En visée seulement : cercle jaune de la zone sous le curseur, cercle blanc de la portée autour du lanceur |
| `std::optional<AdresseSkill> getAdresseEnVisee() const` | Adresse en visée (`EcranJeu` en déduit l'icône à entourer de jaune) |

Le `Controleur` ne garde aucune référence à la barre : `EcranJeu` la lui passe à chaque appel, comme la `Partie`.

### `Escouade` — le point de contrôle (l'ancre)

Un carré vert de 20 × 20 (l'ancre) qui suit le chemin calculé par le A*
jusqu'au point cliqué, et qui possède 4 `Personnage` en formation en carré
autour d'elle. Elle porte le verrou de combat et fait attaquer ses
personnages. Elle ne connaît pas `Monstre`.

Données membres : `m_mobile` (`Mobile`), `m_decalages` (N
`sf::Vector2f`, calculés par `decalagesDeFormation`), `m_orientation`, `m_orientationVoulue` (`sf::Angle`),
`m_pointARegarder` (`std::optional<sf::Vector2f>`, voir `regarderVers`), `m_vitesseRotation` (`float`), `m_personnages`
(`std::vector<Personnage>`), `m_cibleVerrouillee` (`std::optional<int>`),
`m_skillEnAttente` (`std::optional<SkillEnAttente>`, structure privée :
`AdresseSkill adresse` du skill, `Cible cible`, qui peut être un `CiblePoint`), `m_cibleCombat`
(`std::optional<Cercle>`), `m_places` (`std::vector<sf::Vector2f>`, place
de formation de chaque personnage après repli et écartement, recalculée par
`mettreAJour`), `m_degagements` (`std::vector<std::optional<int>>`, par personnage : identifiant du monstre dont il se dégage vers sa place d'attaque, vide = il ne se dégage pas), `m_pointsVises` (`std::vector<sf::Vector2f>`, point vers lequel chaque
personnage marche, calculé par `mettreAJour`), `m_placesDAttaque`
(`std::vector<std::optional<PlaceDAttaque>>`, structure privée `{ int idMonstre; float rayon; sf::Angle angle; }` : place d'attaque
retenue autour du monstre que le personnage approche), `m_monstreAPortee` (`std::vector<bool>`, par personnage :
monstre à portée de son attaque et en vue, calculé par `attaquer`, lu par `mettreAJour`), `m_agresseurs`
(`Agresseurs`, monstres qui ont frappé l'escouade récemment),
`m_provocations` (`Provocations`, monstres provoqués et leur provocateur).

| Fonction | Rôle |
|---|---|
| `Escouade(sf::Vector2f positionDepart, std::vector<DefinitionPersonnage> composition)` | Crée l'escouade et un personnage par définition, aux places 1 à N, en formation (décalages de `decalagesDeFormation`) ; `assert(!composition.empty())`. `Escouade` ne choisit aucune valeur |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Ordre du joueur : annule les coups en exécution (`annulerAttaquesEngagees`), puis délègue à `m_mobile.allerVers` |
| `void regarderVers(sf::Vector2f point)` | Mémorise `m_pointARegarder` : une fois l'ancre arrêtée, la formation se tourne vers ce point (360°/s). Pendant la marche, l'orientation suit la marche. Effacé par `allerVers`, par l'approche du verrou (`attaquer`) et par `verrouiller`. `Escouade` ne sait pas que c'est une porte |
| `void annulerAttaquesEngagees()` | Annule le coup en exécution de chaque personnage, sans recharge consommée ; appelée par `allerVers`, `verrouiller` (autre monstre) et `Partie::cliquer` (sol) |
| `std::vector<Coup> attaquer(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur)` | Avec un verrou dont le monstre est dans `monstres` : mémorise son cercle et approche tant qu'un personnage vivant n'atteint pas la cible (à portée ET en vue : `peutAtteindre`). Fait entrer en activation le skill de zone en attente dès que son lanceur vivant peut lancer au point, et le skill de dégâts en attente dès que son lanceur atteint la cible. Puis fait avancer l'activation de chaque skill engagé (`avancerSkills`) : l'effet part à la fin de l'activation, évalué à ce moment ; ses coups s'ajoutent à la liste. Verrou ou non : chaque personnage choisit pour lui-même le monstre à frapper parmi ceux qu'il atteint (`choisirCibleAFrapper` avec `peutAtteindre`) ; s'il est immobile, sans action engagée et son attaque prête, il **engage** un coup ; le coup engagé avance de `dt` et part à la fin de l'exécution (0,15 s), même si le monstre a bougé. Il n'est annulé que par la mort de son auteur ou la disparition de sa cible (et par un ordre du joueur ou un skill, voir ci-dessus). Renvoie un `Coup` par coup parti |
| `std::vector<Coup> declencherSkill(AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur)` | Skill à cette adresse (place du personnage, de 1 à N, et identifiant du skill). Liste vide si l'adresse est invalide (place invalide ou skill inconnu de ce personnage), si le skill est de type `Zone` (il se lance par `declencherSkillDeZone`), si le lanceur est mort, si le skill est en recharge ou en activation, si un skill est déjà en attente ou si `choisirCible` ne trouve rien. Cible atteinte (à portée ET en vue), ou provocation (sans cible) : l'**activation commence** (`Personnage::engagerSkill`) ; l'effet part à la fin (temps d'activation du skill) dans `attaquer`, ou tout de suite, dans la liste renvoyée, si l'activation est nulle. Provocation : au départ, tous les monstres de `monstres` touchés par sa zone (cercle centré sur le lanceur) ET en vue du lanceur sont provoqués, même s'il n'y en a aucun (la recharge est consommée). Dégâts : un coup (identifiant de la cible, dégâts) ; soin : appliqué sur place à l'allié engagé (liste vide). Non atteinte (seulement avec un verrou pour les dégâts) : le skill est mis en attente (liste vide) |
| `std::optional<EtatSkill> etatDuSkill(AdresseSkill adresse) const` | Skill à cette adresse tel que l'affichage en a besoin (`EtatSkill`) ; vide si l'adresse est invalide |
| `std::optional<TypeSkill> typeDuSkill(AdresseSkill adresse) const` | Type du skill à cette adresse ; vide si l'adresse est invalide ; `[[nodiscard]]` |
| `std::vector<AdresseSkill> adressesDesSkills() const` | Adresses de tous les skills : par place croissante, puis dans l'ordre de la liste de chaque personnage ; donnée à `barreParDefaut` ; `[[nodiscard]]` |
| `std::optional<sf::Color> couleurDuPersonnage(std::size_t place) const` | Couleur du personnage de cette place (bande de l'icône) ; vide si la place est invalide ; `[[nodiscard]]` |
| `bool skillEstPret(AdresseSkill adresse) const` | Le personnage de cette adresse est-il vivant, avec ce skill dont la recharge est écoulée ? Faux si l'adresse est invalide |
| `float rayonZoneDuSkill(AdresseSkill adresse) const` | Rayon de zone du skill à cette adresse ; 0 si l'adresse est invalide |
| `Cercle cercleDePorteeDuSkill(AdresseSkill adresse) const` | Cercle centré sur le lanceur, de rayon (rayon du personnage + portée du skill) ; rayon 0 si la place est invalide |
| `std::vector<Coup> declencherSkillDeZone(AdresseSkill adresse, sf::Vector2f point, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur)` | Lance le skill de type `Zone` à cette adresse sur un cercle centré sur `point` (rayon de zone du skill). Point à portée (bord à bord, comme un cercle de rayon 0) et en vue du lanceur : l'activation commence tout de suite ; au départ, un `Coup` (identifiant, dégâts du skill) par monstre touché (bord à bord 0 ET en vue du point d'impact : `ligneDeVue`, en cases), zone évaluée à ce moment. Le sort lance la recharge même si la zone est vide. Hors portée ou pas en vue : le skill est mis en attente (`CiblePoint`), liste vide, sans recharge ; le lanceur marche vers le point (`mettreAJour`) et son activation commence dans `attaquer` dès que portée et vue sont réunies. Liste vide, sans recharge ni attente, si le point est dans un mur, si l'adresse est invalide, si son skill n'est pas de type `Zone`, si le lanceur est mort ou en recharge ou en activation, ou si un skill est déjà en attente |
| `std::optional<Cible> choisirCible(TypeSkill type, AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte) const` | `Degats` : avec un verrou, le monstre verrouillé trouvé dans `monstres` (atteint ou non, vide s'il n'y est pas) ; sans verrou, le monstre que le lanceur de l'adresse frapperait en autodéfense, avec la portée de son skill et en vue (vide si aucun). `Soin` : le vivant qui a le moins de points de vie parmi ceux qui ne sont pas à pleine vie (égalité : la plus petite place) |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<EntiteVue>& monstres, const std::vector<EntiteDeControle>& monstresDeControle = {})` | Avance l'ancre, tourne la formation, calcule la place de chaque personnage (place théorique ramenée vers l'ancre par `replierVersOrigine` si elle est dans un mur ou cachée derrière un coin, puis écartée à 34 px des places déjà calculées et sortie des monstres — une place dont le centre est à moins de (rayon du monstre + rayon du personnage) d'un monstre est invalide —, voir `docs/formation.md`), met à jour les dégagements (`mettreAJourDegagements` : une fois par image, d'après les positions du début de l'image, avec `monstresDeControle` — les monstres ciblables de la photo de `Partie`, avec leur zone) ; déplace chaque personnage vers son point visé (`pointVoulu`, mémorisé dans `m_pointsVises`) : sa place de formation ; s'il se dégage, sa place d'attaque autour du monstre dont il se dégage, sans s'arrêter à portée ; en combat verrouillé, sur place s'il atteint la cible (à portée et en vue), sinon vers sa place d'attaque autour du monstre ; ancre à l'arrêt : vers la place d'attaque autour de l'agresseur le plus proche dans une laisse de 100 px autour de sa place, ou sur place s'il a déjà un monstre à portée et en vue ; puis sépare les personnages trop proches (`separerPersonnages`, 34 px, poussée plafonnée à 120 px/s, sans obstacle dur) ; fait écouler le délai d'oubli des agresseurs ; le lanceur d'un skill en attente vise sa cible à la place (un soin entre ici en activation à l'arrivée, annulé si le lanceur ou la cible meurt ou si la cible est à pleine vie ; un personnage en exécution ou en activation reste sur place) ; les cercles des `monstres` sont les obstacles bloquants des personnages, jamais de l'ancre ; chaque personnage reçoit en plus `monstresAContourner` (privée) : les cercles de tous les monstres sauf celui qu'il approche (celui de sa place d'attaque, ou le verrou en combat), tous s'il n'en approche aucun |
| `void dessiner(sf::RenderTarget& rendu, bool afficherAncre) const` | Dessine l'ancre (si `afficherAncre`) et les N personnages |
| `std::optional<int> degagementDuPersonnage(std::size_t place) const` | Identifiant du monstre dont le personnage de cette place se dégage ; vide s'il ne se dégage pas ou si la place est invalide (lu par les tests) ; `[[nodiscard]]` |
| `std::optional<sf::Vector2f> pointVise(std::size_t place) const` | Point vers lequel le personnage de cette place (1 à N) marche, tel que le dernier `mettreAJour` l'a calculé ; vide si la place est invalide |
| `void dessinerPointsVises(sf::RenderTarget& rendu) const` | Débogage (F3) : un trait jaune du centre de chaque personnage vivant à son point visé, un petit cercle à ce point, et le chemin restant en cyan (`dessinerTraceDeChemin`) |
| `sf::Vector2f getCentre() const` | Centre de l'ancre (utilisé par la caméra) |
| `std::optional<EntiteVue> proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const` | Proie : le provocateur si ce monstre est provoqué et qu'il est vivant ; sinon le personnage vivant le plus proche de `positionMonstre` ; vide si aucun n'est vivant. `id` = place (1 à 4), `cercle` = celui du personnage |
| `std::vector<EntiteVue> candidatsDeFrappe(int idMonstre) const` | Personnages que ce monstre peut frapper (`id` = place) : seulement son provocateur s'il est provoqué et vivant (provocation stricte), sinon tous les personnages vivants ; vide si aucun n'est vivant |
| `bool placeEstVivante(std::size_t place) const` | Place valide ET personnage vivant |
| `void recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite)` | Applique les dégâts au personnage que `idMonstre` vise (même règle que `proieDuMonstre`) ; ne fait rien si aucun n'est vivant. Délègue à `recevoirCoup` |
| `void recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite)` | Applique les dégâts au personnage de cette place (le coup engagé d'un monstre) ; sans effet s'il est mort. Si `quantite > 0`, enregistre le monstre comme agresseur (ou remet son temps à 3 s) |
| `bool estVivante() const` | Vrai si au moins un personnage est vivant |
| `bool unVivantPresDe(sf::FloatRect zone, float ecart) const` | Un personnage vivant est-il à `ecart` ou moins (bord à bord, `distanceCercleRectangle`) de cette zone ? Escouade ne connaît ni les portes ni les objets ; `[[nodiscard]]` |
| `std::vector<Cercle> cerclesDesVivants() const` | Cercles des personnages vivants |
| `std::vector<EntiteDeControle> entitesDeControle() const` | Une `EntiteDeControle` (id = place de 1 à N, cercle, zone de contrôle) par personnage vivant ; les morts n'engagent personne et ne sont pas engagés ; `Partie` en fait une des deux photos de chaque image ; `[[nodiscard]]` |
| `std::optional<int> puissanceDeLAttaqueDeBase(std::size_t place) const` | Dégâts de l'attaque de base de ce personnage (avant l'armure de la cible), pour son coup gratuit ; vide si la place est invalide ; `[[nodiscard]]` |
| `std::optional<int> pointsDeVieDuPersonnage(std::size_t place) const` | Points de vie du personnage de cette place ; vide si la place est invalide (lu par les tests) ; `[[nodiscard]]` |
| `std::vector<Cercle> cerclesDeVision() const` | Un `Cercle` (centre du personnage, rayon de vision, 320 px) par personnage vivant ; `Partie` les donne à `Vision::mettreAJour` ; `[[nodiscard]]` |
| `void verrouiller(int idCible)` | Pose le verrou de combat sur ce monstre, en remplaçant la cible précédente ; annule un skill de dégâts en attente si l'identifiant change |
| `void deverrouiller()` | Retire le verrou de combat et annule un skill de dégâts en attente |
| `std::optional<int> getCibleVerrouillee() const` | Identifiant du monstre engagé en combat, vide si aucun |
| `const std::vector<sf::Vector2f>& getPlacesDeFormation() const` | Places de formation (après repli) calculées par le dernier `mettreAJour` |
| `void provoquer(int idMonstre, std::size_t place, float duree)` | Enregistre ce monstre comme provoqué par le personnage de la place `place` (1 à N) pour `duree` secondes, ou remet sa durée au maximum s'il l'est déjà. Fin : durée écoulée ou provocateur mort (`mettreAJour`), monstre absent de `monstres` (`attaquer`) |
| `const std::vector<Provocation>& getProvocations() const` | Monstres actuellement provoqués |
| `const std::vector<Agresseur>& getAgresseurs() const` | Monstres qui ont frappé l'escouade récemment (oubliés après 3 s sans coup, ou absents de `monstres` dans `attaquer`) |

### `ObjetInteractif` — les objets avec lesquels on interagit

`ObjetInteractif.hpp` / `.cpp` : des données et une fonction libre, sans état. Pas de classe de base virtuelle : l'effet d'une interaction a besoin du contexte de la `Partie`, qui visite `nature` avec `std::visit` et `Surcharges` (`Cible.hpp`). Voir `docs/carte-et-collision.md`.

`struct Porte { std::vector<sf::Vector2i> cases; }` ; `struct ObjetInteractif { int id; sf::FloatRect zone; std::variant<Porte> nature; }`.

| Fonction | Rôle |
|---|---|
| `ObjetInteractif(int identifiant, Porte porte, float tailleCase)` | Calcule `zone`, rectangle en pixels du monde qui couvre toutes les cases ; `assert` : au moins une case, toutes sur une même ligne ou une même colonne |
| `std::optional<sf::Vector2f> pointDApproche(const Porte&, const TileMap&, sf::Vector2f depuis)` | Porte horizontale (cases de même y) : côtés y−1 et y+1 ; verticale : x−1 et x+1. Le point d'un côté est le milieu des centres de ses cases ; seuls les côtés sans case bloquante sont gardés, le plus proche de `depuis` l'emporte ; vide si aucun. Une porte d'une seule case est traitée comme horizontale ; `[[nodiscard]]` |

### `ChampDeVision` — les cases qu'un observateur voit

`ChampDeVision.hpp` / `.cpp` : trois fonctions libres pures, sans état, sans SFML graphique. Shadowcasting symétrique (Albert Ford), pentes en fractions d'entiers. Un seul balayage (classe `Balayage`, patron à visiteur) sert `champDeVision`, `estVisibleDepuis` et `ligneDeVue`. Utilisé par `Vision::mettreAJour` et, via `ligneDeVue`, par toutes les règles « en vue » : `Escouade::peutAtteindre`, provocation, skill de zone, places d'attaque, `Monstre::peutAtteindre` et détection des monstres. Voir `docs/vision.md`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> champDeVision(const TileMap& carte, sf::Vector2i origine, int rayon)` | Cases vues depuis la case `origine`, à distance (entre centres) `<= rayon` cases. Une case bloquante (`estBloquante`) qui reçoit la lumière est révélée et l'arrête ; une case libre n'est révélée que si son centre est dans l'intervalle de pentes (symétrie : A voit B ⇔ B voit A pour deux cases libres). Hors carte = mur, jamais renvoyé ; l'origine est renvoyée si elle est dans la carte ; doublons permis ; `[[nodiscard]]` |
| `bool estVisibleDepuis(const TileMap& carte, sf::Vector2i origine, sf::Vector2i cible)` | La case `cible` serait-elle révélée par `champDeVision` depuis `origine` avec un rayon assez grand ? Même critère, mais ne balaie que les quadrants qui contiennent la cible (une cible sur une diagonale est dans deux quadrants : vrai si l'un la révèle), jusqu'à sa rangée, et s'arrête dès qu'elle est révélée. Pas de limite de rayon (la portée est vérifiée ailleurs). `origine == cible` : vrai ; cible hors carte : faux ; symétrique pour deux cases libres ; `[[nodiscard]]` |
| `bool ligneDeVue(const TileMap& carte, sf::Vector2f a, sf::Vector2f b)` | Le point `a` voit-il le point `b` (pixels du monde) ? Faux si la case de `a` ou celle de `b` est bloquante (mur, porte fermée, hors carte) ; sinon `estVisibleDepuis` entre leurs cases. Se décide en cases : deux points d'une même case se voient toujours ; symétrique ; `[[nodiscard]]` |

### `Vision` — ce que le joueur sait de la carte

`Vision.hpp` / `.cpp`. Possédée par valeur par `Partie`. Ne connaît ni `Personnage`, ni `Escouade`, ni `Monstre` : elle reçoit des `Cercle`. `enum class EtatCase { Inconnue, Exploree, Visible }`.

Données membres : `m_largeur`, `m_hauteur`, `m_tailleCase` (copiées de la carte), `m_cases` (`std::vector<EtatCase>`, même indexage que `TileMap`).

| Fonction | Rôle |
|---|---|
| `explicit Vision(const TileMap& carte)` | Tout est `Inconnue` |
| `void mettreAJour(const TileMap& carte, const std::vector<Cercle>& sources)` | Tout `Visible` devient `Exploree`, puis pour chaque source (centre et rayon en pixels) le `champDeVision` depuis la case du centre, rayon en cases = rayon / taille de case arrondi vers le bas, devient `Visible`. Une case explorée ne redevient jamais inconnue ; `assert` : mêmes dimensions que la carte de la construction |
| `EtatCase etat(sf::Vector2i coordCase) const` | Hors carte = `Inconnue` ; `[[nodiscard]]` |
| `bool estVisible(sf::Vector2f point) const` | La case qui contient ce point (pixels du monde) est-elle `Visible` ? ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget& rendu) const` | Un seul `sf::VertexArray` (`Triangles`, 6 sommets par case voilée), reconstruit à chaque appel : noir opaque sur l'inconnu, noir d'alpha 150 sur l'exploré, rien sur le visible ; à dessiner avec la vue du monde |

### `Provocations` — les monstres provoqués

Possédée par valeur par `Escouade`. Données privées : `m_provocations`
(`std::vector<Provocation>`, structure publique : `id` du monstre, `place` du
provocateur de 1 à N, `tempsRestant`).

| Fonction | Rôle |
|---|---|
| `void provoquer(int id, std::size_t place, float duree)` | Ajoute la provocation, ou remet la durée au maximum (et change le provocateur) si ce monstre l'est déjà ; pas de doublon. La validation de la place reste dans `Escouade::provoquer` |
| `void mettreAJour(float dt)` | Fait écouler le temps ; retire les provocations échues |
| `void retirerMonstresAbsents(const std::vector<EntiteVue>& monstres)` | Retire les monstres absents de la liste (morts) |
| `void retirerProvocateur(std::size_t place)` | Retire les provocations de ce provocateur (mort) |
| `std::optional<std::size_t> provocateurDe(int id) const` | Place du provocateur de ce monstre, vide s'il n'est pas provoqué |
| `const std::vector<Provocation>& liste() const` | Provocations en cours |

### `Agresseurs` — les monstres qui ont frappé l'escouade

Possédée par valeur par `Escouade`. Données privées : `m_agresseurs`
(`std::vector<Agresseur>`, structure publique : `EntiteVue monstre`,
`tempsRestant`). Mémoire de 3 s sans nouveau coup du même monstre.

| Fonction | Rôle |
|---|---|
| `void enregistrer(int id, sf::Vector2f origine)` | Ajoute l'agresseur (cercle de rayon nul centré sur l'origine du coup), ou remet son temps à 3 s et son centre à l'origine s'il est connu. La règle « coup à 0 = rien » reste dans `Escouade::recevoirDegats` |
| `void mettreAJour(float dt)` | Fait écouler le temps ; oublie les agresseurs échus |
| `void actualiser(const std::vector<EntiteVue>& monstres)` | Oublie les absents (morts) et met à jour le cercle des autres |
| `std::vector<EntiteVue> monstresVus() const` | Les agresseurs en `EntiteVue` ; `Escouade::mettreAJour` l'appelle une fois par image |
| `const std::vector<Agresseur>& liste() const` | Agresseurs retenus |

### `Mobile` — déplacement par chemin

Suit un chemin calculé par le A* puis lissé, avec collision contre les
murs et les cercles obstacles. Possédé par valeur par `Escouade`, `Personnage` et
`Monstre`. Ne connaît ni `Escouade`, ni `Personnage`, ni `Monstre`, ni
`Skill`, et ne dessine rien.

Données membres : `m_position` (coin haut-gauche), `m_taille`, `m_chemin`
(`std::vector<sf::Vector2f>`), `m_indexChemin`, `m_vitesse`, `m_enMouvement` (`bool`), `m_tempsBloque` (`float`, secondes consécutives sans avancer vraiment).

| Fonction | Rôle |
|---|---|
| `Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)` | Crée le mobile à ce centre, cette taille, cette vitesse ; immobile |
| `void allerVers(sf::Vector2f destination, const TileMap& carte, const std::vector<Cercle>& aContourner = {})` | Calcule un chemin vers la destination (A* avec `coutDesObstacles` du rayon du mobile et de `s_penaliteCaseOccupee` = 8, puis lissé en évitant `aContourner`) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe. Remet le compteur de blocage à 0 dans tous les cas |
| `void allerToutDroit(sf::Vector2f destination)` | Remplace le chemin par ce seul point, sans A* : le mobile avance en ligne droite (collision et glissement) ; remet le compteur de blocage à 0 |
| `void arreter()` | Réduit le chemin au centre actuel, pour cesser d'avancer sans reculer ; `estEnMouvement()` devient faux, le compteur de blocage est remis à 0 |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Avance vers le point courant du chemin (collision avec les murs et les cercles obstacles, glissement) |
| `void decaler(sf::Vector2f decalage, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Pousse le mobile (mêmes collisions et même glissement), sans toucher au chemin ni à `estEnMouvement()` ; utilisée par la séparation des personnages |
| `sf::Vector2f getCentre() const` | Centre du mobile |
| `sf::Vector2f getTaille() const` | Taille du mobile |
| `bool estEnMouvement() const` | Définition unique de « en mouvement » : au dernier tic, le point courant du chemin était à plus de 0,01 px, que le mobile ait pu avancer ou non (bloqué = en mouvement ; arrivé à ce tic = encore en mouvement). Faux avant le premier tic et après `arreter()` ; `[[nodiscard]]` |
| `bool estBloque() const` | Définition unique de « bloqué » : à chaque `mettreAJour`, s'il a un point à atteindre (`estEnMouvement`) et a parcouru moins de 10 % de `vitesse × dt`, un compteur augmente de `dt`, sinon il revient à 0 ; vrai à partir de 0,3 s (`s_tempsAvantBlocage`). Jamais vrai à l'arrêt ; `[[nodiscard]]`. Le recalcul du chemin est l'affaire du propriétaire |
| `std::vector<sf::Vector2f> cheminRestant() const` | Points du chemin pas encore atteints, le prochain en premier (débogage F3 et tests) ; `[[nodiscard]]` |
| `std::optional<sf::Vector2f> directionVoulue() const` | Vecteur du centre vers le point courant du chemin ; vide s'il n'y a plus de point à atteindre ou si la distance est nulle |

### `Chemin` — recherche de chemin (A*)

Fonctions libres, sans état, plus `using FonctionDeCout = std::function<int(sf::Vector2i)>` (coût **ajouté** pour entrer dans une case, en plus de 1 par pas ; fonction vide = aucun coût). `trouverChemin` raisonne en cases, les autres en pixels. `trouverChemin`, `coutDesObstacles` et `lisserChemin` sont utilisées par `Mobile::allerVers`, `traverseUnObstacle` aussi par `Personnage::mettreAJour`, `replierVersOrigine` par `Escouade::mettreAJour`. Voir `docs/chemin.md`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee, const FonctionDeCout& cout = {})` | Plus court chemin entre deux cases (4 directions, coût 1 par pas plus `cout(case)`, jamais appelée pour le départ ni l'arrivée ; heuristique de Manhattan, toujours admissible) ; inclut le départ et l'arrivée ; vecteur vide si départ/arrivée est un mur (ou hors carte) ou si l'arrivée est inaccessible |
| `FonctionDeCout coutDesObstacles(const TileMap& carte, const std::vector<Cercle>& obstacles, float rayonMobile, int penalite)` | `penalite` pour une case dont le rectangle chevauche (strictement) un obstacle agrandi de `rayonMobile` ; 0 ailleurs et hors carte. Cases occupées précalculées une fois (`std::vector<bool>`), copiées dans la lambda renvoyée (aucune référence pendante) ; fonction vide si `obstacles` est vide ; `[[nodiscard]]` |
| `bool traverseUnObstacle(sf::Vector2f a, sf::Vector2f b, const std::vector<Cercle>& obstacles, float rayonMobile)` | Le segment passe-t-il à moins de (rayon de l'obstacle + `rayonMobile`) du centre d'un obstacle ? ; `[[nodiscard]]` |
| `std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Vector2f taille, const std::vector<Cercle>& aContourner = {})` | Retire de `points` (positions du centre d'un rectangle `taille`, en pixels) les points de passage inutiles ; un raccourci est refusé s'il traverse un des `aContourner` (`traverseUnObstacle`, rayon du mobile = `taille.x / 2`) ; `depart` n'en fait pas partie dans le résultat |
| `sf::Vector2f replierVersOrigine(const TileMap& carte, sf::Vector2f origine, sf::Vector2f cible, sf::Vector2f taille)` | `cible` si le rectangle `taille` centré sur `origine` peut aller en ligne droite jusqu'à elle sans toucher un mur ; sinon le point libre le plus éloigné de `origine` sur le segment (dichotomie, 6 itérations) ; `origine` si aucun point n'est libre |

### `Personnage` — un membre de l'escouade

Un carré coloré de 20 × 20 qui va vers le point que lui donne `Escouade`,
frappe à portée, a des points de vie et peut mourir. Ne connaît ni la
formation, ni `Escouade`, ni `Monstre`.

Données membres : `m_mobile` (`Mobile`, porte la position et la vitesse),
`m_couleur`, `m_attaqueDeBase` (`Skill`), `m_skills`
(`std::vector<SkillConnu>`, sans doublon d'identifiant), `m_cibleChemin` (`std::optional<sf::Vector2f>`,
cible du dernier A*), `m_pointsDeVie` (`PointsDeVie`), `m_armure` (`int`), `m_rayonVision` (`float`, pixels), `m_zoneDeControle` (`float`, pixels, bord à bord), `m_actionEngagee` (`std::optional<ActionEngagee>`, l'action en exécution — attaque de base ou skill, avec son `IdSkill` — et sa cible ; une seule à la fois ; lue seulement pendant l'exécution).

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::vector<SkillConnu> skills, int armure, float rayonVision, float zoneDeControle = 0.0f)` | Crée le personnage à ce centre, avec cette couleur, cette attaque de base, ces skills (`assert` : pas deux fois le même `IdSkill`), cette armure, ce rayon de vision et cette zone de contrôle en pixels (tous fournis par `Composition`, via `Escouade`, qui passe la zone explicitement ; la valeur par défaut 0 évite d'écrire une zone dans les tests qui n'en parlent pas) |
| `float getRayonVision() const` | Rayon de vision, en pixels ; `[[nodiscard]]` |
| `float getZoneDeControle() const` / `int getPuissanceAttaqueDeBase() const` | Zone de contrôle (0 : il n'engage personne) et dégâts de l'attaque de base ; de simples lectures, elles ne touchent ni la recharge ni le coup engagé ; `[[nodiscard]]` |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles, const std::vector<Cercle>& aContourner)` | Si vivant, avance vers la cible (en ligne droite si le trajet est libre de murs ET ne passe pas près d'un des `aContourner` — `traverseUnObstacle` —, sinon par un chemin A* lissé qui les contourne, recalculé si la cible s'éloigne de plus d'une case de sa fin ou si `estBloque()` ; collision avec les murs et les cercles `obstacles`, glissement) puis décrémente la recharge de l'attaque et de chacun de ses skills ; sinon ne fait rien |
| `void decaler(sf::Vector2f decalage, const TileMap&, const std::vector<Cercle>&)` | Pousse le personnage vivant (délègue à `Mobile::decaler`) ; sans effet s'il est mort |
| `bool estEnMouvement() const` / `float getPorteeAttaqueDeBase() const` | Façades de `Mobile::estEnMouvement` et de la portée de l'attaque de base ; `[[nodiscard]]` |
| `std::vector<sf::Vector2f> cheminRestant() const` | Façade de `Mobile::cheminRestant` (débogage F3) ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage (contour sombre de 1 px vers l'intérieur, éclairci de 50 % pendant l'exécution d'un coup ou l'activation d'un skill) et sa barre de vie ; carré gris sans barre s'il est mort |
| `bool estAPortee(Cercle cible) const` | L'écart bord à bord entre son cercle et `cible` est-il `<=` la portée de `m_attaqueDeBase` ? |
| `bool engagerAttaque(int idCible)` | Engage un coup d'attaque de base sur ce monstre : faux s'il est mort, en mouvement (`Mobile::estEnMouvement`), si une action (attaque ou skill) est déjà engagée, ou si l'attaque n'est pas prête (`Skill::commencer`). La portée et la vue sont vérifiées par `Escouade` |
| `std::optional<int> cibleEngagee() const` | Monstre visé par le coup de base en exécution ; vide hors exécution (et pendant l'activation d'un skill) |
| `int avancerAttaque(float dt, Generateur& generateur)` | Fait avancer l'exécution ; rend les dégâts (puissance de l'attaque) au départ du coup, 0 sinon ; `[[nodiscard]]` |
| `void annulerAttaque()` | Annule le coup de base en exécution, sans recharge consommée ; sans effet sur l'activation d'un skill (`annulerSkill`) |
| `bool estEnExecution() const` | Un coup de base en exécution ou un skill en activation : le personnage reste sur place |
| `std::vector<IdSkill> idsDesSkills() const` | Identifiants de ses skills, dans l'ordre de sa liste ; `[[nodiscard]]` |
| `sf::Color getCouleur() const` | Sa couleur ; `[[nodiscard]]` |
| `std::optional<TypeSkill> typeSkill(IdSkill id) const` | Type de ce skill, vide s'il ne le connaît pas (comme les cinq suivantes : un identifiant inconnu = pas de skill) |
| `bool skillEstPret(IdSkill id) const` | Vivant, avec ce skill en phase prête (recharge écoulée, pas en activation) |
| `std::optional<EtatSkill> etatDuSkill(IdSkill id) const` | Type, progression de la recharge, temps restant, état de vie et activation en cours (vraie seulement si c'est ce skill qui est engagé) ; vide sans ce skill |
| `bool skillEstAPortee(IdSkill id, Cercle cible) const` | Ce skill est-il à portée de `cible` (bord à bord) ? Faux sans ce skill |
| `Cercle zoneDuSkill(IdSkill id) const` | Cercle centré sur le personnage, de rayon le rayon de zone de ce skill (0 sans ce skill) |
| `Cercle cercleDePorteeDuSkill(IdSkill id) const` | Cercle centré sur le personnage, de rayon son rayon + la portée de ce skill (rayon seul sans ce skill) |
| `bool engagerSkill(IdSkill id, std::optional<Cible> cible)` | Engage ce skill : faux s'il est mort, sans ce skill, si un skill (même un autre) est déjà en activation, ou si ce skill n'est pas prêt (`Skill::commencer`). Ne demande pas d'immobilité. Annule le coup de base en exécution. La portée et la vue sont vérifiées par `Escouade` ; la cible (vide pour la provocation) est stockée sans être interprétée |
| `std::optional<ActionEngagee> skillEngage() const` | L'action `Skill` en activation, avec son `IdSkill` et sa cible ; vide sinon |
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
| `sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement, const std::vector<Cercle>& obstacles)` | Essaie le déplacement axe par axe (X puis Y) ; un essai est refusé s'il chevauche un mur ou si `deplacementBloque` est vrai. Si ni X ni Y n'ont passé et qu'un cercle a refusé, essaie un déplacement de même longueur le long de la tangente du cercle refusant le plus proche (côté de la projection du déplacement voulu, à gauche de celui-ci si elle est nulle), accepté seulement hors mur et si `deplacementBloque` est faux. Renvoie la nouvelle position du coin haut-gauche |

### `Monstre` — un ennemi

Un carré rouge de 30 × 30 qui détecte un personnage (à moins de 200 px, dans son territoire et en vue), le poursuit sans dépendre de la vue, frappe
à portée et retourne à son poste (160 px/s ; territoire de 600 px). Au retour, il ne réagit à rien
jusqu'à son arrivée, il est invulnérable, il n'est plus une cible et il regagne ses points de vie
(`docs/monstre.md`). Possédé par `Partie`. Ne connaît ni
`Escouade` ni `Personnage` : il reçoit sa proie, ses candidats de frappe (des
`EntiteVue`) et ses obstacles en paramètres. La proie est celle qu'il poursuit ;
la cible de frappe est celle qui est à portée ET en vue.

Données membres : `m_mobile`, `m_pointsDeVie` (`PointsDeVie`), `m_armure` (`int`), `m_id`,
`m_attaque`, `m_rayonDetection`, `m_rayonTerritoire`, `m_etat` (`Etat`,
`enum class` privé : `EnGarde`, `Poursuite`, `Retour`), `m_poste`,
`m_groupe` (`int`, opaque), `m_alerte` (`bool`, posé par `alerter`,
consommé par `mettreAJour`), `m_soinAccumule` (`float`, points de vie rendus pas encore entiers pendant le retour, 0 ailleurs), `m_idCibleEngagee` (`std::optional<int>`, place du personnage visé par le coup en exécution) ; constantes publiques `static constexpr float s_rayonTerritoire` (600 px) et `s_zoneDeControle` (20 px, bord à bord).

| Fonction | Rôle |
|---|---|
| `Monstre(sf::Vector2f centre, int id, int armure, int groupe)` | Crée le monstre à ce centre, avec cet identifiant, cette armure (20, donnée par `Niveau`), ce groupe (donné par `Niveau`) et 100 points de vie ; son poste est ce centre |
| `int getGroupe() const` | Identifiant de groupe ; `[[nodiscard]]` |
| `float getZoneDeControle() const` / `int getPuissanceAttaque() const` | Zone de contrôle (`s_zoneDeControle`) et dégâts de son attaque ; de simples lectures, elles ne touchent ni la recharge ni le coup engagé ; `[[nodiscard]]` |
| `void alerter()` | Demande d'entrer en poursuite de sa proie habituelle, même hors de la détection, si le monstre est dans son territoire ; prise en compte au prochain `mettreAJour`, sans effet en poursuite ni en retour. `recevoirDegats` (dégâts `> 0`) l'appelle |
| `bool estEnPoursuite() const` | Statut « attaque » (`Etat::Poursuite`), lu par `Partie` ; `[[nodiscard]]` |
| `bool estCiblable() const` | Faux en `Retour` (le monstre n'est la cible de personne, mais reste un obstacle) ; vrai sinon ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget& rendu, bool estCible, bool marquerLeRetour) const` | Dessine le monstre (éclairci de 50 % pendant l'exécution d'un coup ; semi-transparent, alpha 128, s'il est en retour et que `marquerLeRetour`, c'est-à-dire F3) et sa barre de vie ; ajoute un contour jaune si `estCible` |
| `void dessinerChemin(sf::RenderTarget& rendu) const` | Débogage (F3) : trait rouge fin du centre du monstre à travers son chemin restant |
| `void mettreAJour(float dt, std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte, const std::vector<Cercle>& obstacles, const std::vector<Cercle>& aContourner)` | Décrémente la recharge. Pendant l'exécution d'un coup : arrête le monstre et le déplace (immobile), sans toucher à l'état. Sinon met à jour l'état (détection de la proie soumise à la ligne de vue, `proieDetectee`, seulement en `EnGarde` ; la poursuite ne dépend pas de la vue ; `Retour` ne se termine qu'à l'arrivée au poste, sans retour possible vers `Poursuite`), regagne des points de vie en `Retour` (50 % du maximum par seconde, par l'accumulateur `m_soinAccumule`) et donne ses ordres à `m_mobile` (poursuite de la proie, arrêt dès qu'une cible de frappe existe, retour au poste — le chemin du retour est aussi recalculé quand `estBloque()`), puis le déplace ; `obstacles` le bloquent, `aContourner` (les autres monstres, pas les personnages) est ce que ses chemins évitent |
| `bool peutAtteindre(Cercle cible, const TileMap& carte) const` | La cible est-elle à portée d'attaque (bord à bord) ET en vue (`ligneDeVue`, en cases) ? Même règle que `Escouade::peutAtteindre` ; `[[nodiscard]]` |
| `bool engagerAttaque(std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte)` | Engage un coup (0,3 s d'exécution) sur la cible de frappe : faux s'il n'y en a pas, si le monstre est en retour, s'il est en mouvement ou si l'attaque n'est pas prête. La place choisie est retenue jusqu'au départ |
| `std::optional<int> cibleEngagee() const` | Place du personnage visé par le coup en exécution (pas forcément la proie) ; vide hors exécution |
| `std::optional<EntiteVue> cibleDeFrappe(...) const` (privée) | `choisirCibleAFrapper` avec `peutAtteindre` comme prédicat et l'id de la proie comme préféré : la proie si atteignable, sinon le candidat atteignable le plus proche (bord à bord ; égalité : le premier) |
| `int avancerAttaque(float dt, Generateur& generateur)` | Fait avancer l'exécution ; rend les dégâts au départ du coup, 0 sinon ; `[[nodiscard]]` |
| `void annulerAttaque()` | Annule le coup en exécution, sans recharge consommée (appelée par `Partie` quand la proie engagée meurt) |
| `sf::Vector2f getCentre() const` | Centre du monstre |
| `Cercle getCercle() const` | Cercle centré sur le monstre, de rayon 15 |
| `int getId() const` | Identifiant du monstre |
| `bool contient(sf::Vector2f point) const` | Le point (pixels du monde) est-il dans le carré du monstre ? |
| `void recevoirDegats(int quantite)` | Sans aucun effet en `Retour` (invulnérable, pas d'alerte). Sinon retire `appliquerArmure(quantite, m_armure)` points de vie, sans descendre sous zéro (`PointsDeVie::subirDegats`) ; si `quantite > 0`, mémorise que le monstre vient d'être touché |
| `bool estVivant() const` | Points de vie `> 0` |
| `int getPointsDeVie() const` | Points de vie actuels |

### `Skill` — une capacité spéciale

Un `TypeSkill` (`enum class { Degats, Soin, Provocation, Zone }`), une
portée, une puissance (dégâts, points de vie rendus ou durée en secondes,
dégâts d'un skill de zone), une recharge, ainsi qu'un rayon de zone (la
taille de la zone touchée par un skill de zone ; 0, non lu, pour un skill
à cible unique). Sert aussi d'attaque de base : un `Skill` de type
`Degats`, de rayon de zone 0.
Il suit une machine à états (`enum class PhaseSkill { Pret, EnExecution, EnRecharge }`) : `commencer` (prêt → en exécution), `avancer` (fin de l'exécution : le coup part, la recharge est tirée, → en recharge), `annuler` (en exécution → prêt, sans recharge). Les skills des touches 1 à 4 passent par la même machine à états (étape 58 : activation de 0,3 s, 0,2 s, 0,6 s et 0,8 s pour dégâts, provocation, soin et zone) ; avec un temps nul, l'effet part dans l'image de l'engagement. Il ne connaît pas la cible du coup engagé, c'est son propriétaire qui la retient.
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
| `void relancerRecharge(Generateur& generateur)` | Remet la recharge au complet, sans déclencher : `min` si `min == max` (aucun tirage), sinon un tirage uniforme (`std::uniform_real_distribution<float>`) dans `[min, max]` |
| `float getProgressionRecharge() const` | De 0 (juste après le déclenchement) à 1 (prêt) ; 1 si le temps de recharge est nul |
| `float getRechargeRestante() const` | Temps de recharge restant, en secondes (0 si prêt) |
| `TypeSkill getType() const` | Type du skill |
| `float getRayonZone() const` | Rayon de zone du skill |
| `float getPortee() const` | Portée du skill (bord à bord) |

### `IdSkill`, `SkillConnu`, `AdresseSkill` — désigner un skill

`IdSkill.hpp`, `SkillConnu.hpp` et `AdresseSkill.hpp`, sans `.cpp`. Voir `docs/combat.md`.

- `enum class IdSkill { FrappePuissante, Provocation, Soin, Explosion, FrappeSacree, ImpositionDesMains, TirPercant, PluieDeFleches }` : identifiant **stable** d'un skill, indépendant de l'ordre dans la liste d'un personnage et de la touche.
- `struct SkillConnu { IdSkill id; Skill skill; }` : un skill d'un personnage avec son identifiant (`Skill` ne porte pas d'identifiant : il sert aussi d'attaque de base et d'attaque des monstres).
- `struct AdresseSkill { std::size_t place; IdSkill id; }` (comparable, `operator==` par défaut) : le personnage (place de 1 à N) et le skill. Une adresse invalide (place invalide ou skill inconnu de ce personnage) est traitée comme une place invalide : refus, liste vide, `optional` vide.

### `Composition` — de quoi est faite l'escouade

`Composition.hpp` / `.cpp` : une énumération, une structure et deux fonctions libres, sans état. `Escouade` ne choisit plus aucune valeur : elle reçoit sa composition. Voir `docs/formation.md`.

`enum class Rang { Avant, Arriere }` ; `struct DefinitionPersonnage { sf::Color couleur; Skill attaqueDeBase; std::vector<SkillConnu> skills; int armure; float rayonVision; float zoneDeControle; Rang rang; }` (`zoneDeControle` : en pixels, bord à bord, 0 = n'engage personne).

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs)` | Pure. Pour une escouade orientée vers +x : les `Avant` sur x = +25, les `Arriere` sur x = −25 ; dans un rang de n personnages, le k-ième (ordre d'apparition) est sur y = (k − (n−1)/2) × 50. Dans l'ordre de `rangs` ; `[[nodiscard]]` |

### `Specialite` — les classes du catalogue

`Specialite.hpp` / `.cpp` : des données et trois fonctions libres, sans état, **sans hiérarchie de classes** (pas de sous-classes `Guerrier`, `Mage`…). Une spécialité est une recette de départ. Seule source des valeurs de personnages (tailles de portée, armure, skills, couleurs). Voir `docs/formation.md`.

`enum class IdSpecialite { Guerrier, Pretre, Mage, Paladin, Rodeur }` ; `struct Specialite { IdSpecialite id; std::string nom; DefinitionPersonnage definition; }` (`definition.couleur` : la couleur de la classe, sans nuance).

| Fonction | Rôle |
|---|---|
| `const std::vector<Specialite>& catalogue()` | Les 5 classes, dans l'ordre de `IdSpecialite` (guerrier, prêtre, mage, paladin, rôdeur), construites une seule fois (variable `static` locale) ; `[[nodiscard]]` |
| `const Specialite& specialite(IdSpecialite id)` | L'entrée de cet identifiant (indexée par l'enum, `assert` sur l'identifiant) ; `[[nodiscard]]` |
| `std::vector<DefinitionPersonnage> composer(const std::vector<IdSpecialite>& identifiants)` | Une définition par identifiant, dans l'ordre reçu (c'est l'ordre des places). La k-ième occurrence d'une même classe (k = 0, 1, 2) a sa couleur assombrie de 0 %, 30 % puis 55 % (`assombrir`) ; `assert` : pas plus de 3 fois la même classe ; `[[nodiscard]]` |

Classes : guerrier (`Avant`, rouge, armure 50, attaque de mêlée, `FrappePuissante` et `Provocation`), prêtre (`Arriere`, jaune pâle, armure 0, attaque de tireur, `Soin`), mage (`Arriere`, violet, armure 0, attaque de tireur, `Explosion`), paladin (`Avant`, orange, armure 50, attaque de mêlée, `FrappeSacree` et `ImpositionDesMains`), rôdeur (`Arriere`, vert, armure 0, attaque de tireur, `TirPercant` et `PluieDeFleches`). Rayon de vision 320 px pour toutes. Zone de contrôle : 20 px pour le guerrier et le paladin (mêlées), 0 pour les trois autres. Valeurs des skills dans `docs/combat.md`.

### `Coup` — ce qui circule entre `main.cpp` et `Escouade`

`Coup.hpp`, sans `.cpp` : `struct Coup { int idMonstre; int degats; }` (des
dégâts à infliger à ce monstre). Sortie d'`Escouade`.

### `ActionEngagee` — l'action qu'un personnage prépare

`ActionEngagee.hpp`, sans `.cpp` : `enum class TypeAction { AttaqueDeBase, Skill }` et `struct ActionEngagee { TypeAction action; std::optional<Cible> cible; std::optional<IdSkill> skill; }` (`skill` est vide pour une attaque de base). La cible est vide pour la provocation ; pour une attaque de base, c'est une `CibleMonstre` dont le `cercle` n'est pas renseigné. Stockée par `Personnage` (`m_actionEngagee`), résolue par `Escouade`. `TypeAction` sert aussi à `Escouade::peutAtteindre` (portée de l'attaque de base ou du skill).

### `EntiteVue` — une entité telle que l'autre camp la voit

`EntiteVue.hpp` / `.cpp` : `struct EntiteVue { int id; Cercle cercle; }` (l'identifiant d'un monstre, ou la place 1 à N d'un personnage)
(entrée d'`Escouade` et d'`Autodefense`) et une fonction libre.

| Fonction | Rôle |
|---|---|
| `const EntiteVue* trouverEntite(const std::vector<EntiteVue>& entites, int id)` | Entité portant cet identifiant, `nullptr` si elle est absente (`[[nodiscard]]`) |

`main.cpp` a son équivalent pour `std::vector<Monstre>`, `trouverMonstreParId`
(namespace anonyme).

### `TypeSkill` — le type d'un skill

`TypeSkill.hpp`, sans `.cpp` : `enum class TypeSkill { Degats, Soin,
Provocation, Zone }`. Inclus par `Skill`, `EtatSkill`, `BarreDeSkills` et
`Escouade` sans qu'ils dépendent de la classe `Skill`.

### `Autodefense` — quel monstre un combattant frappe

Deux fonctions libres, sans état. `choisirCibleAFrapper` est utilisée par
`Escouade::attaquer`, `Escouade::choisirCible` et `Monstre` (cible de frappe), `choisirAgresseurAViser`
par `Escouade::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `std::optional<int> choisirCibleAFrapper(Cercle origine, const std::function<bool(Cercle)>& estAPortee, const std::vector<EntiteVue>& entites, std::optional<int> idPrefere)` | Identifiant de l'entité préférée (verrouillée, ou proie) si elle est à portée ; sinon de la plus proche (bord à bord) parmi celles à portée (égalité : la première de la liste) ; sinon vide. Même règle pour les deux camps. `estAPortee` dit si un cercle est à portée (celle de l'attaque ou celle d'un skill) |
| `std::optional<EntiteVue> choisirAgresseurAViser(Cercle personnage, sf::Vector2f place, float laisse, const std::vector<EntiteVue>& agresseurs)` | Agresseur (identifiant et cercle) le plus proche du personnage (bord à bord ; égalité : le premier de la liste) parmi ceux dont le centre est à `laisse` ou moins de `place` ; sinon vide |

### `Encerclement` — les places d'un anneau

`Encerclement.hpp` / `.cpp` : trois fonctions libres, sans état, sans dépendance à `Escouade` ni à `Personnage` (réutilisable plus tard par des monstres qui encerclent). Utilisées par `Escouade` pour les places d'attaque autour d'un monstre et les places de formation de secours autour de l'ancre. Voir `docs/formation.md`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Angle> anglesCandidats(sf::Angle depart, float rayon, float ecartMin)` | Angles des places d'un anneau : n places régulières, n le plus grand nombre tel que la **corde** entre voisines soit `>= ecartMin` (au moins 1) ; le premier angle est `depart`, puis alternance gauche/droite (0, +θ, −θ, +2θ…) |
| `sf::Vector2f positionDePlace(sf::Vector2f centre, float rayon, sf::Angle angle)` | Point de l'anneau à cet angle |
| `std::optional<sf::Angle> choisirAngleLibre(const std::vector<sf::Angle>& candidats, sf::Angle angleActuel, const std::function<bool(sf::Angle)>& estLibreEtValide)` | Le candidat accepté le plus proche de l'angle actuel (plus court côté ; égalité : le premier) ; vide si aucun |

### `Engagement` — zones de contrôle et attaque d'opportunité

`Engagement.hpp` / `.cpp` : deux structures de données, une structure de résultat et trois fonctions libres, sans état et sans SFML graphique. Ne connaît ni `Escouade`, ni `Monstre`, ni `Personnage`, ni `Partie` (sur le modèle d'`Autodefense`) : elle reçoit des cercles et des zones. Voir `docs/combat.md`.

`struct EntiteDeControle { int id; Cercle cercle; float zone; }` (id d'un monstre, ou place 1 à N d'un personnage ; zone en pixels, bord à bord ; 0 = n'engage personne). `struct PhotoDesCamps { std::vector<EntiteDeControle> attaquants; std::vector<EntiteDeControle> cibles; }` (les deux camps d'UN sens ; pour l'autre sens, on échange les deux listes). `struct CoupDOpportunite { int idAttaquant; int idCible; }`.

| Fonction | Rôle |
|---|---|
| `bool engage(const EntiteDeControle& a, const EntiteDeControle& b)` | A engage B : zone de A > 0 et `distanceBordABord` <= zone de A. Pas symétrique ; `[[nodiscard]]` |
| `std::vector<CoupDOpportunite> coupsDOpportunite(const PhotoDesCamps& avant, const PhotoDesCamps& apres)` | Coups gratuits d'un sens, d'après les photos du début et de la fin de l'image. Paire (A engage B) présente au début et rompue à la fin, B ayant fui (déplacement projeté sur la droite A→B du début : B s'est écarté au moins autant que A) : A frappe B, sauf si A engage encore quelqu'un à la fin, ou si A ou B est absent de la photo de fin. Au plus un coup par attaquant : le fuyard le plus proche à la fin (bord à bord ; égalité : le premier de `avant.cibles`). Ordre du résultat : celui de `avant.attaquants` ; `[[nodiscard]]` |
| `std::optional<int> monstreDontSeDegager(const EntiteDeControle& personnage, const std::vector<EntiteDeControle>& personnages, const std::vector<EntiteDeControle>& monstres)` | Règle de départ du dégagement (étape 78). `personnage` sans zone (> 0 : jamais), engagé par au moins un monstre, et CHAQUE monstre qui l'engage engage aussi un AUTRE personnage de `personnages` (identifiant différent ; lui-même y figure sans compter). Renvoie l'identifiant du plus proche des monstres qui l'engagent (bord à bord ; égalité : le premier) ; vide sinon. Ne connaît ni l'ancre, ni les skills, ni l'état de dégagement : c'est l'affaire d'`Escouade::degagementVoulu` ; `[[nodiscard]]` |

### `Cible` — ce que vise un skill

`Cible.hpp`, sans `.cpp` : `struct CibleAllie { std::size_t place; }`
(de 1 à N), `struct CibleMonstre { int id; Cercle cercle; }`,
`struct CiblePoint { sf::Vector2f point; }` (point fixé au clic, pour un
skill de zone en attente) et
`using Cible = std::variant<CibleAllie, CibleMonstre, CiblePoint>`. L'absence de
cible est un `std::optional<Cible>` vide. `Cible.hpp` définit aussi
`template<class... Ts> struct Surcharges : Ts... { using Ts::operator()...; }`
pour `std::visit` : une lambda par alternative, et un cas oublié est une
erreur de compilation.

### `EtatSkill` — le skill d'une place, pour l'affichage

`EtatSkill.hpp`, sans `.cpp` : `struct EtatSkill { IdSkill id; TypeSkill type; float progression; float tempsRestant; bool lanceurVivant; bool enActivation; }`. `progression` va de 0 (juste lancé) à 1 (prêt), `tempsRestant` est en secondes, `enActivation` est vrai pendant l'activation du skill. Produite par `Escouade::etatDuSkill`, consommée par `BarreDeSkills`.

### `BarreDeRaccourcis` — les emplacements de raccourcis

`BarreDeRaccourcis.hpp` / `.cpp`, sans SFML graphique. Trois notions séparées : les **emplacements** (le modèle, cette classe), les **touches** (une table touche → emplacement dans `Reglages`) et la **disposition** à l'écran (calculée par la vue seulement). L'emplacement i reste l'emplacement i quelles que soient sa touche et sa place. Voir `docs/boucle-et-entrees.md`. Constante `inline constexpr std::size_t s_emplacementsPrincipaux = 12`.

Classe `BarreDeRaccourcis` : donnée membre `m_emplacements` (`std::vector<std::optional<AdresseSkill>>`). Possédée par `EcranJeu` (la barre d'auras à venir sera une autre instance).

| Fonction | Rôle |
|---|---|
| `explicit BarreDeRaccourcis(std::size_t taille)` | Tous les emplacements sont vides |
| `std::size_t taille() const` | Nombre d'emplacements ; `[[nodiscard]]` |
| `std::optional<AdresseSkill> adresse(std::size_t emplacement) const` | Vide si l'emplacement est vide ; `assert(emplacement < taille())` ; `[[nodiscard]]` |
| `void placer(std::size_t emplacement, AdresseSkill adresse)` | Aucune limite par personnage ; une même adresse peut être placée deux fois ; `assert` |
| `void vider(std::size_t emplacement)` | `assert` |

Trois fonctions libres :

| Fonction | Rôle |
|---|---|
| `BarreDeRaccourcis barreParDefaut(const std::vector<AdresseSkill>& adresses)` | Barre de `s_emplacementsPrincipaux` emplacements : une rangée de 4 par personnage, dans l'ordre des places (place 1 : emplacements 0 à 3…), ses skills dans l'ordre reçu. Le 5ᵉ skill d'un personnage, ou une place sans rangée, n'est pas placé (sans erreur). Ne connaît pas `Escouade` ; `[[nodiscard]]` |
| `sf::FloatRect rectangleDeLEmplacement(std::size_t emplacement, sf::Vector2f tailleVue)` | Grille de 4 colonnes × 3 rangées, icônes de 48 px, 4 px d'écart, centrée en bas de la vue d'interface avec 10 px de marge ; l'emplacement 0 en haut à gauche ; `assert` ; `[[nodiscard]]` |
| `std::optional<std::size_t> emplacementSous(sf::Vector2f point, sf::Vector2f tailleVue)` | Emplacement dont l'icône contient le point ; vide dans un écart ou hors barre. Construite sur `rectangleDeLEmplacement` : une seule source de vérité pour le dessin et le clic ; `[[nodiscard]]` |

### `ClicInterface` — l'interface d'abord

`ClicInterface.hpp` / `.cpp`. `enum class Barre { Raccourcis }` (la barre d'auras s'y ajoutera), `struct ClicInterface { Barre barre; std::size_t emplacement; }` (comparable) et une fonction libre.

| Fonction | Rôle |
|---|---|
| `std::optional<ClicInterface> clicSurInterface(sf::Vector2f point, sf::Vector2f tailleVue)` | Ce que le point (coordonnées de la vue d'interface) touche dans l'interface ; vide s'il tombe sur le monde. Un clic qui touche l'interface est consommé par `EcranJeu` ; `[[nodiscard]]` |

### `BarreDeSkills` — le dessin de la barre de raccourcis

Fonctions libres, sans état : tout est recalculé à chaque image. Ne connaît ni `Escouade` ni `Personnage` ni la barre : elle reçoit des `IconeDeRaccourci`. `struct IconeDeRaccourci { std::optional<EtatSkill> etat; sf::String touche; sf::Color couleurDuLanceur; std::size_t place; bool enVisee; }` (`place` : place du lanceur, 1 à N ; `etat` vide = emplacement vide ; construite par `EcranJeu`).

| Fonction | Rôle |
|---|---|
| `std::string sigleDuSkill(IdSkill id)` | Étiquette de deux lettres : FP, PR, SO, EX, FS, IM, TP, PF ; `switch` sans `default` (un `IdSkill` oublié est un avertissement du compilateur) ; `[[nodiscard]]` |
| `sf::Color couleurDuContour(bool enVisee, bool enActivation)` | Jaune en visée (prioritaire), blanc en activation, gris sinon |
| `int secondesAffichees(float tempsRestant)` | Secondes entières, arrondies vers le haut |
| `std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre)` | Sommets d'un `TriangleFan` : centre, aiguille, coins du carré dans l'intervalle, midi ; vide si la fraction est nulle |
| `void dessinerBarreDeSkills(sf::RenderTarget& rendu, const sf::Font& police, const std::array<IconeDeRaccourci, s_emplacementsPrincipaux>& icones)` | Dessine les 12 icônes à la place de `rectangleDeLEmplacement` (à appeler avec la vue d'interface) : fond, sigle, nom de la touche dans un coin, bande de la couleur du lanceur (14 px) avec la place du lanceur, quartier sombre et chiffre en recharge, grisée si le lanceur est mort, contour jaune si `enVisee`, blanc si `enActivation`. Un emplacement vide est un cadre sombre avec seulement le nom de sa touche |

### `EcranDeFin` — l'écran de fin de partie

Une fonction libre, sans état, sur le modèle de `BarreDeSkills`. Ne connaît ni `Partie` ni `Escouade`. Sert à la défaite et à la victoire.

| Fonction | Rôle |
|---|---|
| `void dessinerEcranDeFin(sf::RenderTarget& rendu, const sf::Font& police, const std::string& titre)` | Voile noir semi-transparent sur toute la vue par défaut, puis `titre` (« Victoire » ou « Défaite ») et « Appuyez sur R pour revenir au choix de l'escouade » au centre ; à appeler avec la vue par défaut, après la barre de skills |

### `BarreDeVie` — la barre de vie d'une entité

Deux fonctions libres, sans état, utilisées par `Personnage::dessiner` et
`Monstre::dessiner`, et une constante `s_hauteurBarreDeVie` (5 px).

| Fonction | Rôle |
|---|---|
| `sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille)` | Coin haut-gauche de la barre d'une entité de ce centre et de cette taille : alignée sur son bord gauche, 6 px au-dessus de son carré |
| `void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio)` | Fond gris, puis rectangle vert proportionnel à `ratio` (0 à 1) |

### `Eclaircir` — éclaircir et assombrir une couleur

Deux fonctions libres, sans état. `eclaircir` est l'indice visuel d'un coup en préparation (`Personnage::dessiner`, `Monstre::dessiner`) ; `assombrir` donne les nuances des doublons de classe (`composer`).

| Fonction | Rôle |
|---|---|
| `sf::Color eclaircir(sf::Color couleur, float fraction)` | La couleur mélangée au blanc à raison de `fraction` (0 = inchangée, 1 = blanc) ; l'opacité est conservée ; `[[nodiscard]]` |
| `sf::Color assombrir(sf::Color couleur, float fraction)` | La couleur mélangée au noir à raison de `fraction` (0 = inchangée, 1 = noir) ; l'opacité est conservée ; `[[nodiscard]]` |

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

### `TraceDeChemin` — le chemin restant, pour le débogage

Une fonction libre, sans état, utilisée par `Escouade::dessinerPointsVises` et `Monstre::dessinerChemin` (F3).

| Fonction | Rôle |
|---|---|
| `void dessinerTraceDeChemin(sf::RenderTarget& rendu, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Color couleur)` | Ligne brisée (`LineStrip`) de `depart` à travers `points`, dans l'ordre |

### `Cercle` — la forme d'une entité

Une structure et une fonction libre, sans comportement.

Données membres : `struct Cercle { sf::Vector2f centre; float rayon; }`.

| Fonction | Rôle |
|---|---|
| `float distanceBordABord(const Cercle& a, const Cercle& b)` | Distance entre les centres moins les deux rayons ; 0 si les cercles se touchent ou se chevauchent (jamais négative) |
| `float distanceCercleRectangle(Cercle cercle, sf::FloatRect rectangle)` | Écart bord à bord entre le cercle et le rectangle plein ; 0 en cas de contact, de chevauchement ou de centre dedans, jamais négatif ; `[[nodiscard]]` |
| `float distancePointSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b)` | Distance du point au segment `[a, b]` (extrémités comprises, segment réduit à un point permis) ; `[[nodiscard]]` |

## Principes de conception à respecter

1. **Anticiper la structure, pas les fonctionnalités.** On ne code que
   les fonctionnalités nécessaires à l'étape en cours : pas de fonction,
   d'option ni de paramètre inutilisés "pour plus tard". En revanche, quand
   une extension est connue (annoncée dans la discussion, ROADMAP.md ou
   docs/brainstorm.md), on choisit dès maintenant une structure qui
   l'accueillera sans réécriture (classe de base, conteneur, point
   d'extension), même si elle n'a qu'un seul utilisateur aujourd'hui. Toute
   structure ajoutée doit servir dès l'étape présente. Le code devenu inutile
   est supprimé (Git le conserve dans l'historique).
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
| `docs/ecrans.md` | `Application`, pile d'écrans (patron State), `Transition`, `Contexte`, vues à hauteur fixe, `Reglages` (dont la table des touches des raccourcis) et `reglages.ini`, menu et options | Avant de modifier `Application`, un `Ecran`, `ListeDeBoutons`, `Reglages` ou `TailleDeVue` |
| `docs/boucle-et-entrees.md` | Qui fait quoi entre `EcranJeu`, `Controleur` et `Partie`, ordre d'une image, entrée souris et clavier, barre de raccourcis et règle « l'interface d'abord », mode visée, combat engagé, défaite, caméra | Avant de modifier `main.cpp`, `Partie`, `Controleur`, `BarreDeRaccourcis`, `ClicInterface` ou `Niveau` |
| `docs/carte-et-collision.md` | Carte, collision et dessin, portes et objets interactifs, déplacement avec glissement et obstacles, plan texte et validation | Avant de modifier `TileMap`, `Deplacement` ou `ObjetInteractif` |
| `docs/generation.md` | Génération de donjon : placement avec rejet, emprise / forme creusée, paramètres, graine unique | Avant de modifier `GenerateurDonjon` |
| `docs/vision.md` | Vision et brouillard de guerre : shadowcasting symétrique, `Vision` (inconnue / explorée / visible), rayon des personnages, règles de clic, dessin, F6, limites | Avant de modifier `ChampDeVision`, `Vision` ou ce que `Partie` en fait |
| `docs/chemin.md` | Recherche de chemin (A*), suivi de chemin (`Mobile`), lissage de chemin | Avant de modifier `Chemin` ou `Mobile` |
| `docs/formation.md` | `Escouade` et `Personnage` : ancre, formation, orientation, verrou de cible, placement en combat, mort et défaite | Avant de modifier `Escouade` ou `Personnage` |
| `docs/combat.md` | `Skill` (dont l'attaque de base), `Cercle`, portée bord à bord, autodéfense, `Cible`, skill en attente, circuit des dégâts, qui bloque qui | Avant de modifier `Cercle`, `Autodefense`, `Coup`, `Skill`, `Cible`, le skill ou les listes d'obstacles |
| `docs/monstre.md` | `Monstre` : identifiant, états, détection, territoire, poursuite, retour au poste, riposte | Avant de modifier `Monstre` |
| `docs/tests.md` | Tests (Catch2, conventions) | Avant d'ajouter ou de modifier un test |
| `docs/audit.md` | Audit du code avant l'étape 49 : constats (R, D, P, F, C, T) et ce qui a été traité | Avant de planifier une refactorisation |
| `docs/brainstorm.md` | Idées à explorer par thème (rythme du combat, défense, gambits, génération procédurale), leur statut et leurs prérequis ; rien n'y est une décision | Avant de proposer une nouvelle étape |
| `docs/historique.md` | Historique détaillé, tel quel | Jamais nécessaire avant de modifier du code |
