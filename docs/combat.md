# Squad-ARPG — Combat

`Skill` (qui sert aussi d'attaque de base), `Cercle`, la portée bord à
bord, le circuit des
dégâts et le blocage entre monstres et personnages. L'escouade et ses
personnages sont dans `docs/formation.md`, le monstre dans
`docs/monstre.md`.

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
- **Ce qui passe par `Skill::estAPortee`** : la frappe des personnages,
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
  portée seule ignore les murs : depuis l'étape 46, `Escouade` y ajoute la
  ligne de vue (voir « Ligne de vue des personnages »).

## Attaque de l'escouade

- **`Escouade::attaquer(monstres, carte)`** reçoit la liste des monstres
  tels que l'escouade les voit (`EntiteVue` : identifiant et cercle,
  dans `EntiteVue.hpp`) et renvoie une liste de `Coup` (identifiant du
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
- **Frapper se fait en deux temps** depuis l'étape 57 (voir « Temps d'exécution et coup engagé »).
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
- **Une fonction libre isolée**, `choisirCibleAFrapper(origine,
  estAPortee, monstres, idPrefere)` (module `Autodefense`), sans état
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
  tireurs) ; pas d'indicateur visuel de la
  cible de chaque personnage.
  La ligne de vue s'ajoute à la portée depuis l'étape 46.

## Agresseurs (étape 40)

- **Pourquoi** : l'autodéfense ne déplace personne. Un monstre qui frappe
  un tireur à l'arrière laissait les mêlées plantées à leur place. Les
  personnages vont maintenant vers ceux qui les frappent.
- **`Escouade` retient ses agresseurs** dans un `Agresseurs` (étape 51,
  `m_agresseurs`, fichiers à part : par agresseur un `EntiteVue` et le
  temps restant) sans connaître `Monstre` : `recevoirDegats`
  reçoit l'identifiant du monstre, `main.cpp` reste l'intermédiaire. Un
  coup de dégâts `> 0` ajoute l'agresseur ou remet son temps à 3 s
  (`s_dureeMemoire`, dans `Agresseurs.cpp`) ; un coup à 0 n'enregistre rien. À
  l'enregistrement le cercle n'a que le centre (rayon nul, `recevoirDegats`
  n'a que l'origine) ; `attaquer` le met à jour à l'image suivante.
- **Oubli** : le temps s'écoule dans `mettreAJour` ; un agresseur absent
  de `monstresVus` (monstre mort) est retiré dans `attaquer`
  (`Agresseurs::actualiser`). Depuis l'étape 51, la liste de `EntiteVue`
  que lit `choisirAgresseurAViser` est construite une fois par image
  (`Agresseurs::monstresVus`, dans `mettreAJour`), non plus une fois par
  personnage.
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

- **Une classe `Skill`** : un `TypeSkill` (`Degats`, `Soin`,
  `Provocation` ou `Zone`), une portée (bord à bord), une puissance, un
  temps de recharge, la recharge courante et un rayon de zone. Le mot
  « dégâts » est caché derrière un nom neutre, la puissance (dégâts
  infligés, points de vie rendus ou durée selon le type). Un `enum class`
  suffit pour le type : ni `std::variant` ni polymorphisme tant que les
  types ne diffèrent que par ce que fait `Escouade` du résultat.
- **Composition, pas héritage** : `Skill` est possédé par valeur par
  `Personnage` (`m_attaqueDeBase`, plus `m_skill` en option) et par
  `Monstre` (`m_attaque`). Aucune classe de base commune, aucune fonction
  virtuelle. Il ne connaît pas la position de son propriétaire : `origine`
  et `cible` (deux `Cercle`) sont reçus en paramètre de `estAPortee` et
  `estAPortee`, plutôt que stockés. Il ne connaît ni `TileMap`, ni
  `Personnage`, ni `Monstre`, ni `Escouade`, et ne dessine rien ; il
  connaît `Cercle`.
- **L'attaque de base est un `Skill` comme un autre**, de type `Degats`,
  rayon de zone 0. Le rôle « attaque de base » est donné par le membre
  qui la range (`m_attaqueDeBase`), pas par un marqueur dans le type.
  Elle garde son circuit propre (autodéfense, combat verrouillé) : elle
  ne passe ni par `choisirCible`, ni par le skill en attente, ni par les
  touches. `Monstre` dépend donc de `TypeSkill` ; ses attaques sont des
  `Skill` `Degats`, et il pourra recevoir des skills plus tard.
- **Portée, puissance, recharge et rayon de zone sont des paramètres du
  constructeur**, puisqu'ils diffèrent selon le propriétaire. La recharge
  est un intervalle `[rechargeMin, rechargeMax]` (voir « Recharge aléatoire »).
  `m_recharge` vaut 0 à la création : le premier déclenchement part tout de
  suite.
- **`mettreAJour(dt)`** décrémente la recharge, sans descendre sous zéro :
  le temps passe même quand le propriétaire n'agit pas.
  **`relancerRecharge`** la remet au complet (nouveau tirage) sans déclencher.
  `declencher` et `declencherSansCible` ont été supprimées à l'étape 62 (le
  jeu ne les appelait plus depuis l'étape 58) : les attaques de base et les
  skills passent par `commencer` / `avancer` / `annuler`. Les tests de
  recharge lancent un skill à temps d'exécution nul par `commencer` +
  `avancer(0)`.
- **Valeurs des attaques de base** : personnages 5 dégâts, recharge de
  0,3 à 0,7 s (mêlées, portée 20 px) ou de 0,4 à 0,6 s (tireurs, portée
  120 px) ; monstre 10 dégâts (de quoi rester dangereux seul face à 4
  personnages), recharge de 0,7 à 1,3 s, portée 15 px. Les moyennes sont
  celles d'avant l'étape 56 (0,5 s ; 0,5 s ; 1 s).
- **La règle « un mort ne frappe pas » reste dans `Personnage::frapper`**,
  avant la délégation : `Monstre` délègue entièrement, sans règle
  supplémentaire.
- **Tests** : `test_Skill.cpp` couvre les cas génériques de recharge et de
  portée ; `test_Personnage.cpp` et `test_Monstre.cpp` ne gardent que ce
  qui prouve le branchement et les valeurs propres à chaque classe.
- **L'absence de skill est un `std::optional<Skill>`**, reçu au
  constructeur de `Personnage` comme l'attaque de base (injection), plutôt
  qu'un skill « vide » à 0 de puissance. Pas de valeur par défaut : chaque
  création de `Personnage` dit explicitement s'il a un skill.
- **Quatre skills** (les deux derniers sont décrits dans « Provocation »
  et « Skill de zone ciblé »), aux
  valeurs écrites dans `Escouade.cpp` :
  - dégâts : personnage 0 (mêlée avant-gauche), 25 dégâts, recharge 5 s,
    portée de mêlée (20 px), activation 0,3 s ;
  - soin : personnage 2 (tireur arrière-gauche), 30 points de vie,
    recharge 8 s, portée 40 px bord à bord, activation 0,6 s.
  Provocation (0,2 s) et zone (0,8 s) : voir leurs sections.
- **Le lanceur est désigné par sa place** : la touche N déclenche le skill
  du personnage à la place N (1 à 4) de la formation, par
  `Escouade::declencherSkill(place, ...)`. Sans champ « type de
  personnage » ni indice mémorisé.
- **Le skill remplace l'attaque de base, pas l'inverse** : quand il
  s'engage, `Personnage::engagerSkill` annule le coup de base en exécution ;
  quand il part, `Personnage::avancerSkill` relance au complet la recharge de
  `m_attaqueDeBase`. Une attaque de base ne touche pas la recharge du skill.
- **Les refus sont répartis selon qui connaît l'information** : « pas de
  skill », « mort », « en recharge » et « en activation »
  (`Personnage::skillEstPret`, vrai seulement en phase prête) ;
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

## État d'un skill pour l'affichage (étape 48)

- **`EtatSkill`** (`EtatSkill.hpp`, sans comportement, sur le modèle de
  `Coup.hpp`) : `type`, `progression` (0 juste lancé, 1 prêt : même sens que
  la barre orange sous les personnages), `tempsRestant` (secondes),
  `lanceurVivant`, `enActivation` (étape 58 : le lanceur prépare le skill).
  `Escouade::etatDuSkill(place)` le renvoie, vide si la
  place est invalide ou sans skill ; la progression d'un mort est celle
  qu'il avait en mourant.
- **Pourquoi une structure plutôt que des getters** : `main.cpp` ne connaît
  pas `Personnage` ; un seul appel par place donne tout ce que la barre
  montre. `Skill` n'a gagné que `getRechargeRestante`.
- **Présentation séparée de la logique** : `Skill` ne sait pas comment il
  est dessiné ; la lettre (D, P, S, Z) vient d'un `switch` sans `default`
  dans `BarreDeSkills`, que le compilateur signale si un type est oublié.
- **Contour** : `couleurDuContour(enVisee, enActivation)`, pure et testée :
  jaune en visée (prioritaire), blanc pendant l'activation, gris sinon ; 3 px
  d'épaisseur dans les deux premiers cas, 1 px sinon.
- **Quartier** : la zone sombre couvre la fraction `1 − progression`, de
  l'aiguille jusqu'à midi, avec des sommets sur le bord du carré
  (projection `demiCote / max(|dx|, |dy|)`), pas sur un cercle.
  `pointsDuQuartier` est pure et testée (aire = fraction × côté²).
- **Limites connues** : le skill en attente n'est pas indiqué, pas
  d'infobulle, d'icône image ni de clic sur la barre.

## Choix de la cible et skill en attente

- **`Cible`** est un `std::variant<CibleAllie, CibleMonstre, CiblePoint>`
  (`CibleAllie` : place de 1 à 4 ; `CibleMonstre` : l'identifiant du monstre et son `Cercle` ;
  `CiblePoint` : le point fixé au clic d'un skill de zone). Pas de
  quatrième cas « aucune » : l'absence est un `std::optional` vide. Les
  cibles n'ont rien de commun, d'où un variant plutôt qu'une classe
  de base.
- **`std::visit` plutôt que `get_if` en cascade** (étape 49) : là où le
  code traite toutes les alternatives (`avancerSkillEnAttente`,
  `declencherSkill`), il utilise `std::visit` avec `Surcharges` (définie
  dans `Cible.hpp`) : une cinquième cible non traitée est une erreur de
  compilation, au lieu d'un `bad_variant_access` à l'exécution.
  `holds_alternative` reste pour les simples tests booléens
  (`skillEnAttenteDeDegats`), et `attaquer` garde ses `get_if` à une
  seule alternative. Dans les lambdas, `avancerSkillEnAttente` visite une
  *copie* de la cible : une lambda peut vider `m_skillEnAttente`, ce qui
  détruirait la cible visitée.
- **Le type du skill décide de la cible cherchée**, dans
  `Escouade::choisirCible(type, place, monstres)`, une fonction à part.
  `Soin` : le vivant qui a le moins de points de vie parmi les blessés
  (soigneur inclus ; égalité : la plus petite place). `Degats` :
  1. avec un verrou, le monstre verrouillé trouvé dans la liste, à
     portée ou non (mise en attente et marche s'il est hors de portée) ;
     s'il n'est pas dans la liste, pas de cible ;
  2. sans verrou, le monstre que le lanceur frapperait en autodéfense
     (`choisirCibleAFrapper`), mais avec la portée de son skill ;
  3. sinon aucune cible : l'appui est refusé, sans recharge.

  `CibleMonstre` porte l'identifiant du monstre : c'est lui qui attribue
  les dégâts dans la liste des coups. Un futur choix manuel par le
  joueur ne devra remplacer que `choisirCible`.
- **Sans verrou, l'activation commence toujours dans l'image de l'appui**
  (la cible est à portée par construction) : jamais de mise en attente dans
  ce cas. Il ne crée pas de verrou et ne déplace personne.
- **À l'appui** : `declencherSkill` refuse si le lanceur n'a pas de skill,
  est mort ou est en recharge, si un skill est déjà en attente, ou si
  `choisirCible` ne trouve rien. Sinon, cible à portée : son activation
  commence tout de suite (`Personnage::engagerSkill`) ; hors de portée : il
  est mis en attente (`m_skillEnAttente`, un seul pour toute l'escouade).
- **Pendant l'attente, le lanceur vise le centre de sa cible** au lieu de
  sa place en formation (même déplacement et mêmes blocages que les
  autres personnages), puis reprend sa place une fois le skill parti ou
  annulé. La cible est fixée à l'appui. Pour un monstre, le cercle lu en
  route est `m_cibleCombat`, mis à jour à chaque `attaquer`, pas la copie
  de l'appui.
- **Où l'activation commence** : à l'appui, dans `declencherSkill` ; pour un
  skill en attente, à l'arrivée du lanceur (à portée et en vue) : un soin dans
  `Escouade::mettreAJour` (`avancerSkillEnAttente`), un skill de dégâts ou de
  zone dans `attaquer` (`engagerSkillDeDegatsEnAttente`, qui reçoit le cercle
  à jour du monstre, et `engagerSkillDeZoneEnAttente`). Le skill quitte alors
  `m_skillEnAttente`. **Où l'effet part** : voir « Temps d'activation des
  skills (étape 58) ».
- **Annulations du skill en attente** (avant l'activation) : lanceur mort
  (les deux types) ; soin : cible morte ou remontée à pleine vie ; dégâts : `deverrouiller()` (mort de la cible,
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
  `monstresVus` touchés par la zone (bord à bord 0) sont provoqués, au départ de
  l'activation de 0,2 s (jamais de mise en attente ni de marche). Aucun monstre dans le
  rayon : elle part quand même et consomme sa recharge (étape 43, comme
  le skill de zone). Un seul déclenchement du `Skill`
  pour la recharge, quel que soit le nombre de monstres. Aucun dégât :
  liste de coups vide. Les refus habituels s'appliquent (lanceur mort,
  recharge, skill en attente) et la recharge de l'attaque de base du
  lanceur est relancée.
- **L'état « provoqué » vit dans `Escouade`**, pas dans `Monstre` :
  `m_provocations`, une classe `Provocations` (étape 51 ; par provocation
  l'identifiant du monstre, la place du provocateur et le temps restant),
  sur le modèle des agresseurs. `Provocations` ne connaît pas `Personnage` :
  `Escouade::mettreAJour` lui signale la mort d'un provocateur
  (`retirerProvocateur`).
  `Monstre` ne change pas : il reçoit toujours sa proie en paramètre.
  `provoquer(id, place, duree)` ajoute ou remet la durée au maximum ;
  elle est publique pour que les tests la préparent, mais le contrôle de
  la recharge et du lanceur est dans `declencherSkill`.
- **Fin d'une provocation** : durée écoulée et provocateur mort
  (`mettreAJour`), monstre absent de la liste (`attaquer`) ; les trois
  retraits sont des fonctions de `Provocations`.
- **Une seule règle pour la proie** : `indiceProieDuMonstre(id,
  position)` renvoie le provocateur s'il est vivant, sinon le vivant le
  plus proche. `proieDuMonstre` (demande de `main.cpp`) et
  `recevoirDegats` s'en servent. Si le provocateur est mort entre-temps,
  la règle retombe tout de suite sur le plus proche, même avant le
  retrait de la provocation.
- **Le reste du monstre ne change pas** : détection, territoire, abandon
  et retour au poste suivent leurs règles, même provoqué. Un monstre
  provoqué dont le provocateur est hors de son territoire ou de sa
  détection abandonne donc comme avant ; en retour (étape 69), il n'est plus
  ciblable et sa provocation est oubliée au prochain `attaquer`.
- **Limites connues** : pas d'indicateur visuel du monstre provoqué ni
  d'effet au déclenchement ; pas de système d'effets général ;
  `provoquer` n'est pas protégée contre un appel direct sans recharge.

## Portée et rayon de zone (étape 42)

- **Deux notions distinctes** : la portée (jusqu'où le skill peut être
  lancé, `m_portee`) et le rayon de zone (taille de la zone touchée,
  `m_rayonZone`, getter `getRayonZone`). Les attaques de base, qui sont
  aussi des `Skill`, n'ont pas de zone. Pas de valeur par défaut au
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
  monstres, carte)`, distincte de `declencherSkill` qui n'a pas de point et
  refuse un skill `Zone` (`choisirCible` renvoie aussi vide). La
  fonction reçoit le point directement ; `CiblePoint` n'existe que pour
  l'attente (étape 43).
  Chaque monstre touché reçoit un `Coup` (dégâts du skill), appliqué par
  `main.cpp` comme les autres. Le sort part et coûte sa recharge même si
  la zone est vide (choix de jeu, contrairement à la provocation) ; il
  relance la recharge de l'attaque de base du lanceur. Refus : place
  invalide, skill non `Zone`, lanceur mort, recharge, skill en attente.
  Pas de dégâts aux alliés. Le `Skill` se déclenche par
  `declencherSansCible` (recharge et puissance, sans portée) ; la vraie
  portée est testée avant, contre le point.
- **`skillEstPret(place)`** permet à `main.cpp` de savoir s'il peut
  entrer en visée sans connaître `Personnage` ; `rayonZoneDuSkill(place)`
  lui donne le rayon du cercle de prévisualisation.
- **Le mode visée est un état d'interface** (voir
  `docs/boucle-et-entrees.md`) : `Escouade` ne connaît ni la souris ni le
  mode.
- **Limites connues** : pas d'effet visuel à l'impact ; pas de dégâts aux
  alliés ni dans le temps.

## Portée de lancement du skill de zone (étape 43)

- **Mesure** : distance bord à bord entre le cercle du lanceur et un
  `Cercle` de rayon 0 centré sur le point (`Personnage::skillEstAPortee`,
  donc `Skill::estAPortee`), comme les autres skills.
- **À portée** : l'activation (0,8 s) commence dans l'image du clic. **Hors portée** :
  `m_skillEnAttente` reçoit `CiblePoint { point }` (point fixé au clic),
  sans coup ni recharge, et le lanceur marche vers le point (même
  déplacement que les autres skills en attente, `avancerSkillEnAttente`).
- **Engagement** : au début de `attaquer`, avant l'approche du verrou et les
  attaques de base : dès que le lanceur vivant est à portée et en vue du
  point, son activation commence (`engagerSkillDeZoneEnAttente`). La zone est
  évaluée au départ de l'activation, sur `monstres` à jour, pas au moment du
  clic.
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

## Ligne de vue du skill de zone (étape 44)

*(Depuis l'étape 73, `ligneDeVue` est la fonction libre de `ChampDeVision`, qui décide en cases ; voir la section « Une seule ligne de vue » plus bas.)*

- **Condition unique « peut lancer au point »** :
  `Escouade::peutLancerAuPoint(place, point, carte)` = point à portée
  (bord à bord) ET `ligneDeVue(carte, centre du lanceur, point)`. Elle
  sert au clic (`declencherSkillDeZone`) et pendant l'attente
  (`attaquer`), pour que les deux chemins ne divergent pas.
- **À portée mais pas en vue** : même traitement que hors de portée,
  mise en attente. Le lanceur marche vers le point ; son `Mobile` passe
  par le A* quand un mur le sépare de sa destination, donc il contourne,
  et le sort part dès que la condition est vraie.
- **Point dans un mur** : refusé (liste vide, ni attente ni recharge) ;
  `main.cpp` sort de la visée comme pour les autres refus.
- **Explosion** : un monstre est touché s'il est dans la zone (règle
  inchangée) ET si `ligneDeVue(carte, point d'impact, centre du monstre)`.
  Sans test sur les bords : un monstre dont le centre est (dans sa case)
  derrière un mur est épargné même si son bord dépasse. La carte est
  un paramètre de `declencherSkillDeZone`, `lancerSkillDeZone` et
  `attaquer`, jamais un membre.
- **Limites** : la prévisualisation de la visée n'indique pas si le point
  est en vue ; un lanceur qui ne peut jamais voir le point reste en
  attente.

## Ligne de vue des personnages (étape 46)

- **Une seule règle, dans `Escouade`** : `peutAtteindre(personnage, arme,
  cible, carte)` = `Skill::estAPortee` (bord à bord) ET
  `ligneDeVue(carte, centre du personnage, centre de la cible)`.
  `Skill` reste pur (il ne connaît pas `TileMap`). Comme `Escouade` ne voit
  pas le `Skill` d'un `Personnage`, une `enum class Arme { AttaqueDeBase,
  Skill }` privée dit laquelle des deux portées utiliser. Aucun autre
  appel à `estAPortee` ne reste dans `Escouade` ; la condition « peut
  lancer au point » du skill de zone (`peutLancerAuPoint`) s'appuie aussi
  dessus (cible : cercle de rayon 0).
- **Autodéfense** : la lambda passée à `choisirCibleAFrapper` appelle
  `peutAtteindre` ; le module `Autodefense` n'a pas changé. Un monstre
  caché n'est pas candidat, le suivant (même plus loin) peut l'être.
- **Combat verrouillé** : l'ancre s'arrête quand tous les vivants
  atteignent la cible, et un personnage s'arrête individuellement quand
  lui l'atteint. Un tireur à portée sans vue ne s'arrête donc pas, et
  l'ancre continue d'avancer jusqu'à ce que tous voient la cible.
- **Agresseurs** : `m_monstreAPortee` exige aussi la vue ; un personnage
  qui ne voit pas son agresseur va vers lui (dans la laisse de 100 px,
  ancre à l'arrêt, comme avant).
- **Skill de dégâts** : avec un verrou, départ immédiat ou en attente
  seulement si le lanceur atteint le monstre ; sinon mise en attente et
  marche, sans recharge. Sans verrou, `choisirCible` passe la même lambda
  que l'autodéfense avec la portée du skill : un monstre caché n'est pas
  candidat, l'appui est refusé.
- **Soin** : le soigneur doit voir l'allié, au clic comme pendant l'attente
  (`avancerSkillEnAttente`) ; sinon mise en attente et marche. Le choix du
  plus blessé ne change pas (vu ou non).
- **Provocation** : un monstre est provoqué s'il est dans la zone ET si
  `ligneDeVue(carte, centre du provocateur, centre du monstre)`, même règle que
  l'explosion du skill de zone. La recharge est consommée dans tous les
  cas.
- **Cartes en paramètre** : `declencherSkill`, `choisirCible` et
  `avancerSkillEnAttente` reçoivent la carte, jamais en membre.
- **Monstres** (étape 62) : la détection et la frappe des monstres exigent
  maintenant la vue (voir `docs/monstre.md`). Hors de portée de l'étape 46 : la
  riposte et la détection des monstres ignoraient alors les murs ; pas de placement pour obtenir la vue (le
  personnage avance simplement vers sa cible) ; pas d'indicateur d'une
  cible non visible ; la vue est mesurée entre les centres, pas bord à bord (la case
  de chacun est celle qui contient son centre).

## Une seule ligne de vue (étape 73)

- **Un seul algorithme de vue dans tout le jeu** : `ligneDeVue(carte, a, b)`, fonction libre
  de `ChampDeVision`, répond par le shadowcasting symétrique de la vision du joueur
  (`docs/vision.md`). Ce qu'un personnage voit est exactement ce qu'il peut viser, et « A
  voit B » équivaut à « B voit A », pour les personnages comme pour les monstres. Avant,
  près d'un coin, le joueur pouvait voir un monstre que ses personnages ne pouvaient pas
  frapper, ou l'inverse.
- **Tous les usages changent d'un coup, sans règle particulière** : `Escouade::peutAtteindre`
  (autodéfense, combat verrouillé, agresseurs, skills de dégâts et de soin), provocation,
  skill de zone (lanceur → point, point d'impact → monstres), places d'attaque (« en vue du
  monstre »), `Monstre::peutAtteindre` et détection des monstres. Les portées et la logique
  autour ne changent pas.
- **Granularité** : la vue se décide d'une case à l'autre. Deux points d'une même case se
  voient toujours ; un point dans un mur ou une porte fermée ne voit rien et n'est vu de
  personne. Le détail fin près d'un mur (1 px de marge) n'existe plus.
- **Deux murs en diagonale** : deux cases libres en diagonale entre deux murs qui se touchent
  **se voient** (la lumière passe par le coin commun). C'est ce que donne l'algorithme de
  Ford, conservé tel quel ; l'ancien segment centre à centre était bloqué. Épinglé par un test.
- **Pas de cache** : `ligneDeVue` est recalculée à chaque appel. Mesuré : aucun
  ralentissement dans la suite de tests (12,3 s avant, 13,3 s après, l'écart venant des
  nouveaux tests d'équivalence). À reprendre seulement si un problème est mesuré en jeu.

## Recharge aléatoire (étape 56)

- **Intervalle plutôt que base ± %** : `Skill` reçoit `rechargeMin` et
  `rechargeMax` (`assert(0 <= min <= max)`). Deux nombres se lisent
  directement (« 0,3 à 0,7 s ») et `min == max` exprime une recharge fixe
  sans cas particulier de configuration. Base ± % aurait demandé un nombre
  de plus à interpréter.
- **Tirage au déclenchement** (début de la recharge), dans `declencher`,
  `declencherSansCible` et `relancerRecharge` : uniforme,
  `std::uniform_real_distribution<float>(min, max)` créée sur place (elle
  ne garde pas d'état utile ; le générateur oui). Si `min == max`, **aucun
  nombre n'est tiré** : l'état du générateur n'avance pas, ce qui garde les
  suites reproductibles et prévisibles pour les skills fixes.
- **`m_tempsRecharge`** devient la durée de la recharge en cours (dernier
  tirage) : `getProgressionRecharge` reste exact pour la barre de recharge.
  Avant le premier tirage il vaut `rechargeMax` (sans effet : la recharge
  restante est 0, donc la progression 1).
- **`relancerRecharge` tire aussi** : quand un skill part, l'attaque de
  base de son lanceur repart pour une recharge complète, donc avec un
  nouveau tirage dans son intervalle.
- **Valeurs** : attaques de base en intervalle (voir « Skill ») ; les
  skills des touches 1 à 4 restent fixes (5, 10, 8 et 8 s) : le joueur
  planifie son prochain soin. Pas d'aléa sur les dégâts.
- **Le `Generateur` descend par paramètre** : `Partie` →
  `Escouade::attaquer` / `declencherSkill` / `declencherSkillDeZone` /
  `mettreAJour` (le soin d'un skill en attente part dans `mettreAJour`) →
  `Personnage::avancerAttaque` / `declencherSkill` / `declencherSkillSansCible` →
  `Skill` ; `Partie` → `Monstre::avancerAttaque` → `Skill`. Toujours `Generateur&` :
  une copie repartirait du même état et redonnerait la même suite.
  Personne ne garde de référence ni de pointeur vers lui.
- **Limites** : distribution uniforme (pas en cloche) ; pas de classe
  `Arme` (les intervalles sont écrits dans `Escouade.cpp` et
  `Monstre.cpp`) ; la barre de skills n'est pas concernée (skills fixes).

## Armure à rendement décroissant (étape 55)

- **Formule** : réduction = armure / (armure + K), dégâts subis = montant ×
  (1 − réduction), avec K = 100 (`s_constanteArmure` dans `Defense.cpp`).
  Chaque point d'armure compte, mais la réduction n'atteint jamais 100 % :
  100 d'armure = moitié des dégâts, 50 = un tiers de moins, 20 = un sixième
  de moins.
- **Code** : la fonction libre pure `appliquerArmure(montant, armure)`
  (module `Defense`) ne dépend ni de SFML ni d'un autre module. Elle est
  écrite `montant × K / (armure + K)` (même valeur, une seule division) :
  avec `montant × (1 − réduction)` en flottant, 15 avec 20 d'armure pouvait
  donner 12,4999 au lieu de 12,5, donc 12 au lieu de 13.
- **Arrondi** : au plus proche (`std::lround`, conversion `static_cast`
  explicite), avec un plancher à 1 : un coup positif inflige toujours au
  moins 1, un coup de 0 reste 0. Exemples : 10 avec 50 → 7, 5 avec 20 → 4,
  15 avec 20 → 13, 25 avec 20 → 21.
- **Qui l'applique** : `Personnage::recevoirDegats` et
  `Monstre::recevoirDegats`, avant `PointsDeVie::subirDegats`.
  `PointsDeVie` ne connaît pas l'armure. Chaque entité a un `m_armure`
  (`int`) reçu par son constructeur ; pas d'accesseur.
- **Valeurs** : mêlées (places 1 et 2) 50, tireurs (places 3 et 4) 0, les
  monstres 20 (`Niveau::armureMonstres`, valeur `s_armureMonstresParDefaut`
  commune à la carte à la main et aux cartes générées).
- **Le soin n'est pas touché** par l'armure. Les dégâts de zone, de skill
  et d'attaque de base passent tous par `recevoirDegats`, donc par l'armure.
- **Limites** : pas d'affichage de l'armure, pas d'esquive ni de blocage,
  pas de types de dégâts ni de résistances, un seul K pour toutes les
  entités, pas d'équipement.

## Temps d'exécution et coup engagé (étape 57)

- **Cycle** : une attaque de base n'est plus instantanée. Prête → **engagée**
  (l'attaquant doit être vivant, immobile, avec une cible à portée et en vue :
  règles d'atteinte inchangées) → **en exécution** (0,15 s pour un personnage,
  0,3 s pour un monstre) → le **coup part** (la recharge est tirée à ce
  moment, comme à l'étape 56) → **en recharge** → prête. Le joueur voit chaque
  coup se préparer et on ne frappe plus en marchant.
- **Machine à états dans `Skill`** (`PhaseSkill`, voir `ARCHITECTURE.md`) :
  une phase explicite plutôt que des booléens qui pourraient se contredire.
  `Skill` ne connaît pas la cible : l'entité retient sur qui porte le coup
  (`Personnage::m_actionEngagee`, une `ActionEngagee` dont la cible est un
  `CibleMonstre` pour l'attaque de base ; `Monstre::m_idProieEngagee` : place du
  personnage).
- **« Immobile »** : défini une seule fois, `Mobile::estEnMouvement()` :
  au dernier tic, le point à atteindre était à plus de 0,01 px. Un mobile
  bloqué qui essaie d'avancer est en mouvement. Une définition « il reste un
  point dans le chemin » ne convenait pas : un personnage qui rattrape sa
  place à chaque image aurait paru immobile pendant que l'ancre marche.
- **Pendant l'exécution**, l'attaquant reste sur place : un personnage ne
  suit ni sa place de formation, ni un agresseur ni un skill en attente
  (`Escouade::pointVoulu`, règle 0) ; un monstre ne poursuit ni ne rentre.
- **Le coup engagé part toujours**, même si la cible a bougé hors de portée
  ou de vue (pas de revérification à la fin). Annulation, sans recharge
  consommée :
  - mort de l'attaquant, disparition ou mort de la cible (`Escouade::attaquer`
    pour un personnage ; `Partie::faireFrapperLeMonstre` pour un monstre,
    qui annule si le personnage visé est mort) ;
  - personnage seulement : un ordre de déplacement (`Escouade::allerVers`,
    donc aussi le déplacement continu), un verrouillage sur un autre monstre,
    un clic au sol (`Escouade::annulerAttaquesEngagees`) ;
  - personnage seulement : un skill de ce personnage qui s'engage (étape 58).
    Il annule le coup ; la recharge de l'attaque de base est relancée au
    départ du skill (règle « un skill remplace l'attaque de base »).
    `Personnage::annulerAttaque` n'annule que l'attaque de base : un ordre du
    joueur n'annule pas l'activation d'un skill.
  Un monstre ne s'annule jamais : même si une provocation change sa proie
  entre-temps, le coup en cours touche le personnage retenu à l'engagement ;
  la nouvelle proie compte pour le coup suivant.
- **La recharge court pendant la marche** : elle démarre au départ du coup.
- **Skills des touches 1 à 4** : pas soumis à l'immobilité pour s'engager,
  mais ils ont leur propre temps d'activation (voir « Temps d'activation des
  skills (étape 58) »).
- **`Escouade::attaquer` reçoit `dt`** : c'est elle qui fait avancer les
  exécutions des personnages, avant `mettreAJour`.
- **Limites** : un coup part sans revérifier portée ni vue ; un personnage
  en mouvement ne peut pas engager (kiting possible tant que les vitesses
  le permettent) ; pas de barre de progression ni d'animation.

## Temps d'activation des skills (étape 58)

- **Même machine à états que l'attaque de base** : les skills des touches 1 à 4
  passent par `Skill::commencer` / `avancer` / `annuler`. Engagé → **en
  activation** (le temps d'exécution du `Skill`) → l'effet part, la recharge
  est tirée → en recharge. Valeurs dans `Escouade.cpp` : dégâts 0,3 s,
  provocation 0,2 s, soin 0,6 s, zone 0,8 s. Le code est général : un skill
  d'activation 0 part dans l'image de l'appui (`avancerSkillDe` avec `dt = 0`
  juste après l'engagement).
- **Action engagée par personnage** (`ActionEngagee.hpp`) :
  `ActionEngagee { TypeAction action; std::optional<Cible> cible; }`, où
  `TypeAction` est `AttaqueDeBase` ou `Skill`. `Personnage::m_actionEngagee`
  remplace l'ancien identifiant de monstre : un personnage n'a qu'une action
  engagée à la fois, par construction, et plusieurs personnages peuvent
  activer en même temps. `Personnage` stocke la cible sans l'interpréter ;
  c'est `Escouade` qui la résout (place d'un allié, identifiant d'un monstre,
  point). La cible est vide pour la provocation. Pour l'attaque de base, le
  `cercle` de la `CibleMonstre` n'est pas renseigné (seul l'identifiant sert).
  Le verrou et `m_skillEnAttente` restent dans `Escouade`.
- **L'appui est un ordre** : il n'exige pas que le lanceur soit immobile.
  L'activation commence et le lanceur s'arrête sur place pendant toute sa
  durée (`Personnage::estEnExecution`, règle 0 de `pointVoulu`) ; le reste de
  l'escouade continue, puis le lanceur rattrape sa place.
- **Au début de l'activation**, le coup de base engagé du lanceur est annulé
  (`engagerSkill`). Aucune attaque de base ne s'engage pendant l'activation
  (`engagerAttaque` refuse). Au départ, la recharge de l'attaque de base est
  relancée.
- **Pendant l'activation le skill n'est pas prêt** (`skillEstPret` repose sur
  la phase) : un nouvel appui sur la même touche est refusé et la touche 4
  n'entre pas en visée. Les skills d'autres personnages restent permis.
- **Interruptions, sans recharge consommée** (`Escouade::avancerSkillDe`) :
  mort du lanceur (tous les types) ; skill de dégâts : mort ou disparition du
  monstre engagé ; soin : mort de l'allié visé. N'annulent **pas** une
  activation : un ordre de déplacement (clic, déplacement continu), un clic au
  sol, un verrouillage sur un autre monstre. Le skill est l'ordre le plus
  récent, il va au bout. Un soin dont la cible est remontée à pleine vie part
  quand même (plafonné par `recevoirSoin`). Pas de revérification de portée
  ni de vue au départ (même règle qu'à l'étape 57).
- **Effet évalué au départ** (`Escouade::effetDuSkill`) : dégâts : le coup va
  au monstre engagé ; soin : à l'allié engagé ; zone : monstres dans la zone au
  moment du départ, autour du point fixé, en vue du point d'impact ;
  provocation : monstres dans la zone autour du provocateur au moment du
  départ, en vue. Tous les départs ont lieu dans `Escouade::attaquer`
  (`avancerSkills`, après les engagements de skills en attente et avant
  l'autodéfense), qui reçoit `dt`. Le soin et la provocation sont appliqués
  dans `Escouade` ; les dégâts et la zone sortent en liste de `Coup`.
- **Skill en attente** : à l'arrivée du lanceur (à portée et en vue) il entre
  en activation au lieu de partir, et quitte `m_skillEnAttente`, ce qui libère
  la place pour un autre skill en attente. Un soin en attente est engagé dans
  `mettreAJour` et ne part donc qu'au `attaquer` de l'image suivante
  (`Partie` appelle les deux à chaque image).
- **Indices visuels** : le carré du lanceur s'éclaircit (même indice qu'à
  l'étape 57) ; l'icône de la barre de skills a un contour blanc
  (`EtatSkill::enActivation`).
- **Tests** : `Personnage` avec des skills d'activation construits dans les
  tests ; `Escouade` avec les vraies valeurs. Aides de `tests/aides.hpp` :
  `declencherEtAttendre`, `declencherZoneEtAttendre` et
  `attendreLeDepartDesSkills` (attaquer par tics ; elles écartent les coups de
  base de 5 dégâts), `simulerAvecAttaque`.
- **Limites** : pas d'interruption de l'activation par les dégâts reçus ; un
  seul skill en attente pour toute l'escouade ; pas de barre de progression
  d'incantation ni d'animation ; pas de revérification de portée ou de vue au
  départ ; un lanceur en mouvement s'arrête sur place sans transition.

## Circuit des dégâts

`Escouade` et `Monstre` ne se connaissent pas : `main.cpp` fait
l'intermédiaire (voir `docs/boucle-et-entrees.md` pour l'ordre dans une
image).

- **Les personnages frappent un monstre** : à chaque image, `main.cpp`
  construit la liste `monstresVus` (identifiant et cercle de chaque
  monstre ciblable : depuis l'étape 69, `Partie::entitesCiblables`, sans ceux qui
  rentrent à leur poste), puis applique, avec une petite lambda `appliquer`, les coups
  renvoyés par `escouade.declencherSkillDeZone(...)` (clic de lancer en
  visée), par `escouade.declencherSkill(...)` (si une touche est
  appuyée) puis par `escouade.attaquer(...)`, chacun au monstre dont
  l'identifiant figure dans le `Coup`. Un soin ne rend aucun coup. Les skills
  partent dans `attaquer` (`avancerSkills`), avant l'autodéfense : au départ,
  la recharge de base du lanceur est relancée, et pendant l'activation il ne
  peut engager aucune attaque de base.
- **Un monstre frappe un personnage** (étape 57) : pour chaque monstre,
  `Partie` demande `escouade.proieDuMonstre(...)` (l'entité du provocateur
  si ce monstre est provoqué, sinon du vivant le plus proche) et
  `escouade.candidatsDeFrappe(...)`, puis met le monstre à jour. Puis `faireFrapperLeMonstre` : si un coup est engagé, il
  avance (annulé si le personnage visé est mort) et, au départ, `Partie`
  appelle `escouade.recevoirCoup(id, place, origine, dégâts)` sur la place
  retenue à l'engagement ; sinon, un monstre immobile avec une cible de
  frappe à portée et en vue engage un coup sur la place choisie par
  `Monstre` (`choisirCibleAFrapper`).
  L'armure du personnage touché réduit ensuite le coup (voir plus haut).
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
  personnage (depuis l'étape 68, sous forme d'`EntiteVue`). Les personnages ne se bloquent pas entre eux : une séparation
  douce (étape 67, plus lente que la marche) écarte ceux qui sont à moins de
  34 px, et des places d'attaque distinctes autour du monstre évitent qu'ils
  visent le même point (`docs/formation.md`).
- **L'ancre ne bloque personne et n'est pas bloquée** : c'est un point
  de contrôle abstrait, pas une entité de combat. `m_mobile` reçoit
  toujours une liste vide.
- **Les morts ne bloquent pas** : un cadavre est franchissable,
  cohérent avec le fait qu'il n'est plus ciblé ni dangereux.
- **Un monstre qui rentre à son poste bloque, mais n'est plus une cible** (étape 69) :
  il reste dans la liste physique (obstacle et à contourner) et sort de la liste des
  cibles (autodéfense, verrou, provocation, skills). Ses coups reçus ne comptent de
  toute façon pas : il est invulnérable (`docs/monstre.md`).
- **`Escouade` et `Monstre` ne connaissent pas les cercles des autres** :
  `Escouade` expose `cerclesDesVivants` plutôt que de laisser `main.cpp`
  parcourir les personnages, et reste seule à connaître `m_personnages`.
- **Bloquer et contourner** (étape 68) : ces listes sont les obstacles
  **bloquants**. Les chemins évitent, eux, une liste distincte (`aContourner`) :
  les monstres contournent les autres monstres, les personnages contournent les
  monstres sauf celui qu'ils approchent (voir `docs/chemin.md`). Contre un
  cercle, le glissement essaie aussi la tangente (`docs/carte-et-collision.md`).
- **Limites connues** : une entité bloquée contourne ce qu'on lui a fait
  contourner (coût ajouté : un passage sans autre issue est traversé, elle attend
  alors) ; la
  formation n'est pas resserrée quand un personnage est bloqué ; un
  chevauchement déjà existant
  peut subsister, la règle de blocage permettant seulement de s'en
  dégager.

## Hygiène du code (étape 49)

- **`trouverEntite`** (`EntiteVue.hpp`) remplace les sept `find_if` par
  identifiant : renvoie un pointeur, `nullptr` si absent, et chaque appelant
  teste avant de déréférencer. `main.cpp` a `trouverMonstreParId` pour ses
  `Monstre`.
- **`[[nodiscard]]`** sur ce dont ignorer le retour est forcément un
  bogue : listes de `Coup`, `Skill::declencher`, `frapper`, les `choisir*`,
  `trouverChemin`, `trouverEntite`, ainsi que les fonctions pures
  (`distanceBordABord`, requêtes de `TileMap`, etc.). Un test qui n'a
  besoin que de l'effet de bord écrit `static_cast<void>(...)`.
- **`TypeSkill.hpp` et `EntiteVue.hpp` séparés** : un module d'affichage
  ou de décision n'a plus besoin de la classe `Skill` ni de `Coup` pour un
  enum ou une entrée.
- **`Skill::declencherSansCible`** (étape 53, audit R4), supprimée à
  l'étape 62 avec `declencher` : depuis l'étape 58, tout passe par la machine
  à états.

## Règle commune aux deux camps et provocation stricte (étape 66)

- **Une seule règle de choix** : `choisirCibleAFrapper(origine, estAPortee,
  entites, idPrefere)` sert aux personnages (préféré = monstre verrouillé) et
  aux monstres (préférée = la proie). `EntiteVue` porte l'identifiant d'un
  monstre ou la place 1 à 4 d'un personnage : le même type circule dans les
  deux sens, prêt pour une escouade de monstres.
- **Candidats fournis par `Escouade`**, là où vivent les provocations :
  `candidatsDeFrappe(idMonstre)`. Provocation **stricte** : un monstre
  provoqué dont le provocateur est vivant n'a que lui comme candidat et ne
  frappe donc personne d'autre, même collé à un autre personnage ; provocateur
  mort ou provocation finie, tous les vivants. Le monstre reste ignorant
  d'`Escouade` : `Partie` lui passe les listes.
- **Provocation souple** (un provoqué qui frappe aussi d'autres personnages) :
  hors de portée, non faite.
