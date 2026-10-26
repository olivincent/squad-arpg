# Squad-ARPG — Tests

## Tests

- **Catch2 3 adopté**, installé via MSYS2
  (`mingw-w64-ucrt-x86_64-catch`), trouvé par `find_package(Catch2 3
  REQUIRED)` comme SFML (pas de `FetchContent`).
- **Un seul exécutable `tests`**, lancé par CTest depuis l'étape 80
  (`ctest --preset debug`, un test CTest par `TEST_CASE`, voir `docs/build.md`) ou directement
  (`./build/debug/tests.exe`, avec ses filtres). Un fichier par module
  (`test_TileMap.cpp`, `test_Chemin.cpp`, `test_Deplacement.cpp`,
  `test_Personnage.cpp`, `test_Skill.cpp`,
  `test_Mobile.cpp`, `test_Cercle.cpp`, `test_Autodefense.cpp`,
  `test_Encerclement.cpp`, `test_BarreDeSkills.cpp`, `test_EntiteVue.cpp`, `test_BarreDeVie.cpp`,
  `test_BarreDeRaccourcis.cpp`, `test_ClicInterface.cpp`, `test_CommandeDeTouche.cpp`, `test_PileDEcrans.cpp`,
  `test_PointsDeVie.cpp`, `test_Provocations.cpp`, `test_Agresseurs.cpp`,
  `test_TextesFlottants.cpp`,
  `test_Controleur.cpp`, `test_ChampDeVision.cpp`, `test_Vision.cpp`, `test_Composition.cpp`…),
  sauf les gros modules, coupés par thème. Depuis l'étape 81, chaque fichier est dans le dossier
  miroir de son module (`tests/base/`, `tests/rendu/`, `tests/monde/`, `tests/combat/`,
  `tests/escouade/`, `tests/monstres/`, `tests/jeu/`, `tests/interface/`) ; un fichier coupé par thème
  reste dans le dossier de son module (`tests/escouade/test_Escouade_*.cpp`) :
  - `Escouade` (étape 50, puis étape 80 pour les skills) :
    - `test_Escouade_deplacement.cpp` : ancre, chemin, formation, places, vivants, rotation ;
    - `test_Escouade_combat.cpp` : verrou, approche, autodéfense, placement en combat ;
    - `test_Escouade_skills_degats.cpp`, `_soin.cpp`, `_provocation.cpp`, `_zone.cpp` : un fichier
      par type de skill, avec son skill en attente (la zone garde aussi `skillEstPret` et
      `rayonZoneDuSkill`) ;
    - `test_Escouade_skills.cpp` : ce qui est commun aux skills : `etatDuSkill`, temps d'activation,
      adresses, lanceur occupé (étape 80) ;
    - `test_Escouade_agresseurs.cpp` : agresseurs, laisse, provocations, `proieDuMonstre` (filet
      d'intégration ; `Provocations` et `Agresseurs` sont testées seules dans leurs fichiers) ;
    - `test_Escouade_vue.cpp` : la ligne de vue (étapes 44 et 46) ;
    - `test_Escouade_places.cpp` : les places d'attaque et `pointVise` (étape 67) ;
    - `test_Escouade_degagement.cpp` : le dégagement des tireurs (étape 78) ;
  - `Partie` (étape 80) : `test_Partie_clics.cpp` (clics et ce qui est cliquable selon la vision),
    `_skills.cpp`, `_monstres.cpp` (combat, coup en préparation, alerte de groupe, provocation stricte,
    retour au poste), `_fin.cpp` (défaite, victoire, partie figée, réaffectation), `_portes.cpp`,
    `_zonesDeControle.cpp` (avec le dégagement), `_evenements.cpp`, `_objets.cpp` (F7, étape 96) ;
  - `Monstre` (étape 80) : `test_Monstre.cpp` (forme, vie, dégâts, groupe, valeurs),
    `_etats.cpp` (garde, détection, poursuite, territoire, alerte, vue de la détection, contournement),
    `_retour.cpp` (retour au poste, invulnérabilité, régénération), `_engagement.cpp` (frappe, temps
    d'exécution, cible de frappe, coup engagé).

  Un cas qui touche plusieurs thèmes est dans le fichier de son sujet principal. Les découpages sont
  des déplacements purs : aucun test modifié, le nombre de `TEST_CASE` inchangé.
- **Aides communes** : en-têtes seuls, fonctions `inline`, pour les aides **identiques** utilisées
  par plusieurs fichiers (les en-têtes sont à la racine de `tests/`, dossier d'inclusion de la cible
  `tests` depuis l'étape 81 : `#include "aides.hpp"` suffit depuis n'importe quel sous-dossier).
  Depuis l'étape 80, trois en-têtes, et chaque fichier n'inclut que celui dont il se sert :
  - **`aides.hpp`** (étape 50) : la base, sans `Escouade` ni `Partie` : graine (`s_graineDeTest`) et
    `generateurDeTest`, `verifierPointProche` (tolérance), `pointCible`, `cibleMonstre`, cartes
    (`carteDegagee10x10`, `carteDegageeEntouree` de 800 × 600 px avec murs autour, `carteOuverte`),
    temps (`s_dureeTic`, `nombreDeTics(duree)` : le nombre de tics est calculé une fois par
    `std::lround`, puis les boucles comptent des entiers ; additionner des `float` faisait 5 tics au
    lieu de 6 pour 0,1 s), `s_rayonVisionDeTest`. Un fichier comme `test_Mobile.cpp` ne compile plus
    toute la logique de jeu ;
  - **`aides_escouade.hpp`** (étape 80) : les fixtures d'`Escouade` et de `Partie` : adresses des
    skills, personnages de test et leurs constantes, `compositionDeTest`, `uneEscouade`, `unePartie`,
    `uneEscouadeADeuxGuerriers`, `uneEscouadeAuSoinCourt`, simulations (`simuler(escouade, ...)`,
    `simulerAvecAttaque`, `attaquerJusquAuDepart`, `attaquerCible`, `declencherEtAttendre`…), niveaux
    et aides de `Partie` (`niveauDegage`, `avancer`, `niveauDeLaFuite`, `fuyard`,
    `jusquAuPremierCoup`…), `unSeulMonstre` et `declencher` ;
  - **`aides_monstre.hpp`** (étape 80) : les aides des `test_Monstre*.cpp` (`carteLongue`,
    `carteAvecMur`, `ecoulerTemps(monstre, ...)`, `engager`, `frapperEnEntier`,
    `amenerEnRetourHorsTerritoire`). Un en-tête à part : `test_Skill.cpp` et `test_Personnage.cpp` ont
    leur propre `ecoulerTemps`, d'une autre signature.

  Quand deux copies différaient, elles ont des noms distincts et explicites (`carteDegagee10x10` pour
  `Mobile`, `Monstre` et `Escouade` ; `carteDegagee5x5` reste dans `test_Personnage.cpp`). Une aide
  utilisée par un seul fichier y reste, dans un `namespace {}` : `ecoulerTemps` de `Skill` et de
  `Personnage`, `simuler(mobile, ...)`, `simulerAttaque`, `simulerAvecMonstre`,
  `carteAvecMurVertical`, `attaquerPendant`, `compter`, etc.
- **Les tests ne dépendent pas du catalogue** (étape 80) : les personnages de test
  (`guerrierDeTest`, `pretreDeTest`, `mageDeTest`, dans `aides_escouade.hpp`) sont construits avec
  des valeurs figées (`s_degatsBaseDeTest`, `s_porteeMeleeDeTest`, `s_degatsFrappeDeTest`…), recopiées
  du catalogue à l'étape 80 mais indépendantes de lui. Un test de règle écrit ses nombres avec ces
  constantes, jamais en littéral (`== s_degatsFrappeDeTest`, pas `== 25`). Les valeurs qui ne sont pas
  encore injectables (points de vie, vitesse et attaque des monstres, points de vie et vitesse des
  personnages, armure de `creerNiveau`) se lisent par les accesseurs (`getPointsDeVie()` d'une entité
  neuve, `getPointsDeVieMax()`, `getAttaque().montant`) ou se mesurent dans le test (la vitesse sur
  le premier tic). **Seul `test_Specialite.cpp` lit les valeurs du catalogue des classes** (et, depuis l'étape 91, les personnages de test lisent l'attaque de leur arme dans le catalogue des objets : voir « Étape 91 ») ;
  `test_EcranChoixEscouade.cpp` le lit aussi, mais seulement pour la liste de ses boutons (une par
  classe). Vérification : changer une valeur d'équilibrage du jeu ne fait échouer que les tests dont
  le sujet est cette valeur (voir « Étape 80 » plus bas).
- **Noms de `TEST_CASE` sans accents ni cédilles** (étape 50) : « a
  portee », « degats », « deja ». Les commentaires gardent leurs
  accents. Raison : filtrer en ligne de commande dans n'importe quel
  terminal, par exemple `./build/debug/tests.exe "*a portee*"` (le filtre est
  un motif entre guillemets, `*` remplace n'importe quel texte) ; la liste
  complète : `./build/debug/tests.exe --list-tests`. Les noms restent uniques
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
- **Un coup sur l'escouade, observé via `proieDuMonstre`
 ** (`Escouade::recevoirDegats` jusqu'à 82.5, aujourd'hui l'aide `infliger`) : le coup n'a pas d'accesseur dédié, mais son
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
  `Skill(TypeSkill::DegatsSurCible, portée, puissance, recharge, 0.0f)`.
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

- **`test_Escouade_agresseurs.cpp`** : enregistrement (coup `> 0`, coup à 0,
  aucun vivant), oubli (3 s, `attaquer` retire l'absent ; depuis 82.5 la mémoire n'a plus de
  cercle, et un agresseur mort dans l'image n'attire plus personne), et déplacement vers l'agresseur sur
  une carte dégagée (`carteDegageeEntouree`, `simulerAvecMonstre` : `attaquer` puis
  `mettreAJour` à chaque tic, le monstre étant obstacle) : mêlée qui
  s'approche, agresseur hors laisse, ancre en mouvement, oubli. Observé par
  `proieDuMonstre` du monstre.
- **Piège** : un coup (`recevoirCoup`, ou l'aide `infliger`) enregistre un agresseur. Les tests
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
- **L'oubli de la place d'attaque** est testé là où il vit, depuis 82.3 et 82.4 :
  `test_PlacesDAttaque.cpp` (`placeRetenue`, `oublierSaufAutourDe`). Côté `Escouade`, qui décide de
  la cible à garder, seul le comportement visible (côtés conservés, réattribution au changement de
  cible, point visé = centre sans place valide) est vérifié.
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
  remplacées par `Monstre::s_rayonTerritoire` (publique pour cela ; depuis l'étape 89.1, c'est
  `monstreOrdinaire().rayonTerritoire`, et depuis l'étape 93
  `definitionDeMonstre(CategorieDeMonstre::Infanterie).rayonTerritoire`).
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

## Étape 74 : escouade de 3 personnages, skills désignés par identifiant

- **735 tests** (716 avant : +6 `Personnage`, +8 `Composition`, +2 `Controleur`, +2 `Escouade`,
  +1 places d'attaque ; les autres sont réécrits).
- **`test_Personnage.cpp`** (+6) : plusieurs skills exposés par identifiant, identifiant
  inconnu traité comme l'absence de skill, second skill refusé pendant l'activation d'un
  autre (recharge de ce dernier intacte), recharges indépendantes, `etatDuSkill` n'indique
  l'activation que pour le skill engagé, `annulerSkill` annule le skill engagé seulement.
  Les constructions passent `{}` (aucun skill) ou `{{IdSkill::..., Skill(...)}}`.
- **`test_Composition.cpp`** (nouveau) : `decalagesDeFormation` (aucun, un seul par rang,
  2 + 2 = carré historique, 3 par rang, 1 devant et 2 derrière, rangs entremêlés) et
  `compositionParDefaut` (sans doublon d'identifiant).
- **`test_Controleur.cpp`** (+2) : table des touches, touche hors table ignorée.
  `getPlaceEnVisee` devient `getAdresseEnVisee`.
- **`test_Escouade_skills.cpp`** (+2) : adresse dont le skill est inconnu du personnage
  traitée comme une place invalide ; `typeDuSkill`.
- **Aides de `aides.hpp`** : `uneEscouade(position)` et `unePartie(niveau, graine)`
  (composition par défaut) ; les constantes `s_adresseFrappe` (1), `s_adresseProvocation` (1),
  `s_adresseSoin` (2) et `s_adresseExplosion` (3) ; `uneEscouadeADeuxGuerriers` (quatre
  personnages : deux guerriers, prêtre, mage) pour les tests de places d'attaque et de
  séparation qui ont besoin de deux mêlées ; `uneEscouadeAuSoinCourt` (prêtre dont le soin
  porte 33 px bord à bord) parce que, dans le triangle, tous les alliés sont à portée du
  soin normal.
- **Réécriture pour 3 personnages** : guerrier en `(centre + (25, 0))`, prêtre en
  `(-25, -25)`, mage en `(-25, +25)`. Les compteurs (`== 4` → `== 3`), les places (`4` → `3`
  pour le mage, `2` → `1` pour la provocation) et les positions de monstres ont été
  recalculés. Les tests de vue (murs fins) ont des monstres en `(275, 210)` face au guerrier ;
  l'orientation de la formation se déduit maintenant de l'angle du décalage du guerrier (0° à
  l'orientation 0) au lieu de « angle + 45° ». Un test « deux mêlées » a deux versions : une
  avec la composition par défaut (guerrier contre tireurs), une avec `uneEscouadeADeuxGuerriers`.

## Étape 75 : barre de raccourcis

- **759 tests** (735 avant : +11 `BarreDeRaccourcis`, +3 `ClicInterface`, +2 `Reglages`, +2 `Escouade`,
  +6 `Controleur` net : 2 supprimés, 8 ajoutés).
- **`test_BarreDeRaccourcis.cpp`** (nouveau, 11) : taille et emplacements vides ; `placer` / `vider`
  sans toucher les voisins ; même adresse sur deux emplacements ; `barreParDefaut` avec la composition
  par défaut (Q = 0, W = 1, A = 4, Z = 8, le reste vide) ; un 5ᵉ skill de la rangée non placé, sans
  toucher la rangée suivante ; une place sans rangée (0 ou 4) ignorée. Disposition (pure, sans
  fenêtre), sur deux largeurs de vue (1067 et 1422 px) : les 12 icônes dans la vue et sans
  chevauchement, grille de 4 colonnes (pas de 52 px, marge de 10 px en bas), barre centrée,
  `emplacementSous` au centre de chaque icône, vide dans un écart ou hors de la barre. Le `assert` hors
  bornes n'est pas testé.
- **`test_ClicInterface.cpp`** (nouveau, 3) : `clicSurInterface` sur une icône, sur le monde, dans l'écart.
- **`test_Reglages.cpp`** (+2) : `emplacementPourTouche` (Q → 0, R → 3, A → 4, Z → 8, V → 11) ; touche
  hors table (chiffre, Échap) → vide.
- **`test_Escouade_skills.cpp`** (+2) : `adressesDesSkills` (ordre place puis liste),
  `couleurDuPersonnage` (couleurs de la composition, vide pour les places 0 et 4).
- **`test_Controleur.cpp`** : la table provisoire (`adresseDeLaTouche`, `appuiTouche`) et ses 2 tests
  sont supprimés ; les tests de visée passent par `activerEmplacement` avec `barreParDefaut`
  (aide locale `barreDe(partie)`, emplacements `s_emplacementExplosion` = 8, etc.). Ajoutés :
  emplacement vide sans effet ; skill direct demandé à la partie (provocation, qui est consommée même
  sans monstre) ; skill direct pendant la visée (la visée continue) ; même adresse sur deux
  emplacements (le second fait sortir de la visée) ; un autre skill de zone remplace la visée (deux
  mages, le second à la place 4) ; `appuiInterface` active l'emplacement, désarme le déplacement
  continu même sur un emplacement vide, et n'est jamais le point d'impact de la visée.
- **Non testé** : `EcranJeu` (conversion du pixel avec la vue d'interface, consommation du clic, R selon
  l'écran de fin, `IconeDeRaccourci`) et `dessinerBarreDeSkills` (noms de touches, bande de couleur),
  vérifiés à la main dans le jeu. La logique testable (`clicSurInterface`, `appuiInterface`) est sortie
  de l'écran pour cette raison.

## Étape 76 : spécialités, sélection et écran de choix

- **795 tests** (759 avant : +14 `Specialite`, +9 `SelectionDEscouade`, +11 `EcranChoixEscouade`,
  +2 `Eclaircir`, +1 `BarreDeSkills` net, −1 `Composition`).
- **`test_Specialite.cpp`** (nouveau, 14) : catalogue (5 classes dans l'ordre de l'énumération ;
  noms non vides, vision de 320 px, skills sans doublon), `specialite(id)`, `composer` (ordre reçu,
  liste vide, classes différentes = couleur de classe, même classe = 0, 30 puis 55 %, compte par
  classe et non sur toute la liste, seule la couleur change entre deux occurrences), valeurs du
  paladin et du rôdeur (type, puissance, portée, zone), trois rôdeurs (décalages en colonne derrière
  l'ancre, trois couleurs distinctes, barre par défaut : une rangée par rôdeur), escouade mixte
  (paladin, rôdeur, guerrier : types de skills par adresse). Les nuances sont comparées à
  `assombrir(...)` plutôt qu'à des octets écrits en dur.
- **`test_SelectionDEscouade.cpp`** (nouveau, 9) : vide au départ ; ajouter jusqu'à 3 dans l'ordre ;
  un 4ᵉ ajout sans effet ; doublons ; retirer au milieu (les suivantes remontent) ; retirer la
  première puis ajouter (redevient complète) ; `caseSous` au centre de chaque case, dans un écart et
  hors des cases ; `rectangleDeLaCase` (empilées, centrées). Le `assert` hors bornes n'est pas
  testé.
- **`test_EcranChoixEscouade.cpp`** (nouveau, 11) : sélection vide ; un clic sur une classe l'ajoute ;
  trois fois la même, pas de 4ᵉ ; case remplie retirée ; case vide sans effet ; « Start »
  inactif tant que la sélection est incomplète, `Empiler` quand elle est complète, de nouveau
  inactif après un retrait, sélection inchangée après « Start » ; « Back » = `Depiler` même
  avec une sélection complète ; clic hors des boutons. Les positions viennent de `centreDesClasses`,
  `centreDesCases` et `centreDesActions` (les mêmes que l'écran) ; la liste des classes est reconstruite
  depuis `catalogue()`, comme l'écran.
- **`test_Eclaircir.cpp`** (+2) : `assombrir` mélange au noir sans toucher l'opacité ; 0 laisse la
  couleur, 1 donne du noir.
- **`test_BarreDeSkills.cpp`** : `lettreDuSkill` est remplacé par `sigleDuSkill` (les huit sigles) et
  un test que les sigles sont tous différents (+1 net).
- **`test_Composition.cpp`** : le test de `compositionParDefaut` passe dans `test_Specialite.cpp`
  (−1) ; il ne reste que `decalagesDeFormation`.
- **`aides.hpp`** : `compositionDeTest()` (appelle `composer({Guerrier, Pretre, Mage})`) remplace
  `compositionParDefaut()` ; `uneEscouade`, `unePartie`, `uneEscouadeADeuxGuerriers` et
  `uneEscouadeAuSoinCourt` s'en servent.
- **Non testé** : le dessin de l'écran de choix (bande de couleur des cases, titre), la conversion
  du pixel en coordonnées d'interface de `traiterEvenement`, le numéro de place dans la bande des
  icônes, et le parcours complet menu → choix → jeu → R (vérifiés à la main). Les constructions
  d'`EcranJeu` sont couvertes seulement par `Commencer` (le test vérifie que l'écran empilé n'est pas nul).

## Étape 77 : zones de contrôle et attaque d'opportunité

- **830 tests** (795 avant : +18 `Engagement`, +6 `Partie`, +4 `Escouade`, +3 `Personnage`, +2
  `Monstre`, +2 `Specialite`).
- **`test_Engagement.cpp`** (nouveau, 18) : tous les cercles ont un rayon de 10 px et une zone de
  20 px, donc A engage B à 40 px entre centres ou moins. `engage` (limite exacte, chevauchement, zone 0,
  non symétrique) ; la cible qui sort de la zone en s'éloignant reçoit un coup ; l'attaquant qui recule
  lui-même ne frappe pas ; les deux s'éloignent (le plus rapide a fui, égalité : la cible) ; la
  projection sur la droite A→B (pas de côté de la cible, de l'attaquant) ; exemption quand un autre
  adversaire reste engagé à la fin ; deux fuyards simultanés : un seul coup, sur le plus proche à la
  fin ; égalité : le premier de la liste ; une cible non engagée au début n'est pas frappée ; zone 0 :
  jamais de coup ; entité absente de la photo de fin (cible, attaquant, les deux) ; paire qui reste
  engagée ; entrée dans la zone ; entité absente du début ; chaque attaquant a son propre coup, même sur
  la même cible ; chaque attaquant est jugé pour lui-même ; centres confondus.
- **`test_Partie.cpp`** (+6, sur de vraies parties avec la graine de test) :
  - *mage seul qui s'éloigne* : le monstre le frappe (premier coup, qui lance sa recharge de 0,7 s au
    moins puis 0,3 s d'exécution), puis un ordre de déplacement ; sur 0,9 s le mage perd **exactement
    10 PV** de plus, le coup gratuit (aucun coup normal n'est possible avant 1 s) ;
  - *guerrier qui reste* : mêmes conditions sans ordre, aucun PV perdu après le premier coup ;
  - *provocateur* : trois guerriers, un monstre au contact du premier seulement ; l'attaque de base du
    premier est reconnaissable (30 dégâts, recharge de 100 s), donc après son premier coup tout
    nouveau coup de 30 sur le monstre est un coup gratuit. Le troisième guerrier provoque ; le
    monstre part, le premier le frappe gratuitement : deux pertes de 30 en 1,2 s ;
  - *même scénario sans zone* sur le premier (zone mise à 0) : une seule perte de 30 ;
  - *monstre tué par un coup gratuit* : premier coup de 60, coup gratuit de 60 ; le monstre est retiré
    en moins de 1,2 s (les autres guerriers ne le tueraient pas si vite) et le verrou est levé ;
  - *monstre qui rentre à son poste* : l'escouade le rejoint, le touche, puis fait demi-tour ; aucun
    personnage ne perd de PV et le monstre non plus (invulnérable).
- **Vérification par mutation** : j'ai neutralisé tour à tour (1) l'application des coups sur les
  monstres, (2) l'application des coups sur les personnages, (3) l'exclusion des monstres en retour ;
  chacune fait échouer au moins un test. Le premier test « retour » ne voyait pas la mutation (le
  guerrier reste collé au monstre jusqu'à l'arrivée au poste) : le demi-tour provoqué dans le test est
  ce qui rompt la paire.
- **`test_Escouade_combat.cpp`** (+4) : `entitesDeControle` (places, cercles, zones), sans les morts,
  `puissanceDeLAttaqueDeBase` (place invalide : vide) et `pointsDeVieDuPersonnage`.
- **`test_Personnage.cpp`** (+3) et **`test_Monstre.cpp`** (+2) : la zone donnée au constructeur (0 par
  défaut) ou la constante du monstre, la puissance de l'attaque, et le contrat « lire la puissance ne
  change ni le coup engagé ni la recharge ».
- **`test_Specialite.cpp`** (+2) : 20 px pour le guerrier et le paladin, 0 pour les trois autres ;
  `composer` garde la zone quelle que soit la nuance de couleur.
- **Nouvelle fonction de lecture pour les tests** : `Escouade::pointsDeVieDuPersonnage(place)`.
- **Non testé** : le dessin des zones de contrôle (F3, vérifié à la main).

## Étape 78 : dégagement des tireurs

- **853 tests** (830 avant : +9 `Engagement`, +13 `Escouade`, +1 `Partie`).
- **`test_Engagement.cpp`** (+9, `monstreDontSeDegager`) : mêmes cercles de 10 px, monstres de zone 20 px.
  Engagé par un monstre tenu par un autre personnage : vrai (et renvoie son identifiant) ; seul au
  contact : faux ; deux monstres dont un seul est tenu : faux ; les deux tenus, par le même personnage
  ou par deux : vrai ; pas engagé (ou aucun monstre) : faux ; le personnage lui-même (ou une liste
  vide) ne compte pas comme « autre » ; zone > 0 : jamais ; plus proche des monstres engageants, et
  égalité : le premier de la liste.
- **`test_Escouade_degagement.cpp`** (nouveau, 13, composition par défaut, monstre immobile entre le
  guerrier et le mage) : l'attaque de base du mage fait 7 dégâts, pour reconnaître ses coups parmi les 5
  des autres. Départ (le mage, pas le guerrier ni le prêtre) ; aucun coup du mage pendant la marche,
  arrivée à 90 % de sa portée (108 px bord à bord, ±1), hors de la zone, le guerrier toujours au contact,
  puis le mage tire ; seul au contact : reste et tire ; le prêtre se dégage aussi ; deux monstres dont
  un seul est tenu : reste ; sans liste de monstres de contrôle : personne ; un coup en exécution part
  d'abord, le départ vient après ; fin si le monstre disparaît ou rentre à son poste (absent de la liste
  de contrôle, encore dans celle des obstacles) ; fin si l'ancre bouge, pas de départ si elle bouge déjà ;
  skill en attente (avant le départ : pas de départ ; pendant : fin) ; skill du joueur à portée : fin et
  le mage reste sur place ; fin si le porteur s'en va ; place invalide. Un agresseur est enregistré
  (`recevoirCoup`) comme dans une vraie partie, sinon le mage arrivé retournerait vers sa place de
  formation.
- **`test_Partie.cpp`** (+1) : guerrier et mage au contact d'un même monstre. Le mage ne perd **aucun**
  point de vie en 3 s (ni coup gratuit, ni coup normal : la proie du monstre est le guerrier), le guerrier
  reste au contact, le mage finit hors de la zone et à portée, puis fait baisser les points de vie du
  monstre d'au moins 7. **Mutation** : sans la photo des monstres passée à `Escouade::mettreAJour`, le
  mage ne se dégage pas et le test échoue.
- **Nouvelle fonction de lecture pour les tests** : `Escouade::degagementDuPersonnage(place)`.
- **Non testé** : l'enchaînement avec une vraie caméra et le ressenti du recul (vérifiés à la main).

## Étape 79 : dégâts flottants

- **887 tests** (853 avant : +3 `PointsDeVie`, +4 `Personnage`, +3 `Monstre`, +5 `Escouade`, +10 `Partie`,
  +9 `TextesFlottants`).
- **Sous-étape 1, montants appliqués** :
  - `test_PointsDeVie.cpp` (+3) : `soigner` renvoie les points rendus, seulement ce qui tient sous le
    maximum (puis 0 à pleine vie), 0 pour un mort ;
  - `test_Personnage.cpp` (+4) : `recevoirDegats` renvoie les dégâts après armure (mêlée : 10 → 7 ; tireur :
    10) et 0 pour un mort ; `recevoirSoin` renvoie ce qui a été rendu, plafonné, 0 à pleine vie et pour
    un mort ;
  - `test_Monstre.cpp` (+3) : `recevoirDegats` renvoie les dégâts après armure (armure 0 : 10 ; armure 20 :
    10 → 8), 0 pour un monstre mort, 0 en retour au poste (invulnérable) ;
  - `test_Escouade_agresseurs.cpp` (+3) : `recevoirCoup` renvoie les dégâts du personnage touché (guerrier
    armure 50 : 7 ; mage : 10), 0 sur un mort ou une place invalide ; `recevoirDegats` renvoie 0 quand tout
    le monde est mort.
- **Sous-étape 2, la file d'événements** :
  - `test_Escouade_skills.cpp` (+2) : `prendreFaits` rend un `SoinApplique` (place 3, 20 points rendus
    alors que le soin vaut 30, position du soigné) puis se vide ; `centreDuPersonnage` (centre de la place,
    vide pour une place invalide) ;
  - `test_Partie.cpp` (+10) : aides `evenementsPendant` (avance tic par tic et rassemble les événements,
    parce que la file se vide à chaque `prendreEvenements`) et `parmi` (filtre par type et camp).
    Skill de dégâts sur un monstre d'armure 20 : un événement de 21 (25 × 100 / 120), id et position
    du monstre ; coup d'un monstre : 7 sur le guerrier et 10 sur le mage, position du personnage ; skill de
    zone : un événement de 15 par monstre touché (trois monstres hors de la détection, pour qu'ils ne
    bougent pas avant le départ du sort) ; coup gratuit : `opportunite` vrai dans les deux sens (monstre
    sur le mage qui recule ; guerrier sur le monstre qui part vers un provocateur), un seul coup normal et
    un seul coup gratuit ; soin : un événement `Soin` sur le guerrier, dont le montant est borné par les
    dégâts reçus et qui rend exact le compte de points de vie ; deux guerriers dont les coups de 200
    partent au même tic : un seul événement, le second coup tombe sur un mort ; ce monstre tué est
    retiré mais garde son événement, avec sa position ; `prendreEvenements` vide la file ; partie
    gagnée ou perdue : aucun nouvel événement ; `estVisible` (salle de départ visible, point lointain non).
- **Sous-étape 3, `test_TextesFlottants.cpp`** (nouveau, 9, sans fenêtre) : opacité à 0, 0,4, 0,6, 0,8 et
  1,5 s (1, 1, 0,5, 0, 0) ; hauteur à 0, 0,4 et 0,8 s (20, 35, 50) ; position à la naissance (20 px
  au-dessus) et après 0,4 s ; retrait à 0,8 s et pas avant (0,4 + 0,3 : encore là) ; trois textes de la même
  image sur la même cible : rangs 0, 1, 2 et 14 px d'écart ; pas d'empilement pour deux cibles différentes
  ni pour un monstre et un personnage de même identifiant ; pas d'empilement entre deux images ;
  libellé (« 12 », « 7! », « +20 »), couleur (blanc, rouge, vert, orange) et taille (16 ou 20).
- **Non testé** : le dessin des textes (`TextesFlottants::dessiner`, il faut une fenêtre) et le filtre du
  brouillard d'`EcranJeu` (on teste `Partie::estVisible`, qu'il utilise) ; vérifiés à la main.

## Étape 80 : filet de sécurité

- **893 tests** (887 avant : +5 « Lanceur occupe : … » dans `test_Escouade_skills.cpp`, +1 pour
  `coutDesObstacles` au bord de la carte dans `test_Chemin.cpp`). Lancés par `ctest --preset debug`
  (un test CTest par `TEST_CASE`, 12 en parallèle, 9 à 10 s) dans le preset `debug` : `assert`
  actifs, assertions de la bibliothèque standard, UBSan en mode piège. Tous passent aussi dans le
  preset `jeu` (Release).
- **Avertissements aussi sur les tests** : 31 avertissements ramenés à zéro : 19 variables
  `generateur` jamais lues, supprimées ; 12 `-Wmissing-field-initializers` sur `Niveau`, supprimés par
  `Niveau::armureMonstres = 0` (la valeur que ces niveaux de test recevaient déjà : plusieurs tests de
  `Partie` comptent sur des monstres sans armure).
- **Un nom accentué** (« le verrou posé sur un monstre… ») a été trouvé par ctest, qui ne savait pas
  le lancer : renommé.
- **Tests indépendants du catalogue (T1)**. Méthode : dans une copie du dépôt hors de `src/`, changer
  une valeur d'équilibrage, relancer toute la suite, corriger les tests qui échouent sans que la valeur
  soit leur sujet. Résultats (nombre de tests en échec) :

  | Valeur changée dans le jeu | Avant | Après |
  |---|---|---|
  | Dégâts de l'attaque de base 5 → 6 | 30 | 0 |
  | Portée de mêlée 20 → 25 px | 10 | 1 (*) |
  | Points de vie des monstres 100 → 120 | 13 | 0 |
  | Vitesse des personnages 240 → 250 px/s | 1 | 0 |
  | Armure des monstres de `creerNiveau` 20 → 25 | 0 | 0 |
  | Points de vie des personnages 100 → 120 | 19 | 0 |
  | Attaque des monstres 10 → 11 | 10 | 1 (*) |

  (*) Tests dont le sujet est la valeur : « le paladin est une melee qui frappe et soigne au contact »
  (`test_Specialite.cpp`) et « un monstre a une zone de controle de 20 px et une attaque de 10 degats »
  (`test_Monstre.cpp`). La même expérience sur les constantes des personnages de test (dégâts de base,
  frappe, zone, soin, portée de mêlée) donne 0 échec : les tests les écrivent par leur nom.
- **Vitesse des monstres** (non injectable, sans accesseur) : la sonde 160 → 170 fait échouer 3 tests,
  sans nombre écrit en dur : ce sont des scénarios de fuite, qui supposent que les personnages vont plus
  vite que les monstres (le mage sort de la zone du monstre ; l'escouade rattrape le monstre qui rentre).
  Ces tests nomment leur hypothèse en commentaire.
- **Choix d'écriture** : une valeur du jeu non injectable se lit sur l'entité neuve (`const int maximum =
  monstre.getPointsDeVie();`) ; un montant après armure se calcule avec `appliquerArmure`, plutôt
  qu'un 21 calculé à la main ; une hypothèse entre constantes se vérifie à la compilation
  (`static_assert(3 * s_degatsBaseDeTest < s_degatsFrappeDeTest)`).
- **Lanceur occupé** (`test_Escouade_skills.cpp`, 5 tests) : la frappe demandée pendant la provocation
  attend puis part ; une frappe en attente ne se perd pas si le monstre arrive pendant l'activation ;
  sans verrou, elle est abandonnée si la cible n'est plus atteinte quand le lanceur est libre ; le
  paladin (frappe sacrée puis imposition des mains) et le rôdeur (tir perçant puis pluie de flèches),
  avec un `paladinDeTest` et un `rodeurDeTest` locaux au fichier. Quatre de ces tests échouent avec
  l'`Escouade` d'avant l'étape 80.
- **Découpages (T2) et aides (T3)** : voir le début de ce fichier. Déplacements purs, sauf deux
  adaptations nécessaires : les aides partagées passent d'un `namespace {}` à `inline` dans un
  en-tête, et les copies identiques de `niveauDegage` et `avancer` de `test_Controleur.cpp` ont été
  retirées au profit de celles d'`aides_escouade.hpp`.

## Étape 82.5 : derniers canaux et accesseurs réservés aux tests

- **`Agresseurs` sans cercle** : `test_Agresseurs.cpp` teste `enregistrer(id)`, `retirerMonstresAbsents`
  et `vusParmi` (le cercle vient de la liste passée, l'agresseur absent est ignoré sans être oublié).
  Dans `test_Escouade_agresseurs.cpp`, « un agresseur mort dans l'image n'attire plus personne » remplace
  le test de l'écart de 82.4 : le montage place le guerrier loin de sa place de formation (il a marché
  jusqu'au monstre), le `REQUIRE` sur cette distance précède l'assertion, sinon « rester sur place » et
  « aller à la formation » donneraient le même point.
- **`infliger(escouade, idMonstre, origine, quantite)`** (`tests/aides_escouade.hpp`) remplace
  `Escouade::recevoirDegats` : `proieDuMonstre` puis `recevoirCoup`, le chemin d'un coup de `Partie` sans
  `Partie`. `recevoirCoup` ne prend plus d'origine.
- **`VueDesMonstres` dans les tests** : aucun champ n'a de valeur par défaut ; une initialisation
  désignée écrit les trois (`.deControle = {}` si aucun monstre de contrôle), `mettreAJour(dt, carte, {})`
  reste valable. Sans cela, `-Wmissing-field-initializers` signale l'oubli.
- **`cerclesAContourner`** (`test_Intention.cpp`) : approcher un monstre, rester autour d'un monstre
  (tous les autres), rester autour de personne et aller à un point (tous), monstre de l'intention
  absent, liste vide. Il remplace le test d'`Escouade` « vers un agresseur sans place libre… » : le
  montage n'existait que pour un agresseur de rayon nul, et la garantie vient maintenant de la
  signature (la fonction ne reçoit rien du placement). L'assemblage reste couvert par « Places
  d'attaque : sans place valide, le personnage vise le centre du monstre ».
- **Tests retirés comme doublons** (même propriété qu'un test d'un type extrait, aucun assemblage de
  plus) : « Un coup d'un agresseur deja connu ne le duplique pas… » et « Un nouveau coup repousse l'oubli
  de l'agresseur » (→ `Agresseurs.enregistrer d'un monstre connu ne le duplique pas et remet 3 s`) ;
  « provoquer enregistre le monstre, le provocateur et la duree » (→ `Provocations.provoquer enregistre
  le monstre, le provocateur et la duree`) ; « Provoquer de nouveau un monstre deja provoque… » (→
  `Provocations.provoquer sur un monstre deja provoque…`) ; « Rotation : la formation tourne par le plus
  court cote » (→ `Formation : tourne par le plus court cote`).
- **Règle des accesseurs réservés aux tests (D3)** : une fonction publique que seuls les tests appellent
  est retirée, sauf si aucun type extrait ni accesseur du jeu ne permet de vérifier la propriété
  autrement ; elle est alors gardée et son commentaire dit « lu par les tests » avec la raison.
  Gardés en 82.5 : `getPlacesDeFormation`, `pointVise`, `degagementDuPersonnage`,
  `pointsDeVieDuPersonnage`, `getAgresseurs`, `getProvocations`, `choisirCible`, `Partie::getMonstres`.
- **Total** : 962 tests.

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Tests** : logique de jeu dans `squad_core`, testée avec Catch2 3
  (`ctest --preset debug`, ou `./build/debug/tests.exe`, 893 tests : 27 sur `Engagement`, 23 sur `ChampDeVision`, 8 sur `Vision`, 5 sur `ObjetInteractif`, 15 sur `Reglages`, 3 sur `TailleDeVue`, 6 sur `ListeDeBoutons`, 4 sur `EcranMenu`, 12 sur `EcranOptions`, 61 sur `GenerateurDonjon`, 36 sur `TileMap`, 28 sur `Chemin`, 226
  sur `Escouade` en onze fichiers par thème (déplacement, combat, quatre fichiers de skills par type plus un commun, agresseurs, vue, places, dégagement),
  7 sur `Composition`, 16 sur `Specialite`, 9 sur `SelectionDEscouade`, 11 sur `EcranChoixEscouade`, 11 sur `Encerclement`, 16 sur `Deplacement`, 78 sur `Monstre` en quatre fichiers (base, états, retour, engagement),
  64 sur `Personnage`, 24 sur `Skill`, 24 sur `Mobile`, 4 sur `Eclaircir`, 10 sur `Cercle`, 13
  sur `Autodefense`, 8 sur `BarreDeSkills`, 11 sur `BarreDeRaccourcis`, 3 sur `ClicInterface`, 4 sur `EntiteVue`, 1 sur
  `BarreDeVie`, 11 sur `PointsDeVie`, 6 sur `Defense`, 9 sur `Provocations`, 8 sur `Agresseurs`, 66 sur `Partie` en sept fichiers (clics, skills, monstres, fin, portes, zones de contrôle, événements), 16 sur `Controleur`, 9 sur `TextesFlottants`). Aides communes dans
  `tests/aides.hpp` (base), `tests/aides_escouade.hpp` (fixtures d'`Escouade` et de `Partie`) et
  `tests/aides_monstre.hpp` ; noms de tests sans accents (`./build/debug/tests.exe "*a portee*"`
  filtre ; ctest en dépend) ; rotation de la formation et `provoquer` à une place invalide couverts.
  **Les tests ne dépendent pas du catalogue** (étape 80) : la composition par défaut des tests
  (`compositionDeTest()` : guerrier, prêtre, mage de test ; `uneEscouade`, `unePartie`) est construite
  avec des valeurs figées (`s_degatsBaseDeTest`, `s_porteeMeleeDeTest`…), les valeurs non injectables
  se lisent par les accesseurs ; changer une valeur d'équilibrage du jeu ne fait plus échouer que les
  tests dont c'est le sujet (seul `test_Specialite.cpp` lit les valeurs du catalogue).
  `uneEscouadeADeuxGuerriers` et `uneEscouadeAuSoinCourt` servent aux cas qui ont besoin de deux mêlées
  ou d'un allié hors de portée du soin. Voir `docs/tests.md`.

## Objets et équipement (étape 83)

- **`tests/objets/`** (30 cas) : `test_Emplacement.cpp` (`peutAller` : chaque catégorie, les deux
  anneaux, le bouclier refusé en main droite, aucune catégorie en main droite),
  `test_CatalogueObjets.cpp` (ordre de l'enum, chaque catégorie va quelque part, sac de départ),
  `test_Equipement.cpp` (pose, ancien objet rendu, retrait, `armureTotale` vide, partielle, complète),
  `test_Inventaire.cpp` (première case libre, sac plein, positions stables après une prise).
- **`tests/escouade/test_Escouade_equipement.cpp`** (18 cas depuis l'étape 85) : `ramasser`,
  `deplacerObjet` (réussites et échecs, échange, mort équipé), compteur d'exemplaires par modèle avant et après
  chaque opération (un objet n'est qu'à un endroit), armure lue par `recevoirCoup`, sac de départ
  d'une `Partie`.
- **Aucun littéral d'armure** : les attentes sont calculées avec `definitionObjet(id).armure` et
  `armureDe(equipement)` (`aides.hpp`, avec `s_equipementMeleeDeTest` et `s_armureMeleeDeTest`), pour
  que rééquilibrer le catalogue d'objets ne casse aucun test de règle. `test_Specialite.cpp` lit
  l'équipement de départ du catalogue des classes ; `test_Personnage.cpp` et `test_Vitalite.cpp`
  couvrent l'armure de départ et `definirArmure`.

## Étape 84 : l'inventaire en lecture (+34 cas, 1 011 → 1 045)

- **`tests/interface/test_CommandeDeTouche.cpp`** (7) : Échap en tout temps ; R seulement en fin de
  partie (sinon c'est l'emplacement 3) ; F3 à F6 en tout temps ; une touche de la barre donne son
  emplacement ; raccourcis ignorés en fin de partie ; touche inconnue ; I ouvre l'inventaire, sauf en
  fin de partie.
- **`tests/interface/test_PileDEcrans.cpp`** (5) : `premierEcranADessiner` avec un faux écran
  (`EcranDeTest`, transparent ou non) : un seul écran, écran opaque au dessus, transparent au dessus,
  plusieurs transparents, le plus bas toujours dessiné.
- **`tests/interface/test_DispositionInventaire.cpp`** (12) : tout tient dans 1200 × 800 et en 16:9, dans
  le panneau ; aucun chevauchement entre les 134 rectangles ; colonnes dans l'ordre des places et
  centrées ; emplacements dans l'ordre de l'enum ; sac de 4 rangées de 10 cases carrées ; `elementSous`
  retrouve chaque emplacement et chaque case, et ne trouve rien dans un écart, sur un en-tête ou
  ailleurs ; `placerInfobulle` (sous le curseur, de l'autre côté près des bords, toujours dans la vue sur
  une grille de positions).
- **`tests/interface/test_AffichageObjets.cpp`** (4) : un nom par emplacement, tous différents ; un nom
  par catégorie ; un sigle de deux lettres par objet du catalogue, tous différents ; lignes de l'infobulle.
- **`tests/interface/test_EcranInventaire.cpp`** (15 depuis l'étape 85) : I et Échap dépilent, les
  autres touches sont ignorées, l'écran laisse voir dessous (avec une escouade de test,
  `aides_escouade.hpp`) ; le glissement (voir « Étape 85 »).
- **`tests/interface/test_BarreDeSkills.cpp`** (+2) : `iconesDeLaBarre` (emplacement garni et vide, seul
  l'emplacement du skill visé est en visée ; le nom de la touche n'est pas testé, il dépend du système).
- **`tests/interface/test_Controleur.cpp`** (+1) : `interrompre` annule la visée et désarme le
  déplacement continu. **`test_EcranMenu.cpp`** : la branche « Options » vérifie le type de l'écran
  empilé (`dynamic_cast<EcranOptions*>`).
- Non testés, parce qu'ils ont besoin d'une `RenderWindow` : `clicGaucheDInterface`,
  `preparerLInterface`, `Application`, `EcranJeu` et tous les `dessiner`.

## Étape 85 : le glisser-déposer (+24 cas, 1 045 → 1 069)

- **`tests/objets/test_PositionDObjet.cpp`** (7) : `deplacementPermis` pour chaque combinaison : source
  vide, même position, sac → sac (vide, occupé), sac → emplacement (compatible, incompatible, occupé),
  emplacement → sac (case vide, compatible, incompatible), emplacement → emplacement (anneaux, entre
  personnages, incompatible). Le cas « permis dans un sens seulement » n'existe pas avec le catalogue
  actuel (`docs/objets.md`, « Déplacements »).
- **`tests/escouade/test_Escouade_equipement.cpp`** (+5) : l'armure suit un objet d'un personnage à
  l'autre (lue par `recevoirCoup`), échange entre deux personnages, sac → sac (case vide, occupée, même
  case), source vide et destinations refusées sans rien changer, `peutDeplacerObjet` ne change rien.
  Le compte d'exemplaires est vérifié après chaque déplacement, réussi ou non.
- **`tests/interface/test_EcranInventaire.cpp`** (+12) : saisir (objet, case vide, vide, hors des
  éléments), rien ne bouge avant le relâchement, relâcher sur un emplacement compatible, sur une case
  du sac, sur un emplacement incompatible, dans le vide, sans glissement, objet porté échangé avec un
  autre personnage, Échap annule puis ferme, I ferme pendant un glissement, `annulerLeGlissement`.
  Les événements SFML, la perte du focus et le dessin (atténuation, objet qui suit le curseur,
  surbrillance) ne sont pas testés : ils se vérifient à l'écran.
- **`tests/interface/test_DispositionInventaire.cpp`** : `elementSous` compare maintenant des
  `PositionDObjet`.

## Étape 86 : nom du skill lancé et « Provoqué » (+10 cas, 1 069 → 1 079)

- Sous-étapes 1 et 2 (refactorisations) : aucun cas ajouté. `test_Partie_evenements.cpp` : `parmi<Detail>`
  filtre par sorte de détail (`std::holds_alternative`), `degats(e)` et `soin(e)` lisent le détail
  (`std::get`) ; `test_TextesFlottants.cpp` construit les événements avec `Degats{…}` / `Soin{…}` ;
  `test_Escouade_skills_soin.cpp` lit `prendreFaits`.
- **`tests/escouade/test_Escouade_skills.cpp`** (+4) : aide `skillsPartis` ; un fait `SkillPart` à la fin de
  l'activation du skill de dégâts, pas avant (place, skill, position du lanceur) ; provocation, soin et
  zone vide en signalent un ; l'attaque de base n'en signale aucun ; provocation : un `MonstreProvoque`
  par monstre dans la zone, aucun pour celui hors zone, de nouveaux après la recharge.
  `test_Escouade_skills_soin.cpp` : le soin donne maintenant deux faits (skill parti, puis soin).
- **`tests/jeu/test_Partie_evenements.cpp`** (+2) : un `SkillLance` sur le lanceur à la fin de
  l'activation, pas au début ; un `Provoque` sur le monstre provoqué, à sa position.
- **`tests/interface/test_TextesFlottants.cpp`** (+2) : libellé, couleur et taille d'un skill lancé et de
  « Provoqué ».
- **`tests/interface/test_AffichageSkills.cpp`** (+2) : un nom par skill, tous différents ; UTF-8.

## Étape 87 : moteur du coup (+3 cas, 1 079 → 1 082)

- **`tests/aides.hpp`** : `s_coupMortel` (100 000), un coup qui tue n'importe quelle entité quelle que soit
  son armure ; remplace les 200 et 1000 écrits en dur dans les tests de mort (la vie passe de 100 à 1000).
  Les valeurs « figées » des personnages de test (`s_degatsBaseDeTest`, etc.) sont à l'échelle × 10.
- **`tests/combat/test_Defense.cpp`** réécrit pour M1 : au-dessus du plancher (100 avec 30 → 70),
  au plancher (20 %), arrondi du plancher, plancher à 1, coup de 0, bornes.
- **`tests/combat/test_ResolutionDuCoup.cpp`** (+2) : `resoudreCoup` rend `appliquerArmure` ; un coup de 0
  reste 0.
- Les attentes de `test_Vitalite`, `test_Monstre` et de quelques tests d'escouade sont recalculées pour M1.

## Étape 88 : caractéristiques (+44 cas, 1 082 → 1 126)

- Sous-étape 1 (C2) : aucun cas ajouté. Les constructions de `Skill` et de `Personnage` des tests passent en
  initialisation désignée (`Skill({.type = …})`, `Personnage(centre, {.couleur = …})`) ; les définitions des
  aides (`guerrierDeTest`…) aussi. Les champs omis (`skills`, `equipementDeDepart`, `caracteristiques`,
  `zoneDeControle`…) ont une valeur par défaut, ce qui évite l'avertissement `-Wmissing-field-initializers`.
- **`tests/combat/test_Caracteristiques.cpp`** (nouveau) : valeurs à 0 par défaut, ordre de l'enum, `seule`,
  addition, `pointsDeVieMax`, `Coefficients`, et `montantEffectif` (neutre à 10, coefficient nul, bonus par
  point, pondération et somme, arrondi au plus proche, jamais négatif).
- **`tests/combat/test_PointsDeVie.cpp`** (+6) : `changerMaximum` (gain, perte, plancher à 1 pour un vivant,
  un mort reste à 0, un vivant plein reste plein, aller-retour).
- **`tests/objets/`** : le catalogue (ordre, armure ou bonus, anneau de puissance et collier d'endurance, sac de
  départ), `Equipement::bonusTotal`.
- **`tests/escouade/test_Personnage.cpp`** : caractéristiques effectives (base, base + équipement, équipement de
  départ), points de vie maximum, équiper et retirer le collier (gain, perte sans tuer, mort inchangé), montant
  effectif de l'attaque de base et d'un skill, provocation sans coefficient.
- **`tests/escouade/test_Specialite.cpp`** : somme 50 et 1000 PV max pour chaque classe ; le guerrier plus
  d'Endurance que le mage, le mage plus de Savoir.
- **`tests/escouade/test_Escouade_equipement.cpp`** : `puissanceDeLAttaqueDeBase` (lue pour les coups
  d'opportunité) suit l'anneau de puissance.
- **`tests/interface/`** : `test_AffichageCaracteristiques.cpp` (noms, ligne avec bonus),
  `test_AffichageObjets.cpp` (infobulle sans « Armure : +0 », une ligne par bonus),
  `test_DispositionInventaire.cpp` (cellules du bloc dans le bloc, bloc entre les emplacements et le sac).

## Étape 89 : le critique (+41 cas, 1 126 → 1 167)

- **Les aides** (`aides.hpp`, `aides_escouade.hpp`, `aides_monstre.hpp`) :
  - `attaqueSansHasard(montant)` (appelée `attaqueSansCritique` à l'étape 89) : une `Attaque` de capacité
    offensive 0 et, depuis l'étape 90, de nature **zone** : **ni critique, ni esquive, ni blocage, donc aucun
    tirage**. Les tests de règle (armure, mort, agresseurs, points de vie…) n'ont pas à connaître le hasard.
  - `subir(entite, montant)` (`Vitalite`, `Personnage` ou `Monstre`) et `subirCoup(escouade, idMonstre, place,
    montant)` : font subir un tel coup avec un générateur neuf (jamais lu) et rendent les dégâts subis ; ce
    sont les anciens `recevoirDegats(montant)` et `recevoirCoup(…, montant)` des tests (remplacés par
    substitution, environ 80 appels). `infliger` s'en sert.
  - `defensesDeTest(armure)` (capacité défensive 100), `defensesDuPersonnageDeTest(armure)` (celle des
    personnages de test, Adresse 0 : 50), `montantOuZero(optional<Attaque>)` (le montant d'un coup parti, 0
    sinon : ce que rendaient `avancerAttaque` et `avancerSkill` avant de rendre une `Attaque`),
    `monstreAvecArmure(armure)` (le monstre ordinaire avec cette armure, pour construire un `Monstre`).
  - `degatsAttendus(attaque, defenses, critique)` : les dégâts que le jeu devait produire, avec ou sans
    critique. Les tests de `Partie` ne contrôlent pas son générateur ; ils lisent `critique` dans
    l'événement et vérifient le montant qui va avec (voir plus bas).
- **Pourquoi le critique ne casse pas les tests de règle** : une `Attaque` de capacité offensive 0 ne tire
  jamais. Les 1 126 tests d'avant ont passé tels quels sauf ceux qui comparaient un montant subi par un
  personnage de test à celui d'un monstre dans une `Partie` (3 cas : un monstre à capacité offensive 100
  contre une capacité défensive de 50 critique 20 % du temps) ; ils comparent maintenant au montant que
  donne `degatsAttendus` selon le `critique` de l'événement.
- **`tests/combat/test_Critique.cpp`** (nouveau, 10) : le rapport ; la chance (0, 1,25 %, 5 %, 20 %, 45 %,
  plafond de 50 %) ; le multiplicateur (1,5 ; 1,75 ; 2 ; bornes) ; `appliquerCritique` (sous la chance :
  critique multiplié ; égal ou au-dessus : rien ; arrondi au plus proche ; rapport nul jamais critique ;
  montant nul).
- **`tests/combat/test_ResolutionDuCoup.cpp`** (réécrit, 11) : l'armure, le coup de 0 ; **aucun tirage**
  (le générateur est inchangé, comparé avec `==`) quand la capacité offensive ou le montant est nul, un
  tirage sinon ; le critique multiplie **avant** l'armure (200 puis l'armure, pas 100 puis l'armure puis ×2) ;
  la fréquence des critiques suit le rapport (rapport 1 : entre 130 et 270 sur 4 000 coups, 5 % attendus ;
  plafond : entre 1 800 et 2 200 ; capacité défensive énorme : aucun) ; même graine, mêmes résultats ;
  `resoudreSoin` (sans capacité offensive : inchangé et sans tirage ; critique ×2 sans armure ; 5 % contre la
  référence de 100).
- **`tests/combat/test_Vitalite.cpp`** (+3) : un critique retire le montant multiplié ; un mort ne tire rien ;
  la capacité défensive des défenses compte pour le critique. `definirArmure` devient `definirDefenses`.
- **`tests/combat/test_StatistiquesDeCombat.cpp`** (nouveau, 4) : Adresse 10 donne 100 ; 5 par point
  (50 à Adresse 0, 120 à 14) ; seule l'Adresse compte ; bornes (offensive 0, défensive 1).
- **`tests/escouade/`** : `test_Personnage.cpp` (statistiques tirées de l'Adresse ; l'attaque de base porte
  la capacité offensive ; `avancerAttaque` rend une `Attaque`), `test_Escouade_skills_soin.cpp` (un soin
  critique multiplie le montant rendu et le fait le dit, sur 40 graines ; plafonné aux points de vie maximum
  même critique), `test_Escouade_equipement.cpp` (`statistiquesDeCombatDuPersonnage`, `attaqueDeBase`).
- **`tests/monstres/`** : `test_Monstre.cpp` (un monstre prend toutes ses valeurs de sa définition ; son
  attaque porte la capacité offensive de la définition), `test_Monstre_retour.cpp` (en retour, invulnérable
  **sans tirer**). Les ~80 constructions deviennent `Monstre(centre, id, groupe, monstreAvecArmure(armure))`.
- **`tests/jeu/test_Partie_evenements.cpp`** (+1) : un guerrier d'Adresse énorme (capacité offensive 5 050,
  50 % de critiques, ×2) frappe un monstre : les événements de dégâts portent `critique`, et leur montant est
  celui de `degatsAttendus` ; il y a des critiques et des coups normaux. `test_Partie_zonesDeControle.cpp` :
  le coup gratuit lit son événement.
- **`tests/interface/`** : `test_TextesFlottants.cpp` (+3 : libellés « Critique ! », couleur jaune et taille de
  18 px même pour un coup gratuit, soin critique vert), `test_AffichageStatistiques.cpp` (nouveau, 1),
  `test_DispositionInventaire.cpp` (+2 : cellules du bloc des statistiques dans le bloc, bloc sous celui des
  caractéristiques et étiquette du sac en dessous).

## Étape 90 : esquive et blocage (+80 cas, 1 167 → 1 247)

- **Isoler le hasard** : trois choix pour que les tests de règle restent déterministes. (1) `attaqueSansHasard`
  est de nature zone (voir « Étape 89 »), donc `subir` et `subirCoup` ne sont jamais esquivés ni bloqués.
  (2) `s_equipementMeleeDeTest` ne contient **plus le bouclier rond** (casque, cuirasse, jambières : l'armure
  de test baisse de 6, et `s_armureMeleeDeTest` la calcule) : un guerrier de test qui recevrait les coups d'un
  monstre en bloquerait 30 % au hasard. Les tests de blocage l'ajoutent explicitement
  (`equipementDeDepart.push_back({Emplacement::MainGauche, IdObjet::BouclierRond})`). Les trois tests
  d'objets qui déplaçaient le bouclier d'un personnage à l'autre déplacent les jambières de fer
  (`Emplacement::Jambes`). (3) `defensesDeTest(armure)` donne `{armure, statistiques}` (capacité défensive 100,
  rien d'autre) ; les tests qui veulent une esquive ou un bouclier modifient ces statistiques.
- **Une sélection de graines plutôt qu'un tirage forcé** : un test qui doit observer un événement rare (un
  coup esquivé à 5 %, un blocage à 30 %) boucle sur des graines (`Generateur generateur(graine)`, de 1 à 50,
  100 ou 300) ou sur un grand nombre de coups jusqu'à l'observer, puis vérifie ses invariants. Tout est
  déterministe : la même suite à chaque exécution. Un test qui ne verrait jamais l'événement échoue
  (`REQUIRE(esquiveVue)`) au lieu de passer en silence.
- **Compter les tirages** : `resoudreCoup` ne tire que si une chance est non nulle ; les tests le vérifient en
  copiant le générateur avant l'appel et en le comparant après (`REQUIRE(generateur == intact)`), et un test
  compte les nombres consommés (un coup esquivé : 1 ; un coup touché à esquive et critique possibles : 2).
- **`tests/combat/test_Esquive.cpp`** (nouveau, 8) : `esquiveEffective` (plafond 50, négatif à 0) ; la chance
  contre le contact, contre les projectiles, jamais contre une zone, plafonnée ; `coupEsquive` (strict, nul
  sans chance).
- **`tests/combat/test_Blocage.cpp`** (nouveau, 9) : la chance (contact et projectile, pas une zone, nulle en
  récupération, nulle sans bouclier, ramenée de 0 à 100 %) ; `appliquerBlocage` (partiel, total, sous la
  chance seulement, strict, jamais sans chance).
- **`tests/combat/test_EvenementDeCombat.cpp`** (nouveau, 5) : `detailsDuCoup` (esquivé : `Esquive` seul ;
  dégâts avec coup gratuit et critique ; blocage partiel : `Degats` puis `Bloque` non total ; blocage total :
  `Bloque` total seul, pas de `Degats` de 0 ; résultat « rien » : liste vide).
- **`tests/combat/test_ResolutionDuCoup.cpp`** (+15) : un coup esquivé s'arrête, sans dégâts ni critique ; la
  fréquence des esquives (5 % : entre 130 et 270 sur 4 000 coups ; 20 % ; plafond de 50 % pour un défenseur à
  90 %) ; l'esquive contre le contact ne joue pas contre les projectiles et inversement ; une zone n'est
  jamais esquivée, sans tirage ; aucun tirage sans esquive ni capacité offensive ; un coup esquivé ne tire pas
  son critique ; **le blocage retire un montant fixe puis l'armure joue sur le reste** ; **un blocage total
  s'arrête sans armure ni plancher (0, pas 1)** ; le blocage retire du montant **déjà critique** ; une zone
  n'est jamais bloquée ; pendant la récupération, ni blocage ni tirage ; sans bouclier, ni blocage ni tirage ;
  un coup esquivé n'est pas bloqué ; la fréquence des blocages (30 % : entre 1 100 et 1 300 sur 4 000).
- **`tests/combat/test_Vitalite.cpp`** (+7) : un coup esquivé ne retire aucun point de vie ; un blocage
  retire le montant bloqué et lance la récupération (`blocagePret()` faux) ; pendant la récupération le coup
  passe entier, sans tirer de blocage ; la récupération s'écoule (`mettreAJour(2)`, puis `0,9`, puis `0,2` :
  prête à 3,1 s) ; un coup de zone ne lance pas la récupération ; un blocage total ne retire aucun point de
  vie ; sans bouclier le blocage est toujours prêt.
- **`tests/combat/test_StatistiquesDeCombat.cpp`** (+3) : sans bonus aucune esquive ; les bonus de
  l'équipement s'ajoutent ; l'addition champ par champ. **`test_Skill.cpp`** (+1) : la nature d'un skill vient
  de sa définition (contact par défaut). `test_Specialite.cpp` (+1) : nature de chaque attaque de base et de
  chaque skill du catalogue.
- **`tests/objets/`** : `test_CatalogueObjets.cpp` (+3 : bottes +5 % contact, casque de cuir +5 %
  projectiles, bouclier rond 30 / 40 / 3 s, et aucun autre objet n'a de bonus de combat), `test_Equipement.cpp`
  (+2 : `bonusDeCombatTotal` additionne et retire).
- **`tests/escouade/`** : `test_Personnage.cpp` (+6 : l'esquive vient de l'équipement et se recalcule, y
  compris à la construction ; un personnage qui esquive ne perd rien ; le bouclier rond donne ses statistiques ;
  après un blocage plus aucun pendant 3 s de `mettreAJour`, puis de nouveau ; la nature d'une attaque de base
  et d'un skill), `test_Escouade_agresseurs.cpp` (+2 : **un coup esquivé et un coup bloqué en entier
  désignent quand même leur auteur comme agresseur**, sans dégât), `test_Escouade_combat.cpp` (+1 : contact
  pour la mêlée, projectile pour les tireurs), nature `Zone` des coups de l'explosion et `Contact` de la frappe
  (assertions ajoutées).
- **`tests/monstres/`** : `test_Monstre.cpp` (+4 : le monstre ordinaire esquive 5 % et 5 % ; un coup esquivé ne
  lui retire rien mais le **réveille** quand même ; son attaque est de contact ; un monstre au bouclier récupère
  son blocage avec le temps qui passe).
- **`tests/jeu/test_Partie_evenements.cpp`** (+2) : sur 40 s, un guerrier infatigable (1 dégât, sans recharge)
  qui frappe un monstre : au moins un `Esquive` sur lui, jamais de `Degats` dans la même image (un seul
  attaquant), la grande majorité des coups touche ; sur 120 s, un guerrier au bouclier rond sous les coups d'un
  monstre : chaque `Bloque` vaut 40, n'est pas total (un monstre frappe pour 100) et est précédé d'un `Degats`
  dans la même image (le texte du blocage est donc au-dessus), et deux blocages sont séparés d'au moins 3 s.
- **`tests/interface/`** : `test_TextesFlottants.cpp` (+3 : libellés « Esquivé », « Bloqué », « Bloqué −N » ;
  gris, 14 px, l'esquive plus claire ; le blocage partiel s'empile au-dessus des dégâts), `test_AffichageStatistiques.cpp`
  (+5 : les cinq lignes du bloc, « Blocage 0 % » sans bouclier, esquive plafonnée à l'affichage, lignes de bonus
  d'objet, virgule des secondes), `test_AffichageObjets.cpp` (+3 : l'infobulle des bottes, du casque de cuir et du
  bouclier rond).

## Étape 91 : les tests ne dépendent pas de la chance, les armes (+56 cas, 1 247 → 1 303)

### Les tests ne dépendent pas de la chance (91.1)

Le moteur du coup tire au hasard (esquive, critique, blocage, recharges). Un test qui n'est vert que parce
que la graine fixe (`s_graineDeTest`) ne produit pas d'esquive est fragile : il casse dès qu'un tirage
s'ajoute ou change d'ordre. Trois règles, appliquées à tous les tests existants.

1. **Les événements rares se testent sur les fonctions d'étape pures, avec un tirage choisi** (0,01 et 0,99 :
   `appliquerCritique`, `coupEsquive`, `appliquerBlocage` reçoivent le tirage). **Jamais dans une boucle qui
   cherche une graine** ni avec un générateur « truqué » par une recherche de graine : une telle aide
   (`generateurQuiTireSous`, essayée à l'étape 91.1 puis supprimée à l'étape 91.4) ne contrôle qu'**un** tirage,
   alors qu'un coup peut en faire trois (esquive, critique, blocage) et qu'une recharge aléatoire en ajoute. Un
   test d'intégration qui a besoin d'un événement rare fait donc l'une des deux choses suivantes :
   - il **lit le résultat ou l'événement** et vérifie, pour **chaque** coup d'une série (un même générateur
     de test, des centaines de coups), ce qui doit être vrai que l'événement ait eu lieu ou non (« esquivé ou
     non, le monstre devient agresseur » ; « critique ou non, le montant est celui de `degatsAttendus` ») ;
   - et, quand il faut que l'événement soit vu au moins une fois, la série est assez longue pour que la chance
     de ne jamais le voir soit de l'ordre de 2⁻⁶⁰ ou moins (50 % sur cent coups, 5 % sur deux mille), quelle
     que soit la graine.
2. **Les tests qui comptent des dégâts exacts n'ont pas de hasard de défenseur.** Dans `Partie`, les monstres
   sont ceux de `monstreSansHasard()` (le monstre ordinaire sans esquive), passés au constructeur de `Partie`
   (`partieDeTest`, `unePartie`) ; `monstreIndestructible()` y ajoute des points de vie énormes pour les tests qui
   durent ; dans les tests de `Monstre`, `monstreAvecArmure`. Reste le critique de l'attaquant, qui n'a **jamais une
   chance nulle** : le test lit alors `critique` dans l'événement et compare à `degatsAttendus(attaque, defenses,
   critique)` (même règle pour un monstre qui frappe un personnage).
3. **Écarter le hasard se fait par une aide explicite, jamais par un détour par une règle de jeu.**
   `attaqueSansHasard` ne repose plus sur la nature `Zone` (qui ne s'esquive ni ne se bloque) : c'est une
   attaque de contact de capacité offensive nulle (jamais critique), et c'est le **défenseur** qui n'a pas de
   chance (`monstreSansHasard`, `defensesDeTest`, un personnage de test sans bottes ni bouclier). Un test qui
   donne le bouclier du catalogue à un défenseur retire le bouclier de son équipement quand il ne parle pas de
   blocage.

**Vérification** : le balayage des graines. Remplacer temporairement `s_graineDeTest` par une valeur lue dans
l'environnement et lancer `tests.exe` sur 60 graines ne doit faire échouer aucun test (neuf tests fragiles
trouvés ainsi à l'étape 91.1 : coups d'un skill et d'attaques de base comptés au point de vie près, critiques et
esquives attendus au premier tir, un personnage surpris en pleine exécution d'un coup à la recharge aléatoire).
Le balayage a été refait à chaque sous-étape suivante.

`Partie` reçoit pour cela un quatrième paramètre optionnel, que le jeu ne précise pas : depuis l'étape 93, le
catalogue des monstres (`CatalogueDeMonstres`), et non plus une définition commune à tous (`docs/monstre.md`).

### Constructeurs de test : le patron *Test Data Builder* (91.4)

Les tests qui créent des personnages le font par des **aides** de `aides_escouade.hpp`, pas par la signature
complète de `DefinitionPersonnage` ou du constructeur de `Personnage` : un futur changement de signature
(comme la disparition de `attaqueDeBase` à l'étape 91.4) ne touche alors que les aides, pas les centaines de
sites d'appel. Les aides : `guerrierDeTest`, `pretreDeTest`, `mageDeTest`, `definitionArmee(arme)` (sans classe, armée
de cette arme), `definitionSansArme()`, `unPersonnageArme(centre, arme)`, `avecArme`, `avecPuissance`,
`avecEnduranceEnorme`, `skillExigeantDesArmes`, `partieDeTest`, `unePartie`. On ne réécrit pas pour autant un test
qui ne casse pas.

### L'attaque de base vient d'une arme du catalogue (91.4)

Les personnages de test portent l'épée longue (mêlée), le sceptre (prêtre) et le bâton (mage), du catalogue des
objets : c'est la seule dépendance au catalogue des objets de leur attaque, et les constantes (`s_degatsBaseDeTest`,
`s_porteeMeleeDeTest`, `s_rechargeMeleeMinDeTest`…, `aides_escouade.hpp`) sont **lues** dans la description de
l'attaque de l'arme (`inline const`, plus `constexpr`), jamais écrites en littéral. Leurs caractéristiques sont
`s_caracteristiquesDeTest` (Puissance de référence 10, les autres à 0) : l'attaque de base, qui profite à plein de la
Puissance, fait exactement les dégâts de l'arme ; Adresse 0 et Endurance 0 comme avant (capacité 50, 240 points de
vie). Une attaque « reconnaissable » (un coup qu'on suit parmi d'autres) s'obtient par une Puissance élevée
(`avecPuissance`) ou par le choix de l'arme (l'épée à deux mains frappe une fois en 1,2 s), pas par une attaque
écrite à la main ; un monstre qui ne doit pas mourir est `monstreIndestructible()`.

### Nouveaux tests

- **`tests/objets/`** : `test_DeplacementDObjets.cpp` (+16 : les deux mains dans les deux sens, première case
  libre, case quittée par l'arme libre pour le bouclier dans un sac plein, sac plein et rien ne se libère = refus
  et rien ne bouge, échange entre deux personnages à ses deux extrémités, retirer une arme à deux mains ne range
  rien, règle de base conservée, `deplacementPossible` ne change rien) ; `test_CatalogueObjets.cpp` (+4 : les sept
  armes, leurs types, mains et attaque ; seules les armes ont des données d'arme et seuls les boucliers des données
  de bouclier ; `estADeuxMains` ; bouclier rond 30 % et 12 points) ; `test_Emplacement.cpp` (+1 : l'arme va en
  main droite seulement) ; `test_Equipement.cpp` (+3 : `typeDArme`, `attaqueDeBase`, le bouclier compte dans les
  statistiques de combat).
- **`tests/escouade/`** : `test_Personnage.cpp` (+10 : zone de contrôle, portée et attaque de base qui suivent l'arme ;
  changer d'arme relance une recharge complète tirée dans l'intervalle de la nouvelle arme, annule le coup en
  cours, et un changement qui laisse l'arme intacte n'y touche pas ; un skill qui exige une arme est prêt, refusé
  sans recharge consommée, annulé si on perd l'arme, inchangé si l'autre arme convient) ; `test_Escouade_equipement.cpp`
  (+5 : les deux mains avec le guerrier, `attaquer` relance la recharge après un changement d'arme) ;
  `test_Escouade_skills.cpp` (+5 : les armes exigées avec les vraies classes, le skill en attente abandonné) ;
  `test_Specialite.cpp` (+5 : armes de départ, exigences, chaque classe porte l'arme de ses skills).
- **`tests/combat/`** : `test_Skill.cpp` (+2 : `convientA`) ; `test_Escouade_skills_degats.cpp` (+1 : le `static_assert` des attaques de base devient un test) ; `test_Vitalite.cpp` : la récupération du blocage est
  la constante commune.
- **`tests/interface/`** : `test_AffichageObjets.cpp` (+3 : infobulles des armes) ; `test_BarreDeSkills.cpp` (+1 :
  `armeAbsente`).

## Étape 92 : seuils, objets inactifs, pénétration (+54 cas, 1 303 → 1 357)

- **Les personnages de test atteignent les seuils de leurs objets.** Sans cela, un objet à seuil de leur équipement
  de départ serait inactif (étape 92.2) et leurs armures et leurs attaques ne s'appliqueraient pas. Aides de
  `aides.hpp` et `aides_escouade.hpp`, qui lisent les seuils au catalogue (jamais écrits en littéral) :
  `caracteristiquesPourPorter(objets)` (la Puissance de référence 10 et juste ce qu'il faut pour les seuils),
  `capableDePorter(definition, objets)`, `capableDeToutPorter(definition)`, `uneEscouadeCapableDeToutPorter(position)` (pour
  les tests qui déplacent des objets entre personnages sans parler de seuils), `avecCaracteristique`. Les
  personnages de test ont donc les caractéristiques de leur équipement (guerrier : Puissance 11 et Endurance 12 ; 312
  points de vie).
- **`tests/objets/test_ObjetsActifs.cpp`** (nouveau, +8) : le point fixe sur des objets inventés (le catalogue n'a aucun
  objet à la fois bonus et seuil) : sans seuil, seuil atteint exactement, bonus d'un autre objet, un objet ne
  compte pas pour son propre seuil, la chaîne (désactiver un objet en désactive un autre, sur plusieurs maillons et dans
  les deux ordres), la remise en service, deux objets qui se soutiennent.
- **`tests/objets/`** : `test_CatalogueObjets.cpp` (+4 : les seuils de chaque objet, ni cuir ni bijou n'en a, `seuilsAtteints`,
  la dague perce 30 % et aucune autre arme) ; `test_DeplacementDObjets.cpp` (+11 : sous le seuil rien ne bouge, seuil
  atteint exactement, les deux seuils de l'épée à deux mains, les bonus des autres objets comptent, vers le sac aucun
  seuil, l'échange entre deux personnages exige les deux côtés, `seuilsManques`) ; `test_Equipement.cpp` (+4 : `actifPour`,
  un objet inactif n'a aucun effet, `caracteristiquesEffectives`).
- **`tests/escouade/`** : `test_Personnage.cpp` (+7 : une arme inactive donne les mains nues, l'anneau de puissance, une armure
  ou un bouclier inactif, une arme inactive refuse les skills, la pénétration de l'attaque de base) ;
  `test_Escouade_equipement.cpp` (+6 : le mage ne reçoit pas la cuirasse, le guerrier et l'épée à deux mains avec
  l'anneau, retirer l'anneau la rend inactive sans la déplacer, `seuilsManques`, `objetPorteEstInactif`, la pénétration
  lue par l'escouade) ; `test_Specialite.cpp` (+2 : chaque classe atteint les seuils de son équipement de départ,
  personne ne porte l'épée à deux mains au départ).
- **`tests/combat/`** : `test_Defense.cpp` (+5 : la pénétration réduit l'armure sans arrondi intermédiaire, plafond de 75 %,
  négative sans effet, jamais moins que sans) ; `test_ResolutionDuCoup.cpp` (+2).
- **`tests/interface/`** : `test_AffichageObjets.cpp` (+5 : lignes de seuil, objet sans seuil, ligne d'objet inactif,
  pénétration de la dague).
- **Vérification des graines** : le balayage de 40 graines est resté vert à chaque sous-étape.

## Étape 93 : les types de monstres (+19 cas, 1 357 → 1 376)

- **`tests/jeu/test_CoherenceDesDonnees.cpp`** (nouveau, +8) : ces tests vérifient les **données** des catalogues, pas du
  code, et accueillent la vérification de chaque donnée ajoutée plus tard. Équipement de départ : chaque classe atteint les
  seuils de tout son équipement de départ (aucun objet de départ inactif). Monstres : chaque définition est valide (valeurs
  positives, recharge minimale au plus égale à la maximale, esquives au plus égales au plafond), deux catégories ne se
  ressemblent ni par la couleur ni par la taille, aucun cercle ne touche les murs d'un chemin par les centres des
  cases (la demi-case de `tailleCase`), la créature garde 4 px de marge. Compositions de groupe : non vides et cohérentes
  (au moins un monstre possible, min au plus égal à max, poids positif), la créature est seule, les paramètres du générateur
  portent les compositions du jeu. Aucune valeur du catalogue n'y est écrite en dur : les règles sont relatives (comparaisons
  entre champs, `ParametresDonjon`).
- **`tests/monstres/test_Monstre_etats.cpp`** (+1) : chaque catégorie traverse un couloir en L (les deux sens, largeur du
  générateur) entre deux salles et atteint sa proie. C'est ce test qui a révélé que 19 px de rayon passent et 20 non
  (`docs/monstre.md`).
- **`tests/jeu/test_Partie_monstres.cpp`** (+1) : chaque monstre du niveau reçoit la définition de sa catégorie (points
  de vie, rayon, attaque, statistiques).
- **`tests/monde/test_GenerateurDonjon.cpp`** (+10, −1) : un mélange donne une infanterie lourde et un ou deux
  infanteries ; une nuée est groupée autour de son premier monstre, une case par monstre ; une nuée a son nombre
  complet dans une salle assez grande, autant qu'il en tient dans une salle trop petite ; la créature est seule dans la
  salle la plus éloignée, à égalité dans la plus petite ; sans composition pour la salle la plus loin, aucune créature ;
  les trois compositions ordinaires sortent (200 graines : en voir une absente est de l'ordre de 0,7^400) ; chaque
  groupe suit une composition ; au plus une créature par niveau. Le test « même armure que `creerNiveau` » disparaît
  avec `Niveau::armureMonstres`. Les tests de nombres (au plus max, une case libre, abandon, groupes) passent par
  l'aide `parametresAvecInfanterie(min, max)` (une seule composition d'infanterie, pas de créature) au lieu de
  `monstresParSalleMin` et `monstresParSalleMax`.
- **Aides** (`aides.hpp`) : `monstreSansHasard()` est l'infanterie sans esquive **ni armure** (les niveaux de test n'ont
  jamais eu d'armure de monstre) ; `monstreAvecArmure(armure)` y est passée (elle était dans `aides_monstre.hpp`) ;
  `catalogueDeTest(infanterie = monstreSansHasard())` est le catalogue du jeu où l'infanterie est celle donnée et où
  les autres catégories n'ont pas d'esquive ; `partieDeTest(niveau, composition, graine, monstre)` en dépend. Un test
  qui veut un monstre indestructible ou sans esquive le dit comme avant (`monstreIndestructible()`), mais cela ne
  vaut que pour l'infanterie ; un niveau de test qui nomme une autre catégorie la reçoit du catalogue du jeu, sans
  esquive.
- **Mesure de performance, cachée** (`tests/jeu/test_Partie_performance.cpp`, étiquette Catch2 `[.][perf]`, donc
  absente de CTest et de `tests.exe` sans argument). Elle cherche la première graine (parmi les 2 000 premières) dont le
  niveau a une nuée de 7, pose l'escouade (increvable, pour que le combat dure) à 3 cases du premier monstre de la
  nuée, avec tous les autres groupes en place, simule 600 images de `Partie` et affiche (par `WARN`) le temps moyen et
  le pire d'une image, sur les 600 images et sur celles où une nuée est en vie. Elle n'affirme rien sur les temps. À
  lancer avec le preset `jeu` (Release) :

  ```bash
  cmake --build --preset jeu
  ./build/jeu/tests.exe "[perf]"
  ```

  Mesure de l'étape 93 (niveau de 19 monstres, 429 images de combat contre la nuée) : moyenne 0,04 ms par image, pire
  0,13 à 0,18 ms, loin des 5 ms et 16 ms qui feraient signaler un problème.
- **Vérification des graines** : non refaite en bloc ; les tests ajoutés ne dépendent d'aucune graine (les propriétés se
  vérifient sur 50 graines, la distribution des compositions sur 200).

## Étape 94 : types de dégâts et résistances (+63 cas, 1 376 → 1 439)

- **94.1** : `Attaque::montant` devient `degats`, `soin` et `dureeDeProvocation`. Aides : `degatsOuZero` (ex-`montantOuZero`),
  `soinSansHasard` ; `lancerSkillEnEntier` (`test_Personnage.cpp`) rend l'`Attaque` entière. Un test : l'attaque d'un skill range
  son montant dans le champ de son type, les deux autres restent à 0.
- **94.2** : `tests/combat/test_Element.cpp` (nouveau) : valeurs à zéro par défaut, accès par élément, total, copie par
  valeur, `seul`, sentinelle (la taille suit l'énumération, `s_elements` dans l'ordre), `elementDominant` (égalité : le
  premier), `enPortions`, `degatsDe` / `degatsPhysiques`. L'événement `Degats` devient `DegatsSubis` (mise à jour des tests
  d'événements et de textes).
- **94.3** : `tests/combat/test_ResolutionDuCoup_portions.cpp` (nouveau, thème « portions » de `test_ResolutionDuCoup`) : un coup
  mixte avec armure et résistance ; l'armure ne touche pas un élément ; la résistance physique ; le critique sur toutes
  les portions ; le blocage ne réduit que le physique ; une portion physique entièrement bloquée sans plancher, avec une
  portion élémentaire qui passe ; un projectile purement élémentaire (aucun tirage, aucune récupération lancée, y compris
  dans `Vitalite`) ; `Bloque` total seulement si tout est absorbé ; plafond de 80 %, vulnérabilité, −300 ramené à −100 ; un
  arrondi par portion (3 × 50 % deux fois fait 4, pas 3) ; le minimum de 1 dans la plus grosse portion ; la pénétration ne
  touche pas la résistance ; un coup esquivé n'a aucune portion ; un coup vide ne tire rien. Étapes pures : `armureAppliquee`,
  `resistanceEffective`, `appliquerResistance` (`test_Defense.cpp`), `coupCritique` (`test_Critique.cpp`), `appliquerBlocage`
  flottant (`test_Blocage.cpp`), résistances dans `StatistiquesDeCombat` (`test_StatistiquesDeCombat.cpp`). **Aucune valeur
  attendue des tests existants n'a changé** ; seule l'aide `degatsAttendus` suit le calcul sans arrondi intermédiaire.
- **94.4** : l'élément dominant dans `detailsDuCoup` (`test_EvenementDeCombat.cpp`, 3) ; les couleurs des nombres
  (`test_TextesFlottants.cpp`, +6 : physique inchangé, feu et sacré distincts, distance RGB d'au moins 50 entre toutes les
  couleurs de nombres, un critique élémentaire reste jaune) ; `test_CoherenceDesDonnees.cpp` (+1, et résistances dans
  [−100, 80] dans la validité d'une définition) ; l'explosion du mage est du feu et les autres skills physiques
  (`test_Specialite.cpp`) ; un skill de zone de feu (`test_Personnage.cpp`, +2) ; contre l'infanterie lourde le feu ignore
  l'armure, la créature encaisse 60 % du feu (`test_ResolutionDuCoup_portions.cpp`, +2). Le mage de test
  (`mageDeTest`) a une explosion de feu, comme la classe.
- **94.5** : la frappe sacrée (`test_Specialite.cpp`, +4 : la masse donne le physique de l'arme et le sacré du bonus, le total
  d'avant à 1 près ; l'épée à deux mains donne un physique plus grand ; sans l'arme exigée elle est refusée, icône grisée ;
  dorée contre l'infanterie lourde, physique si l'épée à deux mains domine) ; `test_Personnage.cpp` (+3 : la pénétration vient
  de l'arme, la nature et les dégâts physiques aussi, changer d'arme change le physique pas le bonus) ; `test_Skill.cpp` (+1).
- **Mesure de performance** (preset `jeu`, `[perf]`) : inchangée, moyenne 0,035 à 0,04 ms par image, pire 0,12 à 0,16 ms.
- Une règle apprise : **un nom de `TEST_CASE` ne contient ni guillemets français ni accents** (« Bloque », pas
  « Bloqué ») : CTest ne retrouve pas le test sinon.

## Étape 95 : les objets ont leurs propres données, le jeu en anglais (+55 cas, 1 439 → 1 494)

- **95.2** : aucun changement de comportement ; les textes comparés par les tests passent à l'anglais
  (`test_AffichageObjets`, `_AffichageCaracteristiques`, `_AffichageSkills`, `_AffichageStatistiques`, `_BarreDeSkills`
  pour les sigles des skills, `_TextesFlottants`, `_Specialite` pour les noms des classes, `_EcranChoixEscouade`) ; le
  séparateur décimal de l'infobulle devient le point. Le test « le nom d'un skill est en UTF-8 » devient « en anglais » :
  plus aucun nom de skill n'a d'accent (le signe moins de « Blocked −N » reste le seul caractère hors ASCII).
- **95.3** : `tests/objets/test_CatalogueAffixes.cpp` (nouveau, +5 : l'ordre de `IdAffixe`, un nom distinct et une
  catégorie permise au moins par affixe, quatre préfixes puis huit suffixes, `definitionAffixe`, `affixePermis`) ;
  `test_CatalogueObjets.cpp` (+9 : un objet sans autre précision est commun, `objetValide` pour tout modèle, pour chaque rareté, les positions, les catégories, la valeur strictement positive, `nomDeLObjet`, et l'égalité par valeur — modèle, rareté, valeur, affixe, position — remplace l'ancien test d'égalité). **Le test d'exemplaires compte par valeur complète** (`test_Escouade_equipement.cpp` :
  `exemplaires` rend les objets du sac et des équipements sous forme de clés triées, `Exemplaires`) ; un test (+1) vérifie
  qu'un objet à affixes garde ses affixes du sac à l'emplacement et retour.
- **95.4** : `tests/objets/test_Equipement_affixes.cpp` (nouveau, +17 : armure d'un préfixe, esquives, caractéristique,
  blocage — la chance s'ajoute, le montant reste —, masse « Blazing » en physique + feu, « Hallowed », pénétration de
  « Keen » ajoutée à celle de l'arme et plafond de 75 % à l'application, un objet inactif perd ses affixes, les seuils ne
  dépendent que du modèle, un suffixe de caractéristique rend un autre objet actif, les affixes d'un objet inactif ne
  comptent pas, une chaîne d'objets actifs par leurs affixes) ; `tests/escouade/test_Personnage_affixes.cpp` (nouveau, +5 :
  la Puissance joue sur chaque portion, une masse sans affixe frappe comme avant, un arc « Keen », le suffixe de Dexterity
  monte les capacités, la recharge n'est refaite que si l'attaque change) ; `test_Specialite.cpp` (+3 : la frappe sacrée
  avec une masse « Blazing » a trois portions, « Hallowed » additionne deux arrondis, la pénétration de l'arme). **Aucune
  attente des tests existants n'a changé** : les attaques de base sont lues avec `degats[Element::Physique]` (aide
  `s_degatsBaseDeTest`).
- **95.5** : `test_AffichageObjets.cpp` (+7 : l'infobulle d'un objet rare, magique et commun, les deux lignes d'armure d'une
  cuirasse renforcée, `ligneDAffixe` pour chaque effet, une ligne non vide pour tout affixe, `couleurDeLaRarete`, le contraste
  d'au moins 4,5 sur le gris à 70 — le fond le plus clair de l'inventaire — ; les tests d'infobulle existants suivent le nouvel
  ordre : les seuils en dernier) ; `test_CommandeDeTouche.cpp` (F7 en tout temps) ; `test_Escouade_equipement.cpp` (+4 :
  `ramasserTout` met les objets dans l'ordre, **refuse tout quand il manque des cases libres**, deux appels de suite, liste
  vide) ; `test_CatalogueObjets.cpp` (+1 : les quatre objets de F7, leurs noms et leurs valeurs). L'effet de F7 sur `EcranJeu` n'est pas testé (cet écran a besoin d'une fenêtre) : il se réduit à un appel de
  `ramasserTout(objetsDeDemonstration())`.
- **95.6** : `test_CoherenceDesDonnees.cpp` (+3 : les objets de démonstration respectent les invariants, le sac de départ est
  fait d'objets communs valides, chaque affixe peut être porté par un modèle du catalogue).
- **Mesure de performance** (preset `jeu`, `[perf]`) : voir `docs/historique.md`, étape 95.
- Une règle apprise : un `Objet` est un agrégat à valeurs par défaut, `Objet{IdObjet::Masse}` est commun ; les tests
  n'écrivent les affixes que quand ils les testent. Une aide de test qui prend un `IdObjet` pour porter une arme n'accepte pas
  d'affixes : les tests d'affixes posent l'objet après la construction (`poserEquipement`).

## Étape 96 : la génération aléatoire des objets (+17 cas, 1 494 → 1 511)

Les tirages utilisent une graine fixe : un test qui échoue échoue toujours de la même façon. Aucun test n'attend un objet
précis pour une graine (la suite dépend de `std::uniform_int_distribution`, `docs/objets.md`, « Génération aléatoire ») ; ils
vérifient des propriétés.
- **96.2** : `tests/objets/test_GenerationDObjets.cpp` (nouveau, +4 : les poids 70, 25, 5 ; tout modèle peut être commun ; un
  bijou peut être magique et jamais rare ; gants, bottes, armures et armes peuvent être rares) ; `test_CatalogueAffixes.cpp`
  (+1 : `affixesPermisPour` ; et Reinforced permis sur les gants et les bottes) ; `test_CoherenceDesDonnees.cpp` (+2 : chaque
  plage a `min >= 1` et `min <= max` ; chaque rareté a un poids positif et au moins un modèle qui peut la porter).
- **96.3** : `test_GenerationDObjets.cpp` (+6, 10 000 tirages d'une graine fixe : tous valides ; chaque rareté, chaque modèle et
  chaque affixe apparaissent et toute valeur est dans sa plage ; les bornes d'une plage sortent ; un anneau ou un collier
  n'est jamais rare alors que des gants et des bottes le sont, avec Reinforced pour préfixe ; **proportions** de raretés à
  deux points des poids ; même graine, même suite, autre graine, autre suite). **La tolérance de deux points** : sur
  10 000 tirages l'écart type d'une proportion est d'au plus 0,5 point (à 70 %), donc deux points valent plus de quatre
  écarts types ; la graine étant fixe, la tolérance ne sert pas à absorber le hasard mais à détecter un algorithme modifié
  (poids changés, rareté retirée après coup parce que le modèle ne convient pas).
- **96.4** : `tests/jeu/test_Partie_objets.cpp` (nouveau, +5 : quatre objets valides de plus dans le sac, rien d'autre ne
  bouge ; le sac juste assez libre accepte ; **sans assez de cases libres, rien n'est ajouté ; un refus ne consomme aucun
  tirage** — deux parties de même graine, l'une essaie 4 puis 3 objets, l'autre 3 : leurs sacs sont identiques ; la graine
  décide des objets) ; `test_Inventaire.cpp` (+1 : `casesLibres`). Les objets de démonstration fixes disparaissent, ainsi que
  leur test (-2) : `test_Escouade_equipement.cpp` utilise son propre lot de quatre objets à affixes (`s_lotDeQuatre`, qu'aucun
  personnage ne porte au départ, pour que les exemplaires se comptent par valeur complète). L'effet de F7 sur `EcranJeu`
  n'est toujours pas testé (cet écran a besoin d'une fenêtre) : il se réduit à un appel de
  `Partie::ramasserDesObjetsAleatoires(4)`.

## Étape 97 : le butin au sol (+51 cas, 1 511 → 1 562)

Le hasard des tests de butin est maîtrisé de deux façons : les règles du tirage sont testées sans `Partie`
(`genererButin`, sur des `Generateur` qu'on compare avec `==`, `std::mt19937` est comparable), et les tests de `Partie`
fixent le butin d'un monstre à 0 ou à 100 %, donc sans chance à tirer.
- **97.1** : aucun test ajouté ; les 1 511 d'avant passent sans changer une valeur (la suite des tirages d'un donjon ou d'un
  objet pour une graine est la même).
- **97.2** : `test_CoherenceDesDonnees.cpp` (+0 : le test « chaque definition de monstre est valide » vérifie en plus une chance
  de 0 à 100 et au moins un objet quand elle est positive). `aides.hpp` : `monstreSansHasard` et `catalogueDeTest` mettent
  `chanceDeButin` à 0 : les monstres des tests existants ne tirent rien à leur mort, donc leurs suites aléatoires ne changent pas.
- **97.3** : `test_GenerationDObjets.cpp` (+4 : `genererButin` — chance nulle, rien et **aucun tirage** ; chance de 100, tous
  les objets valides, ceux de `genererObjet` et **aucun tirage pour la chance** ; chance de 20 % sur 10 000 tirages, rien ou
  tous les objets, dans la proportion ; même graine, même butin) ; `tests/jeu/test_ObjetAuSol.cpp` (nouveau, +6 :
  `positionPourUnObjet` — premier objet au centre de la case de l'origine, quatre côtés avant les diagonales, jamais dans un
  mur ni sur une case prise tant qu'il en reste (21 objets sur 21 cases de sol), jamais sur une porte fermée, tout pris :
  sur la case de l'origine, déterministe) ; `tests/jeu/test_Partie_butin.cpp` (nouveau, +8 : le bon nombre d'objets valides
  à 100 %, aucun à 0 %, jamais dans un mur et sur des cases différentes, identifiants tous différents d'un monstre à l'autre,
  les monstres vivants ne laissent rien, une partie neuve (F5, nouveau niveau, recommencement : `EcranJeu::nouvelleCarte`
  remplace la `Partie`) n'a rien au sol, même graine même butin, la créature laisse toujours).
- **97.4** : `test_Partie_butin.cpp` (+3 : `objetAuSolSous` — l'objet désigné à moins de 14 px et pas à 15 ; le plus proche entre
  deux ; **un objet sur une case inconnue est caché, il se désigne une fois la case explorée, même hors de vue**, avec un mage
  qui voit à 50 px et tue d'un coup d'explosion un monstre hors de sa vue). Le dessin (`AffichageObjetsAuSol`) n'a pas de test :
  il a besoin d'une fenêtre, comme tout le dessin.
- **97.5** : `tests/jeu/test_Partie_ramassage.cpp` (nouveau, +11 : l'objet passe dans le sac quand l'ancre arrive, dans la
  première case libre ; un clic sur le sol et `ordonnerDeplacement` remplacent la visée, rien n'est ramassé ; une nouvelle visée
  remplace la précédente ; **sac plein : l'objet reste au sol, un seul `SacPlein`, l'ordre est terminé** ; l'événement est
  au-dessus de l'escouade entière (`idCible` 0, position de l'ancre) ; **monstre et objet sous le curseur : le clic vise le
  monstre** ; un clic sur un objet rompt le combat ; un clic juste hors de la zone de survol est un clic au sol ; un objet
  sur une case inconnue ne se clique pas) ; `test_TextesFlottants.cpp` (+1 : « Bag full », 14 px, une couleur que les autres
  textes n'ont pas).
- **97.6** : `test_Escouade_equipement.cpp` (+5 : `retirerObjet` — du sac, d'un emplacement avec l'armure qui baisse, position
  vide, l'objet n'est plus nulle part sauf chez celui qui l'a retiré, l'anneau de puissance retiré rend l'épée à deux mains
  inactive) ; `test_Escouade_skills.cpp` (+2 : retirer l'arme exigée rend le skill refusé, et abandonne un skill en attente) ;
  `tests/jeu/test_Partie_jeter.cpp` (nouveau, +6 : `jeterUnObjet` — du sac, au sol près de l'ancre ; porté, l'emplacement se vide
  et l'armure baisse ; jeter l'arme : l'attaque de base devient celle des mains nues ; position vide ; identifiants et cases
  différents ; **le compte des exemplaires (sac, équipements et sol) est conservé à travers jeter puis ramasser**) ;
  `test_EcranInventaire.cpp` (+4 : lâcher hors du panneau jette depuis le sac et depuis un emplacement, annuler ou lâcher sans
  glissement ne jette rien ; le test « lâcher dans le vide » devient « lâcher dans un écart du panneau », puisque la marge jette
  maintenant) ; `test_DispositionInventaire.cpp` (+1 : `rectangleDuPanneau`, tous les éléments y tiennent).
- **Aides** : `aides_escouade.hpp` reçoit `Cle`, `cle` et `exemplaires(const Escouade&)` (déplacés tels quels depuis
  `test_Escouade_equipement.cpp`), `exemplaires(const Partie&)` (sac, équipements et sol) et `viderLeSac` (une `Partie` range
  le sac de départ du jeu dans son sac : les tests d'objets le vident pour compter des cases). `test_EcranInventaire.cpp` construit
  maintenant l'écran avec une `Partie` (`partieSansMonstre` : une partie sans monstre est gagnée, donc figée ; les tests qui
  font avancer le temps ajoutent un monstre dormant dans un coin).
- **Mesure de performance** (preset `jeu`, `[perf]`) : inchangée, moyenne 0,036 ms par image, pire 0,16 ms.

## Étape 98 : les étages (+62 cas, 1 562 → 1 624)

Les tests de l'expédition construisent une `Partie` à partir d'une `Expedition` (`expeditionDeTest()`, profondeur 1,
équipement et sac de départ de la composition de test ; `aides_escouade.hpp`) ; `partieDeTest` et `unePartie` passent par elle,
donc tous les tests d'avant ont le même départ qu'avant. La profondeur se règle sur l'expédition (`expedition.profondeur = 3`)
avant de construire la `Partie`.
- **98.1** (+4) : `test_DispositionInventaire.cpp` (le panneau est de la taille de son contenu, centré, la même dans toutes les
  vues, tous les éléments y tiennent, **une bande de jeu d'au moins 50 px reste visible tout autour** ; le sac a 2 rangées de 20
  cases) ; `tests/jeu/test_ObjetsAuSol.cpp` (anciennement `test_ObjetAuSol.cpp` ; les tests de placement passent par
  `ObjetsAuSol::positionPour` ; +3 : les identifiants ne sont jamais réutilisés même après un retrait, `retirer` et `trouver`
  passent par l'identifiant, retirer un identifiant absent ne change rien) ; `test_EcranInventaire.cpp` construit l'écran avec
  `(escouade, sol, carte, noms)` ; `test_Partie_jeter.cpp` appelle `jeterUnObjet` (fonction libre) ; `ResultatClic::Objet` devient
  `Porte` ou `ObjetAuSol` dans les tests de portes, de clics, de ramassage et de `Controleur`.
- **98.2** (+11) : `tests/jeu/test_Expedition.cpp` (nouveau : `nouvelleExpedition` — profondeur 1, kit porté, sac de départ ;
  **aller-retour : l'équipement, le sac (avec une case que `ramasser` ne donnerait pas) et les caractéristiques de base
  survivent** ; un objet porté mais inactif reste porté et inactif ; **l'étage suivant commence à pleine vie, un mort
  compris** ; **rien de l'étage ne passe** — monstres, objets au sol, agresseurs, provocations, verrou, place de départ ; une
  expédition est une valeur ; `graineDEtage` — déterministe, une graine par profondeur, deux expéditions différentes, jamais la
  graine de l'expédition elle-même).
- **98.3** (+6) : `tests/monstres/test_DefinitionDeMonstre.cpp` (nouveau : `renforcer` — identique au 1er étage ; au 3e étage points
  de vie × 1,5, dégâts × 1,3, capacités + 20, pour chaque catégorie ; valeurs connues de l'infanterie, de la nuée et de la
  créature aux 2e et 5e étages, dont l'arrondi de 34,5 en 35 ; strictement croissant ; tout le reste inchangé champ par champ ;
  la définition reçue n'est pas modifiée) ; `test_Partie_monstres.cpp` (+1 : les monstres d'un étage profond sont renforcés).
- **98.4** (+26) : `test_TileMap.cpp` (+3 : `'>'` est une case `Escalier` que la carte retrouve, un plan sans `'>'` n'en a pas,
  l'escalier ne bloque ni le déplacement ni la vue) ; `test_GenerateurDonjon.cpp` (+8 : **un seul escalier, dans la salle la plus
  éloignée du départ, sur une case intérieure, graines 1 à 50 ; jamais sous un monstre, la créature comprise, graines 1 à 100** ;
  la salle de la créature est celle de l'escalier ; une graine redonne le même escalier, les mêmes monstres et le même plan ;
  `placerEscalier` — la seule case libre, sans monstre, une seule salle, salle pleine de monstres : le centre ; le test « toutes
  les cases des salles sont du sol » accepte l'escalier) ; `test_ObjetsAuSol.cpp` (+3 : un objet évite l'escalier même quand
  l'origine en est une, un tas autour ne le prend jamais tant qu'il reste de la place, faute de mieux il s'y pose) ;
  `tests/jeu/test_Partie_escalier.cpp` (nouveau, +12 : le clic sur l'escalier visible est `Escalier` (tout point de la case, pas la
  voisine) ; une partie sans escalier n'en a pas ; **un escalier sur une case inconnue ne se clique pas, une fois exploré il se
  clique** ; l'escouade marche et le prend quand l'ancre arrive ; passer dessus sans ordre ne le prend pas ; **un clic au sol,
  `ordonnerDeplacement` ou un clic sur un monstre remplacent la visée** ; **avec un monstre ou un objet au sol sous le curseur, le
  clic ne vise pas l'escalier** (49 objets remplissent les 48 cases voisines, le 49e s'empile sur l'escalier) ; on descend en
  plein combat sans avoir rien tué ; l'ordre lève le verrou).
- **98.5** (+15) : `tests/jeu/test_Partie_descente.cpp` (nouveau, +14 : profondeur de la partie ; l'escalier d'un étage ordinaire
  demande de descendre sans gagner ; `etageSuivant` ; **l'étage suivant a la profondeur suivante et garde l'équipement et le
  sac** ; **l'escalier du dernier étage gagne** ; viser l'escalier ne suffit pas ; **tuer tous les monstres ne donne pas la
  victoire** ; une carte sans monstre n'est ni gagnée ni figée ; descendre sans avoir tué personne ; `forcerLaDescente` (F8) à un
  étage ordinaire et au dernier ; une escouade vaincue ne descend ni ne gagne ; une partie qui a pris l'escalier se fige ; les
  monstres de l'étage suivant sont plus forts) ; `test_AffichageProfondeur.cpp` (nouveau, +1 : « Depth 1 / 5 ») ;
  `test_CommandeDeTouche.cpp` (F8 vaut en tout temps) ; **tests adaptés à la nouvelle règle de victoire** : `test_Partie_fin.cpp`
  (« estGagnee est vrai quand tous les monstres sont tués » devient « tuer tous les monstres ne gagne plus », « sans monstre au
  départ la partie est gagnée » devient « n'est pas gagnée », « une partie gagnée se fige » gagne au dernier étage par F8),
  `test_Partie_evenements.cpp` et `test_Partie_portes.cpp` (une partie figée l'est par `forcerLaDescente`, plus « sans monstre »).
- **Ce que les tests de `EcranJeu` ne voient pas** : la descente elle-même (`EcranJeu::descendre`) a besoin d'une fenêtre ; elle se
  réduit à `partieDeLEtage(etageSuivant(m_partie.expedition()))`, dont chaque pièce est testée. L'affichage (marches de
  l'escalier, panneau réduit, ligne de profondeur) se vérifie à l'œil.
- **Les commentaires d'avant disent encore** « une partie sans monstre est gagnée, donc figée » (étapes 63 à 97) : plus vrai depuis
  l'étape 98.
