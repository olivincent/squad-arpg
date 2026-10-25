# Squad-ARPG — Formation

`Escouade` (l'ancre et ses N personnages, 3 par défaut depuis l'étape 74) et `Personnage` (un membre).
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
  `m_mobile`. `Escouade.hpp` ne contient que `class TileMap;`. `Escouade.cpp`
  inclut `TileMap.hpp` (`declencherSkillDeZone` refuse un point dans un mur :
  `caseContenant`, `getTile`), mais ni `Chemin.hpp` ni `Deplacement.hpp` : ces
  dépendances sont celles de `Mobile`.
- **En-têtes SFML précis** dans `Escouade` (`Vector2.hpp`,
  `RenderTarget.hpp`, `RectangleShape.hpp`…), alors que `TileMap.hpp`
  inclut tout `<SFML/Graphics.hpp>`. Non harmonisé pour l'instant.
- **`explicit`** sur le constructeur à un paramètre, pour interdire la
  conversion implicite `sf::Vector2f` → `Escouade`.
- **Noms d'intention** : `allerVers` plutôt que `setCible` ; `getCentre`
  encapsule le calcul « coin haut-gauche + moitié de la taille », pour que
  `main.cpp` ne dépende pas de la convention de position.
- **`Escouade` possède les personnages** (`std::vector<Personnage>` par
  valeur, d'où l'include complet de `Personnage.hpp`), sa `Formation`
  (décalages, orientation et places, depuis l'étape 82.2) et ses `PlacesDAttaque` (depuis
  l'étape 82.3) ; l'ancre reste le point de contrôle dessiné (le carré
  vert). Elle est dessinée et mise à jour avant les personnages, qui
  visent la place de l'image *courante*, calculée une fois l'ancre
  déplacée.
- **`Escouade` ne connaît pas `Monstre`** : elle échange des `Cercle`
  (la cible de `attaquer`, la proie renvoyée par `proieDuMonstre`)
  et un identifiant entier pour le verrou. C'est `main.cpp` qui fait
  l'intermédiaire (voir `docs/boucle-et-entrees.md`).

## Formation

- **Classe `Formation` (étape 82.2, `escouade/Formation`)** : la formation est sortie
  d'`Escouade` (premier pas de R1, `docs/audit.md`), qui la possède **par valeur**
  (`m_formation`), sans héritage. `Formation` possède les décalages, l'orientation voulue et
  courante, le point à regarder, la vitesse de rotation et les places. Elle ne connaît ni
  `Personnage` ni `Escouade` : elle reçoit le centre de l'ancre, sa direction de marche (vide à
  l'arrêt), les rayons des personnages, la carte et les cercles des monstres. Elle se teste donc
  seule, avec ses propres valeurs (`test_Formation.cpp`). `Escouade` garde l'ancre (`Mobile`) et
  l'orchestration ; aucune règle n'a changé.
- **Un seul appel par image (étape 82.3)** : `mettreAJour(dt, centre, directionDeMarche, rayons,
  carte, monstres)`, *après* le déplacement de l'ancre : orientation voulue, rotation, puis places
  calculées autour du nouveau centre. `Escouade` lit la direction de marche
  (`m_mobile.directionVoulue()`) dans une variable locale *avant* de déplacer l'ancre (la direction
  voulue est lue avant l'essai de collision, voir plus bas). `choisirOrientation` est devenue
  privée : plus aucun ordre d'appel n'est imposé à qui utilise `Formation`. À l'étape 82.2, il y
  avait deux appels : `choisirOrientation(direction, centre)` avant le déplacement de l'ancre, puis
  `mettreAJour` après.
- **Pourquoi le résultat est identique** : le centre ne compte que quand l'ancre est à l'arrêt
  (direction vide), et alors il n'a pas bougé. Vérifié dans `Mobile` : `directionVoulue()` est vide
  quand le chemin est fini ou que le point courant est exactement le centre ; dans les deux cas,
  `Mobile::mettreAJour` calcule un déplacement nul, et `deplacerAvecGlissement` rend la même
  position. Toute la suite est restée verte ; les tests de `Formation` ont seulement changé la forme
  de leurs appels (`choisirOrientation(d, c)` puis `mettreAJour` devient un `mettreAJour` qui reçoit
  `d`), avec les mêmes scénarios et les mêmes vérifications.
- **Le point à regarder s'efface par une fonction explicite** (`oublierPointARegarder`), appelée
  par `Escouade` à chaque nouvel ordre à l'ancre (voir « Point à regarder ») ;
  `Escouade::regarderVers` délègue à `Formation::regarderVers`.
- **`getPlacesDeFormation` reste une façade d'`Escouade`** (`m_formation.getPlaces()`) ; son sort
  se décidera en 82.5 (D3).
- **`placeTheorique(indice, centre)` : un seul point de calcul** pour la place
  théorique d'un personnage (`centre + m_decalages[indice].rotatedBy(
  m_orientation)`, fonction privée de `Formation`), utilisé à la fois dans le constructeur et dans
  `calculerPlaces` — pour ne pas dupliquer la formule à deux endroits qui
  pourraient diverger (avant l'étape 82.2 : `Escouade::placePersonnage`).
- **Places repliées (étape 38)** : `placeTheorique` donne la place
  *théorique*. Dans `calculerPlaces`, elle passe par `replierVersOrigine`
  (ancre comme origine, taille du personnage prise sur son rayon :
  `2 × rayon` ; `Escouade` passe le rayon du cercle de chaque personnage, ce qui évite d'ajouter
  un accesseur à `Personnage`). Le
  résultat est rangé dans `Formation::m_places`, utilisé comme point de la règle 5 (`AllerAuPoint`,
  voir « Intentions »), comme centre de la laisse des agresseurs (`Escouade::situationDe`), et lu par
  `getPlacesDeFormation`
  (tests) ; `dessiner` ne le lit plus. Le constructeur de `Formation` y met les places non repliées
  (les personnages y naissent), remplacées au premier `mettreAJour`. Répartition inchangée : l'escouade décide *où*,
  le personnage *comment*. Seules les places de formation sont
  corrigées : les cibles individuelles en combat et la marche vers un
  skill en attente ne le sont pas.
- **Décalages calculés par `decalagesDeFormation`** (module `Composition`, étape 74) pour
  une escouade orientée vers `+x` : voir « Composition et escouade à N personnages » plus bas.
  Avec la composition par défaut (un guerrier devant, un prêtre et un mage derrière) :
  `{{25,0}, {-25,-25}, {-25,25}}` ; l'indice 0 est le guerrier.
- **Le rôle découle des données** : couleur, attaque de base, skills et rang d'un personnage
  sont ceux de sa `DefinitionPersonnage`. Aucune énumération « type de personnage » ; la place
  dans `m_personnages` suffit à dire qui est quoi, comme elle suffit à l'associer à son décalage.
- **Injection** : `Escouade` ne choisit plus aucune valeur. Elle reçoit sa composition
  (`std::vector<DefinitionPersonnage>`) et construit un `Personnage` par définition. Les
  valeurs d'équilibrage (portées, équipement de départ, skills) vivent dans `Specialite.cpp` (depuis
  l'étape 76 ; avant, dans `Composition.cpp`).
- **Orientation voulue et orientation réelle séparées** (membres de `Formation`) :
  `m_orientationVoulue` reçoit la direction voulue de l'ancre
  (`m_mobile.directionVoulue()`, lue par `Escouade` *avant* l'essai de collision et passée à
  `Formation::mettreAJour` : si
  l'escouade est bloquée par un mur et glisse, la formation vise quand
  même la direction qu'elle essayait de suivre). Mise à jour seulement
  si l'ancre a un point à atteindre et que la distance est `> 0`
  (`angle()` d'un vecteur nul n'a pas de sens), sinon la dernière valeur
  est conservée. `m_orientation` est ce qu'utilise `placeTheorique`.
- **Point à regarder** (étape 71) : `regarderVers(point)` mémorise un point
  (`Formation::m_pointARegarder`) vers lequel l'orientation voulue se tourne *seulement*
  quand l'ancre n'a plus de point à atteindre ; en marche, la marche décide.
  Un seul mécanisme, sans seuil de déplacement : le demi-tour observé aux portes
  venait d'un point d'approche à quelques pixels (ou derrière) de l'ancre, qui
  fixait l'orientation voulue sur un déplacement minuscule. Le point est effacé
  (`Formation::oublierPointARegarder`) par tout nouvel ordre à l'ancre (`allerVers`, approche du verrou dans
  `approcherVerrou`, `verrouiller`) : une escouade qui part combattre ne se
  retourne pas vers l'ancienne porte à l'arrêt. L'appelant doit donc appeler
  `regarderVers` *après* l'ordre qui l'efface. Un clic au sol près du groupe le
  fait toujours pivoter vers la direction de marche (seuil gardé en réserve si
  l'agacement se voit ailleurs qu'aux portes).
- **Rotation progressive, par le plus court côté** : `m_orientation`
  se rapproche de `m_orientationVoulue` à chaque `mettreAJour` (`Formation::tourner`, privée), que
  l'ancre bouge ou non (si elle s'arrête en pleine rotation, la
  formation finit de pivoter sur place). L'écart,
  `(m_orientationVoulue - m_orientation).wrapSigned()`, est ramené dans
  `[-180°, 180°]` pour que passer de 350° à 10° tourne de +20° et non de
  -340°. La rotation par image est bornée à `m_vitesseRotation * dt` ;
  si l'écart restant est inférieur ou égal à ce pas, `m_orientation`
  prend exactement la valeur voulue (même logique que le déplacement à
  pas fixe).
- **Limites connues** :
  - pas de collision dure entre personnages, ni entre un personnage et
    l'ancre : une séparation douce (étape 67, voir plus bas) écarte les
    personnages trop proches ;
  - l'ancre traverse les monstres alors que les personnages sont
    bloqués par eux : elle (et la caméra) peut donc s'éloigner des
    personnages bloqués ;
  - un personnage contourne les monstres qu'il n'approche pas (étape 68),
    mais pas les autres personnages (la séparation douce s'en charge) ;
    un monstre qui gêne un passage étroit sans autre issue est traversé au
    prix du coût ajouté, et le personnage attend alors derrière lui ;
  - deux places peuvent se replier presque au même endroit dans un
    passage étroit (les personnages ne se bloquent pas entre eux) ;
  - **chevauchement résiduel lors d'un demi-tour rapide** : les places *idéales*
    de deux personnages ne se chevauchent jamais pendant la rotation (c'est une
    rotation rigide de la formation). Mais chaque personnage poursuit sa cible en
    ligne droite, pas en suivant l'arc : lors d'un demi-tour très
    rapide, il coupe à travers le centre de la formation pendant qu'un
    autre fait le trajet inverse. La rotation progressive réduit
    nettement le phénomène sans l'éliminer ; depuis l'étape 67, la
    séparation douce écarte les personnages trop proches (« Séparation
    douce »). Un déplacement qui suit l'arc reste possible.

## Le personnage

- **Rayon de vision** (étape 72) : `m_rayonVision` (pixels), fourni par `Escouade` à la
  construction comme l'armure (320 px pour tous). `Escouade::cerclesDeVision()` rend
  un `Cercle` par vivant ; `Personnage` ne sait rien du brouillard. Voir `docs/vision.md`.
- **Un personnage sait seulement « aller vers un point »** : il reçoit
  sa cible en paramètre de `mettreAJour`, sans connaître la formation,
  son décalage ni l'escouade. C'est `Escouade` qui décide du point, ce
  qui permet qu'il soit différent de la place en formation (en combat,
  voir plus bas) sans changer l'interface de `Personnage`.
- **Il possède un `Mobile`** (étape 39), comme `Monstre` : composition,
  pas héritage. La position (le centre) et la vitesse vivent dans le
  `Mobile` ; la taille se lit dans `m_mobile.getTaille()` (plus de `m_taille` en double depuis l'étape 49), comme dans `Monstre`.
- **Règle hybride, décidée dans `Personnage::mettreAJour`** : `Escouade`
  décide *où*, `Personnage` *comment*. Si le trajet direct de sa
  position à sa cible est libre (`trajetLibre`, avec sa taille), il
  avance en ligne droite (`allerToutDroit`) et tout chemin en cours est
  abandonné. Sinon il suit un chemin A* lissé (`allerVers`). Même règle
  pour toutes les cibles (place de formation, placement en combat,
  marche vers un skill) : aucun cas particulier. Depuis l'étape 68, « libre »
  veut dire aussi : le trajet ne passe pas près d'un monstre à contourner
  (`traverseUnObstacle`, voir plus bas) ; sinon, A* avec coût.
- **Recalcul limité** : tant que le trajet direct est bloqué, le chemin
  n'est recalculé que s'il n'y en a pas (`m_cibleChemin` vide), s'il est
  fini (`directionVoulue()` vide), ou si la cible est à plus d'une case
  (`s_seuilRecalcul`, 40 px) de la cible du dernier A*. `m_cibleChemin`
  mémorise la cible donnée à `allerVers`, ce qui évite d'ajouter un
  accesseur à `Mobile` pour lire la fin du chemin. Depuis l'étape 68, un
  personnage `estBloque()` (0,3 s sans avancer) recalcule aussi son chemin. Une cible qui bouge de
  moins d'une case laisse donc le personnage finir son chemin ; s'il
  l'a fini sans que le trajet soit libre, il recalcule.
- **Repli en ligne droite** : `allerToutDroit` passe avant `allerVers`
  lors d'un recalcul. Si le A* ne trouve aucun chemin (cible dans un mur,
  zone fermée), `allerVers` laisse le chemin intact et le personnage
  garde la ligne droite. La cible est quand même mémorisée : sans cela,
  un A* voué à l'échec relancerait à chaque image toute la recherche.
- **Obstacles bloquants et obstacles à contourner** : les cercles des monstres
  bloquent le personnage dans les deux modes (`Mobile::mettreAJour`). Depuis
  l'étape 68, `Personnage::mettreAJour` reçoit en plus `aContourner`, que le A*
  (coût) et le lissage évitent (voir « Contourner les monstres »).
- **240 px/s** (contre 180 px/s pour l'ancre), en ligne droite comme sur
  un chemin : un personnage retardé par un mur peut rattraper sa place.
- **`Personnage.hpp` ne contient que `class TileMap;`** : la carte est
  seulement transmise au `Mobile` et à `trajetLibre`, jamais gardée en
  membre.
- **Points de vie et barre de vie** : 100 points, portés avec l'armure par une
  `Vitalite` (étape 82.1, qui contient le `PointsDeVie` de l'étape 53 ; possédée par valeur,
  comme dans `Monstre`, sans héritage). `recevoirDegats`, `recevoirSoin`, `estVivant()`,
  `getPointsDeVie` et `getPointsDeVieMax` sont des façades d'une ligne qui lui délèguent :
  `Escouade` et les tests n'ont pas changé (depuis l'étape 83, l'armure de la `Vitalite` est la somme de
  l'`Equipement` du personnage, recalculée à chaque changement : `docs/objets.md` ; depuis l'étape 89, ses
  défenses sont l'armure et la capacité défensive de ses statistiques de combat), et l'objet n'est pas
  exposé (loi de Déméter). La duplication supprimée avec `Monstre` est la logique (critique, armure puis
  points de vie), pas l'interface. Barre dessinée par la
  fonction commune `dessinerBarreDeVie` (module `BarreDeVie`), à qui
  `Personnage` passe `getRatio()`.
- **Il délègue son attaque à `m_attaqueDeBase`** et ses skills (un
  `std::vector<SkillConnu>`, depuis l'étape 74) à `m_skills` (voir `docs/combat.md`). La règle
  « un mort ne frappe pas » vit dans `Personnage::frapper` et
  `declencherSkill`, avant la délégation : être vivant est l'affaire de
  `Personnage`, pas de l'attaque. Il expose `typeSkill`, `skillEstPret`,
  `skillEstAPortee` et `zoneDuSkill` (cercle centré sur lui, au rayon de
  zone du skill), `cercleDePorteeDuSkill` (rayon du personnage + portée du skill, pour la visée) pour qu'`Escouade` décide quoi faire du skill.
- **Soin** : `recevoirSoin` plafonne au maximum de points de vie et ne
  fait rien sur un mort ; `getPointsDeVie` et `getPointsDeVieMax`
  permettent à `Escouade` de choisir le plus blessé.
- **Cercle** : `getCercle()` renvoie un cercle centré sur le personnage,
  de rayon la moitié du côté (10 px), calculé à la demande ; façade de `Mobile::getCercle`
  (étape 82.1), la même formule que `Monstre` et `Deplacement` (`cercleDuRectangle`).

## Choix du personnage le plus proche

- **Un seul endroit décide** : la fonction privée
  `indiceProieDuMonstre(id, position)` renvoie le provocateur du monstre
  s'il est provoqué et vivant (voir « Provocation » dans
  `docs/combat.md`), sinon `indicePersonnageLePlusProche(point)` (boucle
  sur les personnages **vivants**, distances de centre à centre).
  `proieDuMonstre` (qui renvoie le `Cercle`) et
  `recevoirDegats(idMonstre, origine, quantite)` s'en servent, pour
  qu'ils désignent toujours le même personnage par la même règle, même
  appelés séparément par `main.cpp`. (Depuis l'étape 82.5, `recevoirDegats` d'`Escouade` n'existe plus ;
  `Partie` appelle `recevoirCoup` avec la place désignée.)
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
  posé sur un monstre ciblable), un personnage vivant à portée de
  la cible reçoit **son propre centre** comme point à atteindre au lieu
  de sa place de formation. Il cesse d'avancer, continue de frapper, et
  recommence à suivre sa place dès que la cible n'est plus à portée ou
  que le verrou tombe. Les tireurs s'arrêtent donc à distance pendant
  que les mêlées continuent jusqu'au contact.
  Depuis l'étape 46, « à portée » veut dire à portée ET en vue
  (`Escouade::peutAtteindre`, voir `docs/combat.md`) : un personnage à
  portée mais sans vue continue d'avancer.
- **`Personnage` n'en sait rien** : il reçoit toujours un simple point.
- **Le cercle du verrou est lu là où il sert** (étape 82.4, R2) : `attaquer` (`approcherVerrou`) et
  `mettreAJour` le cherchent chacun dans les monstres ciblables qu'ils reçoivent. Jusqu'à l'étape 82.3,
  `attaquer` le mémorisait dans `m_cibleCombat`, que `mettreAJour` relisait : il fallait appeler
  `attaquer` avant, et la valeur, jamais effacée quand le verrou tombait, pouvait dater d'un ancien
  combat. D'où la règle « verrou posé ET cible connue » : sans verrou, la dernière cible aurait figé
  les personnages qui étaient à sa portée. Un test unitaire le vérifie toujours (sans verrou, un
  personnage déjà à portée suit sa place).
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
- **Limites connues** : l'encerclement et l'écartement viennent de l'étape 67 (« Places d'attaque »),
  mais le placement ne cherche pas de ligne de tir : un personnage à portée s'arrête exactement là où
  il se trouvait en entrant en portée, même si la position est maladroite ; pas de recul si un
  monstre s'approche d'un tireur arrêté ; pas de `Mobile` individuel.

## Lanceur d'un skill en attente

- **Priorité** : la règle 1 de `choisirIntention` (étape 82.4, voir « Intentions » ; règle 0 avant
  elle depuis l'étape 57 : un personnage qui exécute un coup reste sur place). Le lanceur d'un skill en
  attente suit l'intention de son skill, avant le dégagement, le verrou, l'agresseur et la formation.
  `Personnage` reçoit toujours un simple point.
- **`avancerSkillEnAttente(carte, verrou)`** (privée) est appelée une fois par image,
  avant la boucle des personnages : elle annule l'attente si besoin, fait
  partir un soin dès que le lanceur est à portée, et renvoie l'intention de son lanceur
  (`IntentionDuSkill { lanceur; intention; }`, étape 82.4) : `AllerAuPoint` vers le point d'un skill
  de zone ou vers l'allié à soigner, `ApprocherMonstre` (cercle du verrou, portée du skill) pour un
  skill de dégâts. Le skill de dégâts, lui, part dans `attaquer` (voir `docs/combat.md`) ; son lanceur
  ne marche que si le verrou est posé sur un monstre ciblable. Jusqu'à l'étape 82.3, elle renvoyait un
  point, et `pointVoulu` relisait `m_skillEnAttente` pour savoir qui était le lanceur (un invariant
  implicite entre deux fonctions, R2 de l'audit).
- **Annulation sur rupture du combat** dans `verrouiller` et
  `deverrouiller`, plutôt que par une vérification à chaque image : ce
  sont les deux seuls endroits où le verrou change.

## Déplacement vers un agresseur (étape 40)

- **Priorité** : la règle 4 de `choisirIntention` (étape 82.4, voir « Intentions » ; avant,
  `pointVoulu`) : ancre à l'arrêt, un vivant qui a un agresseur dans sa laisse l'approche, jusqu'à sa
  place d'attaque, ou reste sur place s'il a déjà un monstre à portée et en vue. Plus fortes qu'elle :
  l'action en exécution, le skill en attente, **le dégagement** (étape 78 : la place d'attaque autour
  du monstre dont le tireur se dégage, jusqu'au bout, sans s'arrêter à portée) et le combat verrouillé ;
  plus faible : la place en formation. Le détail et le pourquoi sont dans `docs/combat.md`
  (« Agresseurs » et « Dégagement des tireurs »).
- **« Ancre à l'arrêt »** = `m_mobile.directionVoulue()` vide, lu après le
  déplacement de l'ancre dans la même image.
- **`Personnage` n'en sait rien** : il reçoit toujours un simple point.

## Intentions : décider, puis résoudre (étape 82.4)

- **Deux questions séparées** (R1 et R2 de `docs/audit.md`). Jusqu'à l'étape 82.3, `pointVoulu(i)`
  appliquait six règles par priorité et calculait en même temps la place d'attaque. Depuis 82.4,
  `Escouade` calcule une **situation** (`SituationDuPersonnage`), la fonction pure `choisirIntention`
  (`escouade/Intention`) en tire une **intention**, puis `Escouade` **résout** l'intention en un point,
  avec `PlacesDAttaque`. `choisirIntention` ne fait que classer les règles : elle ne connaît ni la
  carte, ni `PlacesDAttaque`, et transmet les cercles sans calcul. C'est là que se brancheront les
  gambits ; ses tests (`test_Intention.cpp`) n'ont ni carte ni géométrie.
- **Trois intentions** (`std::variant<ResterSurPlace, AllerAuPoint, ApprocherMonstre>`) :
  `ResterSurPlace { placeAutourDe }` (le monstre autour duquel sa place d'attaque est gardée ; vide :
  aucun), `AllerAuPoint { point }`, `ApprocherMonstre { idMonstre, cercle, portee }`. Le cercle voyage
  dans l'intention plutôt que d'être retrouvé par l'identifiant : chaque règle le prend à sa source, et
  les listes de monstres ne disent pas toujours la même chose (le verrou et l'agresseur sont lus dans
  les ciblables, le dégagement et les monstres à contourner dans les physiques).
- **Les règles, inchangées** (même ordre qu'à l'étape 78), la première qui s'applique gagne :
  0. une action en exécution : `ResterSurPlace` autour du monstre de sa place actuelle (gardée, quelle
     qu'elle soit) ;
  1. le lanceur d'un skill en attente : l'intention de ce skill ;
  2. un dégagement : `ApprocherMonstre`, sans s'arrêter à portée ;
  3. vivant, avec un verrou : `ResterSurPlace` autour du verrou s'il l'atteint, sinon `ApprocherMonstre` ;
  4. vivant, ancre à l'arrêt, un agresseur dans sa laisse : `ResterSurPlace` autour de l'agresseur s'il
     a déjà un monstre à portée, sinon `ApprocherMonstre` (l'ordre du joueur gagne) ;
  5. `AllerAuPoint` vers sa place de formation.
- **La situation, calculée avant** par `Escouade::situationDe(i)` (`const`), à l'itération `i` de la
  boucle des personnages, au même moment que l'ancien `pointVoulu(i)` : `i` n'a pas encore bougé dans
  l'image, ceux d'indice inférieur si.

  | Champ | Source |
  |---|---|
  | `vivant`, `enExecution` | `Personnage` |
  | `monstreDeLaPlace` | la cible de sa place retenue (`PlacesDAttaque::placeRetenue`) |
  | `skillEnAttente` | `avancerSkillEnAttente`, une fois avant la boucle, pour le seul lanceur |
  | `degagement` | `m_degagements` (`mettreAJourDegagements`, avant la boucle), confirmé par `confirmerDegagement` : monstre parmi les physiques et place libre (`PlacesDAttaque::peutPlacer`, mêmes entrées que le placement qui suivra), sinon effacé comme avant ; cercle des physiques |
  | `verrou`, `verrouAtteint` | le verrou cherché dans les ciblables, une fois avant la boucle ; atteint : `peutAtteindre` avec l'attaque de base |
  | `ancreArretee` | `m_mobile.directionVoulue()` vide, après le déplacement de l'ancre |
  | `agresseur` | `choisirAgresseurAViser` (laisse de 100 px autour de sa place de formation) parmi les agresseurs de la mémoire (`Agresseurs::vusParmi`) que les ciblables contiennent, avec leur cercle ; un agresseur absent des ciblables est ignoré pour l'image |
  | `monstreAPortee` | un ciblable que son attaque de base atteint, à portée et en vue (la règle de l'autodéfense) |
  | `placeDeFormation`, `porteeAttaqueDeBase` | `Formation`, `porteeDe` |

  Ce qui vaut pour tous les personnages (ancre à l'arrêt, verrou, agresseurs, intention du skill) est
  rassemblé une fois par image dans une structure privée, `ContexteDesIntentions`.
- **Résolution** (`pointDeLIntention`) : `ResterSurPlace`, le centre du personnage ; `AllerAuPoint`, le
  point ; `ApprocherMonstre`, la place d'attaque (`placer`) ou, sans place libre, le centre du monstre.
  Un dégagement n'y arrive jamais sans place : `confirmerDegagement` l'a effacé avant. Puis la règle
  d'oubli (« Oubli des places » plus bas) et le déplacement.
- **Contourner** : `cerclesAContourner(physiques, intention)` (module `Intention`, fonction pure) :
  tous les monstres physiques sauf celui de l'intention (`monstreDeLIntention` : celui qu'il approche,
  ou celui de `placeAutourDe`), voir « Contourner les monstres ». Elle ne reçoit rien du placement :
  l'exclusion de l'approché ne peut pas dépendre de la réussite de la recherche d'une place (la garantie
  est dans la signature ; `test_Intention.cpp` la teste).
- **R2 : `attaquer` et `mettreAJour` ne se passent plus rien par des membres.** `m_cibleCombat` et
  `m_monstreAPortee` sont supprimés : le cercle du verrou et « un monstre à portée » sont calculés là où
  ils sont lus, sur les monstres ciblables, que `mettreAJour` reçoit désormais dans
  `VueDesMonstres { physiques; ciblables; deControle; }`. Trois listes nommées, pour qu'on ne
  puisse pas en inverser deux ; aucune n'a de valeur par défaut (GCC signale l'oubli d'un champ,
  `-Wmissing-field-initializers`) : si `Partie` oubliait `deControle`, le dégagement disparaîtrait sans
  aucune erreur (82.5). Les appels `mettreAJour(dt, carte, {})` des tests restent valables. Un
  monstre qui rentre à son poste n'est jamais ciblable, donc jamais « à portée ». L'ordre d'appel n'est
  plus une règle de correction ; `Partie` garde le sien.
- **Écarts de comportement acceptés** (décisions d'Olivier), chacun fixé par un test qui échouait avec
  l'ancien code :
  - un personnage qui approche un agresseur sans place libre ne le contourne plus (avant, il visait son
    centre tout en le contournant) ; depuis 82.5 la garantie est celle de la signature de
    `cerclesAContourner` ;
  - le lanceur d'un skill de zone ou de soin en attente, en combat verrouillé, contourne le monstre
    verrouillé (avant, en combat, personne ne le contournait ; marchant droit dessus, le lanceur restait
    calé contre lui) ;
  - un monstre tué par les coups d'`attaquer` n'est plus dans les ciblables du `mettreAJour` de la même
    image : celui qui l'avait à portée n'est plus retenu sur place (avant, `attaquer` l'avait noté avant
    les coups). Il part une image plus tôt vers sa place de formation. Depuis 82.5, l'agresseur mort,
    encore en mémoire jusqu'au prochain `attaquer`, n'attire plus personne (voir « Bilan de l'étape 82 »).

  Les autres changements de la liste à contourner touchent des personnages qui ne bougent pas (en
  exécution sans place retenue, sur place à portée, morts) : sans effet. Aucun autre test n'a changé de
  résultat ; les appels de `mettreAJour` des tests ont changé de forme (ciblables : la liste du dernier
  `attaquer` du test, vide s'il n'y en a pas eu, ce qui reproduit ce que voyaient les deux membres).
- **Limites connues** :
  - la boucle dépend de l'ordre des personnages (« Oubli des places » plus bas) ;
  - **le contournement évite le cercle d'un monstre, pas sa zone de contrôle** : un personnage qui
    contourne un monstre peut frôler sa zone et subir l'attaque d'opportunité. Piste : la « stratégie
    d'attaque » par type de monstre (`docs/brainstorm.md`, section 4 bis), qui se branchera sur
    `coutDesObstacles` ;
  - le lanceur d'un skill de dégâts en attente ne marche qu'avec un verrou. La raison d'origine
    (`m_cibleCombat` périmé) a disparu avec R2 ; l'exigence reste comme **règle de jeu** (étape 37 :
    sans verrou, personne ne marche vers la cible d'un skill de dégâts), qui vaut quel que soit l'ordre
    d'`attaquer` et de `mettreAJour`. Son commentaire, dans `avancerSkillEnAttente`, le dit.

## Places d'attaque (étape 67)

**Le problème.** Plusieurs personnages qui marchent vers le même monstre
visaient son centre : ils convergeaient vers le même point de contact et
finissaient l'un par-dessus l'autre.

- **Une place d'attaque par personnage et par monstre**, sur un anneau
  centré sur le monstre : rayon du monstre + rayon du personnage + 90 % de la
  portée de l'action avec laquelle il s'approche (attaque de base ou skill de
  dégâts en attente), pour que la place soit à portée. Mêlées et tireurs sont
  donc sur des anneaux différents.
- **C'est un point où l'on marche, pas une destination obligatoire** : les
  règles d'arrêt, de portée et de vue ne changent pas (`peutAtteindre`). Un
  personnage s'arrête dès que le monstre est à portée ET en vue, éventuellement
  avant d'atteindre sa place.
- **Concernés** : les personnages qui visaient le centre d'un monstre. Le
  combat verrouillé (avant, seule l'ancre allait vers le monstre, les
  personnages suivant leur place de formation : un personnage pas encore à
  portée marche maintenant vers sa place d'attaque), l'agresseur et le skill de
  dégâts en attente. Le skill de zone en attente (un point au sol), le soin (un
  allié) et la formation ne changent pas.
- **Géométrie dans `Encerclement`**, sans lien avec `Escouade`. Nombre de
  places = plus grand n tel que la **corde** entre deux places voisines soit
  `>= s_ecartEntrePersonnages` (34 px) ; première candidate face à l'ancre,
  puis alternance gauche/droite. L'énoncé de départ donnait `⌊2πr / 26⌋`
  (longueur d'arc) *et* une corde de 26 px minimum : les deux ne vont pas
  ensemble (la corde est un peu plus courte que l'arc). On a gardé la corde
  (décision d'Olivier) : jamais deux places plus proches que prévu, parfois une
  place de moins.
- **Une candidate est valide** si le rectangle du personnage centré dessus ne
  chevauche aucun mur et si elle voit le centre du monstre (`ligneDeVue`, en cases depuis l'étape 73).
  **Elle est libre** si elle est à plus de 34 px de la place retenue par un
  autre personnage pour ce monstre (quel que soit l'anneau) ET à plus de 34 px
  du centre de tout autre personnage vivant immobile (occupation réelle : un
  mêlée qui frappe en autodéfense sur place n'a pas de place retenue, mais il
  occupe l'endroit où il se tient).
- **Attribution** : la place valide et libre la plus proche de l'angle actuel
  du personnage autour du monstre (`choisirAngleLibre`), égalité : l'ordre des
  candidates. Aucune place : le personnage vise le centre du monstre
  (comportement d'avant), sauf au dégagement (étape 78), abandonné pour l'image.
- **Stabilité** : `PlacesDAttaque` (depuis l'étape 82.3, voir plus bas ; avant,
  `Escouade::m_placesDAttaque`) retient par personnage `{ idCible, rayon,
  angle }`. L'angle est gardé tant que le personnage
  approche ce même monstre ; seule la position est recalculée à chaque image
  depuis le centre actuel du monstre. Le rayon est gardé aussi, parce que le
  test « libre » doit connaître l'anneau de chaque autre personnage (mêlées et
  tireurs n'ont pas le même). La place est réattribuée si elle devient
  invalide (mur, plus de vue) ou occupée. Elle est oubliée par la règle d'oubli : autre monstre, plus
  de monstre, mort du personnage, disparition du monstre. Un coup en cours ou l'arrêt à portée sur le
  même monstre ne la font pas oublier (« Oubli des places » plus bas).
- **Limites** : la place est calculée avec la direction de l'ancre au moment
  de l'attribution ; deux personnages qui s'approchent depuis des côtés
  opposés se gênent encore parfois ; pas de places d'attaque pour les monstres.

## Classe `PlacesDAttaque` (étape 82.3)

- **Le placement autour des monstres est sorti d'`Escouade`** (deuxième pas de R1,
  `docs/audit.md`) : `escouade/PlacesDAttaque`, possédée **par valeur** (`m_placesDAttaque`), sans
  héritage. Elle possède les places retenues de **tous** les assaillants : le test « libre » compare
  la place d'un assaillant à celles des autres. Le contenu d'`Escouade::pointVersMonstre` y a été
  déplacé (`placer`) avec les seules adaptations indispensables ; `pointVersMonstre`, la structure
  privée `PlaceDAttaque` et `s_fractionPorteeAnneau` ont quitté `Escouade`. Aucune règle n'a changé.
- **Elle ne connaît ni `Personnage` ni `Escouade`**, et son vocabulaire est neutre (« assaillant »,
  « cible », « occupant ») : rien ne la lie au camp du joueur. Elle reçoit de la géométrie : la cible
  (`EntiteVue` : identifiant et cercle), l'indice et le cercle de l'assaillant, la portée de son
  action, le point de référence de la première candidate (l'ancre), la carte et les occupants. Elle
  se teste donc seule, avec ses propres valeurs (`test_PlacesDAttaque.cpp`).
- **Une structure plutôt qu'une longue liste de paramètres** (C2 de l'audit) :
  `PlacesDAttaque::Approche { cible; assaillant; cercleAssaillant; portee; reference; }`, remplie par
  `Escouade::approcheDe` en initialisation désignée (`.cible = monstre, .portee = …`). `placer(approche,
  ancienne, occupants, carte)` reçoit à part la mémoire (l'ancienne place), les occupants et la carte.
- **Les occupants** : par assaillant, son centre s'il est vivant et immobile, vide sinon
  (`std::vector<std::optional<sf::Vector2f>>`). `Escouade::occupants()` les construit à chaque appel
  de `placer`, avec les positions du moment, comme le faisait `pointVersMonstre` : les personnages
  déjà déplacés dans l'image y sont à leur nouvelle position. L'entrée de l'assaillant lui-même est
  ignorée. L'audit proposait `struct Occupant { centre; immobile; }` ; l'`optional` suffit, seuls les
  occupants vivants et immobiles comptant.
- **L'écart minimal est passé à la construction** (`Escouade` passe `s_ecartEntrePersonnages`,
  34 px) : les tests de `PlacesDAttaque` utilisent le leur (règle de `docs/tests.md`). La part de la
  portée qui fixe l'anneau (90 %, `s_fractionPorteeAnneau`) reste une constante de `PlacesDAttaque.cpp` ;
  les tests ne l'écrivent pas, ils vérifient que la place est à portée sans contact.
- **« Aucune place libre » est dans le type** : `placer` renvoie un `std::optional<sf::Vector2f>`. Le
  verrou, le skill de dégâts en attente et l'agresseur visent alors le centre du monstre (`value_or`,
  dans la résolution depuis 82.4) ; le dégagement est abandonné pour l'image (depuis 82.4, avant la
  décision : `peutPlacer`). Avant 82.3, `pointVersMonstre` renvoyait le centre du monstre, et le
  dégagement repérait l'échec en relisant `m_placesDAttaque[indice]`.
- **En cas d'échec, `placer` ne touche pas à la place retenue** : comme `pointVersMonstre`, qui
  n'écrivait qu'en cas de succès. Depuis 82.4, la règle d'oubli d'`Escouade` l'oublie ensuite (une
  approche sans place libre n'en garde aucune).
- **Interface de 82.4** : `retirer`, `retenir` et `cibleApprochee`, le mécanisme provisoire de 82.3,
  sont remplacés par `placeRetenue` (lecture), `peutPlacer` (la question en lecture seule du
  dégagement) et `oublierSaufAutourDe` (la règle d'oubli). `placer` et `peutPlacer` partagent une seule
  fonction `const` privée, `trouverPlace` : `peutPlacer` l'interroge, `placer` la complète en retenant
  le résultat (le patron de `peutEngagerSkill` et `engagerSkill`). `placer` garde sa signature :
  `Escouade` lui passe la place retenue comme `ancienne`, et ses appels dans les tests n'ont pas changé.

### Oubli des places (étape 82.4)

Remplace le mécanisme de 82.3, déplacé tel quel d'`Escouade` à cette étape : `pointVoulu(i)` retirait
la place (`retirer`), puis la première règle qui s'appliquait la remettait (`retenir`), en calculait une
autre (`placer`) ou la laissait oubliée.
- **Où et quand** : seulement dans `Escouade::mettreAJour`, dans la boucle des personnages (indice
  croissant), pour chaque personnage `i` : situation, intention, point, **oubli**, déplacement. Rien
  d'autre ne touche les places : ni `attaquer`, ni `verrouiller`, ni `deverrouiller`, ni `allerVers`.
  Entre deux images, elles restent telles quelles.
- **La règle, écrite une fois** (`monstreDontLaPlaceEstGardee` dans `Escouade.cpp`, puis
  `PlacesDAttaque::oublierSaufAutourDe`) : la place retenue n'est gardée qu'autour du monstre de
  l'intention.
  - `AllerAuPoint` : oubliée ;
  - `ApprocherMonstre` : `placer` l'a retenue s'il a trouvé une place, autour de ce monstre : gardée ;
    sans place libre, oubliée ;
  - `ResterSurPlace` : gardée si elle est autour de `placeAutourDe` (règle 0 : le monstre de la place
    actuelle, donc toujours ; règle 3 : le verrou ; règle 4 : l'agresseur), sinon oubliée.

  Le résultat est celui de 82.3, règle par règle.
- **Qui lit** : la situation de `i` lit sa place (`monstreDeLaPlace`, et l'ancienne place passée à
  `placer` et à `peutPlacer`) ; `placer(j)` et `peutPlacer(j)` lisent les places retenues des autres
  (test « libre ») : pour `k < j`, celles décidées plus tôt dans la même image ; pour `k > j`, celles de
  l'image précédente. `degagementVoulu` ne lit pas les places (il lit `m_pointsVises`, le point visé de
  l'image d'avant).
- **Limite connue, gardée exprès : la boucle dépend de l'ordre des personnages.** Les places retenues
  et les occupants qu'un personnage voit dépendent de ceux qui le précèdent (déjà déplacés, place déjà
  décidée dans l'image) et de ceux qui le suivent (place de l'image d'avant) : deux personnages qui
  visent la même place la départagent par leur indice. Rendre la boucle indépendante de l'ordre
  (décider toutes les places, puis déplacer) changerait le comportement : ce n'est pas fait.

## Séparation douce (étape 67)

**Pourquoi pas un blocage entre personnages.** Le choix de l'étape 32 (les
personnages ne se bloquent pas entre eux) a été remis en question à l'étape 67.
Un essai de blocage (chaque personnage reçoit les cercles des autres vivants
comme obstacles) a été abandonné : c'est une contrainte **dure et symétrique**,
qui crée des **interblocages** lors des croisements frontaux. Un demi-tour de
la formation laissait trois personnages en grappe près de l'ancre, et un
lanceur de zone qui marchait derrière un coéquipier aligné ne passait plus.

**La séparation douce** (`Escouade::separerPersonnages`, à la fin de
`mettreAJour`, après le déplacement des personnages) n'empêche jamais
d'avancer. Depuis l'étape 82.2, le calcul est la fonction pure `pousseesDeSeparation`
(`escouade/Separation`) ; `separerPersonnages` ne fait plus que traduire ses personnages en
entrées et appliquer les poussées rendues :

- deux vivants dont les centres sont à moins de `s_ecartEntrePersonnages`
  (34 px) sont écartés le long de la droite qui joint leurs centres, chacun de
  la moitié du manque (le cercle de collision de 10 px ne change pas) ;
- la poussée de chacun est plafonnée à `s_vitesseSeparation` (120 px/s) × `dt` :
  indépendante de la fréquence d'images ;
- un personnage en exécution d'un coup ou en activation d'un skill n'est pas
  poussé (l'autre prend toute la poussée ; si les deux le sont, rien) ; les
  morts ne poussent pas et ne sont pas poussés ; centres confondus : repli sur
  l'axe des x, le plus petit indice vers la gauche ;
- la poussée passe par `Personnage::decaler` / `Mobile::decaler` : murs et
  monstres respectés, avec glissement, sans toucher au chemin ni à
  `estEnMouvement`. Ni l'ancre ni les monstres ne sont poussés.

**Une fonction pure à part (étape 82.2).** `pousseesDeSeparation` reçoit, par personnage, un
`MembreASeparer { centre; etat; }`, l'écart minimal, la vitesse maximale de poussée et `dt`, et
renvoie une poussée par personnage, dans le même ordre ; elle ne connaît ni `Personnage`, ni la
carte, ni les monstres. Les valeurs du jeu (34 px, 120 px/s) sont passées par `Escouade` : les tests
de `Separation` utilisent les leurs (règle de `docs/tests.md`).
- **`enum class EtatDeSeparation { Absent, Fixe, Libre }`** plutôt que deux booléens
  (« pousse », « poussable ») : un mort est `Absent` (il ne pousse pas et n'est pas poussé), un
  personnage en exécution d'un coup ou en activation d'un skill est `Fixe` (il pousse mais n'est
  pas poussé), les autres sont `Libre`. L'état « mort mais poussable » n'est pas représentable.
  Valeur par défaut d'un `MembreASeparer` : `Absent` (un membre oublié ne pousse personne).
- **Calcul simultané, comme avant l'extraction** : toutes les paires sont jugées sur les centres
  du début de la séparation (aucune poussée n'est appliquée pendant la boucle des paires), les
  poussées d'un personnage s'additionnent, puis la somme est plafonnée. `separerPersonnages`
  le faisait déjà : son accumulation dans un tableau de poussées, appliquées après la boucle, a été
  reprise telle quelle. L'ordre d'application ne change rien non plus : `decaler` ne reçoit que
  les cercles des monstres, jamais ceux des autres personnages. Résultat identique, sur toute la
  suite de tests.

**Pourquoi 120 px/s, plus lent que la marche (240 px/s).** Un essai à 300 px/s
(« pour qu'aucune marche ne maintienne un chevauchement ») recréait un
interblocage : une poussée plus forte que la marche devient une contrainte
dure. Un personnage qui marche droit vers un coéquipier immobile aligné sur sa
ligne (par exemple le tireur arrière-droit derrière le mêlée avant-droit
quand la formation regarde vers +x) était repoussé aussi vite qu'il avançait,
alors que le coéquipier regagnait sa place : il restait bloqué, et les tests
du skill de zone ne passaient plus. À 120 px/s, la poussée corrige un
chevauchement sans jamais empêcher de passer.

**Ce qui empêche les superpositions durables, ce sont les cibles distinctes** :
les places d'attaque (« Places d'attaque ») et les places de formation
(ci-dessous). Si deux personnages ne visent jamais de points à moins de 34 px
l'un de l'autre, aucune marche ne maintient un chevauchement. La séparation ne
fait que corriger les croisements en route. Limite : si deux personnages ont
malgré tout la même cible, ils restent à environ 2 px (la marche gagne sur la
poussée) ; elle ne devrait plus arriver.

**Pourquoi 34 px et non un chevauchement des cercles.** Les personnages sont
dessinés en carrés de 20 px mais mesurés en cercles de rayon 10 : en diagonale,
deux carrés qui se touchent ont leurs centres à environ 28 px. 34 ≈ (20 + 4) ×
√2 garde un écart visible de 4 px même en diagonale. Cette valeur est la
constante unique `s_ecartEntrePersonnages` (séparation, places d'attaque,
places de formation), en `inline constexpr` dans un petit en-tête neutre depuis l'étape 82.2
(`escouade/EcartEntrePersonnages.hpp`, comme `EmplacementsPrincipaux`) : `Escouade` et `Formation`
l'incluent sans dépendre l'une de l'autre ; `PlacesDAttaque` (étape 82.3) ne l'inclut pas, elle
reçoit l'écart à sa construction. Chaque personnage a aussi un contour sombre de 1 px
(vers l'intérieur) pour distinguer deux personnages collés.

## Contourner les monstres (étape 68)

- **`Escouade::mettreAJour` reçoit les monstres en `EntiteVue`** (identifiant et
  cercle ; dans `VueDesMonstres::physiques` depuis l'étape 82.4), pas seulement des cercles : il lui
  faut l'identifiant pour savoir quel monstre un personnage approche. Elle en tire les obstacles
  bloquants (tous les cercles) et, par personnage, la liste à contourner (`cerclesAContourner`, fonction
  pure du module `Intention` depuis 82.5 ; fonction du `namespace` anonyme d'`Escouade.cpp`
  sous le nom `monstresAContourner` en 82.4).
- **Qui contourne quoi** : chaque personnage contourne tous les monstres **sauf
  celui de son intention** (étape 82.4, `monstreDeLIntention`) : celui qu'il approche (combat
  verrouillé, agresseur, skill de dégâts en attente, dégagement), avec ou sans place libre, ou celui
  autour duquel il reste sur place. Jusqu'à 82.3 : le monstre de sa place d'attaque ou, en combat sans
  place retenue, le monstre verrouillé, pour tout le monde. Pour aller à un point, tous. Il ne
  contourne pas les autres personnages (séparation douce, étape 67) ni l'ancre,
  qui ne contourne rien. Le monstre approché est exclu, sinon le personnage se
  détournerait de la cible qu'il veut atteindre : sa place d'attaque est au
  contact du monstre, donc dans la zone occupée.
- **Ligne droite ou A*** : voir « Règle hybride » ci-dessus.

## Places de formation hors des monstres (étape 68 bis)

Une place de formation dont le centre est à moins de (rayon du monstre + rayon
du personnage) d'un monstre est **invalide**, comme une place dans un mur.
Cause d'un blocage constaté : la formation traverse un monstre (l'ancre n'est
pas bloquée), la place d'un personnage entre dans le monstre au moment où le
personnage arrive à son contact, et il reste calé contre lui. `calculerPlaces`
(`Formation`, depuis l'étape 82.2 ; elle reçoit les cercles des monstres) déplace donc une telle place avec la **même
recherche que l'étape 67** : premier point valide (trajet libre depuis l'ancre,
hors mur, hors monstre) et libre (34 px des places déjà calculées) sur les
anneaux de 34, 68 et 102 px autour de l'ancre. Aucun point : si seul un monstre
gênait, la place ramenée par le repli est gardée (comportement d'avant) ; si
une autre place la gênait aussi, le centre de l'ancre. Ne concerne que les
places de formation : les places d'attaque sont déjà hors du monstre. Sans
monstre proche, rien ne change.

## Places de formation distinctes (étape 67)

Près d'un mur, `replierVersOrigine` ramenait plusieurs places au même point
(souvent le centre de l'ancre). `calculerPlaces` (`Formation` depuis l'étape 82.2) traite maintenant les
personnages dans l'ordre : si la place ramenée est à moins de 34 px d'une place
déjà calculée, elle est remplacée par le premier point valide (trajet libre
depuis l'ancre, rectangle hors mur) et libre (34 px ou plus des places déjà
calculées) sur des anneaux de 34, 68 et 102 px autour de l'ancre, les candidates
(`Encerclement`) partant de la direction de la place théorique. Aucun point :
le centre de l'ancre. En terrain dégagé, les places sont à 50 px les unes des
autres : rien ne change. La limite « deux places peuvent se replier presque au
même endroit dans un passage étroit » a disparu de la liste des limites.

## Débogage : points visés et ancre (étape 67)

- **`Escouade::pointVise(place)`** (`const`) renvoie le point vers lequel le
  personnage marche à cette image, tel que `mettreAJour` l'a calculé
  (`m_pointsVises`) : place de formation, place d'attaque, cible du skill en
  attente, ou sa propre position s'il reste sur place. Elle sert aux tests
  (un agresseur : point visé = place d'attaque, pas le centre du monstre ; sans
  place valide : le centre du monstre) et à l'affichage.
- **F3** bascule l'affichage : un trait fin jaune du centre de chaque
  personnage vivant à son point visé, un petit cercle à ce point
  (`Escouade::dessinerPointsVises`) et, depuis l'étape 68, son chemin restant
  en cyan ; `Partie::dessinerCheminsDesMonstres` y ajoute celui de chaque monstre
  en rouge. Il remplace les points jaunes de formation
  provisoires. **F4** cache ou réaffiche le carré vert de l'ancre
  (`Escouade::dessiner(rendu, afficherAncre)` ; paramètre, pas membre, comme
  `estCible` de `Monstre::dessiner`).
- **État d'interface dans `EcranJeu`**, pas dans `Partie` ni `Escouade` :
  `OptionsDeDebogage { pointsVises = false; ancre = true; }`, remise à sa valeur
  de départ à chaque nouvelle partie. Actives à tout moment en jeu, y compris
  en visée ; les menus les ignorent. Sur certains portables, les touches F
  peuvent exiger Fn.

## Mort d'un personnage et défaite

- **Un mort reste dans `m_personnages`**, contrairement à `Monstre` que
  `main.cpp` retire du vecteur. L'indice `i` d'un personnage désigne
  aussi son décalage (le i-ième de la `Formation`) : retirer un élément décalerait
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
  vivant) : `Escouade` ne décide de rien en cas de défaite. C'est `Partie`
  qui la décide (`estPerdue`, étape 54), fige le monde et refuse les ordres
  du joueur (voir `docs/boucle-et-entrees.md`).
- **Limites connues** : un mort n'est jamais retiré ni remplacé ; pas de
  résurrection (le soin ne s'applique pas aux morts). Depuis l'étape 54, la
  partie perdue est figée : l'ancre n'achève plus son trajet.

## Hygiène du code (étape 49)

- **Places** : `Escouade::placeValide(place)` et `personnageDeLaPlace(place)`
  (versions `const` et non `const`, `.at()`) sont la seule conversion de la
  place 1 à 4 vers l'indice 0 à 3. Plus de `m_personnages[place - 1]` ni de
  test de bornes ailleurs ; `declencherSkillDeZone` et `lancerSkillDeZone`
  valident la place eux-mêmes. `provoquer` ignore une place invalide (avant :
  comportement indéfini plus tard dans `mettreAJour`). Restent deux
  conversions vers un *indice* (pas un accès) ; depuis l'étape 51 elles passent
  par `indiceDeLaPlace` (namespace anonyme de `Escouade.cpp`), la seule
  écriture de `place - 1`.
- **Skill en attente** : `skillEnAttenteDeDegats()` et
  `annulerSkillDeDegatsEnAttente()` sont utilisées par `verrouiller`,
  `deverrouiller`, `attaquer` et `avancerSkillEnAttente`.
- **Étape 51** : `attaquer` se lit comme la liste de ses étapes privées
  (`actualiserMemoires`, `departSkillDeZoneEnAttente`, `approcherVerrou`,
  `departSkillDeDegatsEnAttente`, `autodefense`). `mettreAJour` appelle
  `ecoulerMemoires`, `tournerFormation`, `calculerPlaces`, puis
  `pointVoulu(i, ...)` pour chaque personnage : la priorité y est écrite
  une fois, en `return` anticipés (skill en attente, puis rester sur place
  à portée du verrou, puis agresseur ancre à l'arrêt, puis place en
  formation). La formation (R2c) n'est pas extraite. Depuis l'étape 82.4 : situation,
  `choisirIntention` et résolution (voir « Intentions »).
- **Ordre d'appel** : `attaquer` avant `mettreAJour` dans une même image,
  documenté au-dessus de la classe : `m_monstreAPortee` et `m_cibleCombat`
  passent de l'une à l'autre. `m_monstreAPortee` était un
  `std::array<bool, s_nombrePersonnages>` ; depuis l'étape 74 (nombre de personnages
  connu à l'exécution) c'est un `std::vector<bool>`. Règle et membres supprimés à l'étape 82.4 (R2,
  voir « Intentions »).
- **Constantes** : tailles, vitesses, points de vie et attaque de base
  (`attaqueDeBase(portee)`, un seul endroit) ont des constantes `s_` ;
  `Personnage::m_taille` est supprimé.
- **Barre de vie** : module `BarreDeVie`, partagé avec `Monstre`. La barre
  orange de recharge sous la barre de vie a été supprimée à l'étape 74 (ambiguë avec
  plusieurs skills : la barre de skills la remplace).

## Composition et escouade à N personnages (étape 74)

- **`Composition.hpp` / `.cpp`** : `enum class Rang { Avant, Arriere }`, `struct
  DefinitionPersonnage { couleur; skills = {}; equipementDeDepart = {}; rayonVision; caracteristiques = {}; rang = Avant; }`
  (étape 88 : base de caractéristiques ; étape 91 : `attaqueDeBase`, `zoneDeControle` et `pointsDeVieDeBase`
  disparaissent, l'attaque de base et la zone viennent de l'arme de l'équipement de départ, les points de vie de
  l'Endurance ; les champs avec valeur par défaut peuvent s'omettre en initialisation désignée),
  `compositionParDefaut()` (remplacée à l'étape 76 par `Specialite`, voir plus bas) et
  `decalagesDeFormation(rangs)`. Pourquoi : `Escouade` ne
  doit plus choisir de valeurs, pour que le joueur puisse choisir sa composition
  (étape 76) sans toucher à `Escouade`. La composition n'appartient pas au `Niveau` :
  `Partie(niveau, composition, graine)` la reçoit, `EcranJeu` passe `composer(specialites)`.
- **`decalagesDeFormation`**, fonction pure : pour une escouade orientée vers `+x`, les
  `Avant` sont sur `x = +25`, les `Arriere` sur `x = -25` ; dans un rang de n personnages,
  le k-ième (ordre d'apparition dans la composition) est sur `y = (k - (n-1)/2) × 50`. Avec
  2 devant et 2 derrière on retrouve exactement l'ancien carré.
- **Composition par défaut** (armures de l'époque ; 9 au lieu de 30 depuis l'échelle × 3 de l'étape 91) : guerrier (Avant, rouge, armure 30 par son équipement de départ, attaque de mêlée, skills
  `FrappePuissante` et `Provocation`), prêtre (Arriere, jaune pâle, armure 0, attaque de
  tireur, skill `Soin`), mage (Arriere, violet, armure 0, attaque de tireur, skill
  `Explosion`). Toutes les valeurs numériques des skills sont celles d'avant.
- **`Escouade(positionDepart, composition)`** : `assert(!composition.empty())`, les places
  vont de 1 à N (taille de la composition). `m_pointsVises`, `m_placesDAttaque` et
  `m_monstreAPortee` (supprimé à l'étape 82.4) sont des `std::vector` dimensionnés à la construction ;
  `s_nombrePersonnages` n'existe plus.
- **Triangle et soin** : dans le triangle par défaut, tous les alliés sont à moins de 56 px
  les uns des autres, donc toujours à portée du soin (40 px bord à bord = 60 px entre
  centres) : un soin « hors de portée » n'arrive plus qu'avec une formation étirée ou des
  murs. Conséquence acceptée.
- **Limites** : un mort n'est toujours pas retiré ; la formation ne se resserre pas à la mort
  d'un personnage ; pas de niveaux ni d'apprentissage de skill en cours de partie.

## Spécialités et choix de la composition (étape 76)

- **`Specialite.hpp` / `.cpp`** : `enum class IdSpecialite { Guerrier, Pretre, Mage, Paladin, Rodeur }`,
  `struct Specialite { id; nom; definition }`, `catalogue()`, `specialite(id)` et `composer(ids)`.
  `compositionParDefaut()` est supprimée ; les valeurs des trois classes d'origine ont été déplacées
  de `Composition.cpp` vers `Specialite.cpp` sans changement (`Composition` ne garde que `Rang`,
  `DefinitionPersonnage` et `decalagesDeFormation`). Une seule source pour les valeurs.
- **Des données, pas une hiérarchie** : pas de classe de base `Specialite` ni de sous-classes
  `Guerrier`, `Mage`… Les classes ne diffèrent que par des valeurs (couleur, équipement de départ, rang, attaque de
  base, skills), qu'une `DefinitionPersonnage` porte déjà ; aucun comportement propre à une classe.
  Une classe est une **recette de départ** : plus tard (arbres de skills, niveaux, hors de portée
  ici), deux personnages de la même classe deviendront différents ; l'objet qui évoluera est le
  `Personnage`, pas la classe.
- **Doublons permis** : une classe peut être prise 1 à 3 fois. Pour qu'on les distingue à l'écran,
  `composer` donne à la k-ième occurrence d'une même classe sa couleur assombrie de 0 %, 30 % puis 55 %
  (`assombrir`, à côté d'`eclaircir`). On n'éclaircit pas : l'éclaircissement est déjà l'indice
  visuel d'un coup ou d'un skill en préparation. Le compte se fait **par classe** (guerrier, mage,
  guerrier : le second guerrier est la 2ᵉ occurrence), pas sur toute la liste.
- **L'ordre de la liste donne les places** 1, 2, 3, donc les rangées de la barre par défaut et
  l'ordre des décalages. La formation dépend du rang de chaque classe : trois guerriers sont tous
  `Avant` (colonne de x = +25), trois rôdeurs tous `Arriere` (x = −25) ; `decalagesDeFormation`
  gérait déjà ces cas.
- **La classe ne fixe plus l'attaque de base** (étape 91.4) : elle garde ses skills, son rang, ses
  caractéristiques de base et son équipement de départ, dont l'arme (guerrier épée longue, paladin masse, prêtre
  sceptre, mage bâton, rôdeur arc) fournit l'attaque de base, la portée et la zone de contrôle ; les skills de
  frappe et de tir exigent un type d'arme (`docs/combat.md`). Les points de vie ne viennent plus que de
  l'Endurance (base commune de 240).
- **Nouvelles classes, sans nouvelle mécanique** : paladin (`Avant`, armure 30 par son équipement, attaque de mêlée,
  skills `DegatsSurCible` et `Soin`, tous deux à 20 px) et rôdeur (`Arriere`, armure 0, attaque de tireur,
  skills `DegatsSurCible` à 200 px et `Zone`). Elles n'emploient que des `TypeSkill` existants ; les valeurs des skills sont dans `docs/combat.md`.
- **Soin au contact** : l'imposition des mains a une portée de 20 px (bord à bord) : le paladin va
  vers le vivant le plus blessé en skill en attente et commence son activation à l'arrivée, comme
  tout soin hors de portée. Le choix de la cible reste celui de `choisirCible`.
- **Limites** : au plus trois fois la même classe (`assert` dans `composer`) ; pas d'arbre de skills,
  de niveaux ni de points à dépenser ; valeurs du paladin et du rôdeur provisoires.

## Zone de contrôle (étape 77, venue de l'arme à l'étape 91)

- **La zone vient de l'arme depuis l'étape 91.4** (avant : `DefinitionPersonnage::zoneDeControle`, 20 px pour le
  guerrier et le paladin, 0 pour les autres, une valeur explicite par classe). Une attaque de base de contact
  engage (`zoneDeControleDe(nature)`, 20 px bord à bord), un projectile non : un guerrier qui reçoit un arc cesse
  d'engager, un mage qui prend une épée engage. Valeur dérivée, pas stockée : `Personnage::getZoneDeControle` la lit
  sur la nature de l'attaque de base ; `Escouade` ne fait que la transmettre. Le dégagement des tireurs
  (étape 78) s'applique à tout personnage dont la zone est 0 : aucun changement de règle.
- **`Escouade::entitesDeControle()`** donne à `Partie` une `EntiteDeControle` par personnage vivant
  (identifiant = place de 1 à N). `Escouade` ne connaît ni la règle ni les monstres : elle ne décide
  rien, elle lit. Les morts n'y sont pas.
- **Aucun changement de placement par la zone elle-même** : les places de formation et d'attaque ne
  tiennent pas compte de la zone. Un personnage qui recule de lui-même d'un monstre au contact subit donc
  le coup gratuit. Seul le dégagement des tireurs (étape 78, section suivante) a une priorité propre
  dans `pointVoulu` (la règle 2 de `choisirIntention` depuis l'étape 82.4).

## Dégagement (étape 78)

- **Une priorité de plus dans `pointVoulu`** (depuis l'étape 82.4, la règle 2 de `choisirIntention`,
  la place libre étant vérifiée avant par `confirmerDegagement`) : après l'exécution d'une action et le
  skill en attente, avant le combat verrouillé, l'agresseur et la formation. Un personnage qui se dégage reçoit comme point
  la place d'attaque autour du monstre dont il se dégage (`PlacesDAttaque::placer` avec la portée de
  l'attaque de base, avant l'étape 82.3 `pointVersMonstre` : anneau de rayon du monstre + rayon du
  personnage + 90 % de la portée). L'angle gardé, la séparation et
  les places libres sont ceux de l'étape 67 ; le monstre approché n'est pas à contourner (c'est le
  monstre de l'intention).
- **La règle d'arrêt « dès que le monstre est à portée et en vue » ne s'applique pas** pendant le
  dégagement : à portée, le tireur serait encore au contact.
- **`Personnage` n'en sait rien** : il reçoit un point. L'état (`m_degagements`) et la décision
  (`degagementVoulu`) sont dans `Escouade`, évalués avant la boucle de déplacement pour que tous
  soient jugés sur les positions du début de l'image. Voir `docs/combat.md`.

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Escouade** : l'ancre (carré vert de 20 × 20, démarre au centre de la première salle
  sur la carte générée, en `(100, 100)` seulement avec `creerNiveau()`,
  180 px/s) suit le chemin calculé par le A* puis lissé (trajets directs en
  terrain dégagé, coins contournés au plus court), s'arrête exactement sur
  le point cliqué, est bloquée par les murs et glisse le long d'eux. Son
  déplacement vit dans `Mobile`, que `Monstre` utilise aussi.
- **Personnages** (étape 74 : trois, venus d'une `Composition` injectée à `Escouade` par
  `Partie`, `composer` des classes choisies) : 3 carrés de 20 × 20 (240 px/s) tiennent une
  formation en triangle autour de l'ancre, bloqués par les murs et par les monstres.
  Chacun possède un `Mobile` : en ligne droite quand le trajet vers son
  point est libre, par un chemin A* lissé quand un mur est dans le chemin
  (recalculé seulement si la cible s'éloigne de plus d'une case de la fin
  du chemin) ; un personnage resté derrière un coin ou un pilier le
  contourne donc. La
  formation pivote progressivement vers la direction de marche (360°/s,
  par le plus court côté). Devant, le guerrier (rouge, mêlée, portée 20 px) ; derrière, le prêtre
  (jaune pâle) et le mage (violet), des tireurs (portée 120 px) ;
  portées mesurées de bord à bord ; 5 dégâts, recharge 0,3–0,7 s (mêlées) ou 0,4–0,6 s (tireurs) ; 100
  points de vie et une barre de vie au-dessus d'eux.
- **Places de formation** : la place théorique de chaque personnage
  (ancre + décalage tourné) est ramenée vers l'ancre si le trajet en ligne
  droite de l'ancre à cette place n'est pas libre (`replierVersOrigine`) :
  près d'un mur, d'un coin ou d'un pilier, la formation se resserre
  d'elle-même.
- **Places d'attaque et séparation** (étape 67) : un personnage qui approche un
  monstre (combat verrouillé, agresseur, skill de dégâts en attente) marche vers
  une **place d'attaque** sur un anneau autour du monstre (rayon du monstre + rayon
  du personnage + 90 % de la portée), distincte de celle des autres : la première
  face à l'escouade, puis alternance gauche/droite, hors mur, en vue du monstre,
  à plus de 34 px des places retenues et des personnages immobiles ; l'angle est
  gardé tant qu'il vise le même monstre. Il s'arrête toujours dès que le monstre
  est à portée et en vue. Les places de formation sont elles aussi distinctes
  (écartées à 34 px près d'un mur). Une **séparation douce** (120 px/s, plus lente
  que la marche) écarte les personnages à moins de 34 px ; elle ne pousse pas un
  personnage en exécution. Chaque personnage a un contour sombre de 1 px. Débogage :
  F3 affiche le point visé et le chemin restant de chaque personnage (cyan) et le
  chemin restant de chaque monstre (rouge) et rend semi-transparent tout monstre
  qui rentre à son poste (étape 69), F4 cache le carré vert de l'ancre.
  Voir `docs/formation.md`.
- **Mort et défaite** : un personnage à 0 point de vie tombe sur place
  (carré gris, sans barre de vie), ne frappe plus, n'est plus ciblé et ne
  bloque plus personne ; il reste dans la formation, sa place est
  simplement vide. Une fois tous morts (`Partie::estPerdue`), le monde se
  fige, un écran de défaite (voile sombre, « Défaite », « Appuyez sur R pour
  revenir au choix de l'escouade ») s'affiche et la touche R ramène à l'écran de choix (étapes 65 et 76) ;
  « Commencer » crée alors une nouvelle `Partie` (nouvelle graine), avec la même sélection.

### Limites connues

- Lors d'un demi-tour rapide, deux personnages peuvent se croiser brièvement
  (la séparation douce les écarte en moins d'une seconde) ; l'ancre traversant
  les monstres, elle (et la caméra) peut s'éloigner des personnages bloqués.
- Un personnage mort n'est jamais retiré et la formation ne se resserre
  pas.
- En combat, un personnage s'arrête où il se trouve en entrant à portée : le placement par places
  d'attaque (étape 67) ne tient pas compte de la ligne de tir.

## Bilan de l'étape 82 (82.1 à 82.5)

L'étape 82 allège `Escouade` (audit : R1, R2, D1, D3).
- **`Escouade.cpp`** : de 1 223 lignes au début de l'étape à 1 067 à la fin. Fonctions publiques : 42 au
  départ, 41 à la fin (privées : 29) ; 11 données membres. Extraits d'`Escouade` :
  `Vitalite` (82.1, commune à `Personnage` et `Monstre`), `Formation` et `Separation` (82.2),
  `PlacesDAttaque` (82.3), `Intention` (82.4, avec `cerclesAContourner` en 82.5).
- **R2 terminé (82.5)** : `attaquer` et `mettreAJour` ne se passent plus rien par des membres. Le dernier
  canal, la mémoire des agresseurs, ne garde plus que l'identifiant et la minuterie de 3 s
  (`Agresseurs::enregistrer(id)`) ; la position est lue, au moment de s'en servir, dans les ciblables
  que `mettreAJour` reçoit (`Agresseurs::vusParmi`). L'argument du brouillard, qui avait fait reculer en
  82.4, était faux : les ciblables ne sont pas filtrés par la vision. Un agresseur absent des ciblables
  est ignoré pour l'image ; la règle de retrait de la mémoire (`retirerMonstresAbsents` dans `attaquer`)
  n'a pas changé. Différences en jeu : positions plus fraîches d'une image au plus, et plus d'image
  « fantôme » vers un agresseur mort. Le paramètre `origine` de `recevoirCoup` n'avait plus d'usage :
  retiré, de même que dans `Partie::infligerAUnPersonnage`.
- **D3 (fonctions publiques que seuls les tests appellent)** : `Escouade::recevoirDegats` est retirée (les
  tests passent par l'aide `infliger`, `tests/aides_escouade.hpp`). Gardées, avec leur raison (aucun
  type extrait ni accesseur du jeu ne permet de vérifier la propriété autrement) : `getPlacesDeFormation`
  et `pointVise` (l'assemblage d'`Escouade` avec `Formation` et la résolution intention → point),
  `degagementDuPersonnage` (la règle `degagementVoulu` n'est pas extraite), `pointsDeVieDuPersonnage`
  (le seul accesseur des points de vie), `getAgresseurs` et `getProvocations` (contenu et minuteries de
  la mémoire), `choisirCible` (le choix de la cible, avant tout temps d'activation), `Partie::getMonstres`
  (l'état des monstres). Chacune porte « lu par les tests » dans son commentaire.
  `VueDesMonstres::deControle` n'a plus de valeur par défaut.
- **Limites** : voir « Intentions » (le contournement évite le cercle d'un monstre, pas sa zone de
  contrôle ; l'exigence d'un verrou pour marcher vers la cible d'un skill de dégâts en attente est une
  règle de jeu). R5 (types forts pour les places et les identifiants) est rattachée à R4 : les deux
  touchent les mêmes signatures (`AdresseSkill`, `Cible`, `EntiteVue`, identifiants des intentions).

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `Escouade` (`escouade/`) — le point de contrôle (l'ancre)

Un carré vert de 20 × 20 (l'ancre) qui suit le chemin calculé par le A*
jusqu'au point cliqué, et qui possède N `Personnage` (3 par défaut) en formation
autour d'elle (décalages de `decalagesDeFormation`). Elle porte le verrou de combat et fait attaquer ses
personnages. Elle ne connaît pas `Monstre`.

Données membres : `m_mobile` (`Mobile`), `m_formation` (`Formation`, étape 82.2 : décalages
calculés par `decalagesDeFormation`, orientation, point à regarder, vitesse de rotation et places
de formation ; voir plus bas), `m_personnages`
(`std::vector<Personnage>`), `m_cibleVerrouillee` (`std::optional<int>`),
`m_skillEnAttente` (`std::optional<SkillEnAttente>`, structure privée :
`AdresseSkill adresse` du skill, `Cible cible`, qui peut être un `CiblePoint`), `m_degagements` (`std::vector<std::optional<int>>`, par personnage : identifiant du monstre dont il se dégage vers sa place d'attaque, vide = il ne se dégage pas), `m_pointsVises` (`std::vector<sf::Vector2f>`, point vers lequel chaque
personnage marche, calculé par `mettreAJour`), `m_placesDAttaque`
(`PlacesDAttaque`, étape 82.3 : place d'attaque retenue de chaque personnage autour du monstre qu'il
approche ; voir plus bas), `m_agresseurs`
(`Agresseurs`, monstres qui ont frappé l'escouade récemment, par identifiant),
`m_provocations` (`Provocations`, monstres provoqués et leur provocateur), `m_faits` (`std::vector<FaitDEscouade>`, faits pas encore pris par `prendreFaits`). `m_cibleCombat` et
`m_monstreAPortee` ont été supprimés à l'étape 82.4 (R2, voir « Intentions »).

Structure publique `VueDesMonstres { std::vector<EntiteVue> physiques; std::vector<EntiteVue>
ciblables; std::vector<EntiteDeControle> deControle; }` (étape 82.4) : les monstres que reçoit
`mettreAJour`, en trois listes nommées. `physiques` : tous, même ceux qui rentrent à leur poste
(obstacles, et à contourner) ; `ciblables` : ceux qu'on peut viser (verrou vu, monstre à portée), sans
valeur par défaut ; `deControle` : ceux que voit la règle des zones de contrôle, avec leur zone (vide :
personne ne se dégage), sans valeur par défaut depuis 82.5. Privées : `IntentionDuSkill`, `ContexteDesIntentions`, `PointVise`, et les
étapes `confirmerDegagement`, `situationDe`, `pointDeLIntention` (voir « Intentions »).

| Fonction | Rôle |
|---|---|
| `Escouade(sf::Vector2f positionDepart, std::vector<DefinitionPersonnage> composition)` | Crée l'escouade et un personnage par définition, aux places 1 à N, en formation (décalages de `decalagesDeFormation`) ; `assert(!composition.empty())`. `Escouade` ne choisit aucune valeur |
| `void allerVers(sf::Vector2f destination, const TileMap& carte)` | Ordre du joueur : annule les coups en exécution (`annulerAttaquesEngagees`), puis délègue à `m_mobile.allerVers` |
| `void regarderVers(sf::Vector2f point)` | Délègue à `Formation::regarderVers` : une fois l'ancre arrêtée, la formation se tourne vers ce point (360°/s). Pendant la marche, l'orientation suit la marche. Effacé par `allerVers`, par l'approche du verrou (`attaquer`) et par `verrouiller`. `Escouade` ne sait pas que c'est une porte |
| `void annulerAttaquesEngagees()` | Annule le coup en exécution de chaque personnage, sans recharge consommée ; appelée par `allerVers`, `verrouiller` (autre monstre) et `Partie::cliquer` (sol) |
| `std::vector<Coup> attaquer(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur)` | `monstres` : les ciblables. Avec un verrou dont le monstre est dans `monstres` : approche tant qu'un personnage vivant n'atteint pas la cible (à portée ET en vue : `peutAtteindre`). Fait entrer en activation le skill de zone en attente dès que son lanceur vivant peut lancer au point, et le skill de dégâts en attente dès que son lanceur atteint la cible (retrouvée par son identifiant dans `monstres`), à condition que le lanceur soit libre (`Personnage::peutEngagerSkill`) ; sinon le skill reste en attente. Sans verrou, une fois le lanceur libre, un skill de dégâts dont la cible n'est plus atteinte (ou absente) est abandonné, sans recharge. Puis fait avancer l'activation de chaque skill engagé (`avancerSkills`) : l'effet part à la fin de l'activation, évalué à ce moment ; ses coups s'ajoutent à la liste. Verrou ou non : chaque personnage choisit pour lui-même le monstre à frapper parmi ceux qu'il atteint (`choisirCibleAFrapper` avec `peutAtteindre`) ; s'il est immobile, sans action engagée et son attaque prête, il **engage** un coup ; le coup engagé avance de `dt` et part à la fin de l'exécution (0,15 s), même si le monstre a bougé. Il n'est annulé que par la mort de son auteur ou la disparition de sa cible (et par un ordre du joueur ou un skill, voir ci-dessus). Renvoie un `Coup` par coup parti |
| `std::vector<Coup> declencherSkill(AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur)` | Skill à cette adresse (place du personnage, de 1 à N, et identifiant du skill). Liste vide si l'adresse est invalide (place invalide ou skill inconnu de ce personnage), si le skill est de type `Zone` (il se lance par `declencherSkillDeZone`), si le lanceur est mort, si le skill est en recharge ou en activation, si un skill est déjà en attente ou si `choisirCible` ne trouve rien. Cible atteinte (à portée ET en vue), ou provocation (sans cible) : l'**activation commence** (`Personnage::engagerSkill`) ; l'effet part à la fin (temps d'activation du skill) dans `attaquer`, ou tout de suite, dans la liste renvoyée, si l'activation est nulle. Provocation : au départ, tous les monstres de `monstres` touchés par sa zone (cercle centré sur le lanceur) ET en vue du lanceur sont provoqués, même s'il n'y en a aucun (la recharge est consommée). Dégâts : un coup (identifiant de la cible, dégâts) ; soin : appliqué sur place à l'allié engagé (liste vide). Non atteinte (seulement avec un verrou pour les dégâts), ou lanceur occupé par l'activation d'un autre de ses skills (dégâts et soin, étape 80) : le skill est mis en attente (liste vide). La provocation d'un lanceur occupé est refusée (pas d'attente sans cible) |
| `std::optional<EtatSkill> etatDuSkill(AdresseSkill adresse) const` | Skill à cette adresse tel que l'affichage en a besoin (`EtatSkill`) ; vide si l'adresse est invalide |
| `std::optional<TypeSkill> typeDuSkill(AdresseSkill adresse) const` | Type du skill à cette adresse ; vide si l'adresse est invalide ; `[[nodiscard]]` |
| `std::vector<AdresseSkill> adressesDesSkills() const` | Adresses de tous les skills : par place croissante, puis dans l'ordre de la liste de chaque personnage ; donnée à `barreParDefaut` ; `[[nodiscard]]` |
| `std::optional<sf::Color> couleurDuPersonnage(std::size_t place) const` | Couleur du personnage de cette place (bande de l'icône) ; vide si la place est invalide ; `[[nodiscard]]` |
| `bool skillEstPret(AdresseSkill adresse) const` | Le personnage de cette adresse est-il vivant, avec ce skill dont la recharge est écoulée ? Faux si l'adresse est invalide |
| `float rayonZoneDuSkill(AdresseSkill adresse) const` | Rayon de zone du skill à cette adresse ; 0 si l'adresse est invalide |
| `Cercle cercleDePorteeDuSkill(AdresseSkill adresse) const` | Cercle centré sur le lanceur, de rayon (rayon du personnage + portée du skill) ; rayon 0 si la place est invalide |
| `std::vector<Coup> declencherSkillDeZone(AdresseSkill adresse, sf::Vector2f point, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur)` | Lance le skill de type `Zone` à cette adresse sur un cercle centré sur `point` (rayon de zone du skill). Point à portée (bord à bord, comme un cercle de rayon 0) et en vue du lanceur : l'activation commence tout de suite ; au départ, un `Coup` (identifiant, dégâts du skill) par monstre touché (bord à bord 0 ET en vue du point d'impact : `ligneDeVue`, en cases), zone évaluée à ce moment. Le sort lance la recharge même si la zone est vide. Hors portée, pas en vue ou lanceur occupé par l'activation d'un autre de ses skills : le skill est mis en attente (`CiblePoint`), liste vide, sans recharge ; le lanceur marche vers le point (`mettreAJour`) et son activation commence dans `attaquer` dès que portée et vue sont réunies et que le lanceur est libre. Liste vide, sans recharge ni attente, si le point est dans un mur, si l'adresse est invalide, si son skill n'est pas de type `Zone`, si le lanceur est mort ou en recharge ou en activation, ou si un skill est déjà en attente |
| `std::optional<Cible> choisirCible(TypeSkill type, AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte) const` | `DegatsSurCible` : avec un verrou, le monstre verrouillé trouvé dans `monstres` (atteint ou non, vide s'il n'y est pas) ; sans verrou, le monstre que le lanceur de l'adresse frapperait en autodéfense, avec la portée de son skill et en vue (vide si aucun). `Soin` : le vivant qui a le moins de points de vie parmi ceux qui ne sont pas à pleine vie (égalité : la plus petite place) |
| `void mettreAJour(float dt, const TileMap& carte, const VueDesMonstres& monstres)` | Avance l'ancre, tourne la formation et calcule la place de chaque personnage (`m_formation.mettreAJour`, après le déplacement de l'ancre, avec la direction de marche lue avant ; place théorique ramenée vers l'ancre par `replierVersOrigine` si elle est dans un mur ou cachée derrière un coin, puis écartée à 34 px des places déjà calculées et sortie des monstres — une place dont le centre est à moins de (rayon du monstre + rayon du personnage) d'un monstre est invalide), fait écouler le délai d'oubli des agresseurs, met à jour les dégagements (`mettreAJourDegagements` : une fois par image, d'après les positions du début de l'image, avec `monstres.deControle`), cherche le verrou dans `monstres.ciblables` et avance le skill en attente (un soin entre ici en activation à l'arrivée, annulé si le lanceur ou la cible meurt ou si la cible est à pleine vie). Puis, pour chaque personnage par indice croissant : situation, `choisirIntention`, point visé (mémorisé dans `m_pointsVises`), règle d'oubli des places, déplacement (voir « Intentions ») ; les cercles de `monstres.physiques` sont les obstacles bloquants des personnages, jamais de l'ancre, et chacun contourne en plus tous ceux qui ne sont pas le monstre de son intention. Enfin, sépare les personnages trop proches (`separerPersonnages`, qui délègue le calcul à `pousseesDeSeparation` : 34 px, poussée plafonnée à 120 px/s, sans obstacle dur). Aucun ordre d'appel imposé avec `attaquer` (étape 82.4) |
| `void dessiner(sf::RenderTarget& rendu, bool afficherAncre) const` | Dessine l'ancre (si `afficherAncre`) et les N personnages |
| `std::optional<int> degagementDuPersonnage(std::size_t place) const` | Identifiant du monstre dont le personnage de cette place se dégage ; vide s'il ne se dégage pas ou si la place est invalide (lu par les tests) ; `[[nodiscard]]` |
| `std::optional<sf::Vector2f> centreDuPersonnage(std::size_t place) const` | Centre du personnage de cette place, mort ou vivant ; vide si la place est invalide (position des événements de combat côté personnages) ; `[[nodiscard]]` |
| `std::vector<FaitDEscouade> prendreFaits()` | Les faits survenus depuis le dernier appel (par `declencherSkill` et `attaquer`), dans l'ordre ; la liste est vidée. `FaitDEscouade = std::variant<SoinApplique, SkillPart, MonstreProvoque>` (publics) : `SoinApplique { std::size_t place; int montant; sf::Vector2f position; bool critique; }` (le personnage soigné, les points réellement rendus — jamais 0 —, son centre à ce moment, et si le soin était un critique, étape 89.4), `SkillPart { std::size_t place; IdSkill skill; sf::Vector2f position; }` (un skill qui part, à la fin de son activation ; jamais l'attaque de base), `MonstreProvoque { int idMonstre; }` (un par monstre provoqué). Même rôle que les `Coup` pour les dégâts, sans changer le type de retour d'`attaquer` ; `Partie` en fait des événements ; `[[nodiscard]]` |
| `std::optional<sf::Vector2f> pointVise(std::size_t place) const` | Point vers lequel le personnage de cette place (1 à N) marche, tel que le dernier `mettreAJour` l'a calculé ; vide si la place est invalide ; lu par les tests (la résolution intention → point est l'assemblage d'`Escouade`) |
| `void dessinerPointsVises(sf::RenderTarget& rendu) const` | Débogage (F3) : un trait jaune du centre de chaque personnage vivant à son point visé, un petit cercle à ce point, et le chemin restant en cyan (`dessinerTraceDeChemin`) |
| `sf::Vector2f getCentre() const` | Centre de l'ancre (utilisé par la caméra) |
| `std::optional<EntiteVue> proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const` | Proie : le provocateur si ce monstre est provoqué et qu'il est vivant ; sinon le personnage vivant le plus proche de `positionMonstre` ; vide si aucun n'est vivant. `id` = place (1 à 4), `cercle` = celui du personnage |
| `std::vector<EntiteVue> candidatsDeFrappe(int idMonstre) const` | Personnages que ce monstre peut frapper (`id` = place) : seulement son provocateur s'il est provoqué et vivant (provocation stricte), sinon tous les personnages vivants ; vide si aucun n'est vivant |
| `bool placeEstVivante(std::size_t place) const` | Place valide ET personnage vivant |
| `ResultatDuCoup recevoirCoup(int idMonstre, std::size_t place, const Attaque& attaque, Generateur& generateur)` | Applique l'attaque au personnage de cette place (le coup d'un monstre, que `Partie` désigne par `proieDuMonstre` ou par la place engagée) ; sans effet (rien : montant 0) si la place est invalide ou le personnage mort. Si `attaque.montant > 0`, enregistre le monstre comme agresseur, par son identifiant seulement (ou remet son temps à 3 s). Renvoie le résultat du coup (`resoudreCoup`, qui tire dans `generateur`) : les dégâts réellement subis (après critique éventuel et armure). `recevoirDegats(idMonstre, origine, quantite)` a été retirée en 82.5 (D3) : les tests passent par les aides `infliger` et `subirCoup` |
| `bool estVivante() const` | Vrai si au moins un personnage est vivant |
| `bool unVivantPresDe(sf::FloatRect zone, float ecart) const` | Un personnage vivant est-il à `ecart` ou moins (bord à bord, `distanceCercleRectangle`) de cette zone ? Escouade ne connaît ni les portes ni les objets ; `[[nodiscard]]` |
| `std::vector<Cercle> cerclesDesVivants() const` | Cercles des personnages vivants |
| `std::vector<EntiteDeControle> entitesDeControle() const` | Une `EntiteDeControle` (id = place de 1 à N, cercle, zone de contrôle) par personnage vivant ; les morts n'engagent personne et ne sont pas engagés ; `Partie` en fait une des deux photos de chaque image ; `[[nodiscard]]` |
| `std::optional<Attaque> attaqueDeBase(std::size_t place) const` | Attaque de base de ce personnage (dégâts avant l'armure de la cible, capacité offensive), pour son coup gratuit ; vide si la place est invalide ; `[[nodiscard]]` (renommée depuis `puissanceDeLAttaqueDeBase` à l'étape 89.3) |
| `std::optional<StatistiquesDeCombat> statistiquesDeCombatDuPersonnage(std::size_t place) const` | Statistiques de combat de ce personnage (capacités offensive et défensive tirées de ses caractéristiques effectives, plus esquives et blocage de son équipement) ; vide si la place est invalide ; lu par l'écran d'inventaire ; `[[nodiscard]]` |
| `std::optional<int> pointsDeVieDuPersonnage(std::size_t place) const` | Points de vie du personnage de cette place ; vide si la place est invalide ; lu par les tests (le seul accesseur des points de vie) ; `[[nodiscard]]` |
| `std::vector<Cercle> cerclesDeVision() const` | Un `Cercle` (centre du personnage, rayon de vision, 320 px) par personnage vivant ; `Partie` les donne à `Vision::mettreAJour` ; `[[nodiscard]]` |
| `void verrouiller(int idCible)` | Pose le verrou de combat sur ce monstre, en remplaçant la cible précédente ; annule un skill de dégâts en attente si l'identifiant change |
| `void deverrouiller()` | Retire le verrou de combat et annule un skill de dégâts en attente |
| `std::optional<int> getCibleVerrouillee() const` | Identifiant du monstre engagé en combat, vide si aucun |
| `const std::vector<sf::Vector2f>& getPlacesDeFormation() const` | Places de formation (après repli) calculées par le dernier `mettreAJour` ; façade de `Formation::getPlaces`, lue par les tests (l'assemblage d'`Escouade` avec `Formation` ne s'observe que là) |
| `void provoquer(int idMonstre, std::size_t place, float duree)` | Enregistre ce monstre comme provoqué par le personnage de la place `place` (1 à N) pour `duree` secondes, ou remet sa durée au maximum s'il l'est déjà. Fin : durée écoulée ou provocateur mort (`mettreAJour`), monstre absent de `monstres` (`attaquer`) |
| `const Inventaire& sac() const` / `const Equipement& equipementDuPersonnage(std::size_t place) const` | Le sac (40 cases, `s_capaciteDuSac`) et l'équipement du personnage de cette place (`assert` : place valide) ; lus par l'écran d'inventaire ; `[[nodiscard]]` |
| `bool ramasser(Objet)` | Met l'objet dans la première case libre du sac ; faux si le sac est plein ; `[[nodiscard]]` |
| `std::optional<Objet> objetA(const PositionDObjet&) const` | L'objet à cette position (case du sac ou emplacement d'un personnage), s'il y en a un ; `assert` : place valide, case < capacité ; lu par l'écran d'inventaire ; `[[nodiscard]]` |
| `bool peutDeplacerObjet(const PositionDObjet& de, const PositionDObjet& vers) const` | Copie le sac et les équipements (`etatDesObjets`, privée) et applique `deplacementPossible` (règle de base et deux mains) ; la surbrillance de l'écran d'inventaire s'en sert ; `[[nodiscard]]` |
| `std::vector<SeuilDeCaracteristique> seuilsManques(const PositionDObjet& de, const PositionDObjet& vers) const` | Les seuils que le personnage qui recevrait l'objet de `de` à `vers` n'atteindrait pas (vide si rien ne manque ou si le déplacement est refusé pour une autre raison) ; l'écran les montre en rouge (étape 92.3) ; `[[nodiscard]]` |
| `bool objetPorteEstInactif(std::size_t place, Emplacement) const` | L'objet porté là est-il inactif (porté, seuil non atteint) ? Faux sans objet (étape 92.3) ; `[[nodiscard]]` |
| `const Equipement& equipementActifDuPersonnage(std::size_t place) const` | L'équipement du personnage sans ses objets inactifs (étape 92.2) ; `[[nodiscard]]` |
| `bool deplacerObjet(const PositionDObjet& de, const PositionDObjet& vers)` | Déplace sur une copie (`deplacerLesObjets`) ; faux sans rien changer si le déplacement est refusé ; sinon recopie le sac, puis dans chaque personnage les différences d'équipement (retraits, puis poses : chaque appel recalcule armure, caractéristiques, défenses et attaque de base) : échange si `vers` est occupé, objet rangé dans le sac pour les deux mains. Abandonne le skill en attente dont le lanceur n'a plus l'arme exigée. Transactionnelle, valable pour un mort ; voir `docs/objets.md` ; `[[nodiscard]]` |
| `const std::vector<Provocation>& getProvocations() const` | Monstres actuellement provoqués, avec leur provocateur et le temps restant ; lue par les tests (`candidatsDeFrappe` ne montre ni l'un ni l'autre) |
| `const std::vector<Agresseur>& getAgresseurs() const` | Monstres qui ont frappé l'escouade récemment : identifiant et temps restant (oubliés après 3 s sans coup, ou absents de `monstres` dans `attaquer`) ; lue par les tests |

### `Personnage` (`escouade/`) — un membre de l'escouade

Un carré coloré de 20 × 20 qui va vers le point que lui donne `Escouade`,
frappe à portée, a des points de vie et peut mourir. Ne connaît ni la
formation, ni `Escouade`, ni `Monstre`.

Données membres : `m_mobile` (`Mobile`, porte la position et la vitesse),
`m_couleur`, `m_caracteristiquesDeBase` (déclaré avant l'équipement : les seuils en dépendent), `m_equipement` (`Equipement`, ce qu'il porte), `m_equipementActif` (`Equipement`, ce qui a un effet, étape 92.2 ; seul `recalculerDepuisLEquipement` l'écrit), `m_descriptionDeBase` (`DescriptionDAttaque` : celle de l'équipement, que `m_attaqueDeBase` met en œuvre), `m_attaqueDeBase` (`Skill`, refait quand l'arme change), `m_rechargeARelancer` (`bool`, étape 91.4 : l'arme a changé, la recharge complète reste à tirer), `m_skills`
(`std::vector<SkillConnu>`, sans doublon d'identifiant), `m_cibleChemin` (`std::optional<sf::Vector2f>`,
cible du dernier A*), `m_caracteristiques` (`Caracteristiques` : les effectives, base plus bonus de l'équipement actif), `m_vitalite` (`Vitalite` : points de vie et armure), `m_rayonVision` (`float`, pixels), `m_actionEngagee` (`std::optional<ActionEngagee>`, l'action en exécution — attaque de base ou skill, avec son `IdSkill` — et sa cible ; une seule à la fois ; lue seulement pendant l'exécution).

| Fonction | Rôle |
|---|---|
| `Personnage(sf::Vector2f centre, DefinitionPersonnage definition)` | Crée le personnage à ce centre depuis sa définition (étape 88.1, C2 : un agrégat à la place de sept paramètres ; `Escouade` lui passe la définition de la composition). Avec la couleur, les skills (`assert` : pas deux fois le même `IdSkill`), cet équipement de départ (son armure, son attaque de base et sa zone de contrôle en sont calculées, `docs/objets.md`) et ce rayon de vision (tous fournis par `Composition`, via `Escouade`). Depuis l'étape 91.4, plus d'attaque de base ni de zone de contrôle dans la définition, et depuis 91.2 plus de points de vie de base (`pointsDeVieMax(caracteristiques)`) |
| `const Equipement& getEquipement() const` | Ce que le personnage porte, objets inactifs compris ; `[[nodiscard]]` |
| `const Equipement& getEquipementActif() const` | Ce qui a un effet : l'équipement sans ses objets inactifs (seuil non atteint, `Equipement::actifPour`), recalculé à chaque changement d'équipement (étape 92.2) ; son armure, ses bonus, son arme et son bouclier sont ceux du personnage ; `[[nodiscard]]` |
| `const Caracteristiques& getCaracteristiquesDeBase() const` | Celles de sa classe, sans l'équipement : la base des seuils d'équipement (étape 92.1) ; `[[nodiscard]]` |
| `const Caracteristiques& getCaracteristiques() const` | Ses caractéristiques effectives : la base de sa classe plus les bonus de son équipement, recalculées en entier à chaque changement d'équipement (`recalculerDepuisLEquipement`, privée, qui change aussi le maximum de points de vie — 240 + 6 × Endurance depuis l'étape 91.2 —, les statistiques de combat et les défenses de la `Vitalite` ; `docs/combat.md`) ; `[[nodiscard]]` |
| `const StatistiquesDeCombat& getStatistiques() const` | Ses statistiques de combat : capacités offensive et défensive (`50 + 5 × Adresse` effective) plus les bonus de combat de son équipement — esquives, blocage (`statistiquesDuPersonnage`, seul endroit qui les calcule, rappelée par `recalculerDepuisLEquipement`) ; `[[nodiscard]]` |
| `std::optional<Objet> poserEquipement(Emplacement, Objet)` / `std::optional<Objet> retirerEquipement(Emplacement)` | Portent un objet (`assert(peutAller)`) ou l'enlèvent, et rendent l'objet remplacé ou retiré ; l'armure et les caractéristiques sont recalculées en entier depuis l'équipement (`recalculerDepuisLEquipement`, privée, seul appelant de `Vitalite::definirDefenses`) ; permis pour un mort ; `[[nodiscard]]` |
| `float getRayonVision() const` | Rayon de vision, en pixels ; `[[nodiscard]]` |
| `void relancerSiLArmeAChange(Generateur& generateur)` | Si l'arme a changé depuis le dernier appel, relance au complet la recharge de l'attaque de base (tirage dans l'intervalle de la nouvelle arme) ; `Escouade::attaquer` l'appelle au début de chaque image ; tant qu'elle est à relancer, `engagerAttaque` refuse (étape 91.4) |
| `bool armeConvientAuSkill(IdSkill id) const` | L'arme portée permet-elle ce skill (`Skill::convientA`) ? Faux sans ce skill (étape 91.5) ; `[[nodiscard]]` |
| `float getZoneDeControle() const` / `Attaque getAttaqueDeBase() const` | Zone de contrôle (0 : il n'engage personne) et attaque de base (dégâts, capacité offensive) ; de simples lectures, elles ne touchent ni la recharge ni le coup engagé ; `[[nodiscard]]` |
| `void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles, const std::vector<Cercle>& aContourner)` | Si vivant, avance vers la cible (en ligne droite si le trajet est libre de murs ET ne passe pas près d'un des `aContourner` — `traverseUnObstacle` —, sinon par un chemin A* lissé qui les contourne, recalculé si la cible s'éloigne de plus d'une case de sa fin ou si `estBloque()` ; collision avec les murs et les cercles `obstacles`, glissement) puis décrémente la recharge de l'attaque et de chacun de ses skills ; sinon ne fait rien |
| `void decaler(sf::Vector2f decalage, const TileMap&, const std::vector<Cercle>&)` | Pousse le personnage vivant (délègue à `Mobile::decaler`) ; sans effet s'il est mort |
| `bool estEnMouvement() const` / `float getPorteeAttaqueDeBase() const` | Façades de `Mobile::estEnMouvement` et de la portée de l'attaque de base ; `[[nodiscard]]` |
| `std::vector<sf::Vector2f> cheminRestant() const` | Façade de `Mobile::cheminRestant` (débogage F3) ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine le personnage (contour sombre de 1 px vers l'intérieur, éclairci de 50 % pendant l'exécution d'un coup ou l'activation d'un skill) et sa barre de vie ; carré gris sans barre s'il est mort |
| `bool estAPortee(Cercle cible) const` | L'écart bord à bord entre son cercle et `cible` est-il `<=` la portée de `m_attaqueDeBase` ? |
| `bool engagerAttaque(int idCible)` | Engage un coup d'attaque de base sur ce monstre : faux s'il est mort, en mouvement (`Mobile::estEnMouvement`), si une action (attaque ou skill) est déjà engagée, ou si l'attaque n'est pas prête (`Skill::commencer`). La portée et la vue sont vérifiées par `Escouade`, qui tente l'engagement à chaque image (l'échec est normal) ; `[[nodiscard]]` |
| `std::optional<int> cibleEngagee() const` | Monstre visé par le coup de base en exécution ; vide hors exécution (et pendant l'activation d'un skill) |
| `std::optional<Attaque> avancerAttaque(float dt, Generateur& generateur)` | Fait avancer l'exécution ; rend l'attaque (dégâts et capacité offensive) au départ du coup, vide sinon ; `[[nodiscard]]` |
| `void annulerAttaque()` | Annule le coup de base en exécution, sans recharge consommée ; sans effet sur l'activation d'un skill (`annulerSkill`) |
| `bool estEnExecution() const` | Un coup de base en exécution ou un skill en activation : le personnage reste sur place |
| `std::vector<IdSkill> idsDesSkills() const` | Identifiants de ses skills, dans l'ordre de sa liste ; `[[nodiscard]]` |
| `sf::Color getCouleur() const` | Sa couleur ; `[[nodiscard]]` |
| `std::optional<TypeSkill> typeSkill(IdSkill id) const` | Type de ce skill, vide s'il ne le connaît pas (comme les cinq suivantes : un identifiant inconnu = pas de skill) |
| `bool skillEstPret(IdSkill id) const` | Vivant, avec ce skill en phase prête (recharge écoulée, pas en activation) et l'arme qu'il exige (étape 91.5) |
| `std::optional<EtatSkill> etatDuSkill(IdSkill id) const` | Type, progression de la recharge, temps restant, état de vie et activation en cours (vraie seulement si c'est ce skill qui est engagé) ; vide sans ce skill |
| `bool skillEstAPortee(IdSkill id, Cercle cible) const` | Ce skill est-il à portée de `cible` (bord à bord) ? Faux sans ce skill |
| `Cercle zoneDuSkill(IdSkill id) const` | Cercle centré sur le personnage, de rayon le rayon de zone de ce skill (0 sans ce skill) |
| `Cercle cercleDePorteeDuSkill(IdSkill id) const` | Cercle centré sur le personnage, de rayon son rayon + la portée de ce skill (rayon seul sans ce skill) |
| `bool engagerSkill(IdSkill id, std::optional<Cible> cible)` | Engage ce skill : faux si `peutEngagerSkill(id)` est faux. Ne demande pas d'immobilité. Annule le coup de base en exécution. La portée et la vue sont vérifiées par `Escouade`, qui ne l'appelle que si `peutEngagerSkill` est vrai (`assert` du résultat) ; la cible (vide pour la provocation) est stockée sans être interprétée ; `[[nodiscard]]` |
| `bool peutEngagerSkill(IdSkill id) const` | Les conditions d'`engagerSkill`, seule source de vérité : vivant, skill connu et prêt, arme exigée portée, aucun skill en activation (le lanceur est libre) ; `[[nodiscard]]` |
| `std::optional<ActionEngagee> skillEngage() const` | L'action `Skill` en activation, avec son `IdSkill` et sa cible ; vide sinon |
| `std::optional<Attaque> avancerSkill(float dt, Generateur& generateur)` | Fait avancer l'activation ; rend l'attaque du skill (montant et capacité offensive ; pour une provocation, le montant est sa durée) au départ de l'effet (et relance au complet la recharge de `m_attaqueDeBase`), vide sinon ; `[[nodiscard]]` |
| `void annulerSkill()` | Annule l'activation, sans recharge consommée |
| `int recevoirSoin(int quantite)` | Rend des points de vie, sans dépasser le maximum ; sans effet sur un mort (délègue à `Vitalite::soigner`) ; renvoie les points réellement rendus |
| `int getPointsDeVie() const` / `int getPointsDeVieMax() const` | Points de vie actuels et maximum |
| `ResultatDuCoup recevoirDegats(const Attaque& attaque, Generateur& generateur)` | Délègue à `Vitalite::recevoirDegats` : critique éventuel, armure, puis retire les dégâts sans descendre sous zéro ; renvoie le résultat (dégâts après armure, non plafonnés par les points de vie restants, critique ou non), rien pour un mort |
| `bool estVivant() const` | Points de vie `> 0` |
| `sf::Vector2f getCentre() const` | Centre du personnage |
| `Cercle getCercle() const` | Cercle centré sur le personnage, de rayon 10 (façade de `Mobile::getCercle`) |

### `Formation` (`escouade/`) — décalages, orientation et places de formation

`Formation.hpp` / `.cpp` (étape 82.2) : possédée par valeur par `Escouade` (`m_formation`), sans
héritage. Ne connaît ni `Personnage` ni `Escouade` : elle reçoit le centre de l'ancre, sa direction de
marche, les rayons des personnages, la carte et les cercles des monstres. Voir « Formation » plus haut.

Données membres : `m_decalages` (`std::vector<sf::Vector2f>`, un par personnage, pour une formation
orientée vers +x), `m_orientation` et `m_orientationVoulue` (`sf::Angle`, 0 = vers +x),
`m_pointARegarder` (`std::optional<sf::Vector2f>`), `m_vitesseRotation` (`float`, degrés par
seconde), `m_places` (`std::vector<sf::Vector2f>`, place de chaque personnage après repli, écartement
et sortie des monstres).

| Fonction | Rôle |
|---|---|
| `Formation(std::vector<sf::Vector2f> decalages, sf::Vector2f centre, float vitesseRotation)` | Orientation de départ 0 ; places de départ : les places théoriques autour de `centre`, sans repli (les personnages y naissent) |
| `void regarderVers(sf::Vector2f point)` | Mémorise le point vers lequel la formation se tourne une fois l'ancre arrêtée |
| `void oublierPointARegarder()` | Efface ce point ; appelée par `Escouade` à chaque nouvel ordre à l'ancre (`allerVers`, approche du verrou, `verrouiller`) |
| `void mettreAJour(float dt, sf::Vector2f centre, std::optional<sf::Vector2f> directionDeMarche, const std::vector<float>& rayons, const TileMap& carte, const std::vector<Cercle>& monstres)` | Le seul appel par image, après le déplacement de l'ancre (étape 82.3). Orientation voulue (`choisirOrientation`) : la direction de marche ; à l'arrêt (vide), la direction du point à regarder s'il y en a un et qu'il n'est pas sur `centre` ; sinon inchangée. Puis rotation vers l'orientation voulue (plus court côté, au plus `m_vitesseRotation × dt`), puis places autour de `centre` : place théorique repliée (`replierVersOrigine`), remplacée si elle est à moins de 34 px d'une place déjà calculée ou dans un monstre par le premier point valide et libre des anneaux de 34, 68 et 102 px. `rayons` : un par personnage (`assert` sur la taille) |
| `const std::vector<sf::Vector2f>& getPlaces() const` | Place de chaque personnage, dans l'ordre des décalages, calculée par le dernier `mettreAJour` ; `[[nodiscard]]` |

Privées : `placeTheorique(indice, centre)` (`centre + m_decalages[indice].rotatedBy(m_orientation)`),
`choisirOrientation(directionDeMarche, centre)` (publique à l'étape 82.2), `tourner(dt)`,
`calculerPlaces(centre, rayons, carte, monstres)`.

### `PlacesDAttaque` (`escouade/`) — les places d'attaque autour des cibles

`PlacesDAttaque.hpp` / `.cpp` (étape 82.3) : possédée par valeur par `Escouade` (`m_placesDAttaque`),
sans héritage. Ne connaît ni `Personnage` ni `Escouade` ; vocabulaire neutre (assaillant, cible,
occupant). Voir « Places d'attaque » et « Classe `PlacesDAttaque` » plus haut.

Structures publiques, sans valeur par défaut (une initialisation désignée qui oublie un champ est
signalée par `-Wmissing-field-initializers`) : `Place { int idCible; float rayon; sf::Angle angle; }`
(place retenue : l'anneau et l'angle autour de la cible ; la position se recalcule depuis le centre
actuel de la cible) ; `Approche { EntiteVue cible; std::size_t assaillant; Cercle cercleAssaillant;
float portee; sf::Vector2f reference; }` (un assaillant, d'indice 0 à N-1, qui approche une cible avec
une action de cette portée, bord à bord ; la première candidate est dans la direction de la cible
vers `reference`).

Données membres : `m_places` (`std::vector<std::optional<Place>>`, par assaillant ; vide = il
n'approche aucune cible), `m_ecartMinimal` (`float`, pixels).

| Fonction | Rôle |
|---|---|
| `PlacesDAttaque(std::size_t nombreDAssaillants, float ecartMinimal)` | Aucune place retenue ; `ecartMinimal` : distance en dessous de laquelle une place n'est pas libre (34 px dans le jeu) |
| `std::optional<sf::Vector2f> placer(const Approche& approche, const std::optional<Place>& ancienne, const std::vector<std::optional<sf::Vector2f>>& occupants, const TileMap& carte)` | Point de la place d'attaque de l'assaillant, sur un anneau de rayon : rayon de la cible + rayon de l'assaillant + 90 % de la portée. L'angle d'`ancienne` est gardé si elle est autour de la même cible, valide et pas occupée ; sinon, la place valide (carré de l'assaillant hors mur, en vue du centre de la cible) et libre (à plus de `ecartMinimal` des places retenues des autres assaillants autour de cette cible, et des autres occupants) la plus proche de l'angle actuel de l'assaillant (`choisirAngleLibre`). `occupants` : par assaillant, son centre s'il occupe sa position, vide sinon ; celui de l'assaillant est ignoré (`assert` sur la taille). Trouvée, la place devient la place retenue de l'assaillant ; vide si aucune n'est libre, la place retenue ne change pas ; `[[nodiscard]]` |
| `bool peutPlacer(const Approche& approche, const std::optional<Place>& ancienne, const std::vector<std::optional<sf::Vector2f>>& occupants, const TileMap& carte) const` | `placer` trouverait-il une place, avec les mêmes entrées ? Rien n'est retenu (étape 82.4 : la question du dégagement) ; `[[nodiscard]]` |
| `std::optional<Place> placeRetenue(std::size_t assaillant) const` | La place retenue de l'assaillant ; vide s'il n'en a pas (étape 82.4) ; `[[nodiscard]]` |
| `void oublierSaufAutourDe(std::size_t assaillant, std::optional<int> idCible)` | La règle d'oubli : la place retenue est gardée si elle est autour de cette cible, oubliée sinon (vide : toujours oubliée) (étape 82.4) |

Privée : `trouverPlace(approche, ancienne, occupants, carte) const`, la place que `placer` retiendrait,
sans rien retenir ; `peutPlacer` l'interroge, `placer` la complète. `retirer`, `retenir` et
`cibleApprochee` (le mécanisme provisoire de 82.3) ont été supprimés à l'étape 82.4.

### `Intention` (`escouade/`) — ce que veut faire un personnage

`Intention.hpp` / `.cpp` (étape 82.4) : des types et trois fonctions libres et pures, sans état. Ne
connaît ni `Personnage`, ni `Escouade`, ni `PlacesDAttaque`, ni la carte. Voir « Intentions » plus haut.

`struct ResterSurPlace { std::optional<int> placeAutourDe; }`, `struct AllerAuPoint { sf::Vector2f
point; }`, `struct ApprocherMonstre { int idMonstre; Cercle cercle; float portee; }` (chacune avec un
`operator==` par défaut, pour les tests ; `Cercle` en a reçu un aussi) ; `using Intention =
std::variant<ResterSurPlace, AllerAuPoint, ApprocherMonstre>`. `struct SituationDuPersonnage { bool
vivant; bool enExecution; std::optional<int> monstreDeLaPlace; std::optional<Intention> skillEnAttente;
std::optional<EntiteVue> degagement; std::optional<EntiteVue> verrou; bool verrouAtteint; bool
ancreArretee; std::optional<EntiteVue> agresseur; bool monstreAPortee; sf::Vector2f placeDeFormation;
float porteeAttaqueDeBase; }`, sans valeur par défaut (remplie en initialisation désignée par
`Escouade::situationDe` : un champ oublié est signalé par `-Wmissing-field-initializers`).

| Fonction | Rôle |
|---|---|
| `Intention choisirIntention(const SituationDuPersonnage& situation)` | Les règles 0 à 5 par priorité, la première qui s'applique gagne (voir « Intentions ») ; les cercles sont transmis tels quels ; `[[nodiscard]]` |
| `std::optional<int> monstreDeLIntention(const Intention& intention)` | Le monstre de l'intention : celui qu'elle approche, ou celui autour duquel elle garde la place ; vide pour un point. Il n'est pas à contourner, et c'est autour de lui que la place d'attaque est gardée ; `[[nodiscard]]` |
| `std::vector<Cercle> cerclesAContourner(const std::vector<EntiteVue>& monstres, const Intention& intention)` | Les cercles à contourner en marchant : tous les monstres sauf celui de l'intention (`monstreDeLIntention`). Ne reçoit rien du placement : l'exclusion ne peut pas dépendre de la réussite de la recherche d'une place ; `[[nodiscard]]` |

### `Separation` (`escouade/`) — la séparation douce

`Separation.hpp` / `.cpp` (étape 82.2) : une énumération, une structure et une fonction libre et
pure, sans état. Appelée par `Escouade::separerPersonnages`, qui traduit ses personnages en entrées
et applique les poussées avec `Personnage::decaler`. Voir « Séparation douce » plus haut.

`enum class EtatDeSeparation { Absent, Fixe, Libre }` (mort ; en exécution ou en activation : pousse
sans être poussé ; libre) ; `struct MembreASeparer { sf::Vector2f centre; EtatDeSeparation etat =
EtatDeSeparation::Absent; }`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2f> pousseesDeSeparation(const std::vector<MembreASeparer>& membres, float ecartMinimal, float vitesseMax, float dt)` | Chaque paire de membres présents à moins de `ecartMinimal` est écartée le long de la droite de leurs centres, chacun de la moitié du manque (un fixe n'est pas poussé, l'autre prend tout ; deux fixes : rien ; centres confondus : axe des x, le plus petit indice vers la gauche). Calcul simultané sur les centres reçus ; la poussée totale de chacun est plafonnée à `vitesseMax × dt`. Une poussée par membre, dans le même ordre ; `[[nodiscard]]` |

### `EcartEntrePersonnages` (`escouade/`) — en-tête neutre

`EcartEntrePersonnages.hpp` (étape 82.2) : `inline constexpr float s_ecartEntrePersonnages = 34.0f`,
distance minimale voulue entre deux centres de personnages, entre deux places, ou entre une place et
un personnage immobile. Inclus par `Escouade` et `Formation` sans qu'elles dépendent l'une de l'autre ;
`Escouade` le passe à la construction de `PlacesDAttaque` (étape 82.3).

### `Composition` (`escouade/`) — de quoi est faite l'escouade

`Composition.hpp` / `.cpp` : une énumération, une structure et deux fonctions libres, sans état. `Escouade` ne choisit plus aucune valeur : elle reçoit sa composition. Voir `docs/formation.md`.

`enum class Rang { Avant, Arriere }` ; `struct DefinitionPersonnage { sf::Color couleur; std::vector<SkillConnu> skills = {}; std::vector<ObjetDeDepart> equipementDeDepart = {}; float rayonVision; Caracteristiques caracteristiques = {}; Rang rang = Avant; }` (`equipementDeDepart` : emplacement et objet de ce que le personnage porte à sa création, voir `docs/objets.md` : son armure, son attaque de base et sa zone de contrôle en viennent ; depuis l'étape 91, ni `attaqueDeBase`, ni `zoneDeControle`, ni points de vie de base).

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2f> decalagesDeFormation(const std::vector<Rang>& rangs)` | Pure. Pour une escouade orientée vers +x : les `Avant` sur x = +25, les `Arriere` sur x = −25 ; dans un rang de n personnages, le k-ième (ordre d'apparition) est sur y = (k − (n−1)/2) × 50. Dans l'ordre de `rangs` ; `[[nodiscard]]` |

### `Specialite` (`escouade/`) — les classes du catalogue

`Specialite.hpp` / `.cpp` : des données et trois fonctions libres, sans état, **sans hiérarchie de classes** (pas de sous-classes `Guerrier`, `Mage`…). Une spécialité est une recette de départ. Seule source des valeurs de personnages (tailles de portée, armure, skills, couleurs). Voir `docs/formation.md`.

`enum class IdSpecialite { Guerrier, Pretre, Mage, Paladin, Rodeur }` ; `struct Specialite { IdSpecialite id; std::string nom; DefinitionPersonnage definition; }` (`definition.couleur` : la couleur de la classe, sans nuance).

| Fonction | Rôle |
|---|---|
| `const std::vector<Specialite>& catalogue()` | Les 5 classes, dans l'ordre de `IdSpecialite` (guerrier, prêtre, mage, paladin, rôdeur), construites une seule fois (variable `static` locale) ; `[[nodiscard]]` |
| `const Specialite& specialite(IdSpecialite id)` | L'entrée de cet identifiant (indexée par l'enum, `assert` sur l'identifiant) ; `[[nodiscard]]` |
| `std::vector<DefinitionPersonnage> composer(const std::vector<IdSpecialite>& identifiants)` | Une définition par identifiant, dans l'ordre reçu (c'est l'ordre des places). La k-ième occurrence d'une même classe (k = 0, 1, 2) a sa couleur assombrie de 0 %, 30 % puis 55 % (`assombrir`) ; `assert` : pas plus de 3 fois la même classe ; `[[nodiscard]]` |

Classes (équipement de départ depuis l'étape 91.4) : guerrier (`Avant`, rouge, armure 9, épée longue et bouclier rond, `FrappePuissante` et `Provocation`), prêtre (`Arriere`, jaune pâle, sceptre, `Soin`), mage (`Arriere`, violet, bâton, `Explosion`), paladin (`Avant`, orange, armure 9, masse et bouclier rond, `FrappeSacree` et `ImpositionDesMains`), rôdeur (`Arriere`, vert, arc, `TirPercant` et `PluieDeFleches`). Rayon de vision 320 px pour toutes. Zone de contrôle : celle de l'arme (20 px pour le guerrier et le paladin, au contact ; 0 pour les trois autres). Points de vie : 240 + 6 × Endurance (guerrier 324, paladin 318, prêtre et rôdeur 300, mage 288). Les skills de frappe et de tir exigent un type d'arme. Valeurs des skills dans `docs/combat.md`.

### `Encerclement` (`monde/`) — les places d'un anneau

`Encerclement.hpp` / `.cpp` : trois fonctions libres, sans état, sans dépendance à `Escouade` ni à `Personnage` (réutilisable plus tard par des monstres qui encerclent). Utilisées par `PlacesDAttaque` pour les places d'attaque autour d'une cible (par `Escouade` avant l'étape 82.3) et par `Formation` pour les places de formation de secours autour de l'ancre (depuis l'étape 82.2). Voir `docs/formation.md`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Angle> anglesCandidats(sf::Angle depart, float rayon, float ecartMin)` | Angles des places d'un anneau : n places régulières, n le plus grand nombre tel que la **corde** entre voisines soit `>= ecartMin` (au moins 1) ; le premier angle est `depart`, puis alternance gauche/droite (0, +θ, −θ, +2θ…) |
| `sf::Vector2f positionDePlace(sf::Vector2f centre, float rayon, sf::Angle angle)` | Point de l'anneau à cet angle |
| `std::optional<sf::Angle> choisirAngleLibre(const std::vector<sf::Angle>& candidats, sf::Angle angleActuel, const std::function<bool(sf::Angle)>& estLibreEtValide)` | Le candidat accepté le plus proche de l'angle actuel (plus court côté ; égalité : le premier) ; vide si aucun |
