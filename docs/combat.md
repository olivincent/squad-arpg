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
- **Valeurs de portée** : mêlée 10 px, tireur 120 px, monstre 15 px,
  skill = portée de mêlée.
- **Ce qui passe par `Attaque::estAPortee`** : la frappe des personnages,
  la riposte des monstres, l'arrêt de l'ancre (`Escouade::attaquer`),
  l'arrêt individuel en combat et l'arrêt du monstre en poursuite. Pour
  transporter la taille de la cible, les signatures échangent un `Cercle`
  plutôt qu'une position suivie d'un rayon : `Escouade::attaquer`,
  `personnageLePlusProche`, `Monstre::mettreAJour` et
  `Monstre::frapper`.
- **Restent mesurés de centre à centre**, volontairement : la détection
  et le territoire du monstre, le choix du personnage le plus proche, le
  clic sur un monstre (`contient`), la formation et le lissage du chemin.
- **Limites connues** : les entités sont dessinées comme des carrés et
  leurs coins dépassent du cercle (4,1 px pour un personnage, 6,2 px pour
  un monstre) — face à face, l'écart visible est bien la portée, mais en
  diagonale une portée de mêlée de 10 px laisse à peine un écart. La
  portée ignore les murs : un personnage peut frapper à travers un mur
  s'il est assez près, une vérification de ligne de vue (via
  `TileMap::trajetLibre`, par exemple) restant une option.

## Attaque de l'escouade

- **`Escouade::attaquer(cible, carte)`** mémorise la cible
  (`m_cibleCombat`), puis : si au moins un personnage **vivant** n'est
  pas à portée, `allerVers(cible.centre, carte)` ; sinon l'ancre
  s'arrête sur place (`m_mobile.arreter()`, pour cesser d'avancer sans
  reculer). Dans les deux cas, chaque personnage frappe ensuite et la
  somme des dégâts est renvoyée à `main.cpp`.
- **Les morts sont ignorés pour l'arrêt** (voir `docs/formation.md`).
- **La formation tient pendant l'attaque** : les personnages continuent
  de viser leur place (`placePersonnage`), sauf ceux déjà à portée, qui
  restent sur place.

## Skill

- **Une classe `Skill` autour d'une `Attaque`** : un `TypeSkill` (`Degats`
  ou `Soin`) et l'`Attaque` qui fournit portée, puissance et recharge.
  `Skill` cache le mot « dégâts » derrière un nom neutre, la puissance
  (dégâts infligés ou points de vie rendus selon le type), et son
  `declencher` n'est qu'un `frapper` renommé. Un `enum class` suffit pour
  le type : ni `std::variant` ni polymorphisme tant que les deux types ne
  diffèrent que par ce que fait `Escouade` du résultat.
- **L'absence de skill est un `std::optional<Skill>`**, reçu au
  constructeur de `Personnage` comme l'attaque de base (injection), plutôt
  qu'un skill « vide » à 0 de puissance. Pas de valeur par défaut : chaque
  création de `Personnage` dit explicitement s'il a un skill.
- **Deux skills**, aux valeurs écrites dans `Escouade.cpp` :
  - dégâts : personnage 0 (mêlée avant-gauche), 25 dégâts, recharge 5 s,
    portée de mêlée (10 px) ;
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
- **Le soin est permis hors combat** ; le skill de dégâts exige un
  monstre verrouillé.

## Choix de la cible et skill en attente

- **`Cible`** est un `std::variant<CibleAllie, CibleMonstre>`
  (`CibleAllie` : place de 1 à 4 ; `CibleMonstre` : un `Cercle`). Pas de
  troisième cas « aucune » : l'absence est un `std::optional` vide. Les
  deux cibles n'ont rien de commun, d'où un variant plutôt qu'une classe
  de base.
- **Le type du skill décide de la cible cherchée**, dans
  `Escouade::choisirCible(type, monstreVerrouille)`, une fonction à
  part : `Degats` prend le monstre verrouillé, `Soin` le vivant qui a le
  moins de points de vie parmi les blessés (soigneur inclus ; égalité :
  la plus petite place). Un futur choix manuel par le joueur ne devra
  remplacer que cette fonction.
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
  l'image, avant les déplacements) ; un skill de dégâts, dans
  `attaquer`, avant les attaques de base, qui reçoit le cercle à jour du
  monstre. Ses dégâts s'ajoutent donc à la somme déjà renvoyée à
  `main.cpp`.
- **Annulations** : lanceur mort (les deux types) ; soin : cible morte ou
  remontée à pleine vie ; dégâts : `deverrouiller()` (mort de la cible,
  clic au sol) ou `verrouiller` avec un autre identifiant (changement de
  cible). Re-cliquer sur le même monstre ne l'annule pas. Un ordre de
  déplacement de l'escouade n'annule rien. Une annulation ne coûte aucune
  recharge.
- **Limites connues** : un seul skill en attente à la fois ; pas de choix
  manuel de la cible ; un lanceur bloqué par un monstre glisse ou attend
  (le A* ignore les entités) ; pas d'effet visuel au déclenchement ni
  sur la cible soignée, pas de HUD ni d'icône de skill ; les skills des
  personnages 2 et 4 n'existent pas encore.

## Circuit des dégâts

`Escouade` et `Monstre` ne se connaissent pas : `main.cpp` fait
l'intermédiaire (voir `docs/boucle-et-entrees.md` pour l'ordre dans une
image).

- **Les personnages frappent un monstre** : `main.cpp` retrouve le monstre
  verrouillé et lui applique la somme renvoyée par
  `escouade.declencherSkill(...)` puis `escouade.attaquer(...)` (un
  skill de dégâts parti après une attente s'ajoute à cette dernière somme ;
  un soin ne rend aucun dégât). Le skill
  passe avant l'attaque de base : s'il part, il relance la recharge de
  base de ce personnage, qui ne frappe donc pas deux fois dans la même
  image.
- **Un monstre frappe un personnage** : pour chaque monstre,
  `main.cpp` demande `escouade.personnageLePlusProche(m.getCentre())`
  (le cercle du vivant le plus proche), met le monstre à jour, relit son
  centre, puis applique `escouade.recevoirDegats(origine,
  m.frapper(*cible))`. Les deux appels à `Escouade` désignent le même
  personnage grâce au même critère. La cible est calculée avant le
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
