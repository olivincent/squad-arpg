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
  - **chevauchement résiduel lors d'un demi-tour rapide** : les 4
    décalages forment deux paires diagonales exactement opposées, donc
    les places *idéales* de deux personnages ne se chevauchent jamais
    pendant la rotation. Mais chaque personnage poursuit sa cible en
    ligne droite, pas en suivant l'arc : lors d'un demi-tour très
    rapide, il coupe à travers le centre de la formation pendant qu'un
    autre fait le trajet inverse. La rotation progressive réduit
    nettement le phénomène sans l'éliminer ; depuis l'étape 67, la
    séparation douce écarte les personnages trop proches (« Séparation
    douce »). Un déplacement qui suit l'arc reste possible.

## Le personnage

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
- **Points de vie et barre de vie** : 100 points, portés par un
  `PointsDeVie` (étape 53, partagé par valeur avec `Monstre`, sans
  héritage). `recevoirDegats`, `recevoirSoin` et `estVivant()` sont des
  façades qui lui délèguent : `Escouade` et les tests n'ont pas changé, et
  l'objet n'est pas exposé (loi de Déméter). Barre dessinée par la
  fonction commune `dessinerBarreDeVie` (module `BarreDeVie`), à qui
  `Personnage` passe `getRatio()`.
- **Il délègue son attaque à `m_attaqueDeBase`** et son skill (un
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

- **Priorité du point visé** dans `Escouade::mettreAJour` (étape 57 : en
  tête, un personnage qui exécute un coup reste sur place, règle 0) : la place en
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

- **Priorité du point visé** dans `Escouade::mettreAJour` (`pointVoulu`), du plus
  faible au plus fort : la place en formation ; la place d'attaque autour de
  l'agresseur (ou le centre du personnage s'il a déjà un monstre à portée),
  seulement si l'ancre est à l'arrêt ; en combat verrouillé, le centre du
  personnage s'il est à portée du monstre verrouillé, sinon sa place d'attaque
  autour de ce monstre ; la cible du skill en attente (pour des dégâts, sa
  place d'attaque). Un coup ou un skill en exécution garde le personnage sur place. Le détail et le
  pourquoi sont dans `docs/combat.md` (« Agresseurs »).
- **« Ancre à l'arrêt »** = `m_mobile.directionVoulue()` vide, lu après le
  déplacement de l'ancre dans la même image.
- **`Personnage` n'en sait rien** : il reçoit toujours un simple point.

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
  chevauche aucun mur et si elle voit le centre du monstre (`ligneDeVue`).
  **Elle est libre** si elle est à plus de 34 px de la place retenue par un
  autre personnage pour ce monstre (quel que soit l'anneau) ET à plus de 34 px
  du centre de tout autre personnage vivant immobile (occupation réelle : un
  mêlée qui frappe en autodéfense sur place n'a pas de place retenue, mais il
  occupe l'endroit où il se tient).
- **Attribution** : la place valide et libre la plus proche de l'angle actuel
  du personnage autour du monstre (`choisirAngleLibre`), égalité : l'ordre des
  candidates. Aucune place : le personnage vise le centre du monstre
  (comportement d'avant).
- **Stabilité** : `Escouade` retient par personnage `{ idMonstre, rayon,
  angle }` (`m_placesDAttaque`). L'angle est gardé tant que le personnage
  approche ce même monstre ; seule la position est recalculée à chaque image
  depuis le centre actuel du monstre. Le rayon est gardé aussi, parce que le
  test « libre » doit connaître l'anneau de chaque autre personnage (mêlées et
  tireurs n'ont pas le même). La place est réattribuée si elle devient
  invalide (mur, plus de vue) ou occupée. Elle est oubliée dès qu'aucune règle
  de `pointVoulu` ne la renouvelle : autre monstre, plus de monstre, mort du
  personnage, disparition du monstre. Un coup en cours ou l'arrêt à portée sur
  le même monstre ne la font pas oublier.
- **Limites** : la place est calculée avec la direction de l'ancre au moment
  de l'attribution ; deux personnages qui s'approchent depuis des côtés
  opposés se gênent encore parfois ; pas de places d'attaque pour les monstres.

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
d'avancer :

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
places de formation). Chaque personnage a aussi un contour sombre de 1 px
(vers l'intérieur) pour distinguer deux personnages collés.

## Contourner les monstres (étape 68)

- **`Escouade::mettreAJour` reçoit les monstres en `EntiteVue`** (identifiant et
  cercle), pas seulement des cercles : il lui faut l'identifiant pour savoir
  quel monstre un personnage approche. Elle en tire les obstacles bloquants
  (tous les cercles) et, par personnage, la liste à contourner
  (`monstresAContourner`, privée).
- **Qui contourne quoi** : chaque personnage contourne tous les monstres **sauf
  celui qu'il approche** : le monstre de sa place d'attaque (`m_placesDAttaque` :
  combat verrouillé, agresseur, skill de dégâts en attente) ou, en combat sans
  place retenue, le monstre verrouillé. Sans monstre approché, tous. Il ne
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
(qui reçoit les monstres) déplace donc une telle place avec la **même
recherche que l'étape 67** : premier point valide (trajet libre depuis l'ancre,
hors mur, hors monstre) et libre (34 px des places déjà calculées) sur les
anneaux de 34, 68 et 102 px autour de l'ancre. Aucun point : si seul un monstre
gênait, la place ramenée par le repli est gardée (comportement d'avant) ; si
une autre place la gênait aussi, le centre de l'ancre. Ne concerne que les
places de formation : les places d'attaque sont déjà hors du monstre. Sans
monstre proche, rien ne change.

## Places de formation distinctes (étape 67)

Près d'un mur, `replierVersOrigine` ramenait plusieurs places au même point
(souvent le centre de l'ancre). `calculerPlaces` traite maintenant les
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
  formation). La formation (R2c) n'est pas extraite.
- **Ordre d'appel** : `attaquer` avant `mettreAJour` dans une même image,
  documenté au-dessus de la classe : `m_monstreAPortee` et `m_cibleCombat`
  passent de l'une à l'autre. `m_monstreAPortee` est un
  `std::array<bool, s_nombrePersonnages>` (et non un `std::vector<bool>`,
  spécialisation particulière).
- **Constantes** : tailles, vitesses, points de vie et attaque de base
  (`attaqueDeBase(portee)`, un seul endroit) ont des constantes `s_` ;
  `Personnage::m_taille` est supprimé.
- **Barre de vie** : module `BarreDeVie`, partagé avec `Monstre`. La barre
  orange de recharge reste dans `Personnage::dessiner`, placée sous la barre
  de vie grâce à `s_hauteurBarreDeVie`.
