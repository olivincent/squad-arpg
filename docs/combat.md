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
  source de vérité. Depuis l'étape 82.1, la formule est écrite une fois, dans
  `cercleDuRectangle` (`base/Cercle`) : `Mobile::getCercle` l'appelle (les deux `getCercle` en sont
  des façades), et le glissement de `Deplacement` aussi. Elle est dans `base/` et pas seulement dans
  `Mobile` parce que `Mobile` utilise déjà `Deplacement` : `Deplacement` ne peut pas dépendre de
  `Mobile` sans dépendance circulaire.
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
    est dans la liste et qu'au moins un personnage **vivant** n'est pas à portée,
    `allerVers(cercle.centre, carte)`, sinon l'ancre s'arrête sur place
    (`m_mobile.arreter()`, pour cesser d'avancer sans reculer). Son cercle n'est
    plus mémorisé (`m_cibleCombat`, supprimé à l'étape 82.4) : `mettreAJour` cherche
    le verrou lui-même dans ses monstres ciblables. Le skill
    de dégâts en attente part aussi ici, dès que son lanceur est à
    portée (voir « Choix de la cible et skill en attente ») ;
  - **les frappes, verrou ou non** : voir « Autodéfense ».
- **Les morts sont ignorés pour l'arrêt** (voir `docs/formation.md`).
- **Frapper se fait en deux temps** depuis l'étape 57 (voir « Temps d'exécution et coup engagé »).
- **Placement pendant l'attaque** : un personnage qui atteint déjà le
  monstre verrouillé (à portée et en vue) reste sur place ; les autres
  marchent vers leur place d'attaque autour de lui (depuis l'étape 67 ;
  `PlacesDAttaque` depuis l'étape 82.3), et non plus vers leur place de
  formation. Voir `docs/formation.md`, « Places d'attaque ».

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
  `m_agresseurs`, fichiers à part : par agresseur un identifiant et le
  temps restant) sans connaître `Monstre` : `recevoirCoup`
  reçoit l'identifiant du monstre, `Partie` reste l'intermédiaire. Un
  coup de dégâts `> 0` ajoute l'agresseur ou remet son temps à 3 s
  (`s_dureeMemoire`, dans `Agresseurs.cpp`) ; un coup à 0 n'enregistre rien. **Depuis 82.5 la
  mémoire ne garde aucune position** : elle est lue, au moment de s'en servir, dans les monstres
  ciblables que `mettreAJour` reçoit.
- **Oubli** : le temps s'écoule dans `mettreAJour` ; un agresseur absent
  des monstres reçus par `attaquer` (monstre mort, ou qui rentre à son poste) est retiré
  (`Agresseurs::retirerMonstresAbsents`). La liste de `EntiteVue` que lit
  `choisirAgresseurAViser` est construite une fois par image (`Agresseurs::vusParmi(ciblables)`,
  dans `mettreAJour`), non plus une fois par personnage ; un agresseur encore en mémoire mais absent
  des ciblables (mort dans l'image, par les coups d'`attaquer`) est ignoré pour l'image : plus
  personne ne marche vers sa dernière position.
- **Choix de la cible** : `choisirAgresseurAViser` (module `Autodefense`,
  fonction libre testée seule) garde les agresseurs dont le centre est à
  100 px ou moins (`s_laisseAgresseur`) de la **place en formation** du
  personnage (pas de sa position actuelle, sinon la laisse suivrait le
  personnage et il pourrait être entraîné sans limite), puis prend le plus
  proche de lui.
- **Règle de déplacement** (règle 4 de `choisirIntention` depuis l'étape 82.4 ; avant,
  `pointVersAgresseur`), ancre à l'arrêt seulement : aucun agresseur dans la laisse → la place ;
  sinon, monstre déjà à portée de l'attaque de base → reste sur place (sans cela il reviendrait à sa
  place, sortirait de portée et oscillerait) ; sinon l'agresseur, à sa place d'attaque autour de lui
  (étape 67), ou son centre s'il n'y a aucune place libre (sans le contourner depuis 82.4). « Monstre
  à portée » est calculé par `mettreAJour`, sur ses monstres ciblables (étape 82.4, R2 ; avant,
  mémorisé par `attaquer` dans `m_monstreAPortee`) : un monstre qui rentre à son poste ne compte
  jamais, un monstre tué dans l'image non plus.
- **Priorités** : celles de `choisirIntention` (`docs/formation.md`, « Intentions ») : le lanceur
  d'un skill en attente vise sa cible ; avec un verrou, un personnage à portée du monstre verrouillé
  reste sur place ; l'ordre du joueur (ancre en mouvement) fait viser la place à tous.
- **Règle générique** : aucun type « mêlée / tireur » ; un tireur à 120 px
  ne bouge presque jamais, un mêlée va au contact. Même `Mobile` que les
  autres déplacements (mêmes blocages).
- **Limites connues** : le contournement des monstres évite leur cercle, pas leur zone de contrôle
  (un personnage peut la frôler et subir l'attaque d'opportunité ; piste : la « stratégie d'attaque »
  par type de monstre, `docs/brainstorm.md`, section 4 bis) ; l'ancre ne bouge pas vers l'agresseur ; pas de
  poursuite au-delà de la laisse ; l'ancre est « à l'arrêt » dès que son
  chemin est fini, même un instant ; un agresseur qui frappe depuis plus
  de 100 px de la place d'un personnage n'attire pas celui-ci.

## Skill

- **Une classe `Skill`** : un `TypeSkill` (`DegatsSurCible`, `Soin`,
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
- **Les skills d'un personnage sont un `std::vector<SkillConnu>`** (étape 74), reçu au
  constructeur de `Personnage` comme l'attaque de base (injection) ; la liste vide dit
  « pas de skill ». Voir « Skills désignés par identifiant (étape 74) ».
- **Quatre skills** (les deux derniers sont décrits dans « Provocation »
  et « Skill de zone ciblé »), aux
  valeurs écrites dans `Escouade.cpp` :
  - dégâts : personnage 0 (mêlée avant-gauche), 25 dégâts, recharge 5 s,
    portée de mêlée (20 px), activation 0,3 s ;
  - soin : personnage 2 (tireur arrière-gauche), 30 points de vie,
    recharge 8 s, portée 40 px bord à bord, activation 0,6 s.
  Provocation (0,2 s) et zone (0,8 s) : voir leurs sections.
- **Le skill est désigné par une adresse** (étape 74) : `Escouade::declencherSkill(
  AdresseSkill, ...)`, avec `AdresseSkill { place, id }`. Avant l'étape 74 la touche N
  désignait le personnage de la place N. Sans champ « type de personnage ».
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
  l'appui. L'ancienne barre orange sous la barre de vie a été supprimée à l'étape 74 ;
  la barre de skills affiche la recharge de chaque skill.
- **Le soin est appliqué dans `Escouade` et n'en sort pas** : `main.cpp`
  ne reçoit aucun dégât pour un soin. `Personnage::recevoirSoin`
  plafonne au maximum et ne soigne pas un mort.
- **Le soin est permis hors combat** ; le skill de dégâts vise le
  monstre verrouillé, ou, sans verrou, le monstre que son lanceur
  frapperait en autodéfense (voir ci-dessous).

## Adresses des skills pour la barre de raccourcis (étape 75)

- **`Escouade::adressesDesSkills()`** donne les `AdresseSkill` de tous les skills de l'escouade,
  par place croissante puis dans l'ordre de la liste de chaque personnage
  (`Personnage::idsDesSkills`). C'est la seule chose que la barre de raccourcis demande à
  l'escouade pour se remplir (`barreParDefaut`) : l'escouade ne sait pas qu'une barre existe.
- **`Escouade::couleurDuPersonnage(place)`** (et `Personnage::getCouleur`) : la couleur du lanceur,
  pour la bande de l'icône. Vide si la place est invalide.
- **Pourquoi par identifiant et non par position** : une adresse reste valable quel que soit
  l'emplacement qui la porte (voir `docs/boucle-et-entrees.md`) ; elle ne dépend ni de la touche ni
  de la place à l'écran.

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
  est dessiné ; l'étiquette (un sigle de deux lettres depuis l'étape 76 : FP, PR, SO, EX…) vient d'un `switch` sans `default`
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
  route est celui du verrou parmi les ciblables de `mettreAJour` (étape 82.4 ;
  avant, `m_cibleCombat`, mis à jour à chaque `attaquer`), pas la copie
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
  plus proche. `proieDuMonstre` (demande de `Partie`) et
  `candidatsDeFrappe` s'en servent (`Escouade::recevoirDegats`, retirée en 82.5, aussi : les tests
  passent par l'aide `infliger`). Si le provocateur est mort entre-temps,
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
- **Agresseurs** : « un monstre à portée » (calculé par `mettreAJour` depuis 82.4) exige aussi la vue ; un personnage
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

## Armure par soustraction, modèle M1 (étapes 55 puis 87)

- **Formule (étape 87.3)** : `subis = max(D − armure, D × plancher)`, avec D le montant du coup. L'armure
  retire un montant fixe par coup, mais le coup garde toujours au moins 20 % de sa force
  (`s_plancherArmure` dans `Defense.cpp`, valeur « à tester »). Elle remplace le rendement décroissant
  de l'étape 55 (réduction = armure / (armure + 100), constante K supprimée). Voir `docs/classes-et-combat.md`,
  section 8.
- **Arrondi** : au plus proche (`std::lround`), avec un plancher à 1 : un coup positif inflige toujours au
  moins 1, un coup de 0 reste 0. Exemples : 100 avec 30 → 70, 50 avec 10 → 40, 100 avec 500 → 20.
- **Échelle × 10 (étape 87.1)** : les points de vie (1000 pour un personnage et un monstre), les dégâts
  (attaque de base d'un personnage 50, du monstre 100), les soins et les skills sont exprimés dix fois
  plus grand qu'avant l'étape 87 : M1 retire un montant fixe et les caractéristiques ajouteront des
  pourcentages, ce que des coups de 5 ne permettent pas en entiers. Les durées (provocation en
  secondes), les portées et les recharges ne changent pas. Les valeurs chiffrées des sections plus
  anciennes de ce document sont à lire × 10 pour les dégâts, les soins et les points de vie.
- **Point d'entrée unique du coup (étape 87.2, étendu aux étapes 89 et 90)** : `resoudreCoup(Attaque, Defenses,
  bool blocagePret, Generateur&)` (module `ResolutionDuCoup`), appelée par `Vitalite::recevoirDegats`. Elle
  enchaîne des fonctions d'étape dans l'ordre de la section 6 de `docs/classes-et-combat.md` : l'esquive, le
  critique, le blocage, l'armure, puis les résistances (étape 94, portion par portion) ; voir « Le critique… (étape 89) »,
  « L'esquive et le blocage (étape 90) » et « Types de dégâts et résistances (étape 94) ». `Defenses` contient
  l'armure et les statistiques de combat du défenseur, dont les résistances.
- **Qui l'applique** : `Vitalite::recevoirDegats`, avant `PointsDeVie::subirDegats`, pour `Personnage`
  et `Monstre` qui lui délèguent. `PointsDeVie` ne connaît pas l'armure. Chaque entité a une `Vitalite`
  dont les défenses (`Defenses` : l'armure et les statistiques de combat) viennent de son constructeur ; pas
  d'accesseur.
- **Origine de l'armure (étape 83)** : celle d'un personnage est la somme de l'armure des objets de son
  `Equipement`, recalculée en entier à chaque changement (`Vitalite::definirDefenses`) ; il n'y a plus
  d'armure de classe (`docs/objets.md`). Celle d'un monstre est fixe, une valeur de sa définition (de sa catégorie
  depuis l'étape 93, `docs/monstre.md`).
- **Valeurs (étape 87.3, provisoires ; à l'échelle × 3 depuis l'étape 91.2)** : depuis l'étape 103, l'armure d'un personnage est celle de
  ses pièces calculée par `effetsDeBase` (guerrier et paladin 4 : Leather Jerkin 3 et Shield 1 ; rôdeur 3 ; prêtre et mage 2 : Robe ;
  9 pour les kits de l'étape 91, 30 avant), les monstres 3 (10 avant l'échelle × 3 ; depuis l'étape 93, 3 pour l'infanterie seulement : 0 pour la
  nuée, 10 pour l'infanterie lourde, 6 pour la créature monstrueuse). Contrôle : un coup de monstre de 30 fait 21 à un guerrier, 30 à un
  mage ; une attaque de base de 15 fait 12 à un monstre.
- **Le soin n'est pas touché** par l'armure. Les dégâts de zone, de skill et d'attaque de base passent
  tous par `recevoirDegats`, donc par l'armure.
- **Limites** : l'armure ne réduit que la portion physique d'un coup (étape 94 : types de dégâts et résistances, voir
  plus loin) ; la pénétration d'armure date de l'étape 92.

## Caractéristiques, points de vie et montant effectif (étape 88)

Détail de conception : `docs/classes-et-combat.md`, section 5.

- **Cinq caractéristiques** (`combat/Caracteristiques`) : `enum class Caracteristique { Puissance,
  Endurance, Adresse, Savoir, Volonte }` et un type valeur `Caracteristiques` (un `std::array<int, 5>`
  indexé par l'enum, `valeur(Caracteristique)`, addition, `somme()` depuis l'étape 100.1). Un tableau plutôt que cinq champs : les bonus
  d'objets, les coefficients et l'affichage parcourent les cinq de la même façon. Elles vivent dans
  `combat/` parce que les objets et les personnages s'en servent (`objets/` peut donc inclure `combat/`).
  Les noms affichés, « Points: N », le brouillon « (+N) » et l'infobulle d'effet (étape 100.2, les nombres lus dans les constantes) sont dans `interface/AffichageCaracteristiques`.
- **Valeur de référence 10** (`s_valeurDeReference`, à la *Pillars of Eternity*) : un effet se calcule par
  rapport à 10, qui ne donne ni bonus ni malus. Provisoire, à tester.
- **Effectives = base + équipement** : la base vient de la classe (`DefinitionPersonnage::caracteristiques`,
  somme 50 pour chacune des cinq classes), les bonus de l'équipement (`DefinitionObjet::bonus`) s'y
  ajoutent. Recalculées en entier à chaque changement d'équipement par
  `Personnage::recalculerDepuisLEquipement` (nommée `recalculerCaracteristiques` avant l'étape 89, quand elle
  ne recalculait que les caractéristiques et les points de vie), le seul endroit qui les écrit (même règle
  que l'armure, jamais d'ajustement incrémental). La Puissance et l'Endurance ont un effet depuis l'étape 88,
  l'Adresse depuis l'étape 89 (les capacités, voir plus bas) ; le Savoir et la Volonté existent et
  s'affichent, sans effet.
- **Endurance → points de vie** : `pointsDeVieMax(pointsDeVieDeBase, caracteristiques)` = base de la classe
  + `s_pvParPointDEndurance` (20) × Endurance effective. Les PV de base de chaque classe (720, 740, 800,
  840, 800) sont ajustés pour que le maximum de départ soit 1000 pour toutes, comme avant l'étape 88. (Depuis les étapes 91.2 et
  99.4 : base commune 240, 6 par point d'Endurance et 8 par niveau au-delà du premier, voir plus bas.)
  Quand le maximum change (équipement), `PointsDeVie::changerMaximum` change la valeur actuelle de la même
  différence ; un vivant garde au moins 1 PV (on ne meurt pas en se déséquipant) ; un mort reste à 0.
- **Puissance → dégâts et soins par des coefficients** : chaque skill et chaque attaque de base de
  personnage déclare dans ses données un `Coefficients` (un flottant par caractéristique, 0 par défaut) ;
  aujourd'hui Puissance 1,0 pour tous les skills de dégâts, de soin et de zone et pour les attaques de base,
  rien pour la provocation (sa durée n'est pas un montant). `montantEffectif(montant, coefficients,
  caracteristiques)` = `montant × (1 + Σ coefficient × s_bonusParPoint × (caractéristique − 10))`, avec
  `s_bonusParPoint = 0,03`, arrondi au plus proche, jamais négatif : c'est **la seule fonction** qui calcule ce
  montant. `Personnage::attaqueDe(skill)` (`montantDe` avant l'étape 89.3) l'appelle pour `avancerAttaque`,
  `avancerSkill` et `getAttaqueDeBase` (lu par `Partie` pour les coups d'opportunité) : tout ce qui inflige
  un dégât ou un soin de personnage passe par là, et y ajoute la capacité offensive du personnage. Un Puissance de 13 (anneau de puissance +3) donne +9 %.
- **Les monstres n'ont pas de caractéristiques** : leurs coefficients sont nuls, leurs montants ne changent
  pas.
- **Limites** : pas d'effet du Savoir et de la Volonté ; pas de points libres, de niveaux ni de
  croissance ; pas de caractéristiques de monstres ; valeurs de référence provisoires.

## Le critique, les statistiques de combat et l'attaque qui circule (étape 89)

Détail de conception : `docs/classes-et-combat.md`, sections 6, 7 et 13. Étape 3 du déroulement d'un coup
(« est-ce un critique ? »), la première après l'esquive (étape 90, section suivante).

- **Le moteur du coup ne connaît pas les caractéristiques** : il lit des **statistiques de combat**
  (`StatistiquesDeCombat`, `combat/`, type valeur). Un personnage les tire de ses caractéristiques effectives
  à **un seul endroit**, `statistiquesDuPersonnage(const Caracteristiques&, bonus d'équipement)`, appelée par
  `Personnage::recalculerDepuisLEquipement` (donc recalculées en entier à chaque changement d'équipement,
  jamais ajustées) ; un monstre, qui n'a pas de caractéristiques, les reçoit de sa définition
  (`DefinitionDeMonstre::statistiques`, `docs/monstre.md`). Un champ n'existe que quand une étape du coup
  s'en sert : à l'étape 89 deux, la capacité offensive et la capacité défensive (l'étape 90 y a ajouté les
  esquives et le blocage).
- **Capacités d'un personnage** : `50 + 5 × Adresse` effective pour les deux (`s_capaciteDeBase`,
  `s_capaciteParPointDAdresse`, « à tester »). Adresse 10 donne 100 ; le rôdeur (14) 120 ; les personnages
  de test (Adresse 0) 50. L'offensive est bornée à 0 par le bas, la défensive à 1 (le rapport la divise).
  **Monstre ordinaire** : 100 et 100. Les bonus de l'équipement passent par l'Adresse, et, depuis l'étape 90,
  directement par les bonus de statistiques de combat des objets (aucun objet ne donne de capacité aujourd'hui).
- **L'attaque qui circule (`Attaque`, `combat/Attaque.hpp`)** : `{ int montant; int capaciteOffensive;
  NatureDAttaque nature; }` (la nature date de l'étape 90), à la place du montant seul. `Personnage::avancerAttaque` et `avancerSkill`, `Monstre::avancerAttaque` la
  rendent en `std::optional` (vide tant que le coup n'est pas parti) ; `Coup` est maintenant `{ idMonstre,
  Attaque }` ; `Escouade::recevoirCoup` et `Monstre::recevoirDegats` la reçoivent ; `Escouade::attaqueDeBase`
  et `Monstre::getAttaque` la donnent pour les coups d'opportunité. Le **défenseur fournit ses statistiques**
  (ses `Defenses`), l'attaquant les siennes : le point d'entrée les rapproche. Pour un skill de soin ou de
  provocation, le « montant » est le soin ou la durée, et la capacité offensive est celle du lanceur (le
  soin s'en sert, la provocation l'ignore).
- **La formule (module `Critique`, fonctions pures)** : `rapport = capacité offensive / capacité défensive`
  (`rapportDeCapacites`, `assert(défensive > 0)`) ;
  - `chanceDeCritique(r) = min(0,05 × r², 0,50)` ;
  - `multiplicateurDeCritique(r) = clamp(1,5 + 0,25 × (r − 1), 1,5 ; 2,0)` ;
  - `appliquerCritique(montant, r, tirage)` : critique si `tirage < chance`, montant multiplié et arrondi au
    plus proche.
  Valeurs « à tester » (constantes nommées dans `Critique.hpp`). Repères : r = 0,5 (Adresse 0 contre 100) →
  1,25 % ; r = 1 (Adresse égale) → 5 %, ×1,5 ; r = 1,2 (rôdeur contre un monstre) → 7,2 %, ×1,55 ; r = 2
  (un monstre contre un personnage d'Adresse 0) → 20 %, ×1,75 ; r = 3 → 45 %, ×2 ; plafond de 50 % dès
  r ≈ 3,16. Le rapport traite donc le **niveau par les nombres** (question 2 de `docs/classes-et-combat.md`) :
  un monstre de zone plus profonde aura de plus grandes capacités, sans formule d'écart de niveau.
- **Le calcul est séparé du tirage** : les fonctions de `Critique` reçoivent le tirage (un nombre de 0
  inclus à 1 exclu). **Le seul tirage du moteur du coup** est dans `ResolutionDuCoup.cpp` (`tirer`, du
  `Generateur` de la `Partie`, passé par référence, jamais copié ni gardé). **Aucun tirage quand la chance
  est 0** (capacité offensive nulle) **ni quand le montant est nul** (un coup de 0 reste 0), comme les
  recharges fixes de l'étape 56 : ni un mort ni un monstre invulnérable en retour (qui n'appellent pas le
  moteur) ne consomment de nombre. Conséquence : les suites aléatoires d'une partie ont changé à l'étape 89
  (un nombre de plus par coup), sans conséquence de règle.
- **Place dans le coup** : après l'esquive (étape 90), avant le blocage et l'armure ; un coup esquivé n'a pas
  de critique (et ne tire donc pas son critique). Le critique multiplie le
  **montant effectif** (Puissance comprise) puis l'armure s'applique au montant multiplié : c'est ce qui en
  fait l'arme naturelle contre l'armure par soustraction (section 6 de `docs/classes-et-combat.md`).
- **Ce qui critique** : les attaques de base et les skills de dégâts des deux camps (la magie comme le reste :
  question 7 tranchée, **la magie critique**), les skills de zone (**un tirage par monstre touché**), les
  coups d'opportunité. Les effets dans le temps ne critiqueront pas (ils n'existent pas encore).
- **Soins critiques (89.4)** : `resoudreSoin(Attaque, Generateur&)` applique la même courbe, en opposant la
  capacité offensive du soigneur à une **capacité défensive de référence de 100**
  (`s_capaciteDefensiveDeReference`, qui suivra le niveau de la zone quand les niveaux existeront). Appelée
  par `Escouade::effetDuSkill` (le seul endroit où un soin est appliqué), avant `Personnage::recevoirSoin` :
  le soin critique reste plafonné aux points de vie maximum, et le fait `SoinApplique` porte `critique`.
  Même règle de tirage (aucun si la capacité offensive est nulle).
- **Événements et affichage (89.3 à 89.5)** : `Degats { montant, opportunite, critique }` et `Soin { montant,
  critique }` ; `Partie` pose `critique` d'après le `ResultatDuCoup`. `TextesFlottants` : un critique est en
  jaune vif, 18 px, suivi de « Critical! » (« 12 Critical! », « 12! Critical! » pour un coup gratuit,
  « +30 Critical! » en vert pour un soin) ; le style critique l'emporte sur celui du coup gratuit (couleur
  et taille), qui garde son « ! ». L'inventaire montre « Offensive Ability N » et « Defensive Ability N »
  de chaque personnage (les textes du jeu sont en anglais depuis l'étape 95 : `docs/glossaire.md`).
- **Pourquoi les tests de `Partie` lisent `critique`** : les personnages de test ont une Adresse de 0 (capacité
  défensive 50 contre 100 pour l'attaque d'un monstre : 20 % de critiques sur eux), et `Partie` garde son
  générateur. Plutôt que de rendre les parties de test sans hasard, un test qui vérifie un montant lit
  `critique` dans l'événement et compare au montant que le jeu devait produire (`degatsAttendus`, aide de
  `aides.hpp`). Les tests de règle appellent les entités avec `attaqueSansHasard` (capacité offensive 0 :
  ni critique ni tirage) par les aides `subir` et `subirCoup` (`docs/tests.md`).
- **Limites** : pas de pénétration ; pas de bonus de critique de l'équipement ni de
  skills qui en donnent ; pas de multiplicateur propre à une arme ; pas de distribution pseudo-aléatoire contre
  les séries (ni de critiques consécutifs) ; la capacité défensive de référence des soins est une constante.

## L'esquive et le blocage (étape 90)

Détail de conception : `docs/classes-et-combat.md`, sections 6, 7 et 13. Étapes 2 (« le coup est-il esquivé ? »)
et 5 (« blocage au bouclier ») du déroulement d'un coup. Depuis l'étape 94, le bouclier ne réduit que la portion
physique d'un coup (question 12 tranchée, voir « Types de dégâts et résistances »).

- **Nature explicite de chaque attaque** : `NatureDAttaque { Contact, Projectile, Zone }`
  (`combat/NatureDAttaque.hpp`), une **donnée de l'attaque**, jamais déduite : `Attaque::nature` (sans valeur par
  défaut, un oubli est signalé par le compilateur), `DefinitionDeSkill::nature` (`Contact` par défaut :
  un soin et une provocation ne sont pas des attaques, leur nature n'est jamais lue), `Skill::getNature()`.
  Aujourd'hui : attaques de base des mêlées et des monstres, frappe puissante, frappe sacrée, coups
  d'opportunité = contact ; attaques de base des tireurs et tir perçant = projectile ; explosion et pluie de
  flèches = zone. À l'étape 91, l'arme fournira la nature de l'attaque de base. **Invariant** (`assert` dans
  `Skill`) : un skill de type `Zone` est de nature `Zone`, et lui seul — on ne peut pas oublier de rendre une
  zone inesquivable.
- **Statistiques ajoutées à `StatistiquesDeCombat`** : `esquiveContact` et `esquiveProjectile` (pour cent),
  `chanceDeBlocage` (pour cent), `montantBloque` (points) et `recuperationDuBlocage` (secondes). Le même type
  sert de **bonus d'objet** (`DefinitionObjet::bonusDeCombat`, vide par défaut) : `statistiquesDuPersonnage`
  additionne à la base d'Adresse la somme des bonus de l'équipement (`Equipement::bonusDeCombatTotal`).
  L'addition (`operator+=`) somme tous les champs **sauf la récupération**, qui est celle du bouclier : on garde
  la plus longue. `Defenses` porte maintenant l'armure et les statistiques de combat du défenseur (au lieu de
  l'armure et de la seule capacité défensive), pour ne pas recopier chaque champ.
- **Esquive (module `Esquive`)** : `chanceDEsquive(nature, defenseur)` = l'esquive contre le contact ou contre les
  projectiles du défenseur selon la nature, **plafonnée à 50 %** (`s_esquiveMaximale`, question 3 tranchée pour
  l'esquive) et jamais négative (`esquiveEffective`, aussi lue par l'affichage : un bonus au-delà du plafond ne
  s'affiche pas) ; **0 pour une zone**, qui ne s'esquive jamais ; `coupEsquive(chance, tirage)` : `tirage <
  chance`. Première étape du coup : un coup esquivé s'arrête (montant 0, pas de critique, pas de blocage, pas
  d'armure) et le résultat porte `esquive = true`.
- **Valeurs (« à tester »)** : monstres 5 % contre le contact et 5 % contre les projectiles
  (`DefinitionDeMonstre::statistiques`) ; personnages 0, plus l'équipement : bottes de cuir +5 % contre le
  contact, casque de cuir +5 % contre les projectiles (`CatalogueObjets.cpp`, `bonusDeCombat`).
- **Blocage (module `Blocage`)** : `chanceDeBlocage(nature, defenseur, blocagePret)` est nulle contre une zone,
  pendant la récupération (`blocagePret` faux) et quand le bouclier n'absorbe rien (`montantBloque <= 0`), sinon
  la chance du défenseur (ramenée de 0 à 100 %) ; `appliquerBlocage(montant, montantBloque, chance, tirage)` :
  si `tirage < chance`, retire `min(montant, montantBloque)` et rend le montant restant et ce qui a été
  absorbé. Il bloque le contact et les projectiles, jamais les zones. **Après le critique, avant l'armure** :
  le bouclier retire un montant fixe du coup (déjà multiplié s'il est critique), puis l'armure M1 joue sur le
  reste. **Blocage total** : si tout est absorbé, le coup s'arrête, **sans armure ni plancher** (le montant est
  0, pas 1) ; `ResultatDuCoup::bloque` dit ce qui a été absorbé (> 0 : il y a eu blocage ; total si `montant ==
  0`).
- **Valeurs du Shield (« à tester », `CatalogueObjets.cpp`)** : depuis l'étape 103, 20 % de chance au niveau 1 (+1 point par niveau) et 8
  points absorbés au niveau 1 (× le facteur de niveau et de qualité), au lieu de 30 % et 12 pour le bouclier rond de l'étape 91 (40
  avant l'échelle × 3), 3 s de récupération (une constante commune depuis l'étape 91.6, plus la donnée du
  bouclier). Depuis l'étape 91.6, chance et montant sont les `DonneesDeBouclier` du bouclier, que `Equipement`
  ajoute aux statistiques de combat. Les monstres n'ont pas de bouclier (`montantBloque` 0), mais une définition de
  monstre peut en avoir un.
- **La récupération est un état de la `Vitalite`** (`m_recuperationDuBlocage`, en secondes, `mettreAJour(dt)`,
  `blocagePret()`) : un blocage réussi (`bloque > 0`) la lance à `s_recuperationDuBlocage` (à `statistiques.recuperationDuBlocage`
  avant l'étape 91.6) ; tant qu'elle court,
  `resoudreCoup` reçoit `blocagePret = false` et ne tente (ni ne tire) aucun blocage. **Pourquoi là** : la
  `Vitalite` est déjà ce qui résout les coups pour les deux camps et ce qui porte les défenses ; l'état de
  combat de l'entité y vit, écrit une fois, et `Personnage::mettreAJour` et `Monstre::mettreAJour` la font
  couler comme ils font couler les recharges de leurs skills. Mettre cet état dans `Personnage` et `Monstre` l'aurait
  dupliqué. Un mort ne récupère pas (`Personnage::mettreAJour` ne fait rien), sans conséquence : il ne bloque plus.
- **Un coup esquivé ou entièrement bloqué compte comme une attaque reçue** : c'est l'intention d'attaquer qui
  compte. `Monstre::recevoirDegats` s'alerte et `Escouade::recevoirCoup` enregistre l'agresseur selon
  `attaque.montant > 0`, jamais selon ce que le coup a donné. Un monstre dont le coup sur un personnage est
  esquivé ou bloqué devient donc agresseur de l'escouade ; le monstre visé s'alerte. Seuls un mort et un
  monstre qui rentre à son poste (invulnérable) ne réagissent à rien (pas d'événement non plus).
- **Les tirages** : jusqu'à trois par coup, dans l'ordre esquive, critique, blocage, **chacun seulement si sa
  chance est non nulle** (esquive nulle ou zone ; capacité offensive nulle ; pas de bouclier, bouclier en
  récupération ou zone). Un coup esquivé n'en consomme qu'un. Les suites aléatoires d'une partie ont donc
  encore changé à l'étape 90.
- **Événements** : `Esquive {}` et `Bloque { montant, total }` s'ajoutent à `DetailEvenement`. `total` (ajouté au
  seul `montant` annoncé) dit si le blocage a tout absorbé : sans lui, l'affichage ne pourrait pas distinguer
  « Blocked » de « Blocked −N » (un `Degats` de 0 n'existe pas). La conversion d'un `ResultatDuCoup` en détails
  est une fonction pure, `detailsDuCoup(resultat, opportunite)` (`EvenementDeCombat.cpp`), que `Partie::signalerLeCoup`
  parcourt : un coup esquivé donne `Esquive` seul ; sinon les `Degats` (rien s'ils sont nuls), **puis**
  `Bloque` (le texte du blocage s'empile ainsi au-dessus des dégâts, même cible, même image). Un résultat
  « rien » ne donne rien. La règle « un montant de 0 ne produit aucun événement » reste pour `Degats` et `Soin`.
- **Affichage** (`TextesFlottants`) : « Dodged » en gris clair, 14 px ; blocage total « Blocked » en gris,
  14 px ; blocage partiel : les dégâts restants comme d'habitude, et « Blocked −12 » (signe moins U+2212 ; −40 avant l'échelle × 3) en gris,
  14 px, au-dessus. **Infobulle des objets** : une ligne par bonus de combat (`lignesDeBonusDeCombat`) :
  « Dodge vs Melee: +5 % », « Dodge vs Projectiles: +5 % » ; le blocage d'un bouclier est
  une ligne à part depuis l'étape 91.6 (« Block: 30 %, 12 points », `lignesDeLInfobulle`).
  **Inventaire** : sous les capacités, « Block 30 % (12) » (« Block 0 % » sans bouclier), « Melee Dodge
  5 % » et « Projectile Dodge 5 % » (valeurs effectives, plafond compris) : `lignesDeStatistiques`.
- **Limites** : pas de skills qui donnent de l'esquive ou du blocage ; pas de bouclier qui déséquilibre ni
  d'armes contre les boucliers ; pas de distribution pseudo-aléatoire contre les séries d'esquives ; le
  blocage ne distingue pas les éléments (question 12) ; les monstres n'ont pas de bouclier dans le catalogue ;
  un blocage partiel d'un coup de zone est impossible (elles ne se bloquent pas) ; les durées de récupération
  ne s'affichent que dans l'infobulle du bouclier, pas à l'écran de jeu.

## Échelle × 3, armes et skills qui exigent une arme (étape 91)

Décisions de conception : `docs/classes-et-combat.md` (sections 5, 12 et 13). Les objets, les armes et les deux
mains sont dans `docs/objets.md`.

- **Échelle × 3 (étape 91.2)** : 1 000 points de vie au départ, c'était trop haut pour un jeu dont les nombres
  grandiront avec les niveaux (*Titan Quest* commence à 300). Toutes les valeurs en points de vie passent à 30 % de
  leur valeur d'avant (arrondies au plus proche) : points de vie, dégâts, soins, armures, montant bloqué. Les
  rapports et pourcentages ne changent pas : +3 % par point de Puissance, capacités `50 + 5 × Adresse` (monstres
  100), esquives, chance de blocage, plancher de 20 %, plafonds. Repères : attaque de base 15, coup de monstre 30,
  frappe puissante 75, soin 90, explosion 45, monstre 300 points de vie (régénération du retour : une fraction du
  maximum, donc inchangée).
- **Points de vie des personnages** : base **commune** (`s_pointsDeVieDeBase`, 240) plus `s_pvParPointDEndurance`
  (6) par point d'Endurance, plus `s_pvParNiveau` (8) par niveau au-delà du premier (étape 99.4),
  `pointsDeVieMax(const Caracteristiques&, int niveau)`. Plus de base par classe
  (`DefinitionPersonnage::pointsDeVieDeBase` supprimé) : l'Endurance fait seule la différence (guerrier 324,
  paladin 318, prêtre et rôdeur 300, mage 288 ; le collier d'endurance, +3, donne +18).
- **L'attaque de base vient de l'arme (étape 91.4)** : voir `docs/objets.md`. Le `Personnage` en fait un `Skill`
  de type `Degats` (coefficient de Puissance 1,0) ; sa nature décide de la zone de contrôle (`zoneDeControleDe`) ;
  une arme qui change relance une recharge complète. Les skills de personnage gardent leurs propres valeurs.
- **Deux familles de skills de dégâts** (à noter dans `docs/classes-et-combat.md`, section 12 ; **seulement la
  documentation à cette étape**, aucun skill ne change) : les **skills de frappe** utiliseront l'arme (ses dégâts
  plus un bonus, sa nature et sa pénétration ; candidats : frappe puissante, frappe sacrée, tir perçant) et les
  **skills à dégâts propres** (explosion, pluie de flèches). La `DescriptionDAttaque` construite à partir de l'arme
  peut servir de point de départ à un skill de frappe.
- **Un skill peut exiger un type d'arme (étape 91.5)** : `DefinitionDeSkill::armesExigees`, une liste de
  `TypeDArme` (vide : aucune exigence, le défaut), lue par `Skill::convientA(std::optional<TypeDArme>)`. Sans l'une
  de ces armes en main droite (les mains nues n'en sont pas une), le skill est **refusé comme celui d'un lanceur
  mort** : `Personnage::skillEstPret` et `peutEngagerSkill` sont faux, donc `Escouade::declencherSkill` et
  `declencherSkillDeZone` renvoient sans activation, sans mise en attente et sans recharge consommée. Perdre l'arme
  (inventaire) annule l'activation en cours et abandonne le skill en attente de l'escouade ; changer pour une autre
  arme qui convient ne touche à rien. `EtatSkill::armeAbsente` le dit, et `BarreDeSkills` grise l'icône (texte
  atténué, ni quartier ni chiffre, comme un lanceur mort). Exigences du catalogue des classes (depuis l'étape 102) : frappe puissante
  et frappe sacrée **toute arme de contact** (dague, épée, hache ou masse à une ou deux mains, lance : la liste `s_armesDeContact`) ;
  tir perçant et pluie de flèches arc ou arbalète ; les autres aucune. Pas de restriction par classe : un guerrier qui reçoit un arc
  tire, mais n'a plus sa frappe.
- **Le blocage est une propriété du bouclier (étape 91.6)** : `DonneesDeBouclier` (chance, montant) ; la
  **récupération est une constante commune** (`s_recuperationDuBlocage`, `Blocage.hpp`, 3 s), lue par
  `Vitalite::recevoirDegats`. `StatistiquesDeCombat::recuperationDuBlocage` a disparu avec la règle « la plus
  longue » ; un monstre au bouclier récupère comme un personnage.
- **Limites** : le blocage et les armes ignorent les éléments jusqu'à l'étape 94 (voir « Types de dégâts et
  résistances »).

## Pénétration d'armure (étape 92.4)

Détail de conception : `docs/classes-et-combat.md`, section 8.

- **Une propriété de l'arme** : `DescriptionDAttaque::penetration`, en pour cent entier ; depuis l'étape 102 la dague 15, l'arc 15, la
  lance 30, l'arbalète 30, les autres armes et les mains nues 0. Elle circule dans l'`Attaque` (`penetration`, 0 par défaut) jusqu'à `resoudreCoup`, qui la
  passe à `appliquerArmure(montant, armure, penetration)`. Le `Personnage` la met dans l'attaque de base
  (`attaqueDeLArme`, depuis l'étape 95 ; avant, `attaqueDe(skill, penetration)`), donc aussi dans les coups
  d'opportunité (`Escouade::attaqueDeBase`) ; un skill de dégâts n'en a pas (il a sa propre puissance ; la frappe sacrée
  prend celle de l'arme), un monstre non plus.
- **Où elle agit** : sur l'armure, **avant** la soustraction M1 : `armure effective = armure × (1 − pénétration)`, sans
  arrondi intermédiaire (l'arrondi reste celui de M1, en fin de calcul), puis `max(montant − armure effective, montant ×
  20 %)`. Le plancher de 20 % ne bouge pas. Un blocage total s'arrête avant l'armure : la pénétration n'y change rien.
- **Plafond** : 75 % (`s_penetrationMaximale`, `Defense.hpp`, « à tester ») ; une pénétration plus grande ou négative est
  ramenée entre 0 et 75 à l'application.
- **Exemple** : 100 contre 40 d'armure, sans pénétration 60 ; à 30 %, l'armure vaut 28 et le coup 72.
- **Limites** : pas de bonus de pénétration par les skills (par l'équipement depuis l'étape 95 : le préfixe « Keen »,
  voir « Les affixes dans le combat ») ; pas de pénétration de résistance (hors de portée de l'étape 94). Depuis l'étape 94, le moteur du coup utilise `armureAppliquee`, la version
  flottante de M1, sur la portion physique.

## Types de dégâts et résistances (étape 94)

Détail de conception : `docs/classes-et-combat.md`, sections 6, 7, 8 et 12.

- **Un coup est un ensemble de portions.** `Element` (`Physique`, `Feu`, `Sacre`, plus la sentinelle `Nombre`) et
  `ParElement<T>` (`combat/Element.hpp`) : une valeur par élément, à zéro par défaut, indexée par `Element` (le
  `static_cast` est caché dans `operator[]`), `seul(élément, valeur)`, `total()`, `elementDominant()` ; la taille vient de
  la sentinelle : ajouter un élément avant elle agrandit tous les tableaux, sans second endroit à tenir à jour (aucun
  `switch` ne porte sur un `Element` : ce qui dépend de l'élément se lit dans un `ParElement`, parcouru avec
  `s_elements`). `Degats = ParElement<int>` (`combat/Degats.hpp`, avec `degatsDe`, `degatsPhysiques`, `enPortions`) ; les
  résistances sont aussi un `ParElement<int>`, en pour cent (`Resistances`). L'événement de combat qui dit ce qu'une
  cible a perdu s'appelle `DegatsSubis` (le nom `Degats` est celui du type).
- **`Attaque::degats` est un `Degats`.** Le soin et la provocation ne sont pas typés : ils gardent leur valeur
  (`Attaque::soin`, `dureeDeProvocation`, séparés des dégâts depuis l'étape 94.1 : un skill ne produit qu'une des trois
  choses, selon son type, et `Personnage::attaqueDe` range son montant dans le bon champ). Toutes les sources
  existantes produisent du physique pur, sauf l'explosion (feu) et la frappe sacrée (physique de l'arme + sacré).
- **Les règles du coup**, dans l'ordre de `resoudreCoup`, chaque étape étant une fonction pure :
  1. esquive, inchangée (elle dépend de la nature de l'attaque, pas de l'élément) ;
  2. critique (`coupCritique`) : le multiplicateur s'applique à **toutes** les portions ; la Puissance aussi, bonus
     élémentaires compris (`montantEffectif`, portion par portion) ;
  3. blocage (`appliquerBlocage`, sur la seule portion physique) : tenté seulement si l'attaque est de contact ou un
     projectile **et** a une portion physique non nulle (sinon ni tirage ni récupération) ; une portion physique
     entièrement absorbée ne subit ni l'armure ni son plancher, les autres portions continuent ; si tout le coup est
     absorbé, il s'arrête là ;
  4. armure (`armureAppliquee`, avec la pénétration de l'attaque) sur la portion physique, plancher de 20 % ;
  5. résistances (`appliquerResistance`) : chaque portion, physique comprise, est multipliée par
     `1 − résistance effective` ; `resistanceEffective` ramène la valeur dans
     [`s_resistanceMinimale` (−100), `s_resistanceMaximale` (80)] (`Defense.hpp`, « à tester ») ;
  6. arrondi : **un seul par portion**, au plus proche, en fin de calcul (aucune étape avant n'arrondit : les étapes
     travaillent sur des `Portions` flottantes) ; le total est la somme ; un coup ni esquivé ni entièrement bloqué
     fait au moins 1, dans la plus grosse portion.
  La pénétration d'armure ne touche que l'armure (pas de pénétration de résistance). `ResultatDuCoup::portions` dit ce
  que chaque élément a donné après les réductions (la somme est `montant`).
- **Le tirage est inchangé** : un nombre par étape dont la chance est non nulle, dans `resoudreCoup`. Le blocage n'en
  tire aucun s'il n'y a pas de portion physique.
- **Où vivent les résistances** : dans `StatistiquesDeCombat::resistances`, avec les esquives et le blocage, et pas à
  côté de l'armure dans `Defenses`. Raison : les bonus d'équipement s'additionnent déjà dans ce type (`bonusDeCombatTotal`,
  `operator+=`), donc l'étape 95 n'aura qu'à remplir le champ des objets ; l'armure reste à part parce qu'elle a son
  propre modèle (M1, pénétration) et sa propre source (le champ `armure` d'un objet). Les personnages : toutes à 0.
  Les monstres : dans leur définition (`DefinitionDeMonstre::statistiques`, voir `docs/monstre.md`) — toutes à 0 sauf la
  créature monstrueuse, 40 % de feu.
- **Sources élémentaires** : `DefinitionDeSkill::element` (physique par défaut) donne l'élément de la puissance d'un
  skill de dégâts ou de zone ; l'explosion du mage est du feu. La frappe sacrée est le premier **skill de frappe** :
  `DefinitionDeSkill::bonusDeFrappe` (un `Degats` typé, ici 45 de sacré) ; l'arme apporte les dégâts physiques, la nature
  et la pénétration (`Personnage::attaqueDeFrappe`, depuis `DescriptionDAttaque` de l'arme portée, **toutes ses portions**
  depuis l'étape 95 : un affixe de feu ou de sacré compte), le skill sa portée,
  sa recharge, son temps d'activation et son bonus. Sa puissance est 0 et sa nature déclarée n'est pas lue. Calibrage (étape 94) :
  masse de 15 + 45 = 60, le total d'avant ; depuis l'étape 102 la masse à une main fait 26 en moyenne (71 en tout), à retoucher en
  jouant. Les portions de l'arme sont **tirées dans sa plage** au départ de la frappe, comme pour l'attaque de base. Le bonus étant un `Degats`, la frappe
  puissante et le tir perçant pourront devenir des skills de frappe avec un bonus physique, sans changer la structure.
- **Affichage** : toujours un seul nombre flottant, le total. `DegatsSubis::element` est celui de la plus grosse portion
  après les réductions (à égalité, le premier de l'énumération : `elementDominant`) ; sa couleur : physique inchangé
  (blanc sur un monstre, rouge sur un personnage), feu orange rougeâtre (255, 95, 10), sacré or pâle (225, 185, 75).
  Le critique (jaune, « Critical! ») et le coup gratuit (orange) gardent la priorité. Un test vérifie que ces couleurs
  restent distinctes (≥ 50 de distance RGB).
- **Exemple** : une explosion de 45 de feu contre l'infanterie lourde (armure 10) fait 45 (l'armure n'y touche pas),
  contre la créature monstrueuse 27 ; une attaque de base de 45 physiques contre l'infanterie lourde fait 35.
- **Limites** : trois éléments seulement (froid, foudre, poison, nécrotique, saignement avec leur première source) ;
  pas de pénétration de résistance, pas de résistance ni de bonus élémentaire sur les objets (étape 95), pas de
  bouclier anti-magie, pas d'effets dans le temps, pas de nombre flottant par portion. Le Savoir et la Volonté n'ont
  toujours aucun effet.

## Les affixes dans le combat (étape 95)

Le modèle des affixes est dans `docs/objets.md` (« Rareté et affixes ») ; ici, ce qui change dans le circuit des coups.

- **L'attaque de base est typée.** `DescriptionDAttaque::degats` est une `PlageDeDegats` (un `Degats` minimum et un `Degats` maximum
  depuis l'étape 102 ; un seul `Degats` depuis l'étape 95) : les armes du catalogue et les mains nues sont physiques
  (`degatsPhysiques`). `Equipement::attaqueDeBase` y ajoute aux deux bornes les portions de feu et de sacré des affixes de l'arme
  (« Blazing » : feu, « Hallowed » : sacré) et leur pénétration (« Keen »). Une masse « Blazing » frappe donc en physique **et** en feu.
  **Chaque coup tire ses dégâts dans la plage** (`tirerLesDegats`, voir plus bas) avant la Puissance et la défense.
- **La Puissance joue sur chaque portion.** `Personnage::attaqueDeLArme` (qui remplace l'usage de `attaqueDe(skill,
  penetration)` pour l'attaque de base ; le `Skill` de l'attaque de base n'a plus que le temps, sa puissance est 0) applique
  `montantEffectif` à chaque portion avec le coefficient de Puissance 1,0 : **un arrondi par portion**. Les coups d'opportunité
  (`Escouade::attaqueDeBase`) lisent la même attaque et en profitent.
- **La frappe sacrée prend toutes les portions de l'arme**, affixes compris, plus son bonus : avec une masse « Blazing » elle
  a trois portions (physique de la masse, feu de l'affixe, sacré du bonus). Pour chaque élément, la portion de l'arme et celle
  du bonus sont arrondies chacune à part, puis additionnées (avec une masse « Hallowed », le sacré est l'arrondi des 4 de
  l'affixe plus celui des 45 du bonus). Aucun changement pour une arme sans affixe.
- **Contre l'armure et les résistances.** Contre l'infanterie ordinaire, le physique domine et le nombre garde sa couleur
  physique ; contre l'infanterie lourde, l'armure écrase le physique et le feu passe : le nombre devient orangé (l'élément
  de la plus grosse portion, étape 94). Aucun changement du moteur du coup (`resoudreCoup`).
- **Pénétration.** Celle d'un affixe s'ajoute à celle de l'arme (dague 30 + « Keen » 15 = 45). La somme n'est pas
  plafonnée dans l'attaque : le plafond de 75 % (`s_penetrationMaximale`) joue à l'application, dans `appliquerArmure` /
  `armureAppliquee`, comme avant.
- **Blocage.** La chance de blocage d'un affixe (« of Blocking ») s'ajoute à celle du bouclier dans
  `Equipement::bonusDeCombatTotal` ; le montant absorbé reste celui du bouclier (sans bouclier, l'affixe n'est même pas
  permis). L'inventaire affiche la somme (« Block 35 % (12) » avec un Shield « of Blocking » +5).
- **Esquive et armure.** « of Evasion » et « of Deflection » ajoutent à l'esquive contre le contact ou les projectiles
  (plafond de 50 % inchangé), « Reinforced » à l'armure : tous passent par les totaux de l'équipement actif.
- **Un objet inactif perd ses affixes** avec le reste (l'objet entier sort de l'équipement actif).
- **Limites** : pas d'affixe de résistance (les monstres ne frappent qu'en physique), pas de pénétration de résistance ; un
  affixe de feu n'ajoute rien aux skills de dégâts propres (explosion, pluie de flèches) ni aux soins.

## Temps d'exécution et coup engagé (étape 57)

- **Cycle** : une attaque de base n'est plus instantanée. Prête → **engagée**
  (l'attaquant doit être vivant, immobile, avec une cible à portée et en vue :
  règles d'atteinte inchangées) → **en exécution** (0,15 s pour un personnage,
  0,3 s pour un monstre) → le **coup part** (la recharge est tirée à ce
  moment, comme à l'étape 56) → **en recharge** → prête. Le joueur voit chaque
  coup se préparer et on ne frappe plus en marchant.
- **Machine à états dans `Skill`** (`PhaseSkill`, voir « Référence des modules ») :
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
  (règle 0 de `choisirIntention`, de `Escouade::pointVoulu` avant l'étape 82.4) ; un monstre ne
  poursuit ni ne rentre.
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
  durée (`Personnage::estEnExecution`, règle 0 de `choisirIntention`) ; le reste de
  l'escouade continue, puis le lanceur rattrape sa place.
- **Au début de l'activation**, le coup de base engagé du lanceur est annulé
  (`engagerSkill`). Aucune attaque de base ne s'engage pendant l'activation
  (`engagerAttaque` refuse). Au départ, la recharge de l'attaque de base est
  relancée.
- **Pendant l'activation le skill n'est pas prêt** (`skillEstPret` repose sur
  la phase) : un nouvel appui sur le même emplacement est refusé et un skill
  de zone n'entre pas en visée. Les skills d'autres personnages restent permis.
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
  appelle `escouade.recevoirCoup(id, place, attaque, générateur)` sur la place
  retenue à l'engagement ; sinon, un monstre immobile avec une cible de
  frappe à portée et en vue engage un coup sur la place choisie par
  `Monstre` (`choisirCibleAFrapper`).
  L'armure du personnage touché réduit ensuite le coup (voir plus haut).
- **Ce qui est réellement appliqué** (étape 79) : `Monstre::recevoirDegats`, `Personnage::recevoirDegats` et
  `Escouade::recevoirCoup` renvoient le résultat du coup (`ResultatDuCoup` : le montant appliqué, après critique,
  blocage et armure, si ce fut un critique, depuis l'étape 89, et s'il a été esquivé ou bloqué, depuis l'étape 90) ; `recevoirSoin` renvoie les points rendus (voir
  « Événements de combat »).
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
  teste avant de déréférencer. `main.cpp` avait `trouverMonstreParId` pour ses
  `Monstre` (aujourd'hui `Partie::trouverMonstre`).
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

## Skills désignés par identifiant (étape 74)

- **`IdSkill`** (`IdSkill.hpp`, sans `.cpp`) : `enum class IdSkill { FrappePuissante,
  Provocation, Soin, Explosion }`. Identifiant **stable** d'un skill : la future barre de
  raccourcis le stockera, il ne doit donc dépendre ni de l'ordre des skills dans la liste
  d'un personnage ni de la touche qui les lance.
- **`Skill` ne porte pas d'identifiant** : il reste générique, il sert aussi d'attaque de
  base et d'attaque des monstres. `SkillConnu { IdSkill id; Skill skill; }`
  (`SkillConnu.hpp`) associe les deux pour un personnage. `Personnage` stocke un
  `std::vector<SkillConnu>` (le nombre de skills grandira avec les niveaux), sans doublon
  d'identifiant (`assert` à la construction).
- **`AdresseSkill { std::size_t place; IdSkill id; }`** (`AdresseSkill.hpp`, comparable) :
  où se trouve un skill. Elle remplace la place partout où la place désignait un skill :
  `Escouade` (`declencherSkill`, `declencherSkillDeZone`, `etatDuSkill`, `skillEstPret`,
  `rayonZoneDuSkill`, `cercleDePorteeDuSkill`, `typeDuSkill`, `choisirCible`),
  `SkillEnAttente`, `Partie` (`demanderSkill`, `lancerSkillDeZone`) et `Controleur`
  (visée). Une adresse invalide (place invalide ou skill inconnu de ce personnage) est
  traitée comme une place invalide : refus, liste vide, `optional` vide, rayon 0.
- **API de `Personnage` par `IdSkill`** : `typeSkill(id)`, `skillEstPret(id)`,
  `etatDuSkill(id)`, `skillEstAPortee(id, cible)`, `zoneDuSkill(id)`,
  `cercleDePorteeDuSkill(id)`, `engagerSkill(id, cible)`. Un identifiant inconnu se
  comporte comme « pas de skill ». `avancerSkill` et `annulerSkill` n'ont pas besoin
  d'identifiant : `ActionEngagee` porte maintenant `std::optional<IdSkill> skill` (vide pour
  une attaque de base).
- **Une seule action engagée à la fois par personnage** : engager un skill pendant
  l'activation d'un autre de ses skills est refusé (sans toucher à la recharge du second).
  Depuis l'étape 80, `Escouade` met alors l'ordre en attente au lieu de le perdre (voir
  « Skill demandé pendant l'activation d'un autre »).
  Les recharges des skills sont **indépendantes** et s'écoulent toutes dans
  `Personnage::mettreAJour`. Règle inchangée : un skill qui part relance la recharge de
  l'attaque de base. Un seul skill en attente pour toute l'escouade, comme avant.
- **`Escouade::peutAtteindre`, `porteeDe` et `pointVersMonstre`** prennent un
  `std::optional<IdSkill>` à la place de `TypeAction` : vide = attaque de base, sinon le
  skill. `TypeAction` ne sert plus qu'à `ActionEngagee`. Depuis l'étape 82.3,
  `pointVersMonstre` n'existe plus : c'est `Escouade::approcheDe` qui reçoit l'`IdSkill` et
  passe la portée à `PlacesDAttaque` (`docs/formation.md`).
- **`effetDuSkill` reçoit l'`IdSkill`** lu dans l'action engagée : la zone de provocation
  et le rayon de la zone d'explosion sont ceux du skill qui part, plus ceux « du skill du
  personnage ».
- **Choix de test** : le triangle par défaut met tous les alliés à portée du soin ;
  `tests/aides.hpp` a `uneEscouadeAuSoinCourt` (prêtre au soin de 33 px bord à bord) pour
  tester un soin hors de portée. Voir `docs/tests.md`.
- **Hors portée** : pas de classe de personnages, pas d'`IdSkill` dans `Skill`, aucun
  nouveau skill, aucune valeur d'équilibrage modifiée.

## Paladin, rôdeur et étiquettes des icônes (étape 76)

- **Quatre `IdSkill` de plus** : `FrappeSacree`, `ImpositionDesMains`, `TirPercant`,
  `PluieDeFleches`. Aucun `TypeSkill` ni mécanique nouveau : chacun est un `DegatsSurCible`, un `Soin` ou
  un `Zone` existant, donc passe par exactement les mêmes chemins (engagement, activation, skill en
  attente, ligne de vue, visée pour la zone). La visée dépend du `TypeSkill::Zone`, pas de
  l'identifiant : la pluie de flèches entre en visée comme l'explosion.
- **Valeurs (provisoires, à ajuster après test)** :

  | Skill | Type | Portée | Puissance | Zone | Recharge | Activation |
  |---|---|---|---|---|---|---|
  | `FrappeSacree` (paladin) | Degats | 20 px | 20 | — | 5 s | 0,3 s |
  | `ImpositionDesMains` (paladin) | Soin | 20 px | 25 | — | 10 s | 0,5 s |
  | `TirPercant` (rôdeur) | Degats | 200 px | 25 | — | 6 s | 0,5 s |
  | `PluieDeFleches` (rôdeur) | Zone | 320 px | 12 | rayon 45 px | 8 s | 0,8 s |

  Le paladin a l'attaque de base de mêlée et l'équipement de départ (armure 30) du guerrier, le rôdeur l'attaque de base
  de tireur et l'absence d'équipement (armure 0) du mage. Les portées sont mesurées de bord à bord, comme les autres.
- **Étiquette par `IdSkill`, plus par `TypeSkill`** : `sigleDuSkill(IdSkill)` (deux lettres : FP,
  PR, SO, EX, FS, IM, TP, PF) remplace `lettreDuSkill(TypeSkill)`, qui donnait la même lettre à tous
  les skills de dégâts (le paladin et le rôdeur en ont deux de plus). Le `switch` n'a **pas** de
  `default` : le compilateur signale un `IdSkill` ajouté sans sigle. `EtatSkill` porte maintenant
  l'`IdSkill` (champ `id`) ; son champ `type` reste (les tests le lisent).
- **Place du lanceur dans la bande** : la bande de couleur au bas de l'icône (14 px, contre 4
  avant) contient le numéro de place du lanceur (1 à 3) ; deux guerriers de nuances proches restent
  ainsi distinguables. `IconeDeRaccourci` reçoit `place`, remplie par `EcranJeu` depuis l'adresse de
  l'emplacement.

## Zones de contrôle et attaque d'opportunité (étape 77)

- **Le problème** : rien ne coûtait de quitter un corps à corps. Un personnage au contact d'un monstre
  pouvait partir sans risque, un monstre pouvait changer de proie sans être inquiété. La règle donne
  un prix à la fuite, dans les deux sens, sans nouveau skill.
- **Zone de contrôle** : un rayon, mesuré **bord à bord** comme la portée (`distanceBordABord`).
  A **engage** B si la zone de A est > 0 et que l'écart de A à B est `<=` cette zone. La relation n'est
  pas symétrique : un tireur (zone 0) peut être engagé, il n'engage personne. La zone est une valeur
  explicite (`DefinitionPersonnage::zoneDeControle`, constante du `Monstre`), pas déduite du rang ni de la
  portée : 20 px pour le guerrier, le paladin et tout monstre, 0 pour le prêtre, le mage et le rôdeur.
- **Attaque d'opportunité** : une paire (A engage B) présente au début de l'image et absente à la fin
  s'est **rompue**. Si c'est B qui a fui, A le frappe gratuitement, sauf si A engage encore un autre
  adversaire à la fin. Les cas particuliers :
  - **Qui a fui** : `u` est la direction du centre de A vers celui de B au début. La contribution de B
    est son déplacement projeté sur `u`, celle de A son déplacement projeté sur `-u`. B a fui si sa
    contribution est `>=` celle de A. Un monstre qui s'éloigne pour poursuivre quelqu'un d'autre est donc
    le fuyard ; un attaquant qui recule lui-même ne frappe pas. Comme `u` n'a besoin que de sa direction,
    on ne le normalise pas ; si les deux centres sont confondus, personne n'a fui.
  - **Exemption à la fin** : l'exemption se vérifie sur la photo de **fin** : deux fuyards simultanés ne
    s'exemptent pas l'un l'autre (aucun des deux n'est engagé à la fin).
  - **Plafond** : au plus un coup gratuit par attaquant et par image, sur le fuyard le plus proche à la
    fin (égalité : le premier de la liste). C'est le seul « délai de réaction » : un attaquant qui voit
    deux fuyards ne frappe qu'une fois.
  - **Entités absentes** : pas de coup si A ou B est absent de la photo de fin (mort, retiré). Un monstre
    qui rentre à son poste est **exclu des deux photos** : il est invulnérable (étape 69) et l'inclure
    réintroduirait l'exploitation de la limite du territoire.
- **Le coup gratuit** : instantané, attaque = attaque de base de l'attaquant (de l'attaque du monstre),
  résolue (critique éventuel, armure de la cible) par le `recevoirDegats` habituel. Il ne touche **ni la recharge
  ni le coup en cours** de l'attaquant, et n'interrompt pas l'action de la cible. Pas de ligne de vue
  exigée : les deux étaient au contact. Côté personnage touché : `Escouade::recevoirCoup(idMonstre, place,
  attaque, générateur)`, donc le monstre devient agresseur comme après un coup normal ; côté monstre :
  `Monstre::recevoirDegats`, qui l'alerte. Les accesseurs d'attaque (`getAttaqueDeBase`, `getAttaque`) sont
  des lectures : la recharge ne peut pas être touchée par construction. Le coup gratuit peut être un
  critique (affiché « N! Critical! »).
- **Module pur `Engagement`** (voir « Référence des modules ») : une photo (`PhotoDesCamps`) des attaquants et des
  cibles d'un seul sens, au début et à la fin de l'image, et la fonction `coupsDOpportunite`. Il ne connaît
  ni `Escouade`, ni `Monstre`, ni `Personnage`.
- **Intégration dans `Partie`** : une photo des deux camps (`EntiteDeControle` : monstres ciblables,
  personnages vivants) juste avant le déplacement de l'image (après le retrait des monstres morts),
  une autre juste après le tour des monstres. Les coups des **deux sens** sont calculés à partir des
  mêmes photos **avant** d'en appliquer un : les deux sont simultanés, un personnage tué par un coup
  gratuit frappe quand même celui qui vient de lui échapper. Les monstres tués sont retirés et le
  verrou levé tout de suite, comme après `attaquer`. Voir `docs/boucle-et-entrees.md`.
- **Débogage** : F3 dessine aussi un cercle fin magenta (rayon de l'entité + zone) autour de chaque
  entité qui a une zone, pour voir les engagements.
- **Toute la poussée compte** : la séparation douce (étape 67) déplace aussi les personnages ; son
  déplacement entre dans la mesure de la fuite, sans distinction. Accepté.
- **Limites connues** :
  - un pas de côté de A (perpendiculaire à A→B) qui rompt le contact a une contribution de 0, comme B
    immobile : à égalité B est le fuyard, A le frappe ;
  - un tireur au contact qui recule de lui-même reçoit le coup gratuit du monstre ; l'étape 78 le fait
    se dégager sans pénalité, mais seulement quand chaque monstre qui l'engage est tenu par un autre
    personnage (voir « Dégagement des tireurs » ci-dessous) ;
  - un coup par image et par attaquant peut sembler généreux ;
  - pas de skill qui annule la pénalité (se faufiler), pas de gambit « rester / se dégager » ;
  - pas d'effet visuel propre au coup gratuit (la barre de vie suffit).

## Dégagement des tireurs (étape 78)

- **Le problème** : depuis l'étape 77, un tireur au contact d'un monstre ne pouvait plus reculer sans
  recevoir un coup gratuit. Quand un guerrier tient ce monstre, le tireur n'a pourtant aucune raison de
  rester collé à lui : il tirerait aussi bien de sa place d'attaque (90 % de sa portée).
- **Pourquoi il part sans pénalité** : la règle des zones n'a pas été touchée. Le monstre qui engage le
  tireur engage aussi le guerrier ; à la fin de l'image il « engage encore quelqu'un », donc l'exemption
  de `coupsDOpportunite` joue (voir ci-dessus). Le dégagement ne fait que choisir quand partir. C'est
  aussi pourquoi il faut que CHAQUE monstre qui engage le tireur ait un autre personnage dans sa zone :
  un seul monstre non tenu suffirait à lui donner son coup gratuit.
- **La règle de départ** est `monstreDontSeDegager` (`Engagement`, fonction pure, voir
  « Référence des modules »), évaluée une fois par image sur les positions du début de l'image. Elle renvoie
  l'identifiant du plus proche des monstres qui engagent le personnage : c'est autour de lui que se
  calcule la place d'attaque, et le renvoyer évite de refaire le tri dans `Escouade`. Elle ne connaît ni
  l'ancre ni les skills : ces conditions sont dans `Escouade::degagementVoulu`.
- **Concernés** : les personnages vivants dont la zone de contrôle est 0 (prêtre, mage, rôdeur). Un
  guerrier ou un paladin ne se dégage jamais de lui-même.
- **Conditions de départ en plus** : l'ancre est à l'arrêt (l'ordre du joueur gagne), aucun skill en
  attente pour ce personnage, aucune action en exécution (un coup de base ou une activation de skill en
  cours part d'abord ; le dégagement commence à l'image suivante), pas déjà en dégagement.
- **Fin** : arrivée à la place (0,01 px, comme le seuil de `Mobile`) ; monstre absent de la photo des
  monstres ciblables (mort, retiré, ou qui rentre à son poste) ; ancre en mouvement ; skill mis en
  attente ou action engagée ; personnage encore engagé alors que la condition n'est plus vraie (le
  porteur est parti ou un second monstre non tenu est arrivé : rester évite le coup gratuit). Un
  personnage qui n'est plus engagé mais n'est pas arrivé continue : sortir de la zone est le but.
- **Pas d'attaque pendant le dégagement** : `autodefense` n'engage pas d'attaque de base (verrou
  compris) pour un personnage qui se dégage. Les skills déclenchés par le joueur restent permis ;
  engagés, ils mettent fin au dégagement (action en exécution), que la règle relance ensuite si le
  personnage est toujours engagé et tenu. Cela vaut aussi pour une explosion lancée pendant le recul :
  la visée et le lancer sont inchangés.
- **Où vit l'état** : `Escouade::m_degagements`, un `std::optional<int>` par personnage (le monstre dont
  il se dégage). `Escouade::degagementVoulu` contient départ et fin. Depuis l'étape 82.4, le point de
  branchement des gambits (rester et tirer, se dégager, passer en mêlée) est `choisirIntention`, dont le
  dégagement est la règle 2 (`docs/formation.md`, « Intentions »). Si le monstre n'a plus de place
  d'attaque libre, le dégagement est abandonné pour l'image et les règles habituelles reprennent (le
  personnage n'ira pas vers le centre du monstre) ; depuis 82.4, `confirmerDegagement` le constate avant
  la décision (`PlacesDAttaque::peutPlacer`, avec les entrées du placement qui suivrait).
- **Données** : `Partie` passe à `Escouade::mettreAJour` la liste d'`EntiteDeControle` de la photo du
  début de l'image (monstres ciblables, avec leur zone) en paramètre supplémentaire, à la place d'un champ
  de zone dans `EntiteVue` : celle-ci sert aussi aux obstacles, qui incluent les monstres qui rentrent.
  Le paramètre est vide par défaut (les tests qui n'en parlent pas ne changent pas).
- **Limites connues** :
  - un tireur bloqué pendant son recul n'en sort que si la règle change ;
  - sans verrou ni agresseur connu, un tireur arrivé retournerait vers sa place de formation (au contact
    du monstre) puis se dégagerait de nouveau ; en partie réelle le monstre frappe presque tout de suite
    le guerrier, qui devient un agresseur, et le tireur reste sur place ;
  - pas de repli quand un monstre approche d'un tireur qui n'est pas encore au contact ;
  - pas de réglage par personnage (gambits).

## Événements de combat et dégâts flottants (étape 79)

- **Le problème** : dégâts et soins s'appliquent au fond du noyau (coups des personnages, skill de zone,
  coups des monstres, attaques d'opportunité, soins), et l'affichage n'en voyait que le résultat, les
  barres de vie. Un coup gratuit, par exemple, était invisible.
- **Le patron : une file d'événements (Event Queue).** Le noyau produit des DONNÉES
  (`EvenementDeCombat`), l'affichage les consomme plus tard. `Partie` ne connaît pas l'affichage : pas de
  callback, pas de `std::function` vers `EcranJeu`, pas d'observateur. Le jour où autre chose voudra les
  lire (sons, journal de combat, statistiques), il lira la même file ; un nouveau type d'événement
  (étape 86 : nom du skill lancé, « Taunted ») s'ajoute au variant `DetailEvenement` sans réécrire ce
  qui existe.
- **La forme (étape 86).** Champs communs (`campCible`, `idCible`, `position`) et un `detail` en
  `std::variant<Degats, Soin, SkillLance, Provoque, Esquive, Bloque>` : `Degats { montant, opportunite,
  critique }`, `Soin { montant, critique }` (le champ `critique` date de l'étape 89), `SkillLance { IdSkill }`,
  `Provoque {}`, `Esquive {}` et `Bloque { montant, total }` (étape 90), puis `SacPlein {}` (étape 97.5 : le sac est
  plein, l'objet visé reste au sol ; `idCible` 0 = l'escouade entière, `position` celle de l'ancre ; produit par
  `Partie::ramasserSiAPortee`, pas par un coup). `TypeEvenement` n'existe plus, le
  variant dit le type. Raison : un skill lancé n'a pas de montant et « Taunted » n'a ni montant ni coup
  gratuit ; on ne représente pas des champs qui n'ont pas de sens. Les lecteurs (`TextesFlottants`)
  utilisent `std::visit` avec `Surcharges` : un détail ajouté sans son texte, sa couleur et sa taille ne
  compile pas.
- **Le montant est ce qui a réellement été appliqué.** Les fonctions qui appliquent dégâts et soins le
  renvoient : dégâts après armure (`appliquerArmure`), soin plafonné aux points de vie maximums
  (`PointsDeVie::soigner` renvoie la différence avant/après). Zéro pour une cible morte (deux coups
  partis dans la même image sur le même monstre : le second tombe sur un mort), un monstre en retour au
  poste (invulnérable) ou un allié déjà plein. **Un montant de 0 ne produit aucun événement** (pour
  `Degats` et `Soin`, les seuls à avoir un montant) : la règle est dans le seul endroit où un événement
  naît, `Partie::signaler`.
- **Pas de plafond par les points de vie restants.** Le coup qui tue affiche son plein montant (après
  armure), pas ce qui restait au monstre : le nombre dit la force du coup, la barre de vie dit l'état.
  Le soin, lui, est plafonné : il dit ce qui a vraiment été rendu.
- **La position est une copie.** `EvenementDeCombat::position` est le centre de la cible au moment de
  l'événement, jamais une référence ni un pointeur : le monstre tué par ce coup est retiré de
  `m_monstres` dans la même image, et un pointeur serait pendant. L'événement lui survit.
- **D'où viennent les événements.** `Partie::appliquer` (coups sur les monstres : skills, attaque de base,
  zone, coups gratuits des personnages) et `Partie::infligerAUnPersonnage` (coups des monstres, coups
  gratuits des monstres) appellent `signaler` avec le montant renvoyé ; `opportunite` vaut vrai pour les
  coups gratuits seulement. **Le reste vient d'`Escouade::effetDuSkill`**, par un canal unique : une
  liste de faits en variant, `m_faits` (`FaitDEscouade = std::variant<SoinApplique, SkillPart,
  MonstreProvoque>`), vidée par `Escouade::prendreFaits`. `Partie::signalerLesFaits` la prend à la fin de
  `resoudreLesCoups` et convertit chaque fait en événement (`std::visit`). Ce canal plutôt qu'un type de
  retour de plus pour `attaquer` et `declencherSkill` : ces deux fonctions renvoient des `Coup` et ont
  environ 200 tests. `Escouade` ne dépend pas de l'affichage ni d'`EvenementDeCombat`.
  - `SoinApplique { place, montant, position, critique }` devient `Soin` sur le soigné.
  - `SkillPart { place, skill, position }` (étape 86) : ajouté en tête de `effetDuSkill`, donc au départ
    de l'effet (fin de l'activation), pas au début ; tous les skills de la barre (dégâts, soin,
    provocation, zone, même sans cible touchée), jamais l'attaque de base. Devient `SkillLance` sur le
    lanceur. Le nom et un soin reçu par le lanceur dans la même image s'empilent (même cible, même
    image).
  - `MonstreProvoque { idMonstre }` (étape 86) : un par monstre que la provocation touche, dans la boucle
    de `effetDuSkill` (pas dans `Escouade::provoquer`, que les tests appellent directement). Pas de
    position : `Partie` la lit sur le monstre (`trouverMonstre`) et n'émet rien s'il a disparu. Un monstre
    déjà provoqué qui l'est de nouveau a de nouveau son texte. Devient `Provoque`, `Camp::Monstres`.
- **Partie figée** : `mettreAJour` ne fait rien après une défaite ou une victoire, donc aucun nouvel
  événement ; ceux de l'image qui a décidé la fin sont dans la file.
- **L'affichage** (`TextesFlottants`, `EcranJeu`) : durée de vie 0,8 s ; le texte naît 20 px au-dessus du
  centre de la cible et monte de 30 px ; opaque pendant la première moitié, puis linéairement jusqu'à 0.
  La hauteur et l'opacité sont des fonctions pures de l'âge : rien d'autre n'est stocké. Les textes
  ajoutés dans la même image pour la même cible (`campCible`, `idCible`) sont décalés de 14 px chacun vers
  le haut (le rang est fixé à l'ajout, `m_image` compte les images) : déterministe, sans hasard ni
  `Generateur`. Styles : dégâts sur un monstre en blanc, sur un personnage en rouge ; soin en vert, « +N » ;
  coup gratuit en orange, 20 px au lieu de 16, « N! » (quel que soit le camp) ; critique (dégâts ou soin) en
  jaune vif pour les dégâts, vert pour le soin, 18 px, suivi de « Critical! » (étape 89 : il l'emporte sur le
  coup gratuit) ; « Dodged » en gris clair et « Blocked » ou « Blocked −N » en gris, 14 px (étape 90) ; nom du skill lancé
  (`nomDuSkill`, `interface/AffichageSkills`) en jaune doré, 14 px ; « Taunted » en rouge orangé, 14 px ;
  contour noir de 1 px. Les libellés sont en UTF-8 (`sf::String::fromUtf8` dans `dessiner`).
- **Le brouillard est un filtre d'`EcranJeu`**, pas de `Partie` : si le brouillard est actif et que
  `Partie::estVisible(position)` est faux, l'événement n'est pas transformé en texte (même règle que pour
  ne pas dessiner un monstre caché). F6 coupe le brouillard : tout s'affiche. La file de `Partie`, elle,
  contient tout.
- **Limites connues** :
  - pas d'« Immunisé » pour un monstre qui rentre ; le nom du skill
    n'apparaît qu'au départ de l'effet, pas au début de l'activation (pas de barre d'incantation) ;
  - la taille du texte est en pixels du monde : elle suit le zoom de la vue ;
  - la visibilité est celle de la fin de l'image ;
  - l'empilement ne concerne que les textes d'une même image : deux coups de la même cible à des images
    proches peuvent se recouvrir en partie.

## Skill demandé pendant l'activation d'un autre (étape 80)

- **Le problème.** Un personnage n'a qu'une action engagée à la fois : `Personnage::engagerSkill`
  refuse un second skill pendant l'activation du premier. Jusqu'à l'étape 79, `Escouade` ignorait ce
  refus (`static_cast<void>`) : un ordre donné pendant l'activation d'un autre skill du même lanceur
  était perdu sans trace (ni activation, ni attente, ni recharge), et un skill déjà en attente
  disparaissait si sa cible devenait atteignable pendant cette activation. Cela concernait les classes
  à deux skills : guerrier (provocation puis frappe), paladin, rôdeur.
- **La règle.** L'ordre est **mis en attente** (ou y reste) et part dès que le lanceur est **libre** :
  `Personnage::peutEngagerSkill` (vivant, skill connu et prêt, aucun skill en activation, les
  conditions mêmes d'`engagerSkill`). Une seule règle pour les six endroits où un skill s'engage :
  dégâts et soin dans `declencherSkill`, zone dans `declencherSkillDeZone`, et les trois skills en
  attente (`engagerSkillDeZoneEnAttente`, `engagerSkillDeDegatsEnAttente`, le soin dans
  `avancerSkillEnAttente`). `engagerSkill` n'y est appelé que si le lanceur est libre :
  `[[maybe_unused]] const bool engage = ...; assert(engage);`. Pendant l'activation, le lanceur reste
  sur place (règle 0) ; à la fin, le skill en attente part s'il atteint sa cible,
  sinon le lanceur marche vers elle comme pour tout skill en attente.
- **Dégâts sans verrou.** Sans verrou, un skill de dégâts ne se mettait jamais en attente (la cible est
  choisie à portée). Il peut maintenant attendre que son lanceur soit libre : `engagerSkillDeDegatsEnAttente`
  retrouve donc la cible par son identifiant dans `monstres`, et non plus par le verrou seulement.
  Sans verrou, personne ne marche vers elle (règle de l'étape 37) : une fois libre, le lanceur la frappe
  s'il l'atteint encore ; sinon (hors de portée, hors de vue, disparue) le skill est abandonné, sans
  recharge. `avancerSkillEnAttente` ne fait marcher vers la cible d'un skill de dégâts qu'avec un
  verrou : à cette étape, `m_cibleCombat` n'était jamais remis à zéro et pouvait dater d'un ancien
  combat. Depuis l'étape 82.4 (R2), ce membre n'existe plus, et l'exigence n'a plus d'effet dans
  `Partie` : sans verrou, le skill est lancé ou abandonné dans `attaquer` dès que son lanceur est
  libre, et tant qu'il ne l'est pas, le lanceur est en exécution (règle 0). Elle est gardée ; on
  décidera en 82.5 si c'est du code mort à supprimer.
- **Ce qui ne change pas.** Un skill en recharge, ou déjà en activation lui-même, est refusé. Un nouvel
  ordre de skill pendant qu'un skill attend est refusé (un seul skill en attente pour l'escouade). Un clic
  au sol (`deverrouiller`) ou sur un autre monstre (`verrouiller`) annule un skill de dégâts en attente.
- **Tests** : `test_Escouade_skills.cpp`, « Lanceur occupe : … » (frappe demandée pendant la provocation,
  frappe en attente quand le monstre arrive pendant l'activation, abandon sans verrou, paladin, rôdeur).
- **Limites connues** :
  - un seul skill en attente pour toute l'escouade : pendant qu'un skill attend que son lanceur soit
    libre (ou marche vers sa cible), l'ordre de skill d'un **autre** personnage est refusé, donc perdu.
    La règle de l'étape 80 rend ces attentes plus fréquentes. La limite disparaîtra avec « un skill en
    attente par personnage » ;
  - la provocation, sans cible, n'a pas de mise en attente : demandée pendant l'activation d'un autre
    skill du guerrier, elle est toujours refusée (explicitement, dans `declencherSkill`) ;
  - rien à l'écran ne montre qu'un skill attend.

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Zones de contrôle et attaque d'opportunité** (étape 77) : une entité qui a une zone de contrôle
  (20 px bord à bord pour le guerrier, le paladin et tout monstre ; 0 pour le prêtre, le mage et le
  rôdeur, qui peuvent être engagés mais n'engagent personne) engage ce qui est dans sa zone. Quand B
  sort de la zone de A en s'écartant au moins autant que A s'écarte de B, A le frappe gratuitement :
  coup instantané, dégâts de son attaque de base (armure de la cible appliquée), sans toucher à sa
  recharge ni à son coup en cours, sans ligne de vue. Pas de coup si A engage encore quelqu'un à la
  fin de l'image ; au plus un coup par attaquant et par image (le fuyard le plus proche) ; un monstre
  qui rentre à son poste n'est ni attaquant ni cible. Dans les deux sens (monstre → personnage,
  personnage → monstre). Module pur `Engagement`, orchestré par `Partie` (deux photos par image, avant
  et après le déplacement). F3 dessine un cercle fin magenta autour de chaque entité qui a une zone.
  Un monstre tué par un coup gratuit est retiré et le verrou levé. Voir `docs/combat.md`,
  `docs/monstre.md` et `docs/boucle-et-entrees.md`.
- **Dégagement des tireurs** (étape 78) : un personnage sans zone de contrôle (prêtre, mage, rôdeur)
  engagé par un ou plusieurs monstres se dégage vers sa place d'attaque (anneau autour du plus proche
  d'entre eux, à 90 % de la portée de son attaque) dès que CHAQUE monstre qui l'engage a un autre
  personnage dans sa zone : il part sans coup gratuit, puisque ce monstre engage encore quelqu'un à la
  fin de l'image. Seul face à un monstre, il reste et tire. Conditions en plus : ancre à l'arrêt, aucun
  skill en attente pour lui, aucune action en exécution (un coup en cours part d'abord). Pendant le
  dégagement il ne frappe pas et ne s'arrête pas à portée ; fin à l'arrivée, si le monstre disparaît ou
  rentre à son poste, si l'ancre bouge, si un skill est mis en attente ou engagé, ou s'il est encore
  engagé alors que le porteur est parti. Priorité de `pointVoulu` (de `choisirIntention` depuis 82.4) : exécution > skill en attente >
  dégagement > verrou > agresseur > formation. Règle pure `monstreDontSeDegager` (`Engagement`), état
  et fin dans `Escouade::degagementVoulu`. Aucun affichage nouveau (F3 montre le point visé). Voir
  `docs/combat.md` et `docs/formation.md`.
- **Dégâts flottants** (étape 79) : au-dessus de chaque entité montent, 0,8 s durant, les dégâts réellement
  subis (après armure) et les soins reçus. Blanc pour les dégâts sur un monstre, rouge sur un personnage,
  vert précédé de « + » pour un soin, orange plus gros (20 px au lieu de 16) suivi de « ! » pour un coup
  gratuit ; contour noir ; le texte monte de 30 px et s'efface dans la seconde moitié de sa vie. Plusieurs
  textes sur la même cible dans la même image s'empilent de 14 px. Rien pour un montant de 0 : monstre qui
  rentre à son poste (invulnérable), soin sur un allié plein. Dans le brouillard, rien n'est montré (F6 :
  tout). Les textes finissent leur animation après une défaite ou une victoire. Mécanisme : `Partie`
  inscrit chaque dégât et chaque soin appliqué dans une file d'`EvenementDeCombat` (données), qu'`EcranJeu`
  prend après chaque image pour nourrir `TextesFlottants` (logique pure, dessin dans la vue du monde) ;
  les fonctions qui appliquent dégâts et soins renvoient le montant réellement appliqué. Voir
  `docs/combat.md`, `docs/boucle-et-entrees.md`.
- **Temps d'exécution** (étape 57) : toute attaque de base a un temps
  d'exécution, 0,15 s pour les personnages, 0,3 s pour les monstres,
  pendant lequel l'attaquant est immobile. Pour engager un coup il faut
  être vivant, immobile (`Mobile::estEnMouvement` faux), avoir l'attaque
  prête et une cible à portée et en vue : on ne frappe plus en marchant.
  Le coup engagé part toujours à la fin de l'exécution, même si la cible a
  bougé. Il est annulé, sans recharge consommée, par la mort de l'auteur ou
  de la cible ; pour un personnage aussi par un ordre de déplacement (clic,
  déplacement continu, clic sur un autre monstre) ou par un de ses skills
  (qui relance la recharge de l'attaque de base). Un monstre va toujours au
  bout de son coup, même si une provocation change sa proie. La recharge
  démarre au départ du coup. Le carré de l'entité s'éclaircit de 50 % pendant
  l'exécution (le contour jaune du monstre verrouillé est inchangé). Les
  skills ont leur propre temps d'activation (voir
  « Skills »). Voir `docs/combat.md`.
- **Aléatoire** (étape 56) : `Partie` possède un `Generateur`
  (`std::mt19937`, `Aleatoire.hpp`) initialisé par une graine que `main.cpp`
  tire avec `std::random_device` et affiche dans la console (au démarrage
  et à chaque R) ; il est passé par référence jusqu'à `Skill`. La recharge
  d'un `Skill` est un intervalle `[min, max]` tiré au déclenchement (aucun
  tirage si `min == max`) : attaques de base des mêlées 0,3–0,7 s, des
  tireurs 0,4–0,6 s, des monstres 0,7–1,3 s ; skills des personnages
  fixes. Voir `docs/combat.md`.
- **Armure** (étapes 55 et 87) : `appliquerArmure` (module `Defense`) réduit les
  dégâts reçus, même formule pour tous : `max(D − armure, D × 20 %)`, arrondi au plus proche, au moins 1
  par coup positif. Mêlées 30, tireurs 0, monstres 10. Le soin n'est pas réduit. Rien n'est affiché à
  l'écran.
- **Combat** : le monstre verrouillé est entouré d'un contour jaune.
  L'escouade s'approche en formation jusqu'à ce que tous ses personnages
  vivants soient à portée, puis s'arrête et chacun frappe selon sa propre
  recharge. En combat, un personnage déjà à portée reste sur place : les
  tireurs s'arrêtent à distance pendant que les mêlées vont jusqu'au
  contact. Le combat ne se rompt que par la mort de la cible, un clic sur
  le sol ou un clic sur un autre monstre.
- **Autodéfense** : sans clic, chaque personnage vivant frappe de
  lui-même un monstre qui se trouve à sa portée, à chaque image (à
  l'arrêt, en marche, en combat). Il choisit le monstre verrouillé s'il
  est à sa portée, sinon le plus proche à sa portée (bord à bord ;
  égalité : le premier de la liste). Deux personnages peuvent frapper
  deux monstres différents. Elle ne déplace personne : les mêlées ne
  frappent que ce qui est à 20 px d'eux, les tireurs à 120 px.
- **Agresseurs** : `Escouade` retient les monstres qui viennent de la
  frapper (3 s sans nouveau coup, ou mort du monstre). Quand l'ancre est
  à l'arrêt, un personnage vivant sans monstre à portée quitte sa place
  pour aller vers l'agresseur le plus proche, à condition que celui-ci
  soit à 100 px ou moins de sa place ; avec un monstre déjà à portée, il
  reste sur place. L'ordre du joueur (ancre en mouvement), le skill en
  attente et le verrou (personnage à portée du monstre verrouillé) passent
  avant.
- **Skills** : quatre, de classe `Skill` (type, portée, puissance, recharge, rayon de zone, temps d'activation).
  **Étape 74** : un skill est désigné par une `AdresseSkill` (place du personnage et `IdSkill`
  stable : `FrappePuissante`, `Provocation`, `Soin`, `Explosion`) ; un personnage connaît une liste de
  skills, une seule action engagée à la fois (engager un second skill pendant l'activation d'un autre est
  refusé), recharges indépendantes. **Étape 75** : les skills se lancent par la barre
  de raccourcis (par défaut Q → frappe du guerrier, W → sa provocation, A → soin du prêtre, Z → explosion
  du mage, au clavier ou au clic ; ci-dessous, « touche 2 », « touche 3 » et « touche 4 » désignent W, A et Z) ; la
  visée dépend du type `Zone` du skill. Frappe (Q) :
  skill de dégâts du guerrier (25 dégâts, recharge 5 s, portée
  de mêlée, 20 px), sur le monstre verrouillé ou, sans verrou, sur le
  monstre à portée que ce guerrier frapperait en autodéfense (alors sans
  marche ni mise en attente). Touche 3 : soin du prêtre
  (30 points de vie, recharge 8 s, portée 40 px), sur le
  vivant le plus blessé (soigneur inclus), permis hors combat. Touche 2 :
  provocation du guerrier (zone de 150 px depuis son centre,
  4 s, recharge 10 s) : tous les monstres dans la zone le prennent pour
  proie au lieu du personnage le plus proche, sans dégâts ni attente
  (consommée même s'il n'y a aucun monstre dans la zone) ; elle prend fin à
  l'écoulement de la durée, à la mort du provocateur ou à celle du
  monstre. Touche 4 : skill de zone du mage (15 dégâts, recharge 8 s,
  zone de 60 px, portée 250 px) en deux temps : 4 fait entrer en
  mode visée (un cercle jaune de 60 px suit le curseur, un cercle blanc
  montre la portée autour du lanceur), puis un clic gauche lance le sort
  au point visé (même sur un monstre : pas de verrou). À portée (bord à
  bord) et en vue (ligne de vue entre le lanceur et le point, sans mur),
  son activation commence tout de suite ; hors portée ou derrière un mur, le lanceur
  quitte sa place, marche vers le point (contournant le mur) et commence
  l'activation au point fixé au clic dès qu'il est à portée et en vue (un point
  dans un mur est refusé, sans recharge ; seule sa mort l'annule : ni verrou, ni clic, ni ordre de
  déplacement). Le sort touche tous les monstres dont le bord est dans le
  cercle au moment du départ et qui sont en vue du point d'impact (centre
  à centre, il ne traverse pas les murs), et coûte sa recharge même si la zone est vide. Clic
  droit, Échap ou 4 annulent sans recharge ; le déplacement continu est
  suspendu pendant la visée. Un skill de dégâts ou de soin hors de portée met son lanceur en marche vers la
  cible et commence son activation à l'arrivée (un seul skill en attente à
  la fois ; annulé par la mort du lanceur, celle de la cible ou son retour
  à pleine vie pour un soin, la rupture du combat pour des dégâts). Une
  recharge, qui ne démarre qu'au départ du skill, est montrée par la barre de skills (la
  barre orange sous les personnages a été supprimée à l'étape 74).
  **Temps d'activation** (étape 58) : chaque skill passe par la même machine
  à états que l'attaque de base : dégâts 0,3 s, provocation 0,2 s, soin
  0,6 s, zone 0,8 s. L'appui est un ordre : il n'exige pas d'être immobile,
  mais le lanceur s'arrête sur place pendant toute l'activation (son carré
  s'éclaircit, l'icône a un contour blanc), puis l'effet part, évalué à ce
  moment (monstres dans la zone, cible du coup ou du soin). Le coup de base
  engagé est annulé au début et sa recharge est relancée au départ.
  Pendant l'activation le skill n'est pas prêt : un nouvel appui est refusé
  et la touche 4 n'entre pas en visée ; les skills d'autres personnages
  restent permis (plusieurs activations simultanées). Seuls la mort du
  lanceur, celle du monstre visé (dégâts) ou celle de l'allié visé (soin)
  l'annulent, sans recharge ; un ordre de déplacement, un clic au sol ou un
  verrouillage ne l'annulent pas. Pas de revérification de portée ni de vue
  au départ. **Lanceur occupé** (étape 80) : un skill demandé pendant l'activation d'un autre skill du
  même personnage (guerrier : provocation puis frappe ; paladin ; rôdeur) n'est plus perdu : il est mis
  en attente et part dès que le lanceur est libre ; un skill déjà en attente le reste si sa cible
  devient atteignable pendant cette activation. Sans verrou, une frappe ainsi en attente part si le
  monstre est encore atteint quand le lanceur est libre, sinon elle est abandonnée, sans recharge.
  Voir `docs/combat.md`.
- **Blocage mutuel** : les monstres se bloquent entre eux et sont bloqués
  par les personnages vivants ; les personnages sont bloqués par les
  monstres. L'ancre ne bloque personne et n'est pas bloquée.

### Limites connues

- Pas d'interruption de l'activation d'un skill par les dégâts reçus ; un
  seul skill en attente pour toute l'escouade (depuis l'étape 80, un skill attend aussi que son
  lanceur ait fini l'activation d'un autre de ses skills : pendant ce temps, l'ordre de skill d'un
  autre personnage est refusé) ; la provocation, sans cible, ne peut pas attendre : demandée pendant
  l'activation d'un autre skill du guerrier, elle est refusée (le mode de cible « Soi » de R4 doit
  lever cette limite) ; pas de barre de progression d'incantation ; pas de revérification de portée
  ni de vue au départ du skill.
- Le skill de zone n'a pas d'effet visuel à
  l'impact et ne touche pas les alliés ; son lanceur, bloqué par un
  monstre ou face à un point inaccessible, peut rester en attente.
- Zones de contrôle (étape 77) : un seul coup gratuit par attaquant et par image, sans délai de
  réaction (peut sembler généreux) ; un pas de côté de A qui rompt le contact compte comme une fuite
  de B (égalité des projections : B est le fuyard) ; toute la poussée de la séparation douce compte
  comme déplacement ; un tireur au contact d'un monstre seul, ou de plusieurs dont un n'est pas tenu,
  qui recule reçoit le coup gratuit (l'étape 78 ne le dégage que si chaque monstre est tenu) ; pas de
  skill qui annule la pénalité ; pas d'effet visuel propre au coup gratuit (la barre de vie suffit).
- Dégagement (étape 78) : un tireur bloqué pendant son recul n'en sort que si la règle change
  (`Personnage` n'expose pas `estBloque`) ; l'arrivée est mesurée à 0,01 px de la place (une poussée de
  la séparation douce peut la retarder) ; une fois arrivé, sans verrou ni agresseur connu, les règles
  habituelles le ramènent vers sa place de formation (en partie réelle le monstre frappe le guerrier et
  devient agresseur : le tireur reste) ; une action déclenchée par le joueur interrompt le recul, que la
  règle relance ensuite si le tireur est toujours engagé et tenu ; un monstre qui change de cible peut
  relancer le dégagement.
- Dégâts flottants (étape 79) : le nombre est le montant après armure, sans plafond par les points de
  vie restants (le coup qui tue affiche son plein montant) ; pas d'« Immunisé » sur un monstre qui
  rentre, pas de critique ni de « raté » ; la taille
  du texte est fixe en pixels du monde, donc elle suit le zoom de la vue ; la visibilité d'un événement
  est jugée à la fin de l'image où il a lieu ; deux textes de la même cible ajoutés dans des images
  différentes mais proches dans le temps peuvent se superposer en partie.
- Provocation stricte (étape 66) : un monstre provoqué ne frappe que son
  provocateur, même s'il est collé à un autre personnage ; si le provocateur
  est caché, il ne frappe personne.
- Un coup engagé part sans revérifier portée ni vue ; seule la marche
  empêche d'engager (pas de ralentissement de l'escouade engagée).
- Les entités sont dessinées en carrés mais mesurées en cercles : en
  diagonale, l'écart visible est plus petit que la portée (d'une dizaine
  de pixels).
- L'autodéfense ne déplace personne : un mêlée attaqué par un monstre
  qui s'arrête à 15 px de lui riposte sur place, mais un tireur ou un
  mêlée qui n'a rien à portée ne réagit pas ; elle ne regarde que la
  portée (pas de notion de « qui attaque qui »). Le déplacement vers
  un agresseur (voir « Agresseurs ») s'y ajoute, mais seulement dans la
  laisse de 100 px et ancre à l'arrêt.

### Le niveau atteint (étape 99.5)

`NiveauAtteint { niveau }` est un détail d'événement de plus (`combat/EvenementDeCombat.hpp`), au-dessus de l'escouade entière
comme `SacPlein` (`Camp::Escouade`, `idCible` 0, position de l'ancre) : un événement par niveau gagné, signalé par
`Partie::monterSiLExperienceLeDonne`. Le texte flottant est « Level up! », 20 px, bleu clair (une couleur que ni le soin, ni le nom
d'un skill, ni « Bag full » n'ont) ; il s'empile avec les autres textes de l'escouade entière.

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `Provocations` (`escouade/`) — les monstres provoqués

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

### `Agresseurs` (`escouade/`) — les monstres qui ont frappé l'escouade

Possédée par valeur par `Escouade`. Données privées : `m_agresseurs`
(`std::vector<Agresseur>`, structure publique : `int id`, `float tempsRestant`). Mémoire de 3 s
sans nouveau coup du même monstre. Aucune position (82.5).

| Fonction | Rôle |
|---|---|
| `void enregistrer(int id)` | Ajoute l'agresseur, ou remet son temps à 3 s s'il est connu. La règle « coup à 0 = rien » reste dans `Escouade::recevoirCoup` |
| `void mettreAJour(float dt)` | Fait écouler le temps ; oublie les agresseurs échus |
| `void retirerMonstresAbsents(const std::vector<EntiteVue>& monstres)` | Oublie les agresseurs absents de la liste (morts, ou plus ciblables) |
| `std::vector<EntiteVue> vusParmi(const std::vector<EntiteVue>& monstres) const` | Les agresseurs sous la forme d'`EntiteVue`, avec le cercle qu'ils ont dans cette liste, dans l'ordre d'enregistrement ; un agresseur absent de la liste est ignoré (et gardé en mémoire) ; `Escouade::mettreAJour` l'appelle une fois par image avec les ciblables ; `[[nodiscard]]` |
| `const std::vector<Agresseur>& liste() const` | Agresseurs retenus |

### `Skill` (`combat/`) — une capacité spéciale

Un `TypeSkill` (`enum class { DegatsSurCible, Soin, Provocation, Zone }`), une
portée, une puissance (dégâts, points de vie rendus ou durée en secondes,
dégâts d'un skill de zone), une recharge, ainsi qu'un rayon de zone (la
taille de la zone touchée par un skill de zone ; 0, non lu, pour un skill
à cible unique). Sert aussi d'attaque de base : un `Skill` de type
`DegatsSurCible`, de rayon de zone 0.
Il suit une machine à états (`enum class PhaseSkill { Pret, EnExecution, EnRecharge }`) : `commencer` (prêt → en exécution), `avancer` (fin de l'exécution : le coup part, la recharge est tirée, → en recharge), `annuler` (en exécution → prêt, sans recharge). Les skills des personnages passent par la même machine à états (étape 58 : activation de 0,3 s, 0,2 s, 0,6 s et 0,8 s pour dégâts, provocation, soin et zone) ; avec un temps nul, l'effet part dans l'image de l'engagement. Il ne connaît pas la cible du coup engagé, c'est son propriétaire qui la retient.
Possédé par valeur par `Personnage` (attaque de base, plus un skill en
option) et par `Monstre` (son attaque). Ne connaît ni la position de son
propriétaire ni sa cible : il reçoit deux `Cercle`. Ne dessine rien.

Données membres : `m_type` (`TypeSkill`), `m_portee` (bord à bord), `m_coefficients` (`Coefficients`, étape 88), `m_nature` (étape 90), `m_armesExigees` (`std::vector<TypeDArme>`, étape 91.5),
`m_element` (`Element`, étape 94.4), `m_bonusDeFrappe` (`std::optional<Degats>`, étape 94.5),
`m_puissance`, `m_rechargeMin`, `m_rechargeMax` (intervalle de recharge,
en secondes), `m_tempsRecharge` (durée de la recharge en cours, dernier
tirage), `m_recharge`, `m_rayonZone` (`float`, en pixels), `m_tempsExecution`, `m_executionRestante` (secondes), `m_phase` (`PhaseSkill`).

| Fonction | Rôle |
|---|---|
| `explicit Skill(const DefinitionDeSkill&)` | Crée le skill depuis ses données nommées (étape 88.1, C2) : `DefinitionDeSkill { type; portee; puissance; rechargeMin; rechargeMax; rayonZone = 0; tempsExecution = 0; coefficients = {}; nature; armesExigees = {}; element = Physique; bonusDeFrappe = vide }` (la `nature` n'a plus de valeur par défaut depuis l'étape 91.1 : chaque skill déclare la sienne, un oubli est une erreur de compilation ; un soin ou une provocation écrit `Contact`), en initialisation désignée (`Skill({.type = TypeSkill::DegatsSurCible, .portee = 80.0f, ...})`). Crée le skill de ce type, avec cette portée, cette puissance, cet intervalle de recharge (`assert(0 <= min <= max)` ; fixe si `min == max`), ce rayon de zone et ce temps d'exécution (`assert >= 0`) ; prêt à la création |
| `bool commencer()` | Prêt → en exécution ; faux, sans rien changer, dans toute autre phase ; `[[nodiscard]]` |
| `bool avancer(float dt, Generateur& generateur)` | En exécution : fait avancer l'exécution ; vrai une seule fois, au départ du coup (la recharge est alors tirée, phase → en recharge). Autres phases : comme `mettreAJour`, faux ; `[[nodiscard]]` |
| `void annuler()` | En exécution → prêt, sans recharge ; sans effet ailleurs |
| `PhaseSkill getPhase() const` | Phase actuelle |
| `int getPuissance() const` | Puissance du skill, brute (sans les caractéristiques : `Personnage::attaqueDe` les applique) |
| `const Coefficients& getCoefficients() const` | Ce que la puissance doit aux caractéristiques du lanceur ; `[[nodiscard]]` |
| `void mettreAJour(float dt)` | Décrémente la recharge, sans descendre sous zéro ; en recharge écoulée, repasse à prêt |
| `bool estAPortee(Cercle origine, Cercle cible) const` | `distanceBordABord(origine, cible)` est-elle `<=` `m_portee` ? |
| `void relancerRecharge(Generateur& generateur)` | Remet la recharge au complet, sans déclencher : `min` si `min == max` (aucun tirage), sinon un tirage uniforme (`std::uniform_real_distribution<float>`) dans `[min, max]` |
| `float getProgressionRecharge() const` | De 0 (juste après le déclenchement) à 1 (prêt) ; 1 si le temps de recharge est nul |
| `float getRechargeRestante() const` | Temps de recharge restant, en secondes (0 si prêt) |
| `TypeSkill getType() const` | Type du skill |
| `float getRayonZone() const` | Rayon de zone du skill |
| `float getPortee() const` | Portée du skill (bord à bord) |
| `Element getElement() const` | L'élément des dégâts de la puissance (physique par défaut ; l'explosion : feu) ; `[[nodiscard]]` |
| `const std::optional<Degats>& getBonusDeFrappe() const` | Le bonus d'un skill de frappe (vide pour tout autre skill) ; `assert` à la construction : un skill de frappe est de type `DegatsSurCible` de puissance 0 ; `[[nodiscard]]` |
| `const std::vector<TypeDArme>& getArmesExigees() const` | Les types d'arme qui permettent le skill (vide : aucune exigence) ; `[[nodiscard]]` |
| `bool convientA(std::optional<TypeDArme> arme) const` | Toujours vrai sans exigence ; sinon l'arme (vide : mains nues, faux) est-elle dans la liste ? ; `[[nodiscard]]` |

### `IdSkill`, `SkillConnu`, `AdresseSkill` (`combat/`) — désigner un skill

`IdSkill.hpp`, `SkillConnu.hpp` et `AdresseSkill.hpp`, sans `.cpp`. Voir `docs/combat.md`.

- `enum class IdSkill { FrappePuissante, Provocation, Soin, Explosion, FrappeSacree, ImpositionDesMains, TirPercant, PluieDeFleches }` : identifiant **stable** d'un skill, indépendant de l'ordre dans la liste d'un personnage et de la touche.
- `struct SkillConnu { IdSkill id; Skill skill; }` : un skill d'un personnage avec son identifiant (`Skill` ne porte pas d'identifiant : il sert aussi d'attaque de base et d'attaque des monstres).
- `struct AdresseSkill { std::size_t place; IdSkill id; }` (comparable, `operator==` par défaut) : le personnage (place de 1 à N) et le skill. Une adresse invalide (place invalide ou skill inconnu de ce personnage) est traitée comme une place invalide : refus, liste vide, `optional` vide.

### `Coup` (`combat/`) — ce qui circule entre `Escouade` et `Partie`

`Coup.hpp`, sans `.cpp` : `struct Coup { int idMonstre; Attaque attaque; }` (une
attaque à infliger à ce monstre ; `int degats` avant l'étape 89.3). Sortie d'`Escouade`.

### `Attaque` (`combat/`) — ce qui part de l'attaquant (étape 89.3)

`Attaque.hpp`, sans `.cpp` : `struct Attaque { Degats degats; int capaciteOffensive; NatureDAttaque nature; int penetration = 0; int soin = 0; int dureeDeProvocation = 0; }`.
Les dégâts en portions par élément (étape 94 ; avant toute réduction, caractéristiques comprises), le soin et la durée
d'une provocation (champs séparés depuis l'étape 94.1 : `montant` a disparu ; un skill n'en produit qu'un), la
capacité offensive de l'attaquant, la nature de l'attaque (étape 90.1 ; sans valeur par défaut) et la pénétration
d'armure en pour cent (étape 92.4 ; 0 par défaut : un skill, un monstre). Produite par `Personnage` (`getAttaqueDeBase`, `avancerAttaque`, `avancerSkill`) et `Monstre`
(`getAttaque`, `avancerAttaque`), portée par `Coup`, reçue par `recevoirDegats` / `recevoirCoup` et
`resoudreCoup`. Voir « Le critique… (étape 89) » et « L'esquive et le blocage (étape 90) ».

### `NatureDAttaque` (`combat/`) — contact, projectile ou zone (étape 90.1)

`NatureDAttaque.hpp`, sans `.cpp` : `enum class NatureDAttaque { Contact, Projectile, Zone }`. Lue par l'esquive
et le blocage (`switch` sans `default` : une nature ajoutée sans règle fait avertir le compilateur).
Portée par `Attaque` et `DefinitionDeSkill` (`Skill::getNature`).

### `StatistiquesDeCombat` (`combat/`) — ce que le moteur du coup lit d'une entité (étape 89.2)

`StatistiquesDeCombat.hpp` / `.cpp`. `struct StatistiquesDeCombat { int capaciteOffensive = 0; int
capaciteDefensive = 0; int esquiveContact = 0; int esquiveProjectile = 0; int chanceDeBlocage = 0; int
montantBloque = 0; Resistances resistances = {}; }` (comparable, `+=` et `+` : tous les champs s'additionnent, les résistances élément par élément ; la récupération du blocage a
disparu à l'étape 91.6, c'est une constante commune) ; esquives et chance de blocage en pour cent ;
constantes `s_capaciteDeBase` (50) et `s_capaciteParPointDAdresse` (5) ; `StatistiquesDeCombat
statistiquesDuPersonnage(const Caracteristiques&, const StatistiquesDeCombat& bonusDEquipement)` : `50 + 5 ×
Adresse` pour les deux capacités plus le bonus, offensive bornée à 0, défensive à 1, les autres champs ceux du
bonus ; `[[nodiscard]]`. Seul endroit qui calcule les statistiques d'un personnage. Même type pour le bonus
d'un objet. Voir « Le critique… (étape 89) » et « L'esquive et le blocage (étape 90) ».

### `Esquive` (`combat/`) — la chance d'esquiver (étape 90.2)

`Esquive.hpp` / `.cpp`, fonctions pures. Constante `s_esquiveMaximale` (50).

| Fonction | Rôle |
|---|---|
| `constexpr int esquiveEffective(int pourCent)` | Ramenée entre 0 et `s_esquiveMaximale` ; lue par le coup et par l'affichage ; `[[nodiscard]]` |
| `float chanceDEsquive(NatureDAttaque, const StatistiquesDeCombat&)` | L'esquive contre le contact ou contre les projectiles du défenseur, plafonnée, en fraction de 0 à 0,5 ; 0 pour une zone ; `[[nodiscard]]` |
| `bool coupEsquive(float chance, float tirage)` | `tirage < chance` ; `[[nodiscard]]` |

### `Blocage` (`combat/`) — le blocage au bouclier (étape 90.3)

`Blocage.hpp` / `.cpp`, fonctions pures. `struct ResultatDuBlocage { float montant; float bloque; }` (flottants depuis l'étape 94 : l'arrondi vient à la fin du coup) ; constante
`s_recuperationDuBlocage` (3 s, étape 91.6) : le temps pendant lequel un bouclier ne bloque plus après un blocage
réussi, le même pour tous.

| Fonction | Rôle |
|---|---|
| `float chanceDeBlocage(NatureDAttaque, const StatistiquesDeCombat&, bool blocagePret)` | La chance (de 0 à 1) de bloquer : nulle contre une zone, pendant la récupération et sans bouclier (`montantBloque <= 0`), sinon `chanceDeBlocage` ramenée de 0 à 100 % ; `[[nodiscard]]` |
| `ResultatDuBlocage appliquerBlocage(float montant, int montantBloque, float chance, float tirage)` | `montant` est la portion **physique** du coup. Si `tirage < chance`, retire `min(montant, montantBloque)` ; rend le montant restant et ce qui a été absorbé (0 : pas de blocage) ; `assert(montant >= 0)` ; `[[nodiscard]]` |

### `Critique` (`combat/`) — la chance et la force du critique (étape 89.3)

`Critique.hpp` / `.cpp`, fonctions pures sans SFML ni état. Constantes `s_chanceParRapportCarre` (0,05),
`s_chanceMaximale` (0,5), `s_multiplicateurDeBase` (1,5), `s_multiplicateurParRapport` (0,25),
`s_multiplicateurMaximal` (2,0).

| Fonction | Rôle |
|---|---|
| `float rapportDeCapacites(int capaciteOffensive, int capaciteDefensive)` | Offensive / défensive ; `assert(défensive > 0)` ; `[[nodiscard]]` |
| `float chanceDeCritique(float rapport)` | `min(0,05 × rapport², 0,5)` ; `[[nodiscard]]` |
| `float multiplicateurDeCritique(float rapport)` | `clamp(1,5 + 0,25 × (rapport − 1), 1,5 ; 2,0)` ; `[[nodiscard]]` |
| `bool coupCritique(float rapport, float tirage)` | `tirage < chanceDeCritique(rapport)` ; le moteur du coup en tire le multiplicateur de chaque portion, sans arrondir ; `[[nodiscard]]` |
| `ResultatCritique appliquerCritique(int montant, float rapport, float tirage)` | `{ montant, critique }` pour un montant seul (le soin) : critique si `coupCritique`, montant multiplié et arrondi au plus proche ; `assert(montant >= 0)` ; `[[nodiscard]]` |

### `EvenementDeCombat` (`combat/`) — ce qui s'est passé, en données

`EvenementDeCombat.hpp` / `.cpp` : `enum class Camp { Escouade, Monstres }`, les détails `DegatsSubis { int montant; bool opportunite; bool critique; Element element = Physique; }` (`element` : celui de la plus grosse portion après les réductions, étape 94 ; `Degats` avant l'étape 94), `Soin { int montant; bool critique; }`, `SkillLance { IdSkill skill; }`, `Provoque {}`, `Esquive {}`, `Bloque { int montant; bool total; }`, `using DetailEvenement = std::variant<DegatsSubis, Soin, SkillLance, Provoque, Esquive, Bloque>` et `struct EvenementDeCombat { Camp campCible; int idCible; sf::Vector2f position; DetailEvenement detail; }`. `idCible` : place du personnage (1 à N) ou identifiant du monstre ; `montant` : ce qui a réellement été appliqué (armure comprise, soin plafonné), jamais 0 ; `position` : COPIE du centre de la cible à ce moment (le monstre tué par ce coup est retiré dans la même image, l'événement lui survit) ; `opportunite` : coup gratuit ; `critique` : le coup ou le soin a été un critique (le montant est déjà multiplié) ; `Bloque::montant` : ce que le blocage a absorbé (jamais 0), `total` : il a tout absorbé (aucun `DegatsSubis` ne suit) ; `SkillLance` : cible = le lanceur ; `Provoque` : cible = le monstre provoqué ; `Esquive` : cible = celui qui a esquivé. La fonction pure `std::vector<DetailEvenement> detailsDuCoup(const ResultatDuCoup&, bool opportunite)` (`[[nodiscard]]`) convertit un résultat de coup : `Esquive` seul s'il a été esquivé, sinon `DegatsSubis` (rien s'ils sont nuls ; son `element` est `portions.elementDominant()`) puis `Bloque` s'il y a eu blocage ; liste vide pour un résultat « rien ». Patron « file d'événements » : produits par `Partie` (`prendreEvenements`), lus par `EcranJeu`, sans callback ni observateur. Voir `docs/combat.md`.

### `TextesFlottants` (`interface/`) — les dégâts et soins à l'écran

`TextesFlottants.hpp` / `.cpp`. Logique pure (sans fenêtre) ; seul `dessiner` utilise SFML. Possédé par `EcranJeu`. Un `TexteFlottant { EvenementDeCombat evenement; float age; int rang; std::size_t image; }` par événement. Donnée membre : `m_textes` (`std::vector<TexteFlottant>`), `m_image` (`std::size_t`, numéro de l'image courante pour l'empilement) ; constante `static constexpr float s_dureeDeVie = 0.8f`.

| Fonction | Rôle |
|---|---|
| `void ajouter(const EvenementDeCombat&)` | Ajoute un texte d'âge 0. Son rang est le nombre de textes déjà ajoutés dans cette image pour la même cible (`campCible` et `idCible`) : empilement déterministe, sans hasard |
| `void mettreAJour(float dt)` | Fait vieillir tous les textes, retire ceux qui ont atteint 0,8 s, passe à l'image suivante |
| `const std::vector<TexteFlottant>& actifs() const` | Les textes à l'écran ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget&, const sf::Font&) const` | Un `sf::Text` par texte, centré sur `positionDuTexte`, contour noir de 1 px, opacité de `opaciteDuTexte` (à appeler avec la vue du monde) |
| `float hauteurDuTexte(float age)` | Fonction libre pure : 20 px au-dessus du centre de la cible à la naissance, 30 px de plus sur toute la durée de vie ; `[[nodiscard]]` |
| `float opaciteDuTexte(float age)` | Pure : 1 pendant la première moitié de la vie, puis linéaire jusqu'à 0 à 0,8 s ; `[[nodiscard]]` |
| `sf::Vector2f positionDuTexte(const TexteFlottant&)` | Pure : position de l'événement, relevée de `hauteurDuTexte(age)` et de 14 px par rang ; `[[nodiscard]]` |
| `std::string libelleDuTexte(const EvenementDeCombat&)` / `sf::Color couleurDuTexte(...)` / `unsigned int tailleDuTexte(...)` | Purs. Dégâts « 12 », coup gratuit « 12! » (orange, 20 px), soin « +12 » (vert) ; critique : le texte suivi de « Critical! » (« 12 Critical! », « 12! Critical! », « +12 Critical! »), jaune vif pour des dégâts (même gratuits), vert pour un soin, 18 px dans les deux cas ; nom du skill lancé (jaune doré, 14 px) ; « Taunted » (rouge orangé, 14 px) ; « Dodged » (gris clair, 14 px), « Blocked » (blocage total) ou « Blocked −N » (partiel, signe moins U+2212), gris, 14 px ; dégâts physiques sur un monstre blancs, sur un personnage rouges, feu orange rougeâtre et sacré or pâle quel que soit le camp (le critique et le coup gratuit gardent leur couleur) ; 16 px sinon ; `std::visit` sur le détail ; `[[nodiscard]]` |

### `ActionEngagee` (`combat/`) — l'action qu'un personnage prépare

`ActionEngagee.hpp`, sans `.cpp` : `enum class TypeAction { AttaqueDeBase, Skill }` et `struct ActionEngagee { TypeAction action; std::optional<Cible> cible; std::optional<IdSkill> skill; }` (`skill` est vide pour une attaque de base). La cible est vide pour la provocation ; pour une attaque de base, c'est une `CibleMonstre` dont le `cercle` n'est pas renseigné. Stockée par `Personnage` (`m_actionEngagee`), résolue par `Escouade`. `TypeAction` sert aussi à `Escouade::peutAtteindre` (portée de l'attaque de base ou du skill).

### `EntiteVue` (`combat/`) — une entité telle que l'autre camp la voit

`EntiteVue.hpp` / `.cpp` : `struct EntiteVue { int id; Cercle cercle; }` (l'identifiant d'un monstre, ou la place 1 à N d'un personnage)
(entrée d'`Escouade` et d'`Autodefense`) et une fonction libre.

| Fonction | Rôle |
|---|---|
| `const EntiteVue* trouverEntite(const std::vector<EntiteVue>& entites, int id)` | Entité portant cet identifiant, `nullptr` si elle est absente (`[[nodiscard]]`) |

`Partie` a son équivalent pour `std::vector<Monstre>`, `trouverMonstre`
(fonction privée).

### `TypeSkill` (`combat/`) — le type d'un skill

`TypeSkill.hpp`, sans `.cpp` : `enum class TypeSkill { DegatsSurCible, Soin,
Provocation, Zone }`. Inclus par `Skill`, `EtatSkill`, `BarreDeSkills` et
`Escouade` sans qu'ils dépendent de la classe `Skill`.

### `Autodefense` (`combat/`) — quel monstre un combattant frappe

Deux fonctions libres, sans état. `choisirCibleAFrapper` est utilisée par
`Escouade::attaquer`, `Escouade::choisirCible` et `Monstre` (cible de frappe), `choisirAgresseurAViser`
par `Escouade::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `std::optional<int> choisirCibleAFrapper(Cercle origine, const std::function<bool(Cercle)>& estAPortee, const std::vector<EntiteVue>& entites, std::optional<int> idPrefere)` | Identifiant de l'entité préférée (verrouillée, ou proie) si elle est à portée ; sinon de la plus proche (bord à bord) parmi celles à portée (égalité : la première de la liste) ; sinon vide. Même règle pour les deux camps. `estAPortee` dit si un cercle est à portée (celle de l'attaque ou celle d'un skill) |
| `std::optional<EntiteVue> choisirAgresseurAViser(Cercle personnage, sf::Vector2f place, float laisse, const std::vector<EntiteVue>& agresseurs)` | Agresseur (identifiant et cercle) le plus proche du personnage (bord à bord ; égalité : le premier de la liste) parmi ceux dont le centre est à `laisse` ou moins de `place` ; sinon vide |

### `Engagement` (`combat/`) — zones de contrôle et attaque d'opportunité

`Engagement.hpp` / `.cpp` : deux structures de données, une structure de résultat et trois fonctions libres, sans état et sans SFML graphique. Ne connaît ni `Escouade`, ni `Monstre`, ni `Personnage`, ni `Partie` (sur le modèle d'`Autodefense`) : elle reçoit des cercles et des zones. Voir `docs/combat.md`.

`struct EntiteDeControle { int id; Cercle cercle; float zone; }` (id d'un monstre, ou place 1 à N d'un personnage ; zone en pixels, bord à bord ; 0 = n'engage personne). `struct PhotoDesCamps { std::vector<EntiteDeControle> attaquants; std::vector<EntiteDeControle> cibles; }` (les deux camps d'UN sens ; pour l'autre sens, on échange les deux listes). `struct CoupDOpportunite { int idAttaquant; int idCible; }`.

| Fonction | Rôle |
|---|---|
| `bool engage(const EntiteDeControle& a, const EntiteDeControle& b)` | A engage B : zone de A > 0 et `distanceBordABord` <= zone de A. Pas symétrique ; `[[nodiscard]]` |
| `std::vector<CoupDOpportunite> coupsDOpportunite(const PhotoDesCamps& avant, const PhotoDesCamps& apres)` | Coups gratuits d'un sens, d'après les photos du début et de la fin de l'image. Paire (A engage B) présente au début et rompue à la fin, B ayant fui (déplacement projeté sur la droite A→B du début : B s'est écarté au moins autant que A) : A frappe B, sauf si A engage encore quelqu'un à la fin, ou si A ou B est absent de la photo de fin. Au plus un coup par attaquant : le fuyard le plus proche à la fin (bord à bord ; égalité : le premier de `avant.cibles`). Ordre du résultat : celui de `avant.attaquants` ; `[[nodiscard]]` |
| `std::optional<int> monstreDontSeDegager(const EntiteDeControle& personnage, const std::vector<EntiteDeControle>& personnages, const std::vector<EntiteDeControle>& monstres)` | Règle de départ du dégagement (étape 78). `personnage` sans zone (> 0 : jamais), engagé par au moins un monstre, et CHAQUE monstre qui l'engage engage aussi un AUTRE personnage de `personnages` (identifiant différent ; lui-même y figure sans compter). Renvoie l'identifiant du plus proche des monstres qui l'engagent (bord à bord ; égalité : le premier) ; vide sinon. Ne connaît ni l'ancre, ni les skills, ni l'état de dégagement : c'est l'affaire d'`Escouade::degagementVoulu` ; `[[nodiscard]]` |

### `Cible` (`combat/`) — ce que vise un skill

`Cible.hpp`, sans `.cpp` : `struct CibleAllie { std::size_t place; }`
(de 1 à N), `struct CibleMonstre { int id; Cercle cercle; }`,
`struct CiblePoint { sf::Vector2f point; }` (point fixé au clic, pour un
skill de zone en attente) et
`using Cible = std::variant<CibleAllie, CibleMonstre, CiblePoint>`. L'absence de
cible est un `std::optional<Cible>` vide. `Cible.hpp` définit aussi
`template<class... Ts> struct Surcharges : Ts... { using Ts::operator()...; }`
pour `std::visit` : une lambda par alternative, et un cas oublié est une
erreur de compilation.

### `EtatSkill` (`combat/`) — le skill d'une place, pour l'affichage

`EtatSkill.hpp`, sans `.cpp` : `struct EtatSkill { IdSkill id; TypeSkill type; float progression; float tempsRestant; bool lanceurVivant; bool armeAbsente; bool enActivation; }`. `armeAbsente` (étape 91.5) : le skill exige un type d'arme que le lanceur ne porte pas ; l'icône est grisée. `progression` va de 0 (juste lancé) à 1 (prêt), `tempsRestant` est en secondes, `enActivation` est vrai pendant l'activation du skill. Produite par `Escouade::etatDuSkill`, consommée par `BarreDeSkills`.

### `BarreDeVie` (`rendu/`) — la barre de vie d'une entité

Deux fonctions libres, sans état, utilisées par `Personnage::dessiner` et
`Monstre::dessiner`, et une constante `s_hauteurBarreDeVie` (5 px).

| Fonction | Rôle |
|---|---|
| `sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille)` | Coin haut-gauche de la barre d'une entité de ce centre et de cette taille : alignée sur son bord gauche, 6 px au-dessus de son carré |
| `void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio)` | Fond gris, puis rectangle vert proportionnel à `ratio` (0 à 1) |

### `Eclaircir` (`base/`) — éclaircir et assombrir une couleur

Deux fonctions libres, sans état, et une constante. `eclaircir` est l'indice visuel d'un coup en préparation (`Personnage::dessiner`, `Monstre::dessiner`) ; `assombrir` donne les nuances des doublons de classe (`composer`). `inline constexpr float s_eclaircissementExecution = 0.5f` : la part de blanc pendant l'exécution d'un coup ou l'activation d'un skill, la même règle visuelle dans les deux camps, écrite une fois depuis l'étape 82.1 (auparavant une constante dans chacun des deux `.cpp`).

| Fonction | Rôle |
|---|---|
| `sf::Color eclaircir(sf::Color couleur, float fraction)` | La couleur mélangée au blanc à raison de `fraction` (0 = inchangée, 1 = blanc) ; l'opacité est conservée ; `[[nodiscard]]` |
| `sf::Color assombrir(sf::Color couleur, float fraction)` | La couleur mélangée au noir à raison de `fraction` (0 = inchangée, 1 = noir) ; l'opacité est conservée ; `[[nodiscard]]` |

### `Aleatoire` (`base/`) — le générateur aléatoire

`Aleatoire.hpp`, sans `.cpp` : `using Generateur = std::mt19937;` et, depuis l'étape 97.1, `tirerEntre(Generateur&, min, max)`
(`inline`, `[[nodiscard]]` : un entier uniforme dans [min, max], bornes incluses, `std::uniform_int_distribution<int>`),
l'unique définition qu'utilisent la génération des donjons et celle des objets (elle existait en double ; la suite des tirages
n'a pas changé). Le reste
du code écrit `Generateur` et ne dépend pas de l'algorithme. Possédé par
`Partie` (un seul, initialisé par une graine), passé PAR RÉFÉRENCE le long
de la chaîne `Partie` → `Escouade` → `Personnage` → `Skill` et `Partie` →
`Monstre` → `Skill` (tirage des recharges) et, depuis l'étape 89, `Partie` → `Escouade` / `Monstre` →
`Personnage` / `Vitalite` → `resoudreCoup` (tirage du critique) ; jamais copié, jamais gardé, jamais global.

### `Caracteristiques` (`combat/`) — les cinq caractéristiques (étape 88)

`Caracteristiques.hpp` / `.cpp`. Ne dépend de rien d'autre.

- `enum class Caracteristique { Puissance, Endurance, Adresse, Savoir, Volonte }`, `s_nombreDeCaracteristiques`
  (5) et `s_caracteristiques` (le tableau de toutes, pour les parcourir).
- `class Caracteristiques` (type valeur, `constexpr`) : un `std::array<int, 5>` indexé par l'enum.
  `Caracteristiques()` (toutes à 0), `explicit Caracteristiques(std::array<int, 5>)`, `static seule(Caracteristique, int)`,
  `valeur(Caracteristique)`, `+=`, `+`, `==` ; `[[nodiscard]]`.
- `class Coefficients` : la même forme en flottants (`seul`, `valeur`, `==`), pour les skills.
- Constantes : `s_valeurDeReference` (10), `s_bonusParPoint` (0,03), `s_pointsDeVieDeBase` (240) et
  `s_pvParPointDEndurance` (6) depuis l'étape 91.2 (avant : 20 par point, base par classe), `s_pvParNiveau` (8, étape 99.4).
- `int pointsDeVieMax(const Caracteristiques&, int niveau)` : 240 + 6 × Endurance + 8 × (niveau − 1) ; `assert(niveau >= 1)` et
  `assert(résultat > 0)` ; `[[nodiscard]]`. Le niveau est celui de l'escouade (`Personnage::getNiveau`).

### `Progression` (`combat/`) — la courbe de l'expérience et la croissance (étape 99)

`Progression.hpp` / `.cpp`. Des fonctions pures, qui ne dépendent que de `Caracteristiques`. Voir `docs/classes-et-combat.md`,
« L'expérience et les niveaux ».

| Élément | Rôle |
|---|---|
| `s_experienceDeLaCourbe` (21) | Le coefficient `k` de la courbe, « à tester » (calibré, `test_CalibrageDeLExperience.cpp`) |
| `int experiencePourAtteindre(int niveau)` | L'expérience cumulée pour atteindre ce niveau : `k × n × (n − 1)`, 0 pour le niveau 1 ; `assert(niveau >= 1)` ; `[[nodiscard]]` |
| `int niveauPour(int experience)` | Le plus grand niveau dont le seuil est atteint (pas de maximum) ; `assert(experience >= 0)` ; `[[nodiscard]]` |
| `float progressionVersLeNiveauSuivant(int experience)` | De 0 (juste arrivé) à moins de 1 : la barre d'expérience ; `[[nodiscard]]` |
| `int experienceManquante(int experience)` | Ce qui manque pour le niveau suivant, au moins 1 (F9) ; `[[nodiscard]]` |
| `using PaireDeCroissance = std::array<Caracteristique, 2>` | Les deux caractéristiques qui grandissent avec le niveau d'une classe, dans l'ordre |
| `Caracteristiques croissanceAuNiveau(int niveau, const PaireDeCroissance&)` | Ce que les niveaux ajoutent à la base d'une classe : +1 par niveau gagné au-delà du premier, la première puis la seconde de la paire en alternance ; rien au niveau 1 ; `assert(niveau >= 1)` ; `[[nodiscard]]` |
| `s_pointsLibresParNiveau` (3) | Étape 100.1 : les points libres qu'un personnage reçoit à chaque niveau gagné au-delà du premier, « à tester » |
| `int pointsLibresRecus(int niveau)` | Les points libres reçus en tout au niveau n : `3 × (n − 1)` ; `assert(niveau >= 1)` ; `[[nodiscard]]` |
| `int pointsLibresDisponibles(int niveau, const Caracteristiques& depenses)` | Ceux qui restent à dépenser : reçus moins `depenses.somme()` ; jamais stockés ; `[[nodiscard]]` |
| `bool depensePermise(int niveau, const Caracteristiques& depenses, const Caracteristiques& depense)` | Aucune valeur de `depense` négative et sa somme au plus égale aux points disponibles ; la règle de `Personnage::depenserDesPoints` ; `[[nodiscard]]` |
- `int montantEffectif(int montant, const Coefficients&, const Caracteristiques&)` : voir plus haut ;
  `assert(montant >= 0)` ; `[[nodiscard]]`.

### `Element` et `Degats` (`combat/`) — les portions d'un coup (étape 94.2)

`Element.hpp` et `Degats.hpp`, sans `.cpp`. Voir « Types de dégâts et résistances (étape 94) ».

- `enum class Element { Physique, Feu, Sacre, Nombre }` : `Nombre` est la sentinelle, pas un élément. `s_nombreDElements`
  (`std::size_t`) et `s_elements` (`std::array<Element, s_nombreDElements>`, dans l'ordre de l'énumération).
- `template <typename T> struct ParElement { std::array<T, s_nombreDElements> valeurs = {}; }` : `operator[](Element)` (const
  et non const), `static constexpr ParElement seul(Element, T)`, `constexpr T total() const` (type numérique),
  `constexpr Element elementDominant() const` (la plus grande valeur ; à égalité, le premier de l'énumération),
  `operator==` par défaut.
- `Degats = ParElement<int>` ; `Portions = ParElement<float>` (le coup en cours de résolution, sans arrondi) ;
  `degatsDe(Element, int)`, `degatsPhysiques(int)`, `enPortions(const Degats&)` : `constexpr`, `[[nodiscard]]`.

### `Defense` (`combat/`) — l'armure

Une fonction libre pure, sans état, utilisée par `Vitalite::recevoirDegats`. Ne dépend d'aucun
autre module ni de SFML.

| Fonction | Rôle |
|---|---|
| `float armureAppliquee(float montant, int armure, int penetration = 0)` | La même armure M1 sans aucun arrondi ni minimum de 1 (étape 94) : ce qu'utilise `resoudreCoup` sur la portion physique ; `appliquerArmure` en est la version entière ; `[[nodiscard]]` |
| `constexpr int resistanceEffective(int pourCent)` | Ramenée entre `s_resistanceMinimale` (−100) et `s_resistanceMaximale` (80) ; `[[nodiscard]]` |
| `float appliquerResistance(float montant, int resistance)` | `montant × (1 − résistance effective / 100)`, sans arrondi ; une résistance négative augmente la portion ; `[[nodiscard]]` |
| `int appliquerArmure(int montant, int armure, int penetration = 0)` | Dégâts réellement subis : `max(montant − armure effective, montant × 20 %)` (modèle M1, `s_plancherArmure`), où l'armure effective est `armure × (1 − penetration / 100)` (pénétration ramenée entre 0 et `s_penetrationMaximale`, 75, sans arrondi intermédiaire), arrondi au plus proche, au moins 1 si `montant > 0`, 0 si `montant == 0` ; `assert(montant >= 0)` et `assert(armure >= 0)` ; `[[nodiscard]]` |

### `PlageDeDegats` (`combat/`) — la plage de dégâts d'une arme (étape 102.3)

`PlageDeDegats.hpp` / `.cpp`. `struct PlageDeDegats { Degats min; Degats max; }` (avec `operator==`), un `Degats` par borne.

| Fonction | Rôle |
|---|---|
| `constexpr PlageDeDegats plageFixe(const Degats&)` | Une plage sans écart (le minimum est le maximum) : les mains nues ; `[[nodiscard]]` |
| `Degats tirerLesDegats(Generateur&, const PlageDeDegats&)` | Pour chaque élément, dans l'ordre de `s_elements`, un entier uniforme entre son minimum et son maximum (`tirerEntre`) ; **aucun tirage quand `min == max`**, donc le `Generateur` n'avance pas pour une plage fixe ; `assert` : le minimum ne dépasse pas le maximum ; `[[nodiscard]]` |

Le tirage a lieu au départ de chaque coup qui utilise l'arme (`Personnage::attaqueDeLArme` et `attaqueDeFrappe`, `Escouade::attaqueDeBase`
pour les coups d'opportunité) ; le résultat passe ensuite à `montantEffectif` (la Puissance, par portion) puis à `resoudreCoup`. Les
gabarits d'armes qui donnent les plages : `docs/objets.md`, « Les gabarits d'armes, le niveau et la qualité ».

### `ResolutionDuCoup` (`combat/`) — le point d'entrée du coup (étape 87.2)

`Defenses { int armure; StatistiquesDeCombat statistiques; }` (initialisation désignée ; `statistiques.capaciteDefensive > 0` ; seules les statistiques défensives sont lues), `ResultatDuCoup { int montant = 0; bool critique = false; bool esquive = false; int bloque = 0; Degats portions = {}; }` (montant : le total après critique, blocage, armure, résistances et arrondi, somme de `portions` ; `esquive` : coup esquivé, montant 0 ; `bloque` : points absorbés, total si `montant == 0` ; `portions` : ce que chaque élément a donné, étape 94 ; le résultat par défaut `{}` est « rien »), et deux fonctions libres, `[[nodiscard]]`, sans état ni SFML :

| Fonction | Rôle |
|---|---|
| `ResultatDuCoup resoudreCoup(const Attaque&, const Defenses&, bool blocagePret, Generateur&)` | Appelée par `Vitalite::recevoirDegats`. Esquive (`coupEsquive`) : un coup esquivé s'arrête ; critique (`coupCritique`, multiplicateur sur toutes les portions) ; blocage (`appliquerBlocage`, portion physique seule) : un blocage total s'arrête, sans armure ; armure (`armureAppliquee`, portion physique) ; résistances (`appliquerResistance`, chaque portion) ; un seul arrondi par portion, au moins 1 au total (voir « Types de dégâts et résistances (étape 94) »). **Le seul tirage du moteur du coup**, au plus un par étape et seulement si la chance est non nulle (esquive nulle ou zone, capacité offensive nulle, pas de bouclier, récupération en cours, pas de portion physique) ; aucun pour un total nul ; `blocagePret` : le blocage n'est pas en récupération ; `assert(montant >= 0)` |
| `ResultatDuCoup resoudreSoin(const Attaque&, Generateur&)` | Étape 89.4 : un soin peut être critique contre `s_capaciteDefensiveDeReference` (100), sans armure ; même règle de tirage. Appelée par `Escouade::effetDuSkill` |

### `PointsDeVie` (`combat/`) — les points de vie d'une entité

Classe à sémantique de valeur, possédée par valeur par `Vitalite` (donc par `Personnage` et
`Monstre`, qui délèguent leurs façades à `Vitalite`).
Ne dépend d'aucun autre module ni de SFML. Invariants : `0 <= valeur <=
maximum`, `maximum > 0`, une valeur à 0 ne remonte plus.

Données membres : `m_valeur`, `m_maximum`.

| Fonction | Rôle |
|---|---|
| `explicit PointsDeVie(int maximum)` | Commence plein ; `assert(maximum > 0)` |
| `void subirDegats(int montant)` | `assert(montant >= 0)` ; valeur bornée à 0 ; un montant de 0 est permis |
| `int soigner(int montant)` | `assert(montant >= 0)` ; valeur bornée au maximum ; sans effet si la valeur est à 0 ; renvoie les points réellement rendus (0 à pleine vie ou pour un mort) |
| `void changerMaximum(int nouveauMaximum)` | `assert(nouveauMaximum > 0)` ; un vivant : la valeur change de la même différence que le maximum, bornée à `[1, nouveauMaximum]` ; un mort reste à 0 |
| `bool estVivant() const` | Valeur `> 0` (`[[nodiscard]]`) |
| `int getValeur() const` / `int getMaximum() const` | Valeur actuelle et maximum |
| `float getRatio() const` | Valeur / maximum |

### `Vitalite` (`combat/`) — points de vie et défenses d'une entité

Classe à sémantique de valeur (étape 82.1, D1 de `docs/audit.md`), possédée par valeur par
`Personnage` et `Monstre` (composition, pas d'héritage), qui lui délèguent leurs façades
(`recevoirDegats`, `estVivant`, `getPointsDeVie`…) : la logique commune aux deux camps (l'armure
puis les points de vie) est écrite une fois, les interfaces des deux classes n'ont pas changé. Les
règles propres à un camp restent dans sa classe (invulnérabilité et régénération du monstre en
retour). L'interface ne contient que ce que `Personnage` et `Monstre` utilisent. `PointsDeVie` et
`Defense` sont inchangés ; `Vitalite` n'absorbera `Defense` que quand les armures varieront. Ne
dépend que de `PointsDeVie` et `Defense`, ni de SFML.

Données membres : `m_pointsDeVie` (`PointsDeVie`), `m_defenses` (`Defenses` : l'armure et les statistiques de
combat, étape 89 ; avant, un `int m_armure`), `m_recuperationDuBlocage` (`float`, secondes, étape 90.3 ; 0 : le blocage est prêt ; lancée à `s_recuperationDuBlocage` depuis l'étape 91.6).

| Fonction | Rôle |
|---|---|
| `Vitalite(int pointsDeVieMax, const Defenses& defenses)` | Commence pleine ; `assert(armure >= 0)` et `assert(capaciteDefensive > 0)` (et `maximum > 0` par `PointsDeVie`) |
| `void definirDefenses(const Defenses& defenses)` | Remplace les défenses (celles d'un personnage sont recalculées depuis son équipement et ses caractéristiques) ; mêmes préconditions ; remplace `definirArmure` |
| `ResultatDuCoup recevoirDegats(const Attaque& attaque, Generateur& generateur)` | `resoudreCoup(attaque, défenses, blocagePret(), générateur)`, puis lance la récupération du blocage si un blocage a eu lieu, puis `PointsDeVie::subirDegats` ; renvoie le résultat (dégâts après critique, blocage et armure, non plafonnés par les points de vie restants ; critique, esquivé, bloqué) ; rien (montant 0, aucun tirage) pour un mort |
| `void mettreAJour(float dt)` | Fait couler la récupération du blocage (sans descendre sous 0) ; appelée par `Personnage::mettreAJour` (vivant) et `Monstre::mettreAJour` |
| `bool blocagePret() const` | La récupération est écoulée ; `[[nodiscard]]` |
| `void definirMaximum(int maximum)` | `PointsDeVie::changerMaximum` (précondition `maximum > 0`) |
| `int soigner(int montant)` | `PointsDeVie::soigner` : plafonné au maximum, 0 pour un mort ; renvoie les points réellement rendus |
| `bool estVivant() const` | Points de vie `> 0` ; `[[nodiscard]]` |
| `int getValeur() const` / `int getMaximum() const` | Points de vie actuels et maximum ; `[[nodiscard]]` |
| `float getRatio() const` | Valeur / maximum, pour la barre de vie ; `[[nodiscard]]` |

### `Cercle` (`base/`) — la forme d'une entité

Une structure et des fonctions libres, sans comportement.

Données membres : `struct Cercle { sf::Vector2f centre; float rayon; }`.

| Fonction | Rôle |
|---|---|
| `Cercle cercleDuRectangle(sf::Vector2f coin, sf::Vector2f taille)` | Cercle de collision d'une entité rectangulaire de coin haut-gauche `coin` : centré sur le rectangle, de rayon la demi-largeur (étape 82.1) ; seul endroit où cette formule est écrite (`Mobile::getCercle`, `deplacerAvecGlissement`) ; `[[nodiscard]]` |
| `float distanceBordABord(const Cercle& a, const Cercle& b)` | Distance entre les centres moins les deux rayons ; 0 si les cercles se touchent ou se chevauchent (jamais négative) |
| `float distanceCercleRectangle(Cercle cercle, sf::FloatRect rectangle)` | Écart bord à bord entre le cercle et le rectangle plein ; 0 en cas de contact, de chevauchement ou de centre dedans, jamais négatif ; `[[nodiscard]]` |
| `float distancePointSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b)` | Distance du point au segment `[a, b]` (extrémités comprises, segment réduit à un point permis) ; `[[nodiscard]]` |

### `AffichageStatistiques` (`interface/`) — les statistiques de combat à l'écran (étape 89.5)

`AffichageStatistiques.hpp` / `.cpp` : `std::vector<std::string> lignesDeStatistiques(const StatistiquesDeCombat&)`
(`[[nodiscard]]`), UTF-8, cinq lignes dans l'ordre où `DispositionInventaire` remplit le bloc des statistiques :
« Offensive Ability N », « Defensive Ability N », « Block 30 % (12) » (« Block 0 % » sans bouclier),
« Melee Dodge N % » et « Projectile Dodge N % » (esquives effectives, plafond compris) ; et
`std::vector<std::string> lignesDeBonusDeCombat(const StatistiquesDeCombat&)` (`[[nodiscard]]`), une ligne par bonus
non nul pour l'infobulle d'un objet (« Dodge vs Melee: +5 % »… ; plus de ligne de blocage depuis l'étape 91.6). Comme `AffichageCaracteristiques`, les mots sont dans
l'interface, le noyau ne connaît que les nombres.

### `AffichageSkills` (`interface/`) — les noms des skills

`AffichageSkills.hpp` / `.cpp` : `std::string nomDuSkill(IdSkill)` (`[[nodiscard]]`), `switch` sans `default` (un skill ajouté sans nom fait avertir le compilateur), UTF-8, en anglais depuis l'étape 95 : « Power Strike », « Taunt », « Heal », « Explosion », « Holy Strike », « Lay on Hands », « Piercing Shot », « Rain of Arrows ». Le noyau ne connaît que `IdSkill` ; comme `AffichageObjets`, les mots sont dans l'interface.
