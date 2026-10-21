# Squad-ARPG — Architecture

Version courte, importée par `CLAUDE.md` à chaque session : dossiers, une ligne par module,
règles de dépendance, principes. Les données membres et les interfaces publiques de chaque module
sont dans la section « Référence des modules » du document de `docs/` de son sujet (tableau à la
fin), avec les décisions et les limites connues.

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
| Build | CMake + Ninja, presets `debug` et `jeu` (`docs/build.md`) |
| Langage | C++20 |
| Bibliothèque graphique | SFML 3.0.2 |
| Tests | Catch2 3 (paquet MSYS2 `mingw-w64-ucrt-x86_64-catch`), lancés par CTest |
| Analyse | clang-tidy et clang-format (paquet `mingw-w64-ucrt-x86_64-clang-tools-extra`) |

## Structure du dépôt

```
squad-arpg/
├── CMakeLists.txt, CMakePresets.json, .clang-format, .clang-tidy, .gitignore
├── CLAUDE.md, ARCHITECTURE.md, ROADMAP.md
├── src/
│   ├── base/       Aleatoire, Cercle, Eclaircir
│   ├── rendu/      BarreDeVie, TraceDeChemin
│   ├── monde/      TileMap, Chemin, Deplacement, Mobile, ChampDeVision, Vision,
│   │               ObjetInteractif, Niveau, GenerateurDonjon, Encerclement
│   ├── combat/     Skill, SkillConnu, IdSkill, TypeSkill, AdresseSkill, Cible, ActionEngagee,
│   │               EtatSkill, Coup, EvenementDeCombat, EntiteVue, Autodefense, Engagement,
│   │               Defense, PointsDeVie, Vitalite
│   ├── objets/     Emplacement, Objet, CatalogueObjets, Equipement, Inventaire, PositionDObjet
│   ├── escouade/   Escouade, Personnage, Formation, PlacesDAttaque, Intention, Separation,
│   │               EcartEntrePersonnages, Composition, Specialite, SelectionDEscouade,
│   │               Provocations, Agresseurs
│   ├── monstres/   Monstre
│   ├── jeu/        Partie
│   ├── interface/  Ecran, Transition, Contexte, PileDEcrans, EntreeDInterface, EcranMenu,
│   │               EcranOptions, EcranChoixEscouade, EcranJeu, EcranInventaire,
│   │               DispositionInventaire, AffichageObjets, AffichageSkills, ListeDeBoutons, Reglages, TailleDeVue,
│   │               EmplacementsPrincipaux, BarreDeRaccourcis, CommandeDeTouche, ClicInterface,
│   │               BarreDeSkills, EcranDeFin, TextesFlottants, Controleur
│   └── app/        main.cpp, Application
├── tests/
│   ├── aides.hpp, aides_escouade.hpp, aides_monstre.hpp   (aides communes, à la racine)
│   └── base/, rendu/, monde/, combat/, objets/, escouade/, monstres/, jeu/, interface/
│                   (miroir de src/ : test_<Module>.cpp, ou test_<Module>_<thème>.cpp)
├── assets/fonts/   DejaVuSans.ttf, LICENSE-DejaVu.txt (copiés à côté de main.exe)
├── docs/           un fichier par sujet (tableau à la fin)
└── reglages.ini    créé par le jeu à côté de main.exe, ignoré par Git
```

Un module = un `.hpp` et souvent un `.cpp` de même nom, dans le même dossier. Liste exacte des
sources et des tests : `CMakeLists.txt`.

Deux exécutables, `main` (le jeu) et `tests` (Catch2), liés à la bibliothèque statique
`squad_core`, qui contient tout `src/` sauf `app/` : logique de jeu et écrans, testables sans
fenêtre ni boucle. `main` ne contient que `app/` (`main.cpp` et `Application`).

## Qui possède quoi

Détail : `docs/boucle-et-entrees.md`, « Qui possède quoi ».
- `main.cpp` construit et lance l'`Application`, qui possède la fenêtre, la police, les
  `Reglages` et la pile d'écrans (patron State) ; seul l'écran du dessus reçoit les événements et
  est mis à jour, mais la pile est dessinée de bas en haut sous un écran qui laisse voir dessous.
  Un écran vit toujours plus longtemps que ceux qui sont au-dessus de lui.
- `EcranChoixEscouade` possède la `SelectionDEscouade` ; `EcranJeu` possède la `Partie`, le
  `Controleur`, la `BarreDeRaccourcis`, les `TextesFlottants` et la caméra, et traduit les
  événements SFML en appels au `Controleur` ; `EcranInventaire`, empilé sur lui, ne possède rien : il
  lit l'`Escouade` de l'`EcranJeu` du dessous.
- `Partie` possède la `TileMap`, l'`Escouade`, les `Monstre`, les `ObjetInteractif`, la `Vision`
  et le seul `Generateur`. Elle fait l'intermédiaire entre `Escouade` et `Monstre`, qui ne se
  connaissent pas (ils échangent des `EntiteVue`, des `Coup`, des `EntiteDeControle`), et tient la
  file des `EvenementDeCombat`.
- `Escouade` possède son ancre (`Mobile`), sa `Formation`, ses `PlacesDAttaque`, N `Personnage`,
  des `Provocations`, des `Agresseurs` et le sac de l'escouade (`Inventaire`, 40 cases).
  `Personnage` et `Monstre` possèdent un `Mobile`, des `Skill` et une `Vitalite` ; `Personnage`
  possède aussi un `Equipement`, d'où vient son armure (`docs/objets.md`).
- `Controleur` porte l'état d'interface (visée, déplacement continu) et reçoit la `Partie` à
  chaque appel.

## Modules (une ligne par module)

`base/` — outils sans règle de jeu (`docs/combat.md`) :
- `Aleatoire` : `using Generateur = std::mt19937`, passé par référence, jamais copié ni global.
- `Cercle` : forme d'une entité, `cercleDuRectangle`, `distanceBordABord`, `distanceCercleRectangle`,
  `distancePointSegment`.
- `Eclaircir` : `eclaircir` et `assombrir` une couleur, `s_eclaircissementExecution`.

`rendu/` — primitives de dessin :
- `BarreDeVie` : position et dessin de la barre de vie d'une entité (`docs/combat.md`).
- `TraceDeChemin` : ligne brisée du chemin restant, débogage F3 (`docs/chemin.md`).

`monde/` — la carte et ce qui s'y déplace :
- `TileMap` : grille de cases (sol, mur, portes), collision, dessin (`docs/carte-et-collision.md`).
- `Chemin` : A*, coût des obstacles, lissage, repli vers une origine (`docs/chemin.md`).
- `Deplacement` : collision et glissement contre les murs et les cercles (`docs/carte-et-collision.md`).
- `Mobile` : suivi de chemin, état « en mouvement » et « bloqué » (`docs/chemin.md`).
- `ChampDeVision` : shadowcasting symétrique, `estVisibleDepuis`, `ligneDeVue` (`docs/vision.md`).
- `Vision` : cases inconnues, explorées, visibles, et voile du brouillard (`docs/vision.md`).
- `ObjetInteractif` : portes et point d'approche (`docs/carte-et-collision.md`).
- `Niveau` : données d'un niveau, `creerNiveau()` pour les tests (`docs/generation.md`).
- `GenerateurDonjon` : salles, couloirs, monstres, portes, `genererNiveau` (`docs/generation.md`).
- `Encerclement` : places régulières sur un anneau (`docs/formation.md`).

`combat/` — skills et règles communes aux deux camps (`docs/combat.md`) :
- `Skill` : type, portée, puissance, recharge, zone, machine à états `commencer`/`avancer`/`annuler`.
- `TypeSkill` : `Degats`, `Soin`, `Provocation`, `Zone`.
- `IdSkill`, `SkillConnu`, `AdresseSkill` : identifiant stable d'un skill, skill d'un personnage, place + skill.
- `Cible` : `std::variant` allié, monstre ou point, et `Surcharges` pour `std::visit`.
- `ActionEngagee` : l'action (attaque de base ou skill) qu'un personnage prépare, et sa cible.
- `EtatSkill` : le skill d'une place, tel que l'affichage en a besoin.
- `Coup` : dégâts à infliger à un monstre, sortie d'`Escouade`.
- `EvenementDeCombat` : champs communs + détail en variant (dégâts, soin, skill lancé, monstre provoqué), en données (file de `Partie`).
- `EntiteVue` : une entité telle que l'autre camp la voit, et `trouverEntite`.
- `Autodefense` : quel ennemi frapper, quel agresseur viser.
- `Engagement` : zones de contrôle, coups d'opportunité, monstre dont se dégager.
- `Defense` : `appliquerArmure`, rendement décroissant.
- `PointsDeVie` : valeur et maximum, invariants de vie et de mort.
- `Vitalite` : points de vie et armure d'une entité, commune à `Personnage` et `Monstre`.

`objets/` — les objets, ce qu'on porte et le sac (`docs/objets.md`) :
- `Emplacement` : les 10 emplacements, les 8 catégories d'objet et `peutAller`, seule source de la règle.
- `Objet` : `IdObjet` et `Objet`, l'instance (un identifiant), distincte de son modèle.
- `CatalogueObjets` : `DefinitionObjet` (nom, catégorie, armure), `catalogueObjets()`, `definitionObjet()`
  et `sacDeDepart()`.
- `Equipement` : un objet au plus par emplacement, `armureTotale()` ; possédé par chaque `Personnage`.
- `Inventaire` : le sac, des cases à position fixe ; possédé par l'`Escouade`.
- `PositionDObjet` : une case du sac ou un emplacement d'un personnage, et `deplacementPermis`, seule source
  de la règle des déplacements.

`escouade/` — le camp du joueur (`docs/formation.md`, sauf mention) :
- `Escouade` : l'assemblage : ancre, verrou, skills en attente, attaque, et la résolution des intentions en
  points visés ; elle possède `Formation`, `PlacesDAttaque`, `Agresseurs`, `Provocations`, le sac
  (`Inventaire`) et les `Personnage` ; `ramasser` et `deplacerObjet` déplacent les objets.
- `Personnage` : un membre : déplacement, attaque de base, skills, vie, vision, équipement (son armure
  en est la somme).
- `Formation` : décalages, orientation, point à regarder et places de formation, sans `Personnage`.
- `PlacesDAttaque` : place d'attaque retenue par assaillant autour de sa cible, sans `Personnage`.
- `Intention` : ce qu'un personnage veut faire, sa situation, `choisirIntention`, les règles par
  priorité où se brancheront les gambits, et `cerclesAContourner` (fonctions pures, sans carte).
- `Separation` : `pousseesDeSeparation`, séparation douce en fonction pure (`EtatDeSeparation`).
- `EcartEntrePersonnages` : `s_ecartEntrePersonnages`, en-tête neutre.
- `Composition` : `DefinitionPersonnage` (dont l'équipement de départ), `Rang` et `decalagesDeFormation`.
- `Specialite` : catalogue des 5 classes et `composer`.
- `SelectionDEscouade` : les 3 classes choisies, cases de l'écran de choix (`docs/ecrans.md`).
- `Provocations` : monstres provoqués et leur provocateur (`docs/combat.md`).
- `Agresseurs` : monstres qui ont frappé l'escouade, mémoire de 3 s par identifiant, sans position
  (`docs/combat.md`).

`monstres/` :
- `Monstre` : états, détection, territoire, poursuite, retour au poste, frappe (`docs/monstre.md`).

`jeu/` :
- `Partie` : règles de jeu, ordre d'une image, intermédiaire escouade/monstres (`docs/boucle-et-entrees.md`).

`interface/` — écrans, HUD, réglages, entrées :
- `Ecran`, `Transition`, `Contexte` : base des écrans, changements de pile, contexte d'appel (`docs/ecrans.md`).
- `EcranMenu`, `EcranOptions`, `EcranChoixEscouade` : menu, options d'affichage, choix des classes (`docs/ecrans.md`).
- `PileDEcrans` : la pile et `premierEcranADessiner`, quels écrans dessiner (`docs/ecrans.md`).
- `EntreeDInterface` : clic gauche et curseur en coordonnées d'interface, pour les écrans à vue d'interface (`docs/ecrans.md`).
- `EcranJeu` : l'écran de la partie : événements, caméra, dessin, textes flottants (`docs/ecrans.md`).
- `EcranInventaire` : le sac et l'équipement, déplacement des objets par glisser-déposer, par-dessus la partie figée, touche I (`docs/ecrans.md`).
- `DispositionInventaire` : rectangles de l'inventaire, `elementSous` (une `PositionDObjet`), placement de l'infobulle (`docs/ecrans.md`).
- `AffichageObjets` : noms des emplacements et des catégories, sigles, lignes de l'infobulle (`docs/objets.md`).
- `AffichageSkills` : `nomDuSkill`, le nom affiché d'un skill (`docs/combat.md`).
- `ListeDeBoutons` : colonne de boutons sans position stockée (`docs/ecrans.md`).
- `Reglages` : réglages d'affichage, `reglages.ini`, table des touches des raccourcis (`docs/ecrans.md`).
- `TailleDeVue` : vue à hauteur fixe et vue d'interface (`docs/ecrans.md`).
- `EmplacementsPrincipaux` : `s_emplacementsPrincipaux`, en-tête neutre (P1, `docs/boucle-et-entrees.md`).
- `BarreDeRaccourcis` : modèle des emplacements, disposition par défaut et à l'écran (`docs/boucle-et-entrees.md`).
- `CommandeDeTouche` : `commandePourTouche`, ce que fait chaque touche, fonction pure (`docs/boucle-et-entrees.md`).
- `ClicInterface` : ce qu'un clic touche dans l'interface (`docs/boucle-et-entrees.md`).
- `BarreDeSkills` : dessin des icônes de la barre et `iconesDeLaBarre` (`docs/boucle-et-entrees.md`).
- `EcranDeFin` : voile et titre de victoire ou de défaite (`docs/boucle-et-entrees.md`).
- `TextesFlottants` : dégâts et soins à l'écran, lus dans la file d'événements (`docs/combat.md`).
- `Controleur` : état d'interface, visée, déplacement continu (`docs/boucle-et-entrees.md`).

`app/` :
- `main.cpp` et `Application` : fenêtre, police, boucle SFML, pile d'écrans (`docs/ecrans.md`).

## Règles de dépendance

Un dossier n'inclut que lui-même et ceux qui le **précèdent** dans cet ordre :

```
base → monde → combat → objets → escouade, monstres → jeu → interface → app
rendu : ne dépend que de base
objets : ne dépend que de base
```

- `escouade/` et `monstres/` ne s'incluent pas entre eux : ils communiquent par `combat/`
  (`EntiteVue`, `Coup`, `EntiteDeControle`) ; `jeu/Partie` est le seul à les relier.
- Le noyau (`base` à `jeu`) n'inclut jamais `interface/` ni `app/`.
- Les types valeur de SFML (`sf::Vector2f`, `sf::FloatRect`, `sf::Color`, `sf::Angle`) sont
  permis partout.
- Contrôle : chaque `#include` donne son dossier, donc un `grep` sur `#include "` suffit (fait
  à l'étape 81 : aucune inclusion montante, aucune entre `escouade/` et `monstres/`).

**Exceptions transitoires, jusqu'à M1** (séparer le dessin du noyau, `docs/audit.md`) : des
classes du noyau se dessinent elles-mêmes.
- Inclusions de `rendu/` depuis le noyau : `escouade/Escouade.cpp` → `rendu/TraceDeChemin.hpp` ;
  `escouade/Personnage.cpp` → `rendu/BarreDeVie.hpp` ; `monstres/Monstre.cpp` →
  `rendu/BarreDeVie.hpp` et `rendu/TraceDeChemin.hpp`.
- Cible et formes de dessin SFML dans le noyau (`RenderTarget` dans le `.hpp`, formes dans le
  `.cpp`) : `monde/TileMap` (`RectangleShape`), `monde/Vision` (`VertexArray`), `escouade/Escouade`
  (`CircleShape`, `RectangleShape`, `Vertex`), `escouade/Personnage` (`RectangleShape`),
  `monstres/Monstre` (`RectangleShape`), `jeu/Partie` (`CircleShape`).

Après M1, le dessin passera au-dessus du noyau et `squad_core` pourra être coupée en une
bibliothèque par dossier, qui ne se lie qu'aux précédentes : une inclusion montante deviendra
une erreur de compilation.

## Conventions de structure

Les conventions de code (noms, `const`, `[[nodiscard]]`, constantes…) sont dans `CLAUDE.md`.
- Chaque nouveau fichier naît dans le dossier de sa famille, en respectant les règles de
  dépendance.
- `#include` à chemin complet depuis `src/` (`#include "combat/Skill.hpp"`), partout, même à
  l'intérieur d'un dossier. `src/` est le seul dossier d'inclusion de `squad_core`.
- Une source de `squad_core` est listée dans `add_library(squad_core ...)` avec son dossier
  (`src/combat/Skill.cpp`) ; une seule bibliothèque pour l'instant.
- Les tests vont dans le dossier miroir (`tests/combat/test_Skill.cpp`) et dans
  `add_executable(tests ...)` ; ils incluent les aides par `#include "aides.hpp"` (racine de
  `tests/`, dossier d'inclusion de la cible `tests`).
- Première ligne de chaque fichier : son chemin complet (`// ./src/combat/Skill.hpp`).

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

Les décisions techniques détaillées (le pourquoi, les limites connues) et la référence des
modules (données membres, fonctions publiques) vivent dans `docs/`, un fichier par sujet. Avant
de modifier un module, lire le fichier correspondant :

| Fichier | Contenu | Quand le lire |
|---|---|---|
| `docs/build.md` | Build (CMake, `squad_core`, dossiers et chemins d'inclusion, exécutables), presets `debug` et `jeu`, CTest, `clang-format` et `clang-tidy`, choix de SFML | Avant de modifier `CMakeLists.txt`, `CMakePresets.json`, `.clang-tidy` ou d'ajouter un fichier source |
| `docs/ecrans.md` | `Application`, pile d'écrans (patron State, dessin sous un écran), `Transition`, `Contexte`, `EcranJeu`, `EcranInventaire` et sa disposition, vues à hauteur fixe, `Reglages` (dont la table des touches) et `reglages.ini`, menu, options, choix de l'escouade ; référence de `app/` et des écrans | Avant de modifier `Application`, un `Ecran`, `PileDEcrans`, `EntreeDInterface`, `DispositionInventaire`, `ListeDeBoutons`, `Reglages`, `TailleDeVue` ou `SelectionDEscouade` |
| `docs/boucle-et-entrees.md` | Qui possède quoi, qui fait quoi entre `EcranJeu`, `Controleur` et `Partie`, ordre d'une image, souris et clavier, barre de raccourcis et « l'interface d'abord », mode visée, combat engagé, défaite, caméra ; référence de `Partie`, `Controleur` et du HUD | Avant de modifier `Partie`, `Controleur`, `BarreDeRaccourcis`, `CommandeDeTouche`, `ClicInterface`, `BarreDeSkills` ou `EcranDeFin` |
| `docs/carte-et-collision.md` | Carte, collision et dessin, portes et objets interactifs, déplacement avec glissement et obstacles, plan texte et validation | Avant de modifier `TileMap`, `Deplacement` ou `ObjetInteractif` |
| `docs/generation.md` | Génération de donjon : placement avec rejet, emprise / forme creusée, paramètres, graine unique ; `Niveau` | Avant de modifier `GenerateurDonjon` ou `Niveau` |
| `docs/vision.md` | Vision et brouillard de guerre : shadowcasting symétrique, `Vision` (inconnue / explorée / visible), rayon des personnages, règles de clic, dessin, F6, limites | Avant de modifier `ChampDeVision`, `Vision` ou ce que `Partie` en fait |
| `docs/chemin.md` | Recherche de chemin (A*), suivi de chemin (`Mobile`), lissage de chemin, trace de débogage | Avant de modifier `Chemin`, `Mobile` ou `TraceDeChemin` |
| `docs/formation.md` | `Escouade` et `Personnage` : ancre, formation, orientation, séparation douce, verrou de cible, placement en combat, places d'attaque, intentions, mort et défaite ; `Formation`, `PlacesDAttaque`, `Intention`, `Separation`, `Composition`, `Specialite`, `Encerclement` | Avant de modifier `Escouade`, `Personnage`, `Formation`, `PlacesDAttaque`, `Intention`, `Separation`, `Composition`, `Specialite` ou `Encerclement` |
| `docs/combat.md` | `Skill` (dont l'attaque de base), `Cercle`, portée bord à bord, autodéfense, `Cible`, skill en attente, circuit des dégâts, qui bloque qui, armure, aléatoire, zones de contrôle, événements et textes flottants | Avant de modifier un module de `combat/` ou de `base/`, `Provocations`, `Agresseurs`, `TextesFlottants`, `BarreDeVie` ou les listes d'obstacles |
| `docs/monstre.md` | `Monstre` : identifiant, états, détection, territoire, poursuite, retour au poste, riposte | Avant de modifier `Monstre` |
| `docs/objets.md` | Objets : emplacements et catégories, catalogue, `Equipement`, `Inventaire`, armure calculée depuis l'équipement, opérations `ramasser` / `deplacerObjet` de l'`Escouade`, `PositionDObjet` ; référence des modules d'`objets/` | Avant de modifier un module d'`objets/`, `AffichageObjets` ou l'équipement d'un `Personnage` |
| `docs/tests.md` | Tests (Catch2, conventions, dossiers, aides) | Avant d'ajouter ou de modifier un test |
| `docs/audit.md` | Audit du code avant l'étape 80 : constats par famille (R, M, P, D, C, E, L, V, T, B, K), organisation en dossiers, passages de `clang-tidy` | Avant de planifier une refactorisation |
| `docs/brainstorm.md` | Idées à explorer par thème (rythme du combat, défense, gambits, génération procédurale), leur statut et leurs prérequis ; rien n'y est une décision | Avant de proposer une nouvelle étape |
| `docs/historique.md` | Liste des étapes terminées et leur détail | Jamais nécessaire avant de modifier du code |
