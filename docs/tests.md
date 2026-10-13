# Squad-ARPG — Tests

## Tests

- **Catch2 3 adopté**, installé via MSYS2
  (`mingw-w64-ucrt-x86_64-catch`), trouvé par `find_package(Catch2 3
  REQUIRED)` comme SFML (pas de `FetchContent`).
- **Un seul exécutable `tests`**, dix fichiers (`test_TileMap.cpp`,
  `test_Chemin.cpp`, `test_Escouade.cpp`, `test_Deplacement.cpp`,
  `test_Monstre.cpp`, `test_Personnage.cpp`, `test_Attaque.cpp`,
  `test_Skill.cpp`, `test_Mobile.cpp`, `test_Cercle.cpp`,
  `test_Autodefense.cpp`), lancé directement
  (`./build/tests.exe`) — pas de CTest ni de `catch_discover_tests`.
- **Petits plans dédiés** : chaque test construit son propre plan texte
  (3×3, 4×4, 5×5…), jamais la carte de `main.cpp`, pour isoler exactement
  le cas testé.
- **Teste uniquement l'interface publique** de `TileMap`, `Chemin`,
  `Escouade`, `Personnage`, `Monstre`, `Attaque`, `Mobile` et `Cercle`
  (`distanceBordABord`, `getCercle`, `getTile`,
  `chevaucheUnMur`, `trajetLibre`, `ligneDeVue`, `getLargeur`/`getHauteur`,
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
  - **`mettreAJour` : détection, arrêt à portée** : depuis
    que `Monstre` se déplace par son propre `Mobile`, la fonction
    d'aide `ecoulerTemps(monstre, duree, cible, carte)` prend en plus une
    cible (`std::optional<Cercle>`, vide = aucune ;
    les tests y passent un point de rayon nul, `pointCible`) et une
    `TileMap` (carte dégagée dédiée, `carteDegagee()`, même principe que
    pour `Mobile`/`Escouade`) — des résultats (le centre du monstre),
    jamais une trajectoire exacte. Cas couverts : une cible dans le
    rayon de détection (200 px) rapproche le monstre ; une cible hors de
    ce rayon, ou son absence, le laisse immobile ; une durée généreuse
    face à une cible dans le rayon l'amène à portée d'attaque sans
    l'atteindre — à 30 px au plus de son centre
    (portée de 15 px bord à bord + rayon du monstre de 15 px, la cible
    étant un point).
  - **Territoire et retour au poste** : `carteLongue()`,
    une carte dégagée plus large que `carteDegagee()`, dédiée aux cas
    où une cible ou un trajet de retour dépasse les 400 px du
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
  d'`Attaque`, de `Personnage` et de `Monstre` donnent leurs cibles
  comme des cercles de rayon nul (fonction d'aide `pointCible`, ou
  `point` dans `test_Attaque.cpp`) : la portée reste alors facile à
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
- **`Escouade::attaquer` (`test_Escouade.cpp`)** : comme pour les autres
  fonctions d'`Escouade`, des résultats plutôt que des trajectoires
  exactes. `attaquer` reçoit une liste de `MonstreVu` et renvoie des
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
  - `test_Autodefense.cpp` : `choisirMonstreAFrapper` seule, via la
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
- **`Attaque` (`test_Attaque.cpp`)** : couvre les six cas de
  recharge/portée en un seul endroit, plutôt que dupliqués dans
  `test_Personnage.cpp` et `test_Monstre.cpp` — dégâts à portée,
  0 hors de portée, distance exactement égale à la portée (cas limite,
  `<=`), 0 au second appel immédiat (recharge en cours), 0 après une
  durée simulée trop courte puis de nouveau les dégâts après une durée
  généreuse (même fonction d'aide `ecoulerTemps(attaque, duree)` que
  dans les autres fichiers, adaptée à une classe sans position ni
  carte), et un coup tenté hors de portée qui ne déclenche pas la
  recharge. `test_Personnage.cpp` et `test_Monstre.cpp` ne gardent que
  les cas qui prouvent leur propre branchement (voir plus haut), pas
  ces six cas génériques.
- **Skill** : chaque `Personnage` des tests existants reçoit
  `std::nullopt` comme skill (paramètre sans valeur par défaut) ; ceux qui
  testent le skill de dégâts passent un
  `Skill(TypeSkill::Degats, Attaque(...))`.
  - `test_Attaque.cpp` : `relancerRecharge` empêche de frapper avant une
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
  - `test_Escouade.cpp`, `declencherSkill(place, monstres)` (fonctions
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
  d'y entrer ; les appels existants passent `{}`. `test_Escouade.cpp` :
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
- **Soin** (`test_Personnage.cpp`, `test_Escouade.cpp`) :
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
  (`carteDegageeSoin`). Soin : le soigneur marche, soigne, puis revient à
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

## Agresseurs (étape 40)

- **`test_Escouade.cpp`** : enregistrement (coup `> 0`, coup à 0, doublon
  rafraîchi, aucun vivant), oubli (3 s, nouveau coup, `attaquer` retire
  l'absent et met à jour le cercle), et déplacement vers l'agresseur sur
  une carte dégagée (`carteDegagee`, `simulerAvecMonstre` : `attaquer` puis
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
- **`test_Escouade.cpp`** : `provoquer` (enregistrement, doublon dont la
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
- **`test_Escouade.cpp`, provocation** : la zone se mesure depuis le centre
  du lanceur, monstre compris : touché à 165 px de distance entre centres
  (150 + 15), pas à 166.
- **`test_Escouade.cpp`, skill de zone** (lanceur de la place 4 en
  (185, 235), monstres de rayon 15, donc touchés jusqu'à 75 px de
  distance entre centres) : `skillEstPret` (prêt, place invalide, mort,
  recharge de 8 s) ; `rayonZoneDuSkill` ; `declencherSkillDeZone`
  (monstres touchés et épargnés, bord à bord exact, zone vide qui part
  quand même, portée non lue, refus pendant la recharge, lanceur mort,
  skill en attente, place invalide ou sans skill `Zone`, recharge de
  l'attaque de base relancée) ; `declencherSkill` sans point refuse un
  skill `Zone` et `choisirCible` renvoie vide.
- **`test_Escouade.cpp`, ligne de vue du skill de zone** (carte 15 × 10
  avec un mur vertical épais de 40 px, `carteAvecMurVertical`, contournable
  par le haut ou le bas) : point à portée et en vue (départ immédiat),
  derrière le mur (attente, sans recharge), dans le mur (refus, sans
  attente), lanceur qui contourne puis lance ; monstre dans la zone mais
  derrière le mur épargné, monstre du même côté touché.
- **`test_TileMap.cpp`, `ligneDeVue`** : terrain dégagé, mur entre les
  points, points identiques, coin de deux murs en diagonale (bloqué),
  segment à 1 px d'un mur (libre), arrivée dans un mur.
- **Non testé** : le mode visée et le cercle, qui vivent dans `main.cpp`
  (vérifiés à la main dans le jeu).
