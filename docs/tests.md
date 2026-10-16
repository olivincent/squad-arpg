# Squad-ARPG — Tests

## Tests

- **Catch2 3 adopté**, installé via MSYS2
  (`mingw-w64-ucrt-x86_64-catch`), trouvé par `find_package(Catch2 3
  REQUIRED)` comme SFML (pas de `FetchContent`).
- **Un seul exécutable `tests`**, lancé directement (`./build/tests.exe`)
  — pas de CTest ni de `catch_discover_tests`. Un fichier par module
  (`test_TileMap.cpp`, `test_Chemin.cpp`, `test_Deplacement.cpp`,
  `test_Monstre.cpp`, `test_Personnage.cpp`, `test_Skill.cpp`,
  `test_Mobile.cpp`, `test_Cercle.cpp`, `test_Autodefense.cpp`,
  `test_Encerclement.cpp`, `test_BarreDeSkills.cpp`, `test_EntiteVue.cpp`, `test_BarreDeVie.cpp`,
  `test_PointsDeVie.cpp`, `test_Provocations.cpp`, `test_Agresseurs.cpp`, `test_Partie.cpp`,
  `test_Controleur.cpp`, `test_ChampDeVision.cpp`, `test_Vision.cpp`),
  sauf `Escouade`, coupée par thème en six fichiers à plat (pas de
  sous-dossier) :
  - `test_Escouade_deplacement.cpp` : ancre, chemin, formation, places,
    vivants, rotation de la formation ;
  - `test_Escouade_combat.cpp` : verrou, approche, autodéfense, placement
    en combat ;
  - `test_Escouade_skills.cpp` : les quatre types de skill, le skill en
    attente, `etatDuSkill`, la visée et la portée du skill de zone ;
  - `test_Escouade_agresseurs.cpp` : agresseurs, laisse, provocations,
    `proieDuMonstre` (filet d'intégration ; les fonctions de `Provocations`
    et d'`Agresseurs` sont testées seules dans leurs fichiers, étape 51) ;
  - `test_Escouade_vue.cpp` : la ligne de vue (étapes 44 et 46) ;
  - `test_Escouade_places.cpp` : les places d'attaque et `pointVise` (étape 67).
  Un cas qui touche plusieurs thèmes est dans le fichier de son sujet
  principal. Le plus gros (`test_Escouade_skills.cpp`) fait environ 790
  lignes.
- **`tests/aides.hpp`** (étape 50) : en-tête seul, fonctions `inline`, pour
  les aides **identiques** utilisées par plusieurs fichiers :
  `verifierPointProche`, `pointCible`, `cibleMonstre`,
  `carteDegagee10x10`, `carteDegageeEntouree` (800 × 600 px, murs
  autour), `carteOuverte`, `simuler(escouade, ...)`, `attaquerCible`,
  `placeAlliePourSoin`, ainsi que `s_dureeTic` et `nombreDeTics(duree)`
  (étape 51 : le nombre de tics est calculé une fois par `std::lround`, puis
  les boucles comptent des entiers ; additionner des `float` jusqu'à la
  durée accumulait des arrondis et faisait 5 tics au lieu de 6 pour 0,1 s). Quand deux copies différaient, elles ont des noms
  distincts et explicites (`carteDegagee10x10` pour `Mobile`, `Monstre` et
  `Escouade` ; `carteDegagee5x5` reste dans `test_Personnage.cpp`). Une
  aide utilisée par un seul fichier y reste, dans un `namespace {}` (plus
  de `static`) : `ecoulerTemps` (trois signatures différentes, une par
  fichier : `Skill`, `Monstre`, `Personnage`), `simuler(mobile, ...)`,
  `declencher`, `unSeulMonstre`, `simulerAttaque`, `simulerAvecMonstre`,
  `carteAvecMurVertical`, `carteAvecMursFins`, etc. `#include
  "aides.hpp"` suffit : l'en-tête est dans le dossier des `.cpp`.
- **Noms de `TEST_CASE` sans accents ni cédilles** (étape 50) : « a
  portee », « degats », « deja ». Les commentaires gardent leurs
  accents. Raison : filtrer en ligne de commande dans n'importe quel
  terminal, par exemple `./build/tests.exe "*a portee*"` (le filtre est
  un motif entre guillemets, `*` remplace n'importe quel texte) ; la liste
  complète : `./build/tests.exe --list-tests`. Les noms restent uniques
  dans tout l'exécutable.
- **Petits plans dédiés** : chaque test construit son propre plan texte
  (3×3, 4×4, 5×5…), jamais la carte de `main.cpp`, pour isoler exactement
  le cas testé.
- **Teste uniquement l'interface publique** de `TileMap`, `Chemin`,
  `Escouade`, `Personnage`, `Monstre`, `Skill`, `Mobile` et `Cercle`
  (`distanceBordABord`, `getCercle`, `getTile`,
  `chevaucheUnMur`, `trajetLibre`, `getLargeur`/`getHauteur`,
  `caseContenant`/`centreDeCase`, le constructeur, `trouverChemin`,
  `lisserChemin`, `allerVers`, `arreter`, `attaquer`, `mettreAJour`,
  `getCentre`, `getTaille`, `directionVoulue`, `estAPortee`, `frapper`,
  `declencherSkill`, `relancerRecharge`, `getProgressionRecharge`,
  `recevoirDegats`, `estVivant`, `proieDuMonstre`,
  `estVivante`) — aucune fonction n'a été ajoutée seulement pour
  faciliter les tests.
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
- **`Monstre` testé par son interface publique** (`getCentre`, `getId`,
  `contient`, `recevoirDegats`, `estVivant`, `mettreAJour`, `frapper`) :
  bords d'inclusion/exclusion de `contient` vérifiés séparément sur
  chaque côté (gauche/haut inclus, droit/bas exclus) ; `recevoirDegats`/
  `estVivant` couvrent le cas d'un coup qui dépasse le reste des points
  de vie et celui d'un coup supplémentaire une fois déjà mort ; `getId`
  vérifié sur deux monstres créés avec des identifiants différents, qui
  renvoient chacun le leur. Le branchement dans `main.cpp`
  (verrouillage de cible par identifiant, vecteur de monstres,
  `std::erase_if`, contour sur la cible, riposte) n'est pas testé —
  tout se passe dans `main.cpp`, hors de portée de `tests/`.
  - **`frapper`** : deux cas (dégâts à portée avec la valeur
    du monstre — 10 —, 0 hors de portée) et un troisième qui prouve que
    `mettreAJour` avance bien la recharge de `m_attaque` (dégâts de
    nouveau après une durée simulée généreuse). **Noms de `TEST_CASE`
    distincts de ceux de `test_Personnage.cpp`** (préfixés `Monstre.`) :
    Catch2 exige des noms de cas de test uniques dans tout l'exécutable
    `tests`, pas seulement dans un fichier.
  - **Coup reçu** (étape 47) : 7 cas, l'état observé par le
    déplacement (le centre avance vers la proie ou non) : touché en garde
    devant une proie à 250 px (hors détection) ; même cas sans coup ;
    coup de 0 ; touché sans proie, puis un appel sans coup (indicateur
    consommé) ; touché en retour dans le territoire (poursuit) et hors du
    territoire (continue de rentrer) ; touché en poursuite. La fonction
    d'aide `amenerEnRetourHorsTerritoire` fait sortir le monstre du
    territoire. Piège : une cible à `(400, 100)` est dans un mur sur
    `carteDegagee10x10` (10 cases de 40 px), le A* échoue.
  - **Groupe et alerte** (étape 64) : `getGroupe`, et trois cas
    `alerter` (en garde dans le territoire, hors territoire, en
    poursuite) sur le même modèle que « Coup reçu ». Côté `Partie`
    (`test_Partie.cpp`, aide `niveauDegageEnGroupes`) : voisin de groupe
    alerté hors détection, autre groupe non alerté, monstre seul,
    personne alerté sans poursuite, skill de zone sur un monstre. Côté
    générateur : groupe commun dans une salle, groupes distincts entre
    salles, pas de numéro sauté.
  - **`mettreAJour` : détection, arrêt à portée** : depuis
    que `Monstre` se déplace par son propre `Mobile`, la fonction
    d'aide `ecoulerTemps(monstre, duree, cible, carte)` prend en plus une
    cible (`std::optional<Cercle>`, vide = aucune ;
    les tests y passent un point de rayon nul, `pointCible`) et une
    `TileMap` (carte dégagée dédiée, `carteDegagee10x10()`, même principe que
    pour `Mobile`/`Escouade`) — des résultats (le centre du monstre),
    jamais une trajectoire exacte. Cas couverts : une cible dans le
    rayon de détection (200 px) rapproche le monstre ; une cible hors de
    ce rayon, ou son absence, le laisse immobile ; une durée généreuse
    face à une cible dans le rayon l'amène à portée d'attaque sans
    l'atteindre — à 30 px au plus de son centre
    (portée de 15 px bord à bord + rayon du monstre de 15 px, la cible
    étant un point).
  - **Territoire et retour au poste** : `carteLongue()`,
    une carte dégagée plus large que `carteDegagee10x10()`, dédiée aux cas
    où une cible ou un trajet de retour dépasse les 600 px du
    territoire. Toujours des résultats (le centre du monstre, comparé
    au poste `{100.0f, 100.0f}` donné au constructeur dans tous ces
    tests), jamais une trajectoire exacte. Cas couverts : une cible
    poursuivie qui s'éloigne continue d'être poursuivie tant que le
    *monstre* reste dans son territoire (peu importe la distance à la
    cible elle-même) ; une cible à 300 px du poste est rejointe à
    portée et le reste ; une cible à 1000 px du poste, ou qui disparaît
    après détection, fait rentrer le monstre à son poste après une
    durée généreuse ; une cible hors du territoire mais transitoirement
    proche du monstre pendant le retour est ignorée (le monstre rentre
    quand même) ; une cible replacée dans le territoire et proche du
    monstre pendant le retour relance la poursuite (la distance entre
    les deux diminue de nouveau).
- **Cibles ponctuelles ou de la taille d'un monstre** :
  depuis que la portée se mesure entre deux `Cercle`, les tests
  de `Skill`, de `Personnage` et de `Monstre` donnent leurs cibles
  comme des cercles de rayon nul (fonction d'aide commune `pointCible`) : la portée reste alors facile à
  vérifier à la main (distance entre centres moins le seul rayon de
  l'attaquant). Les tests de combat d'`Escouade`, eux, utilisent une
  cible de rayon 15 (`cibleMonstre`), comme le monstre que `main.cpp`
  passe réellement : avec un point, la géométrie de la formation
  empêchait les mêlées (portée 10 px, aujourd'hui 20) d'arriver
  jamais à portée (voir `docs/formation.md`), et ces tests ne
  décriraient pas la situation de jeu.
- **`Cercle` (`test_Cercle.cpp`)** : `distanceBordABord` sur
  deux cercles séparés, qui se touchent (0), qui se chevauchent (0, pas
  une valeur négative) et de rayons différents. Valeurs choisies pour
  être exactes en virgule flottante (comparaison `==` sans tolérance).
  `getCercle` est testé dans `test_Personnage.cpp` (rayon 10) et
  `test_Monstre.cpp` (rayon 15).
- **`Personnage` (`test_Personnage.cpp`)** :
  - `estAPortee` : point proche à portée, point lointain hors portée,
    distance exactement égale à la portée (cas limite, `<=`).
  - `frapper` : renvoie les dégâts à portée ; 0 hors portée ; 0 au
    second appel immédiat (recharge en cours) ; 0 après une durée
    simulée trop courte, de nouveau les dégâts après une durée simulée
    généreuse (**durées qui évitent les égalités strictes en virgule
    flottante** : 0,3 s et 0,6 s plutôt que 0,25 s ou 0,5 s pile, pour
    ne pas dépendre de l'arrondi exact de l'accumulation de `dt`) ; un
    coup tenté hors de portée ne déclenche pas la recharge.
  - **Simuler le temps sans déplacer le personnage** : fonction d'aide
    `ecoulerTemps` qui appelle `mettreAJour` par tics de 1/60 s avec le
    centre du personnage comme cible (il reste immobile, seule la
    recharge évolue), sur un petit plan dégagé dédié — même principe
    que `simuler` pour `Escouade`.
  - **Points de vie** : `estVivant` à la création, encore
    vivant après 80 dégâts, mort après 100, mort après un coup qui
    dépasse le reste — mêmes quatre cas que `Monstre`. `getCentre`
    vérifié directement sur le centre donné au constructeur.
  - **Un mort ne bouge plus** : `recevoirDegats(100)` puis
    `ecoulerTemps` avec une cible lointaine pendant une durée
    généreuse ; le centre reste strictement égal (`==`, pas de
    tolérance nécessaire puisqu'aucun mouvement n'a lieu) à celui
    d'avant. Preuve que `mettreAJour` revient bien immédiatement sans
    rien calculer, pas seulement que le résultat final coïncide par
    hasard.
- **`Escouade::attaquer` (`test_Escouade_combat.cpp`)** : comme pour les autres
  fonctions d'`Escouade`, des résultats plutôt que des trajectoires
  exactes. `attaquer` reçoit une liste de `EntiteVue` et renvoie des
  `Coup`.
  - Fonction d'aide `attaquerCible` : appelle `attaquer` avec un seul
    monstre (identifiant 1) et renvoie le total des dégâts de la liste.
    `simulerAttaque` l'appelle puis `mettreAJour` à chaque tic et cumule
    les dégâts sur toute la durée simulée.
  - **L'approche exige le verrou** : les tests d'approche appellent
    `verrouiller(1)`. Cible lointaine en terrain dégagé : le premier
    appel renvoie 0 et l'escouade s'en rapproche (distance réduite après
    un tic) ; après une durée généreuse, le total des dégâts cumulés est
    `> 0`, mais l'escouade ne finit pas sur la cible (elle s'arrête à
    portée).
  - Cible déjà à portée des 4 personnages dès le départ : le premier
    appel renvoie 20 (4 × 5) et l'escouade ne bouge pas. Cette cible est
    placée entre les deux mêlées (et non plus au centre de l'escouade, à
    35 px de chacun d'eux), pour que leur portée de 20 px bord à bord
    soit atteinte.
  - **Les morts sont ignorés pour décider de l'arrêt** :
    `recevoirDegats` tue le personnage le plus proche d'un point
    lointain, l'escouade se déplace pour le laisser loin derrière
    (lui ne bouge plus), puis `attaquer` est appelée avec une cible à
    portée seulement des 3 survivants — le premier appel renvoie 15
    (3 × 5, pas 20) et un tic de `mettreAJour` supplémentaire ne fait
    pas bouger l'escouade. Sans ce filtre, le mort resté loin derrière
    empêcherait indéfiniment la condition « tous à portée » de devenir
    vraie.
  - **Autodéfense, sans verrou** : tous les personnages frappent un
    monstre à leur portée et l'ancre ne bouge pas (4 coups de 5, tous
    attribués au bon identifiant) ; un monstre hors de portée ne reçoit
    aucun coup ; seuls les tireurs frappent un monstre éloigné ; un
    verrou hors de portée n'empêche pas de frapper un autre monstre à
    portée ; un verrou à portée passe avant un monstre plus proche ; deux
    personnages frappent deux monstres différents dans la même image ;
    une escouade entièrement morte ne frappe plus.
  - `test_Autodefense.cpp` : `choisirCibleAFrapper` seule, via la
    fonction d'aide `choisir(personnage, ...)` qui lui passe le cercle et
    la portée d'un `Personnage` de portée 20 : aucun monstre à portée,
    un seul, verrou à portée avant un monstre plus proche, verrou hors de
    portée ou absent de la liste, plusieurs à portée (le plus proche),
    égalité (le premier de la liste).
- **`Escouade::proieDuMonstre`** : test par
  propriété plutôt que par position de formation exacte, comme les
  autres tests d'`Escouade` — pour un point lointain, la distance entre
  le centre renvoyé et ce point est strictement inférieure à la
  distance entre le centre de l'escouade et ce point (un personnage en
  formation est forcément plus proche d'un point extérieur que le
  centre de l'escouade lui-même, quelle que soit l'orientation exacte
  de la formation).
- **`Escouade::recevoirDegats`, observé via `proieDuMonstre`
 ** : `recevoirDegats` n'a pas d'accesseur dédié, mais son
  effet devient visible dès que les dégâts tuent un personnage — le
  filtre des morts dans `indicePersonnageLePlusProche` change alors ce
  que `proieDuMonstre` renvoie. Trois cas : des dégâts
  insuffisants pour tuer le plus proche ne changent pas le centre
  renvoyé ; le tuer en change le centre renvoyé (un autre personnage) ;
  tuer les 4 rend `proieDuMonstre` vide (`std::optional`), y
  compris après un coup supplémentaire sur une escouade déjà entièrement
  morte.
- **`Escouade::estVivante`** : trois cas, calqués sur ceux
  de `proieDuMonstre` ci-dessus (même point lointain, mêmes
  dégâts) — vraie à la création, encore vraie après la mort d'un seul
  personnage, fausse une fois les 4 morts.
- **`Escouade::cerclesDesVivants`** : trois cas, même point
  lointain et mêmes dégâts que ci-dessus — 4 cercles à la création, 3
  après une mort (le cercle du mort, repéré avant par
  `proieDuMonstre`, n'y figure plus), liste vide une fois les 4
  morts. Le branchement dans `main.cpp` n'est pas testé (vérifié
  visuellement).
- **`Skill` (`test_Skill.cpp`), cas génériques de recharge et de
  portée** (ex-tests d'`Attaque`) : couverts en un seul endroit, plutôt
  que dupliqués dans `test_Personnage.cpp` et `test_Monstre.cpp` — portée
  (proche, lointain, distance exactement égale à la portée, cas limite
  `<=`), 0 au second appel immédiat (recharge en cours), 0 après une
  durée simulée trop courte puis de nouveau la puissance après une durée
  généreuse (fonction d'aide `ecoulerTemps(skill, duree)`), un
  déclenchement hors de portée qui ne lance pas la recharge.
  `test_Personnage.cpp` et `test_Monstre.cpp` ne gardent que les cas qui
  prouvent leur propre branchement.
- **Skill** : chaque `Personnage` des tests existants reçoit
  `std::nullopt` comme skill (paramètre sans valeur par défaut) ; ceux qui
  testent le skill de dégâts passent un
  `Skill(TypeSkill::Degats, portée, puissance, recharge, 0.0f)`.
  - `test_Skill.cpp` : `relancerRecharge` empêche de frapper avant une
    recharge complète, même sur une attaque jamais utilisée ;
    `getProgressionRecharge` vaut 1 à la création, 0 juste après
    `relancerRecharge`, strictement entre les deux après 0,3 s, et de
    nouveau 1 après 0,6 s (valeurs extrêmes exactes, comparées par `==`,
    puisque la recharge est ramenée à 0 pile).
  - `test_Personnage.cpp` (skill de portée 80 px, 25 dégâts, 5 s) :
    dégâts du skill à portée ; 0 sans skill, 0 si mort, 0 hors de
    portée, 0 pendant la recharge puis 25 de nouveau après (4,8 s puis
    0,4 s, en évitant 5 s pile). Les cas « sans skill » et « hors de
    portée » vérifient aussi que l'attaque de base n'est pas touchée.
    **Preuve de la relance complète** : attaque de base, 0,3 s
    (il lui reste 0,2 s), skill, 0,3 s → l'attaque de base renvoie
    encore 0 (elle serait partie sans la relance), puis 5 après 0,3 s
    de plus.
  - `test_Skill.cpp` : le type donné au constructeur, la puissance à
    portée, 0 hors de portée, 0 pendant la recharge puis la puissance de
    nouveau (4,8 s puis 0,4 s), progression de la recharge.
  - `test_Escouade_skills.cpp`, `declencherSkill(place, monstres)` (fonctions
    d'aide `declencher`, qui additionne les dégâts des coups, et
    `unSeulMonstre`) : cible de la taille d'un monstre entre les deux
    mêlées, à portée de chacun. **Sans verrou**, le skill part sur un
    monstre à portée (25, coup attribué à son identifiant, aucun verrou
    créé), vise le plus proche parmi plusieurs (le premier de la liste
    est le plus éloigné, pour prouver que ce n'est pas l'ordre), est
    refusé sans monstre à portée et sans recharge consommée (un monstre à
    portée est frappé aussitôt après). **Avec un verrou**, 25 depuis la
    place 1, le monstre verrouillé passe avant un monstre plus proche,
    refusé si le verrou est absent de la liste ; 0 hors de portée, pour
    une place sans skill ou hors formation (0, 2, 5) et au second appel
    immédiat.
- **Obstacles** : `test_Deplacement.cpp` couvre
  `deplacementBloque` (aucun obstacle, entrée dans un obstacle, contact
  exact, chevauchement existant en s'éloignant puis en se rapprochant,
  un obstacle bloquant parmi plusieurs) et `deplacerAvecGlissement` avec
  obstacles (axe bloqué, déplacement oblique où l'axe libre passe) ;
  les appels existants passent `{}`. `test_Mobile.cpp` : un mobile qui
  va vers un point derrière un obstacle s'arrête au plus à son contact.
  Le blocage entre monstres dans `main.cpp` n'est pas testé (visuel).
- **Obstacles pour les personnages** : `test_Personnage.cpp` :
  un personnage envoyé vers un point derrière un obstacle s'arrête avant
  d'y entrer ; les appels existants passent `{}`. `test_Escouade_deplacement.cpp` :
  avec un obstacle sur le trajet, l'ancre le dépasse alors qu'aucun
  cercle de `cerclesDesVivants()` ne le chevauche à la fin. Dans les deux
  cas on compare la distance entre centres à la somme des rayons, et non
  `distanceBordABord`, qui est bornée à 0 (un `>= 0` serait toujours
  vrai). Le branchement dans `main.cpp` n'est pas testé (visuel).
- **`Mobile` (`test_Mobile.cpp`)** : repris quasiment tel
  quel des tests d'`Escouade` dont le code a été déplacé (même fonction
  d'aide `simuler(mobile, carte, duree)`, même tolérance
  `verifierPointProche`, mêmes principes — des résultats, pas des
  trajectoires) : `getCentre`/`getTaille` donnés au constructeur, aucun
  déplacement sans ordre, avance de `vitesse * dt` sur un tic, atteint
  le point exact en terrain dégagé, contourne un mur, ignore un ordre
  vers un mur, `arreter()` stoppe un trajet en cours,
  `directionVoulue()` vide à la création et une fois arrivé, pointe
  vers la destination après `allerVers`.
- **Soin** (`test_Personnage.cpp`, `test_Escouade_skills.cpp`) :
  - `Personnage::recevoirSoin` : points de vie rendus, plafond au maximum,
    sans effet sur un mort.
  - `choisirCible(TypeSkill::Soin, 3, {})`, observé par la fonction d'aide
    `placeAlliePourSoin` (extrait la place du `CibleAllie`) : personne de
    blessé, un seul blessé, le moins de points de vie, égalité (plus
    petite place), morts ignorés, tous morts. Les blessés sont créés par
    `recevoirDegats` à la position exacte d'une place de la formation
    (escouade en `(50, 50)`, orientée vers `+x` : places en `(85, 35)`,
    `(85, 85)`, `(35, 35)`, `(35, 85)`).
  - Soin de la place 3 hors combat : allié à 50 px (à portée), allié à
    environ 70,7 px (hors de portée, mis en attente), pleine vie sans
    recharge consommée, recharge juste après le lancement, soigneur mort.
- **Skill en attente** : comme pour les autres tests d'`Escouade`, des
  résultats plutôt que des trajectoires, sur une carte dégagée dédiée
  (`carteDegagee10x10`). Soin : le soigneur marche, soigne, puis revient à
  sa place (`proieDuMonstre` près de sa place) ; annulation si la
  cible ou le soigneur meurt (sans recharge : un nouveau soin part
  aussitôt) ; un ordre de déplacement n'annule rien ; un seul skill en
  attente (un skill de dégâts à portée est refusé). Dégâts : **un tic qui
  rapporte 25 ou plus prouve que le skill est parti**, l'attaque de base
  des 4 personnages plafonnant à 20 par tic ; annulation par
  `deverrouiller`, par un changement de cible, et pas par un
  `verrouiller` sur la même cible ; un soin est refusé pendant l'attente
  d'un skill de dégâts. Le branchement des touches dans `main.cpp` n'est
  pas testé (visuel).

## Barre de skills (étape 48)

- **`test_Escouade_skills.cpp`, `etatDuSkill`** : place invalide (0 et 5) → vide ;
  type de chaque place ; skill prêt (progression 1, temps 0, lanceur
  vivant) ; juste après un déclenchement (la provocation de la place 2 part
  même sans monstre : progression 0, temps `> 0`) ; lanceur mort (4 coups
  mortels au même point).
- **`test_BarreDeSkills.cpp`** : lettre de chaque type ; secondes affichées
  (4,2 → 5, 1,0 → 1, 0,01 → 1, 0 → 0) ; quartier : fraction nulle → aucun
  sommet, premier sommet = centre, dernier = midi, aire = fraction × côté²
  pour k/8 (formule du lacet, tolérance 0,5), sommets dans le carré.
- **Non testé** : `dessinerBarreDeSkills` (couleurs, positions, chiffre,
  contour), vérifié à la main dans le jeu.

## Agresseurs (étape 40)

- **`test_Escouade_agresseurs.cpp`** : enregistrement (coup `> 0`, coup à 0, doublon
  rafraîchi, aucun vivant), oubli (3 s, nouveau coup, `attaquer` retire
  l'absent et met à jour le cercle), et déplacement vers l'agresseur sur
  une carte dégagée (`carteDegageeEntouree`, `simulerAvecMonstre` : `attaquer` puis
  `mettreAJour` à chaque tic, le monstre étant obstacle) : mêlée qui
  s'approche, agresseur hors laisse, ancre en mouvement, oubli. Observé par
  `proieDuMonstre` du monstre.
- **Piège** : `recevoirDegats` enregistre maintenant un agresseur. Les tests
  qui blessent un personnage avec un monstre « fantôme » puis simulent
  longtemps appellent `escouade.attaquer({}, carte)` pour l'oublier,
  sinon un personnage se met en marche vers ce fantôme.
- **`test_Autodefense.cpp`** : `choisirAgresseurAViser` seule (aucun
  agresseur, hors laisse, laisse mesurée depuis la place, pile à la
  laisse, le plus proche, égalité).

## Provocation (étape 41)

- **`test_Skill.cpp`, `test_Personnage.cpp`** : un `Skill` de type
  `Provocation` (type, portée, puissance = durée, recharge) ; un
  personnage qui en expose le type et le déclenche.
- **`test_Escouade_agresseurs.cpp`** : `provoquer` (enregistrement, doublon dont la
  durée est remise au maximum), fin d'une provocation (durée, mort du
  provocateur, monstre absent de `attaquer`) ; `proieDuMonstre` (sans
  provocation, monstre provoqué, autre monstre non concerné, provocateur
  mort) ; `recevoirDegats` frappe le même personnage que `proieDuMonstre` ;
  touche 2 (monstres dans le rayon seulement, aucun dégât, refus sans
  monstre et recharge intacte, recharge de 10 s, lanceur mort, skill en
  attente). Escouade en (200, 200) : le lanceur est en (235, 235).
- **Renommage** : `personnageLePlusProche(point)` est devenu
  `proieDuMonstre(idMonstre, point)` ; les tests anciens passent
  l'identifiant 1, sans provocation.

## Skill de zone (étape 42)

- **Signature de `Skill`** : le constructeur reçoit un rayon de zone
  (0 pour les skills à cible unique) ; tous les `Skill(...)` des tests
  existants l'ont reçu. `test_Skill.cpp` vérifie `getRayonZone` et que
  rayon et portée sont indépendants ; la provocation a désormais une
  portée de 0 et une zone de 150, et le lanceur se prend pour cible.
- **`test_Personnage.cpp`** : `zoneDuSkill` (centre sur le personnage,
  rayon du skill, 0 sans skill).
- **`test_Escouade_skills.cpp`, provocation** : la zone se mesure depuis le centre
  du lanceur, monstre compris : touché à 165 px de distance entre centres
  (150 + 15), pas à 166.
- **`test_Escouade_skills.cpp`, skill de zone** (lanceur de la place 4 en
  (185, 235), monstres de rayon 15, donc touchés jusqu'à 75 px de
  distance entre centres) : `skillEstPret` (prêt, place invalide, mort,
  recharge de 8 s) ; `rayonZoneDuSkill` ; `declencherSkillDeZone`
  (monstres touchés et épargnés, bord à bord exact, zone vide qui part
  quand même, portée non lue, refus pendant la recharge, lanceur mort,
  skill en attente, place invalide ou sans skill `Zone`, recharge de
  l'attaque de base relancée) ; `declencherSkill` sans point refuse un
  skill `Zone` et `choisirCible` renvoie vide.
- **`test_Escouade_vue.cpp`, ligne de vue du skill de zone** (carte 15 × 10
  avec un mur vertical épais de 40 px, `carteAvecMurVertical`, contournable
  par le haut ou le bas) : point à portée et en vue (départ immédiat),
  derrière le mur (attente, sans recharge), dans le mur (refus, sans
  attente), lanceur qui contourne puis lance ; monstre dans la zone mais
  derrière le mur épargné, monstre du même côté touché.
- **`test_Escouade_vue.cpp`, ligne de vue des personnages** (étape 46) :
  `carteOuverte()` (4000 × 4000 px sans mur) est passée aux appels
  existants de `declencherSkill` et `choisirCible` ; les nouveaux tests
  utilisent `carteAvecMursFins` (300 × 300 px en cases de 2 px, deux murs
  fins posés hors des bandes de repli de la formation) ou
  `carteAvecMurVertical`. Autodéfense (monstre caché non frappé, visible
  frappé, le plus proche caché → l'autre) ; combat verrouillé (ancre qui
  continue d'avancer ou s'arrête, seule la place 4 laissée en vie) ;
  agresseur caché ; skill de dégâts avec et sans verrou ; soin caché ;
  provocation avec un monstre caché. « Dès que la vue est obtenue » est
  simulé en repassant une carte sans mur (un personnage ne contourne pas
  un mur de 2 px : son A* ignore sa taille). Un test existant (deux
  personnages, deux monstres) a été décalé pour que les monstres soient
  dans la carte : hors carte = mur, donc jamais en vue.
- **`test_TileMap.cpp`, `ligneDeVue`** *(supprimés à l'étape 73 : la fonction est sortie de
  `TileMap`, voir « Une seule ligne de vue (étape 73) » plus bas)* : terrain dégagé, mur
  entre les points, points identiques, coin de deux murs en diagonale (bloqué), segment à 1
  px d'un mur (libre), arrivée dans un mur.
- **Non testé** : le mode visée et le cercle, qui vivent dans `main.cpp`
  (vérifiés à la main dans le jeu).

## Hygiène du code (étape 49)

- **`test_EntiteVue.cpp`** : `trouverEntite` (monstre trouvé, premier de la
  liste, identifiant absent, liste vide).
- **`test_BarreDeVie.cpp`** : `positionBarreDeVie` seule. `dessinerBarreDeVie`
  n'est pas testée (dessin, vérifié dans le jeu).
- **`[[nodiscard]]`** : 39 appels de tests (38 dans `test_Escouade.cpp`, un
  dans `test_Skill.cpp`) ont été écrits `static_cast<void>(...)` à
  l'étape 49 ; l'étape 50 les a tous revus. 30 sont devenus une
  vérification de la valeur renvoyée : liste de `Coup` vide pour une
  provocation, un skill refusé ou mis en attente, un seul `Coup` pour une
  zone qui touche, `== 25` pour `Skill::declencher`. Les 9 restants sont
  des `attaquer` appelés seulement pour avancer l'état (agresseurs
  oubliés, boucles de simulation) : les coups ne sont pas l'objet du
  test, et un commentaire le dit.
- **Avertissements** : pas activés sur la cible `tests` (macros de Catch2).

## Réorganisation des tests (étape 50)

- **Tests de rotation** (`test_Escouade_deplacement.cpp`) : aucun
  accesseur ne donne l'orientation, et on n'en a pas ajouté. L'aide
  `orientationDeLaFormation` la déduit de `getPlacesDeFormation()[0] -
  getCentre()` : à l'orientation 0, ce décalage vaut (25, -25), soit
  -45°, donc orientation = angle du décalage + 45°. Valable en terrain
  dégagé, où aucune place n'est repliée. Quatre cas : la formation finit
  tournée vers la direction de marche (90° vers le bas) ; plus court côté
  (vers le haut, l'orientation devient négative, puis -90°, au lieu de
  +270°) ; vitesse bornée (après 0,1 s, entre 0° exclu et 36°) ; la
  rotation se termine après l'arrêt de l'ancre (destination à 10 px,
  l'ancre arrêtée avant la fin de la rotation, qui finit quand même).
  Piège : `simuler(escouade, carte, 0.1f)` ne fait que 5 tics
  (`0.1f / (1.0f / 60.0f)` est juste sous 6) ; l'aide
  `simulerUnDixiemeDeSeconde` en fait 6.
- **`provoquer` à une place invalide** (0 et 5) : aucune provocation, la
  mise à jour suivante ne plante pas, `proieDuMonstre` reste défini.


## Partie et Controleur (étape 52)

- **`test_Partie.cpp`** (7 cas, +4 à l'étape 54) : niveaux construits dans le test
  (`niveauDegage`, 15 × 15 cases sans mur, escouade en `(200, 200)`, centre
  en `(210, 210)`). Clic sur un monstre (`Monstre`, verrou posé, identifiant
  2 pour le second) ; clic au sol (`Sol`, verrou levé) ; monstre tué retiré
  et verrou levé ; aucun monstre mort dans le vecteur après chaque
  `mettreAJour` ; un monstre approché poursuit et frappe (observé par
  `getAgresseurs`) ; un skill demandé ne part qu'à `mettreAJour` (sa recharge
  démarre) ; escouade vaincue par 30 monstres : `cliquer` renvoie `Ignore`,
  les demandes sont sans effet.
- **Double frappe** (étape 53) : un skill et `attaquer` dans la même image
  ne font pas frapper deux fois le même personnage. Monstre à portée du
  mêlée avant-gauche (skill de 25) et des deux tireurs (5 chacun) :
  `Monstre::getPointsDeVie` vaut 100 − 25 − 5 − 5, le mêlée ne frappant
  pas avec son attaque de base puisque le skill en relance la recharge.
- **`test_Controleur.cpp`** (7 cas) : 4 entre en visée ; 4 n'entre pas si le
  skill n'est pas prêt (recharge après un lancer) ; un clic en visée lance
  (rien ne part avant `mettreAJour`) et sort de la visée ; clic droit, Échap
  et 4 annulent sans consommer la recharge ; un bouton tenu à l'entrée en
  visée ne déplace pas l'escouade, ni pendant la visée ni après le lancer,
  mais un nouvel appui au sol la déplace ; la mort du lanceur (défaite)
  fait sortir de la visée au `debutImage` suivant ; un clic au sol arme le
  déplacement continu, un clic sur un monstre le désarme (le monstre est à
  l'opposé du curseur, l'escouade avance vers lui et non vers le curseur).
  Depuis l'étape 54, `Controleur` n'a plus de référence à la `Partie` : les
  tests construisent `Controleur controleur;` et passent `partie` à chaque
  appel (même contenu).
- **Non testé** : `dessinerApercu` (dessin, vérifié dans le jeu) et la
  traduction des événements dans `main.cpp`.

## PointsDeVie et declencherSansCible (étape 53)

- **`test_PointsDeVie.cpp`** (8 cas) : départ plein ; dégâts qui retirent des
  points ; borne à 0 ; dégâts de 0 sans effet ; soin ; soin borné au
  maximum ; soin d'un mort sans effet ; `getRatio` (plein, moitié, mort).
  Les `assert` (montant négatif, maximum nul) ne sont pas testés : un
  `assert` qui échoue arrête le programme.
- **`test_Skill.cpp`** (+2 cas) : `declencherSansCible` renvoie la
  puissance sans portée ; relance la recharge, refuse tant qu'elle court,
  puis repart.
- **`test_Partie.cpp`** (+1 cas) : la double frappe (voir plus haut).
- Les tests existants de `Personnage`, `Monstre`, provocation et zone n'ont
  pas été modifiés. 377 tests.

## Générateur aléatoire (étape 56)

- **`tests/aides.hpp`** : `s_graineDeTest` (12345) et `generateurDeTest()`
  (un `Generateur` neuf à graine fixe). Chaque test qui en a besoin crée le
  sien (`Generateur generateur = generateurDeTest();`) et le passe par
  référence : jamais de générateur partagé entre tests, donc aucune
  dépendance à l'ordre d'exécution. Les aides `simuler`, `attaquerCible`
  et les petites fonctions de simulation en créent un local. Les tests de
  `Partie` passent `s_graineDeTest` au constructeur.
- **Règle : un test qui ne mesure pas le rythme ne doit pas dépendre du
  hasard.** Il vérifie « au moins un coup », une durée plus généreuse que
  la borne haute de la recharge (`Monstre.frapper` attend 1,5 s, pas 1,2 s :
  le maximum est 1,3 s), ou un résultat sans rapport avec le tirage. La
  suite complète a été lancée avec 7 graines différentes (1, 7, 99, 2024,
  31337, 777777, 12345) : tout passe. Lancer `tests.exe` deux fois de suite
  donne exactement le même résultat.
- **`test_Skill.cpp`** (+4 cas) : `min == max` donne exactement cette valeur
  et ne touche pas à l'état du générateur (comparé par `==`) ; sur 200
  déclenchements, chaque recharge est dans `[min, max]` et elles ne sont pas
  toutes égales ; deux générateurs de même graine donnent la même suite de
  recharges ; `relancerRecharge` tire dans l'intervalle.
- Les constructions de `Skill` reçoivent `min == max` (ancienne valeur) ;
  les appels qui descendent le générateur en reçoivent un. 396 tests.

## Armure (étape 55)

- **`test_Defense.cpp`** (6 cas) : armure 0 = montant inchangé ; 100
  d'armure = moitié ; arrondi au plus proche (10/50 → 7, 5/20 → 4,
  15/20 → 13, 25/20 → 21) ; plancher à 1 ; montant 0 → 0 ; la réduction
  n'atteint jamais 100 % avec une armure énorme.
- **`test_Personnage.cpp`** (+2 cas) : un mêlée (armure 50) perd 7 sur un
  coup de 10, un tireur 10 ; le soin n'est pas réduit par l'armure.
- **`test_Monstre.cpp`** (+3 cas) : armure 20, un coup de 5 retire 4 ; armure
  0, un coup de 5 retire 5 ; un coup de 1 retire 1 et réveille toujours le
  monstre.
- **Tests existants adaptés** : les constructions directes de `Personnage`
  et de `Monstre` reçoivent une armure de 0 (leurs attentes sont
  inchangées). Dans les tests d'`Escouade`, les mêlées ont maintenant
  50 d'armure : les coups « pour tuer » passent de 100 à 200 dégâts (200 ×
  2/3 = 133 ≥ 100), et le test d'égalité du soin frappe le mêlée avec 30
  au lieu de 20 (30 × 2/3 = 20, comme le tireur). 392 tests.

## Défaite et recommencement (étape 54)

- **`test_Partie.cpp`** (+4 cas) : `estPerdue` faux au départ ; faux tant
  qu'un personnage survit (observé avec `cerclesDesVivants().size() == 1`
  pendant que 30 monstres vainquent l'escouade) puis vrai quand les 4 sont
  morts ; une partie perdue se fige (3 s de `mettreAJour` ne changent ni la
  position ni les points de vie des monstres, qui sans cela rentreraient à
  leur poste) ; une `Partie` réaffectée avec un niveau neuf n'est plus
  perdue, a 6 monstres à 100 points de vie et l'escouade au départ (vérifie
  aussi que `Partie` se déplace). Aide `groupeDeMonstres()` (30 monstres).
- **Non testé** : `dessinerEcranDeFin` (dessin, vérifié dans le jeu) et
  la touche R dans `main.cpp`. 381 tests.

## Temps d'exécution des attaques (étape 57)

- **`aides.hpp`** : `attaquerJusquAuDepart` appelle `attaquer(s_dureeTic, …)`
  pendant 0,2 s (plus que l'exécution de 0,15 s, moins que la plus petite
  recharge, 0,3 s) : chaque personnage qui atteint un monstre l'engage et le
  frappe une fois, au plus. `attaquerCible` s'en sert. Sans `mettreAJour`,
  personne n'est « en mouvement ».
- **Tests adaptés, pas supprimés** : les tests d'`Escouade` qui comptaient les
  coups d'un seul `attaquer` passent par `attaquerJusquAuDepart` ; l'ancien
  `Personnage::frapper` / `Monstre::frapper` devient un helper local
  `frapperEnEntier` (portée, engagement, avancée jusqu'au départ) ; « un
  personnage déjà à portée suit son emplacement » redonne l'ordre à chaque
  image (bouton tenu), qui annule le coup que le tireur vient d'engager ;
  le test de `Partie` « skill et attaquer » laisse passer 0,25 s.
- **`test_Skill.cpp`** (+7) : `commencer` seulement depuis prêt, départ unique
  en fin d'exécution, recharge tirée au départ, `annuler` sans recharge,
  exécution 0 = départ immédiat, `declencher` refusé hors prêt.
- **`test_Mobile.cpp`** (+3) : `estEnMouvement` à l'arrêt, en marche, après
  `arreter`, bloqué par un obstacle.
- **`test_Personnage.cpp`** (+6) et **`test_Monstre.cpp`** (+6) : engagement
  (immobilité, portée, mort), départ unique, immobilité du monstre pendant
  l'exécution, annulation sans recharge, recharge après le départ, skill qui
  annule le coup.
- **`test_Escouade_combat.cpp`** (+10) : aucun coup pendant la marche (l'ordre
  n'est donné qu'une fois, sinon l'annulation par l'ordre masquerait la règle
  d'immobilité : vérifié en retirant la règle, le test échoue), coups qui
  reprennent à l'arrêt, départ à 0,15 s, ordre qui annule sans recharge,
  coup engagé qui part malgré l'éloignement, disparition de la cible, mort de
  l'attaquant, skill qui annule, `recevoirCoup`, `placeDeLaProie`.
- **`test_Partie.cpp`** (+2) : un monstre ne bouge pas pendant son exécution,
  puis reprend la poursuite. Non couvert à ce niveau : le coup sur une cible
  éloignée et l'annulation par mort (les points de vie des personnages ne sont
  pas exposés) ; couverts dans `test_Monstre.cpp` et `test_Escouade_combat.cpp`.
- **`test_Eclaircir.cpp`** (2) : mélange au blanc, opacité conservée.
  432 tests.

## Temps d'activation des skills (étape 58)

- **`aides.hpp`** : `attendreLeDepartDesSkills` appelle `attaquer` par tics
  pendant 1 s (plus que la plus longue activation, 0,8 s), sans `mettreAJour`
  (les recharges ne courent pas), et écarte les coups de 5 dégâts (attaques de
  base) pour ne garder que ceux des skills. `declencherEtAttendre` et
  `declencherZoneEtAttendre` déclenchent puis attendent le départ ;
  `simulerAvecAttaque` fait `attaquer` puis `mettreAJour` à chaque tic, dans
  l'ordre d'une image de `Partie`.
- **Tests adaptés, pas supprimés** : les tests d'`Escouade` qui comptaient les
  coups de skill à l'appui passent par ces aides ; quelques tests ont été
  retouchés à la main (skill en attente derrière un mur, soin en attente,
  changement de cible, coup de base annulé par un skill, `Partie`).
  `Escouade::mettreAJour` n'a plus de paramètre `Generateur&`.
- **`test_Personnage.cpp`** : les tests de `declencherSkill` deviennent
  `lancerSkillEnEntier` (engager puis avancer jusqu'au départ) ; nouveaux
  tests avec un skill d'activation construit dans le test : puissance rendue
  une seule fois, skill non prêt et non réengageable pendant l'activation,
  engagement sans immobilité, `annulerSkill` sans recharge, `annulerAttaque`
  qui n'annule pas un skill, aucune attaque de base pendant l'activation,
  recharge de base relancée au départ.
- **`test_Escouade_skills.cpp`** (+9) : avec les vraies valeurs ; départ à
  0,3 s, second appui refusé, lanceur immobile et ordre de déplacement qui
  n'annule pas, mort du lanceur ou de la cible (monstre, allié) sans recharge,
  zone et provocation évaluées au départ, deux lanceurs simultanés.
  Non couvert : un soin dont la cible remonte à pleine vie pendant l'activation
  (`Escouade` n'expose aucun moyen de soigner de l'extérieur) ; à vérifier en jeu.
- **`test_BarreDeSkills.cpp`** (+1) : `couleurDuContour`. 449 tests.

## Ligne de vue des monstres et nettoyage de Skill (étape 62)

- **`test_Skill.cpp`** (−6 cas) : `declencher` et `declencherSansCible`
  n'existent plus. L'aide locale `lancer(skill, generateur)` fait
  `commencer` + `avancer(0)` (temps d'exécution nul) ; les cas de recharge
  (refus pendant la recharge, progression, recharge fixe sans tirage,
  intervalle, même graine) l'utilisent. Retirés : les cas qui ne vérifiaient
  que la portée dans `declencher` (couverte par `estAPortee`) et les doublons.
- **`test_Monstre.cpp`** (+7 cas), avec `carteAvecMur` (mur de 4 cases : un
  monstre en (100, 100) ne voit pas (250, 100), voit (200, 260)) et
  `carteAvecPilier` (mur en case (1, 1) ; monstre en (30, 60) ou (25, 65)) :
  proie derrière un mur non détectée, même distance en vue détectée,
  poursuite qui continue sans la vue, monstre touché actif sans vue,
  `peutAtteindre` (portée ET vue), `engagerAttaque` refusée si cachée et
  acceptée en vue, poursuite qui ne s'arrête pas à portée mais cachée. Les
  appels existants à `engagerAttaque` reçoivent `carteOuverte()`. Pas de
  test du choix entre deux personnages : la cible de frappe est la proie
  (limite acceptée, voir `docs/monstre.md`).


## Victoire (étape 63)

- **`test_Partie.cpp`** (+6 cas) : `estGagnee` faux avec un monstre (et
  `monstresRestants` à 1) ; vrai quand le seul monstre est tué (et pas
  perdue, compteur à 0) ; une partie perdue n'est pas gagnée (monstres
  restants) ; sans monstre au départ la partie est gagnée ; le compteur baisse
  quand un monstre meurt (deux monstres, un hors de portée) ; une partie
  gagnée se fige (un ordre de déplacement n'a aucun effet).
- **`test_Controleur.cpp`** : `niveauDegage({})` devient
  `niveauDegage({s_monstreLointain})`, un monstre hors de détection : une
  partie sans monstre est gagnée donc figée, ce qui bloquait les cas qui
  font avancer la partie.
- **Non testé** : `dessinerEcranDeFin`, le compteur et la touche R dans
  `main.cpp` (vérifiés dans le jeu). Le cas « derniers monstres et derniers
  personnages morts dans la même image » n'est pas reproduit à coup sûr ; la
  règle `estGagnee = vide && !estPerdue` le garantit par construction. 503
  tests.

## Écrans, boutons, réglages et vues (étape 65)

- **`test_Reglages.cpp`** (13 cas) : aller-retour écriture puis lecture, format
  exact (`largeur=1200\nhauteur=800\nmode=fenetre\n`), flux vide, clé inconnue
  et ligne mal formée ignorées, espaces et fins de ligne Windows tolérés,
  valeur invalide ou suivie de texte (`1600px`) = défaut, résolution hors
  liste ou mélange de deux résolutions = 1200 × 800, et le filtrage par la
  taille du bureau (largeur ET hauteur). Les flux sont des `std::istringstream`
  et `std::ostringstream` : aucun fichier sur le disque.
- **`test_TailleDeVue.cpp`** (3 cas) : 1200 × 800 donne 1200 × 800, 1920 × 1080
  donne environ 1422,2 × 800 ; `vueDInterface` a son coin haut gauche en (0, 0).
- **`test_ListeDeBoutons.cpp`** (6 cas) : le centre de chaque bouton le désigne,
  la colonne est centrée, l'espace entre deux boutons et les points hors de la
  colonne ne désignent rien, la disposition suit le `centre` reçu. Les tests
  placent leurs points avec `rectangleDuBouton`, sans recopier la géométrie.
- **`test_EcranMenu.cpp`** (4 cas) et **`test_EcranOptions.cpp`** (12 cas) :
  la fonction `cliquer` de chaque écran, sans fenêtre : Commencer et Options
  empilent un écran, Quitter quitte, un clic hors des boutons ne fait rien ;
  options : bascule du mode (et retour à la résolution choisie), flèches de
  résolution (suivante, précédente, bouts de liste, clics successifs), milieu
  de ligne sans effet, ligne inactive en plein écran (puis réactivée) ou sans
  résolution disponible, Retour. Le menu appelle `sf::VideoMode::getDesktopMode()`
  pour le bouton Options : le test suppose un bureau (Windows).
- **Non testé** : `Application` (fenêtre, boucle, `ouvrirFenetre`, lecture et
  écriture du fichier `reglages.ini`), la conversion des événements SFML des
  écrans et leur dessin (vérifiés dans le jeu). 553 tests.

## Cible de frappe des monstres (étape 66)

- **`Escouade::candidatsDeFrappe`** (`test_Escouade_combat.cpp`) : sans
  provocation tous les vivants avec leur place ; morts exclus ; provoqué,
  seulement le provocateur ; provocateur mort, tous les vivants ; personne de
  vivant, liste vide. `proieDuMonstre` renvoie la place dans `id`
  (`placeDeLaProie` n'existe plus).
- **`Monstre`** (`test_Monstre.cpp`) : les anciens tests passent par des aides
  (`ecoulerTemps`, `engager`) où la cible est à la fois la proie et l'unique
  candidat ; les nouveaux passent proie et candidats : proie atteignable
  préférée à un candidat plus proche, proie cachée et autre candidat en vue
  frappé, candidat atteignable le plus proche, aucun atteignable (refus et
  poursuite), arrêt en poursuite à cause d'un autre candidat, coup engagé sur
  la place choisie. Piège : `pointCible` a un rayon nul, la portée de 15 px
  devient alors une distance de centres de 30 px.
- **`Partie`** : un monstre provoqué n'engage que sur le provocateur (cible
  engagée lue par `Monstre::cibleEngagee`). Le cas « proie cachée, autre
  personnage en vue » n'est pas testé au niveau de `Partie` : avec la formation
  fixe (personnages à 50 px) et la portée de 15 px, aucun mur ne cache le plus
  proche en laissant un autre à portée ; il est couvert dans `test_Monstre.cpp`.

## Places d'attaque et séparation (étape 67)

- **`test_Encerclement.cpp`** : nombre de places selon le rayon (au moins 1),
  première candidate dans la direction donnée, alternance gauche/droite, corde
  entre voisines `>=` l'écart minimal (rayons de 13 à 140 px), position sur
  l'anneau, angle libre le plus proche de l'angle actuel (par le plus court
  côté), angles refusés sautés, égalité départagée par l'ordre, aucun angle libre.
- **Séparation** (`test_Escouade_deplacement.cpp`) : un personnage cloué (activation
  jamais terminée, car on n'appelle pas `attaquer`) est croisé par les autres lors
  d'un demi-tour : le chevauchement dure moins d'une seconde (le test vérifie qu'il y
  en a un, sinon il ne prouve rien) ; après un demi-tour, les quatre rejoignent leurs
  places à 2 px et restent à 34 px ; un mort ne pousse pas (la place du personnage
  suivant est atteinte à 0,05 px, ce qu'un mort qui poussait empêcherait) ; un
  personnage en activation n'est pas poussé ; la poussée ne fait entrer personne dans
  un mur (allers-retours collés au mur). Piège : tuer un personnage avec
  `recevoirDegats(1, origine, 200)` enregistre `origine` comme agresseur, et les
  autres personnages vont vers lui ; il faut une origine lointaine.
- **Places de formation distinctes** (`test_Escouade_deplacement.cpp`) : ancre collée
  à un mur, les quatre places sont à 34 px ou plus les unes des autres et hors mur ;
  après 2 s les personnages ne se chevauchent pas.
- **Places d'attaque** (`test_Escouade_places.cpp`, carte ouverte, les monstres sont
  passés comme obstacles) : deux mêlées en combat verrouillé arrivent de côtés
  différents (angle d'au moins 30°), à portée et à 34 px l'un de l'autre ; les tireurs
  s'arrêtent dès qu'ils sont à portée (pas au contact) sur des angles différents ;
  l'ordre autour du monstre est gardé quand il s'éloigne ; au changement de cible
  chacun prend sa place autour du nouveau monstre ; près d'un mur personne n'entre
  dedans ; un agresseur sans verrou, un mêlée déjà à portée qui frappe et l'autre qui
  le rejoint : après 2 s les deux sont à portée sans se superposer, et le point visé
  du second est sa place d'attaque (à environ 43 px du centre), pas le centre ;
  monstre dans une case fermée : tous les points visés sont son centre.
  `pointVise` : place de formation hors combat, vide pour une place invalide.
- **Pas de test automatique** de l'oubli de la place d'attaque (elle n'a pas d'accès
  public) : seul le comportement visible (côtés conservés, réattribution au
  changement de cible) est vérifié.
- **`choisirAgresseurAViser`** renvoie une `EntiteVue` (identifiant et cercle) pour
  que la mémoire des places connaisse le monstre : ses tests lisent `cercle.centre`.

## Étape 68 : chemins qui contournent les obstacles

- **`test_Chemin.cpp`** : `trouverChemin` sans coût identique à avant, un coût
  vide ou nul ; `coutDesObstacles` (case touchée à 19 px, case juste hors de
  portée à 21 px, hors carte, liste vide, lambda qui survit à la liste d'origine) ;
  contournement dans un passage large, traversée d'un passage bouché, départ et
  arrivée jamais interrogés (fonction « espion ») ; `lisserChemin` sans cercle,
  raccourci refusé, raccourci accepté à plus de la somme des rayons.
- **`test_Cercle.cpp`** : `distancePointSegment` (perpendiculaire, extrémité la
  plus proche, segment réduit à un point).
- **`test_Mobile.cpp`** : contournement par `allerVers` (aucun segment à moins de
  la somme des rayons, destination atteinte), `cheminRestant`, `estBloque`
  (0,3 s, pas avant ; jamais arrêté ; jamais en marche normale ; remise à 0 par
  `allerVers` et `allerToutDroit`).
- **`test_Deplacement.cpp`** : glissement tangent (poussée droit contre un
  cercle, mur qui refuse la tangente, autre cercle qui la refuse, pas de
  tangente si un axe a passé). `deplacementBloque` n'est pas modifié.
- **`test_Monstre.cpp`** : contournement d'un autre monstre, blocage sans liste à
  contourner, personnage entre le monstre et sa proie non contourné (le monstre
  le frappe), retour bloqué qui recalcule. `ecoulerTemps` reçoit `obstacles` et
  `aContourner` (vides par défaut).
- **`test_Personnage.cpp`** : ligne droite gardée loin d'un obstacle, contournement
  sur la ligne droite, blocage sans liste à contourner, recalcul sur blocage
  (deux ouvertures dans un mur, l'une bouchée après le premier chemin : sans le
  recalcul, le personnage reste devant le bouchon).
- **`test_Escouade_deplacement.cpp`** : un personnage contourne un monstre non
  visé (les quatre passent, y compris le tireur arrière aligné), aucune place de
  formation dans un monstre.
- **Mutations vérifiées à la main** (le test échoue si on retire la règle) :
  recalcul sur blocage, test « place dans un monstre », glissement tangent.
- Un test existant a dû changer : `Mobile.estEnMouvement reste vrai quand le
  mobile est bloque…` compare maintenant la position à 0,5 px près, car poussé
  droit contre un cercle le mobile fait des corrections minuscules au lieu
  d'être figé.

## Étape 69 : retour au poste

- **`test_Monstre.cpp`** (8 tests de plus) : en retour, une proie détectable ou revenue
  dans le territoire ne relance pas la poursuite (`distanceApres > distanceAvant`) ; un
  monstre touché ou alerté en retour ne perd rien et ne se réveille pas ;
  `engagerAttaque` renvoie faux en retour ; redétection normale une fois arrivé au
  poste ; `estCiblable` (vrai en garde et en poursuite, faux en retour) ; régénération
  (de 10 % à 100 % en 2 s, plafonnée au maximum, présente à 60 images par seconde,
  absente hors retour). Les valeurs codées en dur de l'étape 68 (400 px, 120 px/s) sont
  remplacées par `Monstre::s_rayonTerritoire` (publique pour cela).
- **`amenerEnRetourHorsTerritoire(monstre, carte, degats)`** : l'aide accepte des dégâts
  à infliger *avant* le retour, puisqu'en retour le monstre est invulnérable.
- **`test_Partie.cpp`** (5 tests de plus) : un couloir de 2400 × 600 px où l'escouade
  fuit vers l'est (`niveauDeLaFuite`, `attirerHorsDuTerritoire`) : le monstre la suit
  jusqu'à 600 px de son poste, abandonne et rentre. Un clic sur lui donne `Sol` ; une
  provocation ne l'enregistre pas ; les personnages le touchent sans le traverser (le test
  exige un vrai contact : écart minimal égal à la somme des rayons, à 1 px près) ; le
  verrou posé juste avant l'abandon est levé ; l'exploit de la laisse (escouade immobile
  juste au-delà du territoire pendant 25 s) : la vie ne baisse jamais pendant le retour,
  jamais en poursuite pendant le retour, et le monstre arrive plein à son poste.
- **Mutations vérifiées à la main** (le test échoue si on casse la règle) : clic sans
  `estCiblable`, liste physique à la place de la liste des cibles dans `resoudreLesCoups`,
  liste des cibles à la place de la liste physique dans `escouade.mettreAJour`, verrou non
  levé. La dernière a obligé à renforcer le test de blocage (5 s et destination plus
  loin, sinon aucun personnage n'atteignait le monstre).
- **Pas de test automatique de l'autodéfense contre un monstre qui rentre** : ses coups
  seraient de toute façon sans effet (invulnérabilité), la liste des cibles est vérifiée
  par la provocation (liste observable). Pas de test du dessin (alpha sous F3).
- **Limite du test d'`engagerAttaque` en retour** : un monstre en retour est presque
  toujours en mouvement, donc `engagerAttaque` renverrait faux même sans la garde ; le
  test vérifie la règle, pas sa seule cause.

## Vision et brouillard de guerre (étape 72)

- **`test_ChampDeVision.cpp`** (9 cas) : salle vide (toutes les cases à distance `<= rayon`,
  aucune au-delà) ; rayon 0 (l'origine seule) ; un mur est révélé, la case derrière non ; ombre
  d'un pilier (4 cases cachées derrière, cases des côtés visibles) ; porte fermée (révélée, salle
  derrière cachée) et porte ouverte (salle visible) ; origine dans un coin ou sur un bord avec un
  rayon plus grand que la carte (rien hors carte, toute la carte vue) ; **symétrie** sur un plan
  13 × 13 à piliers et à coins (rayon 6) et sur trois cartes générées (graines 1, 2 et 3, rayon
  8, portes une sur deux ouvertes). L'aide `verifierSymetrie` compare toutes les paires de cases
  libres à distance `<= rayon` et compte aussi les paires cachées (`cachees > 0`), pour prouver
  que le test éprouve quelque chose. Une assertion par paire coûterait des centaines de milliers
  d'assertions Catch2 : les asymétries sont collectées dans un vecteur, vérifié vide à la fin.
  Mutation vérifiée à la main : sans le critère « centre dans l'intervalle », les deux tests de
  symétrie échouent.
- **`test_Vision.cpp`** (8 cas) : tout inconnu à la construction ; le champ d'une source est
  visible ; rayon en cases arrondi vers le bas (2,9 cases = 2) ; une case quittée devient
  explorée et ne redevient jamais inconnue, même sans source ; union de deux sources ; hors
  carte = inconnue ; `estVisible` en pixels (coins de case, case voisine, hors carte) ; un mur
  arrête la vue.
- **`test_Escouade_deplacement.cpp`** (+3) : `cerclesDeVision` (4 cercles de rayon 320 aux
  centres de `cerclesDesVivants`, un mort n'y figure pas, vide si tous sont morts).
  **`test_Personnage.cpp`** (+1) : `getRayonVision`. Les ~45 constructions de `Personnage` des
  tests reçoivent le nouveau paramètre (`s_rayonVisionDeTest`, dans `aides.hpp`).
- **`test_Partie.cpp`** (+6) : les niveaux sont des salles de 30 × 11 cases. Un monstre voisin
  est cliquable avant tout `mettreAJour` (la salle de départ est visible dès la construction) ;
  un monstre à 610 px ou derrière un mur : `Sol`, pas de verrou ; le même monstre devient
  `Monstre` après que l'escouade s'est approchée (vue à 280 px, sans détection à 200 px) ; une
  porte à 800 px : `Sol`, puis `Objet` une fois approchée.
- **Test adapté** (`test_Controleur.cpp`, « un clic au sol arme le deplacement continu… ») : la
  première marche passe de 30 à 10 tics, sinon l'escouade s'éloignait à plus de 320 px du
  monstre, qui n'était plus cliquable. La règle n'a pas été affaiblie.
- **Non testé** : `Vision::dessiner`, le dessin conditionnel de `Partie::dessiner` et la touche
  F6 (vérifiés dans le jeu). 704 tests.

## Une seule ligne de vue (étape 73)

- **`test_ChampDeVision.cpp`** (9 cas de l'étape 72 inchangés, **+14** : 23 en tout).
  - `estVisibleDepuis` (7 cas) : une case se voit elle-même, même dans un mur ; cible sur un axe
    ou une diagonale en terrain dégagé ; un mur cache la cible derrière lui sur un axe comme sur
    une diagonale (cible d'une diagonale dans deux quadrants : les deux doivent échouer) ; porte
    fermée (révélée mais cache la salle) et ouverte, dans les deux sens ; cible hors carte ;
    **équivalence** avec `champDeVision` et **symétrie**, sur le plan à piliers et sur les
    cartes générées 1, 2 et 3 (rayon 10). L'aide `verifierEquivalence` compare, pour chaque case
    libre A, `estVisibleDepuis(A, B)` à l'appartenance de B à `champDeVision(A, 10)` pour toutes
    les cases B à distance euclidienne `<= 10` (B quelconque), et la symétrie pour B libre. Elle
    compte les paires cachées (`cachees > 0`) et collecte les divergences dans un vecteur,
    comme `verifierSymetrie`.
  - `ligneDeVue` libre (6 cas, préfixe « ligneDeVue libre », pour ne pas heurter les anciens
    noms) : faux si un point est dans un mur, une porte fermée ou hors carte ; vrai dans la
    même case ; terrain dégagé ; mur entre deux salles ; porte fermée puis ouverte ; symétrie
    sur 2000 paires de points tirés au hasard (graine 42) dans les cases libres d'une carte
    générée.
  - **+1** : deux cases libres en diagonale entre deux murs qui se touchent **se voient**
    (comportement de Ford conservé et épinglé ; l'ancien segment était bloqué).
- **`test_TileMap.cpp`** (−6) : les six cas `ligneDeVue` ont disparu, plus les assertions
  `ligneDeVue` de deux cas de portes (la porte est couverte par `test_ChampDeVision.cpp`).
- **`test_Monstre.cpp`, fixtures adaptés à la vue en cases** (aucune assertion affaiblie). Le
  pilier de 40 px mettait le monstre en case (0, 1) et la cible en case (1, 0), des cases
  diagonales que le pilier ne sépare plus ; avec des cases de 40 px, aucune paire cachée n'est
  à portée de 40 px. Deux fixtures :
  - `carteAvecMurMince` (20 × 20 cases de 10 px, un mur en case (4, 4)) pour les tests qui ne
    déplacent pas le monstre : `peutAtteindre`, `engagerAttaque` (refus si cachée, autre
    candidat), coup engagé. Monstre en (25, 45), cible cachée en (60, 45), visible en (25, 15).
  - `carteAvecPilier` (inchangée : 5 × 5 cases de 40 px, mur en case (1, 1)) pour les tests où
    le monstre doit marcher. Sur des cases de 10 px le A* le coincerait (son carré de 30 px ne
    passe pas entre les centres de cases) et « il ne bouge pas » ne distinguerait plus « arrêté »
    de « coincé ». La proie cachée est un cercle de rayon 55 centré en (100, 60), à portée
    malgré le mur de 40 px.
  - Le cas « …derriere un coin » devient « …derriere un mur ».
- **Durée** : la suite passe de 12,3 s à 13,3 s, l'écart venant des tests d'équivalence
  (~1,4 s). Les tests de `Partie` qui durent ~1,7 s le faisaient déjà.
- **`test_GenerateurDonjon.cpp`** (+4, 61 en tout) : `contientUnCoinDiagonal` détecte le damier
  dans les deux orientations, ne détecte rien dans un plan sans damier (mur plein, escalier,
  plan vide, une seule ligne), le détecte sur le bord du plan (coins haut-gauche, bas-gauche et
  haut-droit) ; et un **test de propriété** : aucun coin diagonal pour les graines 1 à 50. Ce
  dernier est un échantillon : un sondage de 20 000 graines (programme jetable, hors tests) en
  trouve 145 (0,7 %, première graine 391). Voir `docs/generation.md`.
- 716 tests.
