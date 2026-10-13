# Squad-ARPG — Formation

`Escouade` (l'ancre et ses 4 personnages) et `Personnage` (un membre).
L'attaque, la portée, le skill et les obstacles sont dans
`docs/combat.md` ; le déplacement par chemin de l'ancre dans
`docs/chemin.md`.

## L'ancre et ses personnages

- **État de jeu séparé de l'affichage** : le `sf::RectangleShape` de
  l'ancre n'existe que dans `dessiner`. Même modèle que `TileMap`.
- **Composition avec `Mobile`** : la position, le chemin, la vitesse et
  la collision/glissement de l'ancre vivent dans `m_mobile` (voir
  `docs/chemin.md`), auquel `Escouade` délègue (`allerVers`, `getCentre`,
  l'arrêt dans `attaquer`, le déplacement dans `mettreAJour`). Les
  valeurs fixes (taille 20 × 20, vitesse 180 px/s, rotation 360°/s)
  sont écrites dans le constructeur ; elles deviendront des paramètres
  quand il le faudra.
- **La carte n'est jamais gardée** : elle est reçue en `const TileMap&`
  par `allerVers`, `attaquer` et `mettreAJour`, et transmise à
  `m_mobile`. `Escouade.hpp` ne contient que `class TileMap;`, et
  `Escouade.cpp` n'inclut ni `TileMap.hpp`, ni `Chemin.hpp`, ni
  `Deplacement.hpp` : ces dépendances sont celles de `Mobile`.
- **En-têtes SFML précis** dans `Escouade` (`Vector2.hpp`,
  `RenderTarget.hpp`, `RectangleShape.hpp`…), alors que `TileMap.hpp`
  inclut tout `<SFML/Graphics.hpp>`. Non harmonisé pour l'instant.
- **`explicit`** sur le constructeur à un paramètre, pour interdire la
  conversion implicite `sf::Vector2f` → `Escouade`.
- **Noms d'intention** : `allerVers` plutôt que `setCible` ; `getCentre`
  encapsule le calcul « coin haut-gauche + moitié de la taille », pour que
  `main.cpp` ne dépende pas de la convention de position.
- **`Escouade` possède les personnages** (`std::vector<Personnage>` par
  valeur, d'où l'include complet de `Personnage.hpp`) et les décalages
  de formation ; l'ancre reste le point de contrôle dessiné (le carré
  vert). Elle est dessinée et mise à jour avant les personnages, qui
  visent la place de l'image *courante*, calculée une fois l'ancre
  déplacée.
- **`Escouade` ne connaît pas `Monstre`** : elle échange des `Cercle`
  (la cible de `attaquer`, la proie renvoyée par `proieDuMonstre`)
  et un identifiant entier pour le verrou. C'est `main.cpp` qui fait
  l'intermédiaire (voir `docs/boucle-et-entrees.md`).

## Formation

- **`placePersonnage(indice)` : un seul point de calcul** pour la place
  d'un personnage (`getCentre() + m_decalages[indice].rotatedBy(
  m_orientation)`), utilisé à la fois dans le constructeur et dans
  `mettreAJour` — pour ne pas dupliquer la formule à deux endroits qui
  pourraient diverger.
- **Places repliées (étape 38)** : `placePersonnage` donne la place
  *théorique*. Dans `mettreAJour`, elle passe par `replierVersOrigine`
  (ancre comme origine, taille du personnage prise sur son cercle :
  `2 × rayon`, ce qui évite d'ajouter un accesseur à `Personnage`). Le
  résultat est rangé dans `m_places`, utilisé comme point voulu, et lu par
  `dessiner` (qui n'a pas la carte) et par `getPlacesDeFormation`
  (tests). Le constructeur y met les places non repliées, remplacées au
  premier `mettreAJour`. Répartition inchangée : l'escouade décide *où*,
  le personnage *comment*. Seules les places de formation sont
  corrigées : les cibles individuelles en combat et la marche vers un
  skill en attente ne le sont pas.
- **Quatre décalages** `{{25,-25}, {25,25}, {-25,-25}, {-25,25}}` pour
  une escouade orientée vers `+x` : les indices 0 et 1 sont à l'avant,
  les indices 2 et 3 à l'arrière.
- **Le rôle découle de la place** : les deux personnages avant sont des
  mêlées (rouge, portée de bord à bord 20 px), les deux arrière des
  tireurs (cyan, 120 px). Aucune énumération ni champ « type » : un
  personnage est défini par ses données (couleur, attaque de base, skill) et
  la place dans `m_personnages` suffit à dire qui est quoi, comme elle
  suffit déjà à l'associer à son décalage.
- **Injection** : `Escouade` choisit les valeurs (constantes nommées
  dans un espace de noms anonyme d'`Escouade.cpp`) et les passe au
  constructeur de chaque `Personnage` (couleur, attaque de base, skill
  éventuel). Deux personnages ayant la même couleur, la même attaque et
  le même skill seraient indiscernables, et c'est voulu.
- **Orientation voulue et orientation réelle séparées** :
  `m_orientationVoulue` reçoit la direction voulue de l'ancre
  (`m_mobile.directionVoulue()`, lue *avant* l'essai de collision : si
  l'escouade est bloquée par un mur et glisse, la formation vise quand
  même la direction qu'elle essayait de suivre). Mise à jour seulement
  si l'ancre a un point à atteindre et que la distance est `> 0`
  (`angle()` d'un vecteur nul n'a pas de sens), sinon la dernière valeur
  est conservée. `m_orientation` est ce qu'utilise `placePersonnage`.
- **Rotation progressive, par le plus court côté** : `m_orientation`
  se rapproche de `m_orientationVoulue` à chaque `mettreAJour`, que
  l'ancre bouge ou non (si elle s'arrête en pleine rotation, la
  formation finit de pivoter sur place). L'écart,
  `(m_orientationVoulue - m_orientation).wrapSigned()`, est ramené dans
  `[-180°, 180°]` pour que passer de 350° à 10° tourne de +20° et non de
  -340°. La rotation par image est bornée à `m_vitesseRotation * dt` ;
  si l'écart restant est inférieur ou égal à ce pas, `m_orientation`
  prend exactement la valeur voulue (même logique que le déplacement à
  pas fixe).
- **Limites connues** :
  - pas de collision entre personnages, ni entre un personnage et
    l'ancre ;
  - l'ancre traverse les monstres alors que les personnages sont
    bloqués par eux : elle (et la caméra) peut donc s'éloigner des
    personnages bloqués ;
  - le chemin d'un personnage ignore les monstres : bloqué par un
    monstre, il glisse ou attend ;
  - deux places peuvent se replier presque au même endroit dans un
    passage étroit (les personnages ne se bloquent pas entre eux) ;
  - **chevauchement résiduel lors d'un demi-tour rapide** : les 4
    décalages forment deux paires diagonales exactement opposées, donc
    les places *idéales* de deux personnages ne se chevauchent jamais
    pendant la rotation. Mais chaque personnage poursuit sa cible en
    ligne droite, pas en suivant l'arc : lors d'un demi-tour très
    rapide, il coupe à travers le centre de la formation pendant qu'un
    autre fait le trajet inverse. La rotation progressive réduit
    nettement le phénomène sans l'éliminer ; le corriger demanderait une
    collision personnage-personnage ou un déplacement qui suit l'arc.

## Le personnage

- **Un personnage sait seulement « aller vers un point »** : il reçoit
  sa cible en paramètre de `mettreAJour`, sans connaître la formation,
  son décalage ni l'escouade. C'est `Escouade` qui décide du point, ce
  qui permet qu'il soit différent de la place en formation (en combat,
  voir plus bas) sans changer l'interface de `Personnage`.
- **Il possède un `Mobile`** (étape 39), comme `Monstre` : composition,
  pas héritage. La position (le centre) et la vitesse vivent dans le
  `Mobile` ; `Personnage` garde `m_taille` pour le dessin et le cercle.
- **Règle hybride, décidée dans `Personnage::mettreAJour`** : `Escouade`
  décide *où*, `Personnage` *comment*. Si le trajet direct de sa
  position à sa cible est libre (`trajetLibre`, avec sa taille), il
  avance en ligne droite (`allerToutDroit`) et tout chemin en cours est
  abandonné. Sinon il suit un chemin A* lissé (`allerVers`). Même règle
  pour toutes les cibles (place de formation, placement en combat,
  marche vers un skill) : aucun cas particulier.
- **Recalcul limité** : tant que le trajet direct est bloqué, le chemin
  n'est recalculé que s'il n'y en a pas (`m_cibleChemin` vide), s'il est
  fini (`directionVoulue()` vide), ou si la cible est à plus d'une case
  (`s_seuilRecalcul`, 40 px) de la cible du dernier A*. `m_cibleChemin`
  mémorise la cible donnée à `allerVers`, ce qui évite d'ajouter un
  accesseur à `Mobile` pour lire la fin du chemin. Une cible qui bouge de
  moins d'une case laisse donc le personnage finir son chemin ; s'il
  l'a fini sans que le trajet soit libre, il recalcule.
- **Repli en ligne droite** : `allerToutDroit` passe avant `allerVers`
  lors d'un recalcul. Si le A* ne trouve aucun chemin (cible dans un mur,
  zone fermée), `allerVers` laisse le chemin intact et le personnage
  garde la ligne droite. La cible est quand même mémorisée : sans cela,
  un A* voué à l'échec relancerait à chaque image toute la recherche.
- **Obstacles inchangés** : les cercles des monstres bloquent le
  personnage dans les deux modes (`Mobile::mettreAJour`) ; le A* les
  ignore.
- **240 px/s** (contre 180 px/s pour l'ancre), en ligne droite comme sur
  un chemin : un personnage retardé par un mur peut rattraper sa place.
- **`Personnage.hpp` ne contient que `class TileMap;`** : la carte est
  seulement transmise au `Mobile` et à `trajetLibre`, jamais gardée en
  membre.
- **Points de vie et barre de vie** : 100 points, `recevoirDegats`
  (jamais sous zéro), `estVivant()`. Barre dessinée par `Personnage`
  lui-même (fond sombre + rectangle vert proportionnel, au-dessus du
  carré), identique à celle de `Monstre`. La duplication avec `Monstre`
  est assumée : aucune factorisation tant que le besoin ne se confirme
  pas.
- **Il délègue son attaque à `m_attaque`** et son skill (un
  `std::optional<Skill>`) à `m_skill` (voir `docs/combat.md`). La règle
  « un mort ne frappe pas » vit dans `Personnage::frapper` et
  `declencherSkill`, avant la délégation : être vivant est l'affaire de
  `Personnage`, pas de l'attaque. Il expose `typeSkill`, `skillEstPret`,
  `skillEstAPortee` et `zoneDuSkill` (cercle centré sur lui, au rayon de
  zone du skill), `cercleDePorteeDuSkill` (rayon du personnage + portée du skill, pour la visée) pour qu'`Escouade` décide quoi faire du skill.
- **Soin** : `recevoirSoin` plafonne au maximum de points de vie et ne
  fait rien sur un mort ; `getPointsDeVie` et `getPointsDeVieMax`
  permettent à `Escouade` de choisir le plus blessé.
- **Cercle** : `getCercle()` renvoie un cercle centré sur le personnage,
  de rayon la moitié du côté (10 px), calculé à la demande.

## Choix du personnage le plus proche

- **Un seul endroit décide** : la fonction privée
  `indiceProieDuMonstre(id, position)` renvoie le provocateur du monstre
  s'il est provoqué et vivant (voir « Provocation » dans
  `docs/combat.md`), sinon `indicePersonnageLePlusProche(point)` (boucle
  sur les personnages **vivants**, distances de centre à centre).
  `proieDuMonstre` (qui renvoie le `Cercle`) et
  `recevoirDegats(idMonstre, origine, quantite)` s'en servent, pour
  qu'ils désignent toujours le même personnage par la même règle, même
  appelés séparément par `main.cpp`.
- **L'absence de vivant est exprimée dans le type** : l'indice et le
  cercle sont des `std::optional` (vide = aucun vivant) plutôt qu'une
  valeur sentinelle ; `recevoirDegats` ne fait rien si l'indice est
  vide.
- **`Escouade` ne filtre pas les morts dans `attaquer`** pour la frappe :
  elle appelle `frapper` pour chaque personnage qui a une cible et c'est
  `Personnage` qui renvoie 0 s'il est mort. Elle les ignore en revanche pour décider de
  s'arrêter (voir « Mort et défaite »).

## Verrou de cible

- **`m_cibleVerrouillee` (`std::optional<int>`, vide = pas de combat)** :
  l'identifiant du monstre engagé, jamais un indice, un pointeur ni un
  itérateur (un monstre peut être retiré du vecteur entre deux images).
  `verrouiller(id)`, `deverrouiller()` et `getCibleVerrouillee()`.
- **`Escouade` ne décide de rien elle-même** : elle ne pose ni ne retire
  le verrou de sa propre initiative. C'est `main.cpp` qui appelle ces
  fonctions en réponse aux clics et qui retrouve le monstre à partir de
  l'identifiant (voir `docs/boucle-et-entrees.md` pour les trois cas qui
  rompent le combat).
- **Le combat n'est pas lié au bouton de souris** : tant que le verrou
  est posé, `attaquer` approche le monstre verrouillé à chaque image.
  `main.cpp` appelle `attaquer` à chaque image même sans verrou, pour
  l'autodéfense (voir `docs/combat.md`), mais l'approche, elle, exige le
  verrou.

## Placement individuel en combat

- **La règle d'arrêt de l'ancre ne change pas** (tous les vivants à
  portée, voir `docs/combat.md`), mais une règle individuelle s'y ajoute
  dans `Escouade::mettreAJour` : si l'escouade est en combat (verrou
  posé et `m_cibleCombat` renseignée), un personnage vivant à portée de
  la cible reçoit **son propre centre** comme point à atteindre au lieu
  de `placePersonnage(i)`. Il cesse d'avancer, continue de frapper, et
  recommence à suivre sa place dès que la cible n'est plus à portée ou
  que le verrou tombe. Les tireurs s'arrêtent donc à distance pendant
  que les mêlées continuent jusqu'au contact.
  Depuis l'étape 46, « à portée » veut dire à portée ET en vue
  (`Escouade::peutAtteindre`, voir `docs/combat.md`) : un personnage à
  portée mais sans vue continue d'avancer.
- **`Personnage` n'en sait rien** : il reçoit toujours un simple point.
- **`m_cibleCombat` (`std::optional<Cercle>`)** est le seul moyen pour
  `Escouade` de connaître la position et la taille de la cible : elle
  est mise à jour à chaque `attaquer` où le monstre verrouillé est dans
  la liste des monstres.
- **La règle exige le verrou, pas seulement une cible connue** :
  `m_cibleCombat` n'est pas effacée quand le verrou tombe, et `main.cpp`
  appelle `attaquer` à chaque image même sans verrou. Sans cette double
  condition, la dernière cible resterait mémorisée et figerait les
  personnages qui étaient à sa portée. Un test unitaire le vérifie.
- **La taille de la cible compte pour les mêlées**, à cause de la
  formation : un mêlée est à 35,4 px du centre de l'ancre (décalage
  `(25, ±25)`), dont 25 px de côté par rapport à l'axe d'approche. Face
  à une cible de rayon nul, il ne passe donc jamais à moins d'environ
  25 px de son centre. Avec la portée de 10 px d'avant l'étape 37, il
  lui en fallait 20 (portée 10 + son rayon 10) : la condition « tous à
  portée » ne devenait jamais vraie et l'ancre avançait jusque sur la
  cible. Avec la portée de 20 px, le seuil est de 30 px, ce qui est
  atteignable même face à un point ; avec un vrai monstre (rayon 15) il
  est de 45 px. Les tests de combat d'`Escouade` utilisent quand même
  une cible de la taille d'un monstre (voir `docs/tests.md`).
- **Limites connues** : pas de placement « intelligent » (encerclement,
  écartement, ligne de tir) — un personnage à portée s'arrête exactement
  là où il se trouvait en entrant en portée, même si la position est
  maladroite ; pas de recul si un monstre s'approche d'un tireur arrêté ;
  pas de `Mobile` individuel.

## Lanceur d'un skill en attente

- **Priorité du point visé** dans `Escouade::mettreAJour` : la place en
  formation, remplacée par le centre du personnage s'il est en combat et
  à portée de la cible, remplacée à son tour par le centre de la cible de
  son skill s'il est le lanceur d'un skill en attente. `Personnage` reçoit
  toujours un simple point.
- **`avancerSkillEnAttente()`** (privée) est appelée une fois par image,
  avant la boucle des personnages : elle annule l'attente si besoin, fait
  partir un soin dès que le lanceur est à portée, et renvoie le point à
  viser. Le skill de dégâts, lui, part dans `attaquer` (voir
  `docs/combat.md`) ; `avancerSkillEnAttente` ne fait alors que renvoyer
  `m_cibleCombat->centre`, ou rien tant que `attaquer` n'a pas été appelée
  avec le monstre verrouillé dans la liste.
- **Annulation sur rupture du combat** dans `verrouiller` et
  `deverrouiller`, plutôt que par une vérification à chaque image : ce
  sont les deux seuls endroits où le verrou change.

## Déplacement vers un agresseur (étape 40)

- **Priorité du point visé** dans `Escouade::mettreAJour`, du plus faible
  au plus fort : la place en formation ; le point de l'agresseur (ou le
  centre du personnage s'il a déjà un monstre à portée), seulement si
  l'ancre est à l'arrêt ; le centre du personnage s'il est à portée du
  monstre verrouillé ; la cible du skill en attente. Le détail et le
  pourquoi sont dans `docs/combat.md` (« Agresseurs »).
- **« Ancre à l'arrêt »** = `m_mobile.directionVoulue()` vide, lu après le
  déplacement de l'ancre dans la même image.
- **`Personnage` n'en sait rien** : il reçoit toujours un simple point.

## Mort d'un personnage et défaite

- **Un mort reste dans `m_personnages`**, contrairement à `Monstre` que
  `main.cpp` retire du vecteur. L'indice `i` d'un personnage désigne
  aussi son décalage (`m_decalages[i]`) : retirer un élément décalerait
  les indices et associerait chaque survivant au décalage d'un autre.
  La formation ne se resserre donc pas, la place du mort reste vide.
- **« Un mort ne bouge pas » vit dans `Personnage::mettreAJour`** :
  retour immédiat si `!estVivant()` (ni déplacement, ni recharge).
  `Escouade::mettreAJour` appelle les 4 personnages sans distinction :
  `Personnage` sait quoi faire de son propre état.
- **`Escouade::attaquer` ignore les morts pour décider de s'arrêter** :
  la condition d'approche ne porte que sur les vivants. Sinon un mort
  immobile resté loin derrière empêcherait « tous à portée » de devenir
  vrai et l'escouade avancerait indéfiniment. S'il n'y a aucun vivant,
  « tous les vivants sont à portée » est vrai par vacuité : l'escouade
  s'arrête, ce qui est le comportement voulu.
- **Apparence** : carré gris (`sf::Color(100, 100, 100)`, valeur fixe ;
  `m_couleur` est conservée mais ignorée) et pas de barre de vie ni de
  barre de skill.
- **Un mort n'est plus ciblé** (filtre de
  `indicePersonnageLePlusProche`) **et ne bloque personne**
  (`cerclesDesVivants`).
- **`estVivante()` est une simple lecture** (au moins un personnage
  vivant) : `Escouade` ne décide de rien en cas de défaite, c'est
  `main.cpp` qui bloque les ordres du joueur (voir
  `docs/boucle-et-entrees.md`). Les monstres, eux, rentrent à leur poste
  comme chaque fois qu'ils perdent leur proie
  (`proieDuMonstre` renvoie vide), sans code propre à la
  défaite.
- **Limites connues** : un mort n'est jamais retiré ni remplacé ; pas de
  résurrection (le soin ne s'applique pas aux morts) ; pas d'écran de défaite ni de moyen de
  recommencer ; l'ancre ne s'arrête pas à la mort du dernier
  personnage (si elle avait un trajet en cours, elle le termine).
