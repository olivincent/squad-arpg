# Squad-ARPG — Combat

`Attaque`, `Cercle`, la portée bord à bord, le skill, le circuit des
dégâts et le blocage entre monstres et personnages. L'escouade et ses
personnages sont dans `docs/formation.md`, le monstre dans
`docs/monstre.md`.

## Attaque

- **Composition, pas héritage** : `Attaque` (portée, dégâts, temps de
  recharge, recharge courante) est possédée par valeur par `Personnage`
  (`m_attaque`), par `Skill` et par `Monstre`. Aucune classe
  de base commune, aucune fonction virtuelle.
- **Elle ne connaît pas la position de son propriétaire** : `origine` et
  `cible` (deux `Cercle`) sont reçus en paramètre de `estAPortee` et
  `frapper`, plutôt que stockés. Elle ne connaît ni `TileMap`, ni
  `Personnage`, ni `Monstre`, ni `Escouade`, et ne dessine rien ; elle
  connaît `Cercle`.
- **Portée, dégâts et temps de recharge sont des paramètres du
  constructeur**, puisqu'ils diffèrent selon le propriétaire (mêlée,
  tireur, monstre, skill). `m_recharge` vaut 0 à la création : le
  premier coup part tout de suite.
- **`frapper`** renvoie les dégâts et relance la recharge si la cible est
  à portée et la recharge écoulée ; sinon renvoie 0 sans toucher la
  recharge (un coup tenté hors de portée ne la déclenche pas).
  **`mettreAJour(dt)`** décrémente la recharge, sans descendre sous zéro :
  le temps passe même quand le propriétaire n'attaque pas.
- **Valeurs actuelles** : personnages 5 dégâts, 0,5 s de recharge ;
  monstre 10 dégâts (de quoi rester dangereux seul face à 4
  personnages), 1 s de recharge.
- **La règle « un mort ne frappe pas » reste dans `Personnage::frapper`**,
  avant la délégation : `Monstre` délègue entièrement, sans règle
  supplémentaire.
- **Tests** : `test_Attaque.cpp` couvre les cas génériques de
  recharge/portée ; `test_Personnage.cpp` et `test_Monstre.cpp` ne gardent
  que ce qui prouve le branchement et les valeurs propres à chaque
  classe.

## Cercle et portée bord à bord

- **La portée est l'écart entre les bords de deux cercles** : « portée
  10 » signifie 10 px de vide entre l'attaquant et sa cible, quelle que
  soit leur taille. Mesurée de centre à centre, la même portée donnait un
  écart visible différent selon la taille de la cible, et obligeait un
  personnage à entrer dans un monstre pour le frapper.
- **Le cercle comme forme** : identique sous tous les angles, donc
  compatible avec une future orientation des entités sans calcul
  d'angle. Un rectangle orienté ou plusieurs cercles par entité ont été
  écartés : rien ne les justifie encore.
- **Une structure sans comportement et une fonction libre**, dans leur
  propre module, plutôt qu'une classe : un cercle n'a aucun invariant à
  protéger (tout centre et tout rayon positif ou nul est valide), et la
  distance bord à bord n'appartient à aucun des deux cercles plus qu'à
  l'autre — même raisonnement que `trouverChemin` ou
  `deplacerAvecGlissement`. Les membres de `Cercle` sont publics et sans
  préfixe `m_`.
- **`distanceBordABord` n'est jamais négative** : deux cercles qui se
  chevauchent sont à 0.
- **Un seul cercle par entité, calculé et non stocké** : `getCercle()`
  de `Personnage` (rayon 10) et de `Monstre` (rayon 15) recalcule le
  rayon (demi-côté) à partir de la taille déjà stockée — une seule
  source de vérité.
- **Valeurs de portée** : mêlée 20 px, tireur 120 px, monstre 15 px,
  skill de dégâts = portée de mêlée. La mêlée est passée de 10 à 20 px
  (étape 37) : un monstre s'arrête à 15 px de sa proie, et avec 10 px
  un mêlée attaqué n'aurait pas pu riposter sans bouger.
- **Ce qui passe par `Attaque::estAPortee`** : la frappe des personnages,
  la riposte des monstres, l'arrêt de l'ancre (`Escouade::attaquer`),
  l'arrêt individuel en combat et l'arrêt du monstre en poursuite. Pour
  transporter la taille de la cible, les signatures échangent un `Cercle`
  plutôt qu'une position suivie d'un rayon : `Escouade::attaquer`,
  `proieDuMonstre`, `Monstre::mettreAJour` et
  `Monstre::frapper`.
- **Restent mesurés de centre à centre**, volontairement : la détection
  et le territoire du monstre, le choix du personnage le plus proche, le
  clic sur un monstre (`contient`), la formation et le lissage du chemin.
- **Limites connues** : les entités sont dessinées comme des carrés et
  leurs coins dépassent du cercle (4,1 px pour un personnage, 6,2 px pour
  un monstre) — face à face, l'écart visible est bien la portée, mais en
  diagonale l'écart visible est plus petit que la portée (d'une
  dizaine de pixels). La
  portée ignore les murs : un personnage peut frapper à travers un mur
  s'il est assez près, une vérification de ligne de vue (via
  `TileMap::trajetLibre`, par exemple) restant une option.

## Attaque de l'escouade

- **`Escouade::attaquer(monstres, carte)`** reçoit la liste des monstres
  tels que l'escouade les voit (`MonstreVu` : identifiant et cercle,
  dans `Coup.hpp`) et renvoie une liste de `Coup` (identifiant du
  monstre, dégâts) que `main.cpp` applique. Elle ne connaît toujours pas
  `Monstre`. Deux parties indépendantes :
  - **l'approche, seulement avec un verrou** : si le monstre verrouillé
    est dans la liste, son cercle est mémorisé (`m_cibleCombat`) ; si au
    moins un personnage **vivant** n'est pas à portée,
    `allerVers(cercle.centre, carte)`, sinon l'ancre s'arrête sur place
    (`m_mobile.arreter()`, pour cesser d'avancer sans reculer). Le skill
    de dégâts en attente part aussi ici, dès que son lanceur est à
    portée (voir « Choix de la cible et skill en attente ») ;
  - **les frappes, verrou ou non** : voir « Autodéfense ».
- **Les morts sont ignorés pour l'arrêt** (voir `docs/formation.md`).
- **La formation tient pendant l'attaque** : les personnages continuent
  de viser leur place (`placePersonnage`), sauf ceux déjà à portée du
  monstre verrouillé, qui restent sur place.

## Autodéfense

- **Chaque personnage vivant choisit pour lui-même, à chaque image, le
  monstre qu'il frappe** : le monstre verrouillé s'il est à sa portée
  (la cible prioritaire passe toujours en premier), sinon le plus
  proche (bord à bord) parmi ceux à sa portée (égalité : le premier de
  la liste), sinon aucun. Deux personnages peuvent donc frapper deux
  monstres différents dans la même image. Un mort ne frappe pas
  (`Personnage::frapper` renvoie 0), la recharge est celle de chacun.
- **Une fonction libre isolée**, `choisirMonstreAFrapper(origine,
  estAPortee, monstres, idVerrouille)` (module `Autodefense`), sans état
  et testée seule. Elle reçoit une origine et un test de portée
  (`std::function<bool(Cercle)>`) plutôt qu'un `Personnage`, pour que le
  choix de la cible d'un skill sans verrou réutilise la même règle avec
  la portée du skill, au lieu d'en dupliquer une copie.
- **Elle ne déplace personne** : ni l'ancre, ni les personnages. La
  formation et le placement individuel en combat restent ceux d'avant
  (le placement ne regarde que le verrou). Elle s'applique tout le
  temps : à l'arrêt, en marche, en combat verrouillé, et pour un lanceur
  en route vers la cible de son skill. `main.cpp` demande les coups à
  chaque image tant que l'escouade a un vivant.
- **Le clic sur un monstre garde son rôle** : désigner la cible
  prioritaire et aller l'engager. Seul le monstre verrouillé a le
  contour jaune.
- **Limites connues** : la règle ne regarde que la portée (pas de notion
  de « qui attaque qui ») ; aucun déplacement provoqué par l'autodéfense
  (pas de pas vers l'attaquant, pas de poursuite, pas de recul des
  tireurs) ; la portée ignore les murs ; pas d'indicateur visuel de la
  cible de chaque personnage.

## Agresseurs (étape 40)

- **Pourquoi** : l'autodéfense ne déplace personne. Un monstre qui frappe
  un tireur à l'arrière laissait les mêlées plantées à leur place. Les
  personnages vont maintenant vers ceux qui les frappent.
- **`Escouade` retient ses agresseurs** (`m_agresseurs` : identifiant,
  cercle, temps restant) sans connaître `Monstre` : `recevoirDegats`
  reçoit l'identifiant du monstre, `main.cpp` reste l'intermédiaire. Un
  coup de dégâts `> 0` ajoute l'agresseur ou remet son temps à 3 s
  (`s_dureeMemoireAgresseur`) ; un coup à 0 n'enregistre rien. À
  l'enregistrement le cercle n'a que le centre (rayon nul, `recevoirDegats`
  n'a que l'origine) ; `attaquer` le met à jour à l'image suivante.
- **Oubli** : le temps s'écoule dans `mettreAJour` ; un agresseur absent
  de `monstresVus` (monstre mort) est retiré dans `attaquer`.
- **Choix de la cible** : `choisirAgresseurAViser` (module `Autodefense`,
  fonction libre testée seule) garde les agresseurs dont le centre est à
  100 px ou moins (`s_laisseAgresseur`) de la **place en formation** du
  personnage (pas de sa position actuelle, sinon la laisse suivrait le
  personnage et il pourrait être entraîné sans limite), puis prend le plus
  proche de lui.
- **Règle de déplacement** (`pointVersAgresseur`), ancre à l'arrêt
  seulement : aucun agresseur dans la laisse → la place ; sinon, monstre
  déjà à portée de l'attaque de base → reste sur place (sans cela il
  reviendrait à sa place, sortirait de portée et oscillerait) ; sinon
  centre de l'agresseur. « Monstre à portée » est mémorisé par `attaquer`
  (`m_monstreAPortee`), comme `m_cibleCombat`.
- **Priorités** : le lanceur d'un skill en attente vise sa cible ; avec un
  verrou, un personnage à portée du monstre verrouillé reste sur place ;
  l'ordre du joueur (ancre en mouvement) fait viser la place à tous.
- **Règle générique** : aucun type « mêlée / tireur » ; un tireur à 120 px
  ne bouge presque jamais, un mêlée va au contact. Même `Mobile` que les
  autres déplacements (mêmes blocages).
- **Limites connues** : l'ancre ne bouge pas vers l'agresseur ; pas de
  poursuite au-delà de la laisse ; l'ancre est « à l'arrêt » dès que son
  chemin est fini, même un instant ; un agresseur qui frappe depuis plus
  de 100 px de la place d'un personnage n'attire pas celui-ci.

## Skill

- **Une classe `Skill` autour d'une `Attaque`** : un `TypeSkill` (`Degats`,
  `Soin`, `Provocation` ou `Zone`), l'`Attaque` qui fournit portée, puissance et recharge, et un rayon de zone.
  `Skill` cache le mot « dégâts » derrière un nom neutre, la puissance
  (dégâts infligés, points de vie rendus ou durée selon le type), et son
  `declencher` n'est qu'un `frapper` renommé. Un `enum class` suffit pour
  le type : ni `std::variant` ni polymorphisme tant que les deux types ne
  diffèrent que par ce que fait `Escouade` du résultat.
- **L'absence de skill est un `std::optional<Skill>`**, reçu au
  constructeur de `Personnage` comme l'attaque de base (injection), plutôt
  qu'un skill « vide » à 0 de puissance. Pas de valeur par défaut : chaque
  création de `Personnage` dit explicitement s'il a un skill.
- **Quatre skills** (les deux derniers sont décrits dans « Provocation »
  et « Skill de zone ciblé »), aux
  valeurs écrites dans `Escouade.cpp` :
  - dégâts : personnage 0 (mêlée avant-gauche), 25 dégâts, recharge 5 s,
    portée de mêlée (20 px) ;
  - soin : personnage 2 (tireur arrière-gauche), 30 points de vie,
    recharge 8 s, portée 40 px bord à bord.
- **Le lanceur est désigné par sa place** : la touche N déclenche le skill
  du personnage à la place N (1 à 4) de la formation, par
  `Escouade::declencherSkill(place, ...)`. Sans champ « type de
  personnage » ni indice mémorisé.
- **Le skill remplace l'attaque de base, pas l'inverse** : quand il part,
  `Personnage::declencherSkill` relance au complet la recharge de
  `m_attaque`, pour les deux types. La relance n'a lieu que si le skill
  part réellement (puissance `> 0`). Une attaque de base ne touche pas la
  recharge du skill.
- **Les refus sont répartis selon qui connaît l'information** : « pas de
  skill », « mort » et « en recharge » (`Personnage::skillEstPret`) ;
  « skill déjà en attente » et « aucune cible » par `Escouade`, qui seule
  porte le verrou, le skill en attente et la liste des alliés. Un appui
  refusé est perdu (pas de file d'attente).
- **La recharge s'écoule en permanence**, dans `Personnage::mettreAJour`,
  en combat ou non, et ne démarre qu'au moment où le skill part, pas à
  l'appui. Barre orange dessinée par `Personnage`, 3 px sous la barre de
  vie, proportionnelle à `getProgressionRecharge()` ; ni pour un mort ni
  pour un personnage sans skill.
- **Le soin est appliqué dans `Escouade` et n'en sort pas** : `main.cpp`
  ne reçoit aucun dégât pour un soin. `Personnage::recevoirSoin`
  plafonne au maximum et ne soigne pas un mort.
- **Le soin est permis hors combat** ; le skill de dégâts vise le
  monstre verrouillé, ou, sans verrou, le monstre que son lanceur
  frapperait en autodéfense (voir ci-dessous).

## Choix de la cible et skill en attente

- **`Cible`** est un `std::variant<CibleAllie, CibleMonstre>`
  (`CibleAllie` : place de 1 à 4 ; `CibleMonstre` : l'identifiant du monstre et son `Cercle`). Pas de
  troisième cas « aucune » : l'absence est un `std::optional` vide. Les
  deux cibles n'ont rien de commun, d'où un variant plutôt qu'une classe
  de base.
- **Le type du skill décide de la cible cherchée**, dans
  `Escouade::choisirCible(type, place, monstres)`, une fonction à part.
  `Soin` : le vivant qui a le moins de points de vie parmi les blessés
  (soigneur inclus ; égalité : la plus petite place). `Degats` :
  1. avec un verrou, le monstre verrouillé trouvé dans la liste, à
     portée ou non (mise en attente et marche s'il est hors de portée) ;
     s'il n'est pas dans la liste, pas de cible ;
  2. sans verrou, le monstre que le lanceur frapperait en autodéfense
     (`choisirMonstreAFrapper`), mais avec la portée de son skill ;
  3. sinon aucune cible : l'appui est refusé, sans recharge.

  `CibleMonstre` porte l'identifiant du monstre : c'est lui qui attribue
  les dégâts dans la liste des coups. Un futur choix manuel par le
  joueur ne devra remplacer que `choisirCible`.
- **Sans verrou, le skill part toujours dans l'image de l'appui** (la
  cible est à portée par construction) : jamais de mise en attente dans
  ce cas. Il ne crée pas de verrou et ne déplace personne.
- **À l'appui** : `declencherSkill` refuse si le lanceur n'a pas de skill,
  est mort ou est en recharge, si un skill est déjà en attente, ou si
  `choisirCible` ne trouve rien. Sinon, cible à portée : le skill part
  tout de suite ; hors de portée : il est mis en attente
  (`m_skillEnAttente`, un seul pour toute l'escouade).
- **Pendant l'attente, le lanceur vise le centre de sa cible** au lieu de
  sa place en formation (même déplacement et mêmes blocages que les
  autres personnages), puis reprend sa place une fois le skill parti ou
  annulé. La cible est fixée à l'appui. Pour un monstre, le cercle lu en
  route est `m_cibleCombat`, mis à jour à chaque `attaquer`, pas la copie
  de l'appui.
- **Où le skill part** : un soin, dans `Escouade::mettreAJour` (début de
  l'image, avant les déplacements) ; un skill de dégâts à portée, tout de
  suite, dans `declencherSkill`, qui renvoie un `std::vector<Coup>` (vide
  ou un coup) ; un skill de dégâts mis en attente, dans `attaquer`, avant
  les attaques de base, qui reçoit le cercle à jour du monstre : son coup
  s'ajoute alors à la liste renvoyée à `main.cpp`, attribué à
  l'identifiant de sa cible.
- **Annulations** : lanceur mort (les deux types) ; soin : cible morte ou
  remontée à pleine vie ; dégâts : `deverrouiller()` (mort de la cible,
  clic au sol) ou `verrouiller` avec un autre identifiant (changement de
  cible). Re-cliquer sur le même monstre ne l'annule pas. Un ordre de
  déplacement de l'escouade n'annule rien. Une annulation ne coûte aucune
  recharge.
- **Limites connues** : un seul skill en attente à la fois ; pas de choix
  manuel de la cible ; un lanceur bloqué par un monstre glisse ou attend
  (le A* ignore les entités) ; pas d'effet visuel au déclenchement ni
  sur la cible soignée, pas de HUD ni d'icône de skill ; le skill du
  personnage 4 se lance en deux temps (voir « Skill de zone ciblé »).

## Provocation (étape 41)

- **Troisième `TypeSkill`**, `Provocation`, pour le personnage de la
  place 2 (mêlée avant-droit) : rayon de zone 150 px (étape 42 : portée 0
  et zone centrée sur le centre du provocateur, voir « Portée et rayon de
  zone »), durée 4 s (sa puissance : pour ce type, une durée en
  secondes), recharge 10 s. Valeurs écrites dans `Escouade.cpp`. On
  garde `enum class` + `if`/`switch` ; un nouveau type ne doit jamais
  tomber par défaut dans la branche d'un autre (`choisirCible` renvoie
  vide pour une provocation, qui ne passe pas par elle).
- **Skill de zone, sans cible** : pas de cas ajouté à `Cible`.
  `declencherSkill` le traite avant `choisirCible` : tous les monstres de
  `monstresVus` touchés par la zone (bord à bord 0) sont provoqués, dans l'image de
  l'appui (jamais de mise en attente ni de marche). Aucun monstre dans le
  rayon : elle part quand même et consomme sa recharge (étape 43, comme
  le skill de zone). Un seul déclenchement du `Skill`
  pour la recharge, quel que soit le nombre de monstres. Aucun dégât :
  liste de coups vide. Les refus habituels s'appliquent (lanceur mort,
  recharge, skill en attente) et la recharge de l'attaque de base du
  lanceur est relancée.
- **L'état « provoqué » vit dans `Escouade`**, pas dans `Monstre` :
  `m_provocations` (`Provocation` : identifiant du monstre, place du
  provocateur, temps restant), sur le modèle des agresseurs.
  `Monstre` ne change pas : il reçoit toujours sa proie en paramètre.
  `provoquer(id, place, duree)` ajoute ou remet la durée au maximum ;
  elle est publique pour que les tests la préparent, mais le contrôle de
  la recharge et du lanceur est dans `declencherSkill`.
- **Fin d'une provocation** : durée écoulée et provocateur mort
  (`mettreAJour`), monstre absent de la liste (`attaquer`).
- **Une seule règle pour la proie** : `indiceProieDuMonstre(id,
  position)` renvoie le provocateur s'il est vivant, sinon le vivant le
  plus proche. `proieDuMonstre` (demande de `main.cpp`) et
  `recevoirDegats` s'en servent. Si le provocateur est mort entre-temps,
  la règle retombe tout de suite sur le plus proche, même avant le
  retrait de la provocation.
- **Le reste du monstre ne change pas** : détection, territoire, abandon
  et retour au poste suivent leurs règles, même provoqué. Un monstre
  provoqué dont le provocateur est hors de son territoire ou de sa
  détection abandonne donc comme avant.
- **Limites connues** : pas d'indicateur visuel du monstre provoqué ni
  d'effet au déclenchement ; pas de système d'effets général ;
  `provoquer` n'est pas protégée contre un appel direct sans recharge.

## Portée et rayon de zone (étape 42)

- **Deux notions distinctes** : la portée (jusqu'où le skill peut être
  lancé) reste dans l'`Attaque` ; le rayon de zone (taille de la zone
  touchée) est dans `Skill` (`m_rayonZone`, getter `getRayonZone`).
  `Attaque` est partagée avec les attaques de base des personnages et des
  monstres, qui n'ont pas de zone. Pas de valeur par défaut au
  constructeur ; pas de `std::optional` : un skill à cible unique reçoit 0
  et ne le lit pas (c'est le `TypeSkill` qui décide).
- **Règle commune des skills de zone** : un `Cercle` centré sur le point
  d'effet, de rayon le rayon de zone ; un monstre est touché si
  `distanceBordABord(zone, monstre) == 0`. `Personnage::zoneDuSkill()`
  donne ce cercle centré sur le personnage (provocation).
- **Provocation** : portée 0, zone 150 px depuis le **centre** du
  provocateur (avant l'étape 42 : 150 px depuis son bord ; la zone a
  rétréci de 10 px). Pour relancer la recharge, le lanceur se prend
  lui-même pour cible (distance 0, portée 0) ; la portée n'est pas un
  vrai test ici.

## Skill de zone ciblé (étape 42, touche 4)

- **Quatrième `TypeSkill`**, `Zone`, pour le personnage de la place 4
  (tireur arrière-droit) : 15 dégâts (la puissance), recharge 8 s, zone
  60 px, portée 250 px (valeurs dans `Escouade.cpp`). La portée de
  lancement est lue depuis l'étape 43 (voir « Portée de lancement »).
- **Lancer à un point** : `Escouade::declencherSkillDeZone(place, point,
  monstres)`, distincte de `declencherSkill` qui n'a pas de point et
  refuse un skill `Zone` (`choisirCible` renvoie aussi vide). La
  fonction reçoit le point directement ; `CiblePoint` n'existe que pour
  l'attente (étape 43).
  Chaque monstre touché reçoit un `Coup` (dégâts du skill), appliqué par
  `main.cpp` comme les autres. Le sort part et coûte sa recharge même si
  la zone est vide (choix de jeu, contrairement à la provocation) ; il
  relance la recharge de l'attaque de base du lanceur. Refus : place
  invalide, skill non `Zone`, lanceur mort, recharge, skill en attente.
  Pas de dégâts aux alliés. Pour le déclenchement du `Skill`, le lanceur
  est sa propre cible (distance 0) ; la vraie portée est testée avant,
  contre le point.
- **`skillEstPret(place)`** permet à `main.cpp` de savoir s'il peut
  entrer en visée sans connaître `Personnage` ; `rayonZoneDuSkill(place)`
  lui donne le rayon du cercle de prévisualisation.
- **Le mode visée est un état d'interface** (voir
  `docs/boucle-et-entrees.md`) : `Escouade` ne connaît ni la souris ni le
  mode.
- **Limites connues** : le sort traverse les murs (pas de ligne de vue) ;
  pas d'effet visuel à l'impact, de HUD ni d'icône ; pas de dégâts aux
  alliés ni dans le temps.

## Portée de lancement du skill de zone (étape 43)

- **Mesure** : distance bord à bord entre le cercle du lanceur et un
  `Cercle` de rayon 0 centré sur le point (`Personnage::skillEstAPortee`,
  donc `Attaque::estAPortee`), comme les autres skills.
- **À portée** : le sort part dans l'image du clic. **Hors portée** :
  `m_skillEnAttente` reçoit `CiblePoint { point }` (point fixé au clic),
  sans coup ni recharge, et le lanceur marche vers le point (même
  déplacement que les autres skills en attente, `avancerSkillEnAttente`).
- **Départ** : au début de `attaquer`, avant l'approche du verrou et les
  attaques de base : dès que le lanceur vivant est à portée du point, le
  sort part (`lancerSkillDeZone`, partagée avec le départ immédiat) et ses
  coups s'ajoutent à la liste renvoyée. La zone est évaluée à ce moment,
  sur `monstres` à jour, pas au moment du clic.
- **Annulation** : seulement par la mort du lanceur, sans recharge. Ni le
  verrou, ni un clic, ni un ordre de déplacement n'annulent un
  `CiblePoint` : `verrouiller`/`deverrouiller` n'annulent que les
  `CibleMonstre`. Un seul skill en attente pour toute l'escouade : un
  autre skill (ou un autre lancer de zone) est refusé pendant ce temps.
- **`Cible` est lue par `get_if`/`holds_alternative`, pas `std::visit`** :
  le compilateur ne signale pas un cas oublié. À chaque nouveau cas du
  variant, relire `declencherSkill`, `avancerSkillEnAttente`, `attaquer`,
  `verrouiller` et `deverrouiller`.
- **Visée** : `Escouade::cercleDePorteeDuSkill(place)` donne à `main.cpp`
  le cercle de portée (centre du lanceur, rayon du lanceur + portée).
- **Limites** : le lanceur marche en ligne droite ou par A* comme les
  autres personnages (bloqué par les monstres) ; si le point est
  inaccessible il ne l'atteint peut-être jamais, et le sort reste en
  attente jusqu'à sa mort.

## Circuit des dégâts

`Escouade` et `Monstre` ne se connaissent pas : `main.cpp` fait
l'intermédiaire (voir `docs/boucle-et-entrees.md` pour l'ordre dans une
image).

- **Les personnages frappent un monstre** : à chaque image, `main.cpp`
  construit la liste `monstresVus` (identifiant et cercle de chaque
  monstre), puis applique, avec une petite lambda `appliquer`, les coups
  renvoyés par `escouade.declencherSkillDeZone(...)` (clic de lancer en
  visée), par `escouade.declencherSkill(...)` (si une touche est
  appuyée) puis par `escouade.attaquer(...)`, chacun au monstre dont
  l'identifiant figure dans le `Coup`. Un soin ne rend aucun coup. Le
  skill passe avant l'attaque de base : s'il part, il relance la
  recharge de base de ce personnage, qui ne frappe donc pas deux fois
  dans la même image.
- **Un monstre frappe un personnage** : pour chaque monstre,
  `main.cpp` demande `escouade.proieDuMonstre(m.getId(), m.getCentre())`
  (le cercle du provocateur si ce monstre est provoqué, sinon du vivant
  le plus proche), met le monstre à jour, relit son centre, puis applique
  `escouade.recevoirDegats(m.getId(), origine, m.frapper(*cible))`. Les
  deux appels à `Escouade` désignent le même personnage grâce à la même
  règle (`indiceProieDuMonstre`). La cible est calculée avant le
  déplacement du monstre et `origine` relu après, pour que les dégâts
  partent de la position à jour.
- **Un monstre frappe qu'il soit lui-même attaqué ou non** : sa riposte
  ne dépend ni du verrou, ni de son statut — uniquement de la distance
  et de sa recharge. S'il n'y a aucun personnage vivant, il ne frappe
  pas.

## Qui bloque qui

Mécanisme dans `docs/carte-et-collision.md` : une liste de cercles
obstacles donnée à celui qui bouge, qui refuse un déplacement qui
l'enfoncerait dans un obstacle.

- **Les monstres** reçoivent les cercles des personnages vivants et ceux
  des autres monstres (identifiant différent). La liste des personnages
  est calculée une fois par image après `escouade.mettreAJour`
  (`Escouade::cerclesDesVivants`), puis copiée comme base de la liste de
  chaque monstre. Les monstres sont mis à jour l'un après l'autre : le
  second à bouger voit déjà la nouvelle place du premier.
- **Les personnages** reçoivent les cercles de tous les monstres, calculés
  une fois par image avant `escouade.mettreAJour` (donc à leur position
  de l'image précédente). `Escouade::mettreAJour` les transmet à chaque
  personnage. Les personnages ne se bloquent pas entre eux.
- **L'ancre ne bloque personne et n'est pas bloquée** : c'est un point
  de contrôle abstrait, pas une entité de combat. `m_mobile` reçoit
  toujours une liste vide.
- **Les morts ne bloquent pas** : un cadavre est franchissable,
  cohérent avec le fait qu'il n'est plus ciblé ni dangereux.
- **`Escouade` et `Monstre` ne connaissent pas les cercles des autres** :
  `Escouade` expose `cerclesDesVivants` plutôt que de laisser `main.cpp`
  parcourir les personnages, et reste seule à connaître `m_personnages`.
- **Limites connues** : une entité bloquée ne contourne pas (le A* ignore
  les entités), elle glisse si l'angle le permet, sinon elle attend ; la
  formation n'est pas resserrée quand un personnage est bloqué ; un
  chevauchement déjà existant
  peut subsister, la règle de blocage permettant seulement de s'en
  dégager.
