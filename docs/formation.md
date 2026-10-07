# Squad-ARPG — Formation

## Escouade

- **État de jeu séparé de l'affichage** : le `sf::RectangleShape` de
  l'ancre n'existe que dans `dessiner`. Même modèle que `TileMap`.
- **Composition avec `Mobile` depuis l'étape 23** : la position, le
  chemin, la vitesse et la collision/glissement de l'ancre ne sont plus
  des données d'`Escouade` elle-même — elles vivent dans `m_mobile`
  (voir `docs/chemin.md`), qu'`Escouade` délègue (`allerVers`,
  `getCentre`, l'arrêt dans `attaquer`, la partie « ancre » de
  `mettreAJour`).
- **Dépendance à `TileMap` le temps d'un appel, mais plus directe** :
  la carte est reçue en `const TileMap&` par `allerVers`, `attaquer` et
  `mettreAJour`, et transmise telle quelle à `m_mobile` — `Escouade` ne
  la garde pas en membre et ne l'inspecte jamais elle-même.
- **Déclaration anticipée malgré tout** : `Escouade.hpp` contient
  `class TileMap;` (une référence ne demande pas la définition
  complète), uniquement pour ces paramètres de passage. Depuis
  l'étape 23, `Escouade.cpp` n'inclut même plus `TileMap.hpp` (ni
  `Chemin.hpp`, ni `Deplacement.hpp`) : ces dépendances sont désormais
  celles de `Mobile.cpp`, pas les siennes.
- **En-têtes SFML précis** dans `Escouade` (`Vector2.hpp`,
  `RenderTarget.hpp`, `RectangleShape.hpp`…), alors que `TileMap.hpp`
  inclut tout `<SFML/Graphics.hpp>`. Non harmonisé pour l'instant.
- **`explicit`** sur le constructeur à un paramètre, pour interdire la
  conversion implicite `sf::Vector2f` → `Escouade`.
- **Noms d'intention** : `allerVers` plutôt que `setCible` ; `getCentre`
  encapsule le calcul « coin haut-gauche + moitié de la taille », pour que
  `main.cpp` ne dépende pas de la convention de position.

## Formation de l'escouade

- **Un personnage sait seulement « aller vers un point »** (décision de
  conception retenue en vue de l'avenir — combat, actions) : il reçoit
  sa cible en paramètre de `mettreAJour`, sans connaître la formation ni
  son décalage. C'est `Escouade` qui décide de ce point et le lui donne.
- **`Escouade` possède les personnages** (`std::vector<Personnage>`
  stocké par valeur) et les décalages de formation ; elle reste le
  point de contrôle dessiné (le carré vert), dont le comportement
  (chemin, vitesse, collision, `getCentre`) n'a pas changé.
- **`placePersonnage(indice)` : un seul point de calcul** pour la place
  d'un personnage (`getCentre() + m_decalages[indice].rotatedBy(
  m_orientation)`), utilisé à la fois dans le constructeur (pour créer
  les personnages à leur place) et dans `mettreAJour` (pour leur donner
  leur cible à chaque tic) — éviter de dupliquer cette formule à deux
  endroits qui pourraient diverger.
- **Ordre dans `mettreAJour` et `dessiner`** : l'ancre d'abord, les
  personnages ensuite. Les personnages visent la place de l'image
  *courante* (calculée après que l'ancre a bougé), pas celle d'avant.
- **Module `Deplacement` extrait** (voir la description du module dans
  `ARCHITECTURE.md`) pour que `Personnage` réutilise la même
  collision/glissement que l'ancre, sans dépendre d'`Escouade`.
- **Orientation voulue et orientation réelle séparées** : `m_orientationVoulue`
  reçoit la direction voulue de l'ancre (vecteur vers le point courant du
  chemin, *avant* l'essai de collision — si l'escouade est bloquée par un
  mur et glisse, la formation vise quand même la direction qu'elle
  essayait de suivre) ; mise à jour seulement si l'ancre a un point à
  atteindre et que la distance est `> 0` (`sf::Vector2f::angle()` sur un
  vecteur nul n'a pas de sens), sinon la dernière valeur est conservée.
  `m_orientation`, elle, est ce qu'utilise réellement `placePersonnage`.
- **Rotation progressive, par le plus court côté** (étape 14) :
  `m_orientation` se rapproche de `m_orientationVoulue` à chaque
  `mettreAJour`, que l'ancre bouge ou non (si elle s'arrête en pleine
  rotation, la formation finit quand même de pivoter sur place).
  L'écart, `(m_orientationVoulue - m_orientation).wrapSigned()`, est
  ramené dans `[-180°, 180°]` pour que passer de 350° à 10° tourne de
  +20° et non de -340°. La rotation par image est bornée à
  `m_vitesseRotation * dt` (`m_vitesseRotation` = 360°/s, fixe, comme
  `m_vitesse`) ; si l'écart restant est inférieur ou égal à ce pas,
  `m_orientation` prend exactement la valeur voulue (même logique que
  le déplacement à pas fixe de l'ancre).
- **Limites connues** : pas de collision entre personnages ni entre un
  personnage et l'ancre ; un personnage peut rester accroché derrière un
  coin de mur tant que l'escouade ne repasse pas par là.
  - **Chevauchement résiduel lors d'un demi-tour rapide** : les 4
    décalages forment deux paires diagonales exactement opposées
    (`avant-gauche`/`arrière-droite`, `avant-droite`/`arrière-gauche`),
    donc les places *idéales* de deux personnages quelconques ne se
    chevauchent jamais pendant la rotation. Mais chaque personnage
    poursuit sa cible en ligne droite (`Personnage::mettreAJour`), pas
    en suivant l'arc de la rotation : lors d'un demi-tour très rapide,
    la cible d'un personnage peut passer presque instantanément de
    l'avant à l'arrière (ou l'inverse), et il coupe alors tout droit à
    travers le centre de la formation — pendant qu'un autre personnage
    fait le trajet inverse au même moment. La rotation progressive
    réduit nettement ce phénomène (elle a remplacé un saut instantané
    par un mouvement continu), sans l'éliminer. Le corriger demanderait
    une vraie collision personnage-personnage, ou un déplacement qui
    suit l'arc plutôt qu'une ligne droite — les deux explicitement hors
    de portée de l'étape 14.

## Attaque par les personnages (étape 17)

- **La portée, les dégâts et la recharge appartiennent à `Personnage`**,
  pas à `Escouade` ni à `Monstre` : à terme, ces valeurs dépendront de
  l'arme et des capacités de chacun. Pour l'instant, valeurs fixes dans
  le constructeur, identiques pour les 4, comme `m_vitesse` :
  `m_portee` (80 px, distance du centre du personnage au point visé),
  `m_degats` (5), `m_tempsRecharge` (0,5 s, ne change jamais) et
  `m_recharge` (secondes avant le prochain coup possible, 0 à la
  création — le premier coup part donc immédiatement).
- **`m_recharge` décrémentée dans `Personnage::mettreAJour`**, sans
  descendre sous zéro : le temps passe même quand le personnage
  n'attaque pas, comme la recharge d'attaque de l'étape 16 dans
  `main.cpp` avant d'être déplacée ici.
- **`Personnage` ne connaît toujours pas `Monstre`** : il raisonne sur un
  simple point (`sf::Vector2f`), pas sur une cible typée.
  - `estAPortee(cible)` : distance entre le centre et `cible`
    `<=` `m_portee`.
  - `frapper(cible)` : si la cible est à portée et que `m_recharge` est
    à zéro, relance la recharge (`m_tempsRecharge`) et renvoie
    `m_degats` ; sinon renvoie 0, sans toucher la recharge (un coup
    tenté hors de portée ne la déclenche pas).
- **`Escouade` ne connaît pas `Monstre` non plus.** `attaquer(cible,
  carte)` raisonne sur le même point que `Personnage` :
  - si au moins un personnage n'est pas à portée (positions
    *actuelles*, avant le tic) : `allerVers(cible, carte)`, comme un
    ordre de déplacement classique ;
  - si les 4 sont à portée : l'ancre s'arrête sur place (`m_chemin`
    réduit à son centre actuel, comme dans le constructeur), pour
    qu'elle cesse d'avancer sans pour autant reculer ;
  - dans les deux cas, `frapper(cible)` est ensuite appelée sur chaque
    personnage, et la fonction renvoie la somme des dégâts — `main.cpp`
    la passe directement à `monstre.recevoirDegats`.
- **La formation tient pendant l'attaque** : les personnages continuent
  de viser leur place en formation (`placePersonnage`) dans
  `mettreAJour`, qu'`attaquer` ait été appelée ou non ce tic-là ; rien
  ne change dans leur déplacement.
- **Limite connue** : la portée ignore les murs — un personnage peut
  frapper à travers un mur s'il est à moins de 80 px de la cible, même
  sans ligne de vue dégagée. Accepté pour l'instant ; une vérification
  de ligne de vue (via `TileMap::trajetLibre`, par exemple) reste une
  option pour plus tard.

## Points de vie des personnages et riposte des monstres (étape 20)

- **`Personnage` gagne les mêmes points de vie que `Monstre`** :
  `m_pointsDeVie`/`m_pointsDeVieMax` (100 chacun), `recevoirDegats`
  (jamais sous zéro) et `estVivant()`, plus une barre de vie dans
  `dessiner`, visuellement identique à celle de `Monstre` (fond sombre
  + rectangle vert proportionnel, au-dessus du personnage). Duplication
  assumée entre les deux classes — aucune n'hérite de l'autre, aucune
  structure commune n'a été introduite ; ce sera factorisé dans une
  étape dédiée si le besoin se confirme.
- **`Personnage::getCentre() const`** ajoutée (même rôle que
  `Monstre::getCentre`) : nécessaire pour qu'`Escouade` puisse comparer
  les positions des 4 personnages entre eux et avec un point donné.
- **`Escouade` désigne elle-même le personnage le plus proche d'un
  point**, sans passer par `Monstre` ni connaître la notion de monstre :
  - `indicePersonnageLePlusProche(point)` (privée) : seul endroit du
    code qui fait cette recherche (boucle sur les 4 personnages,
    comparaison de distances au centre) ;
  - `personnageLePlusProche(point)` (publique, lecture) : renvoie le
    centre de ce personnage ;
  - `recevoirDegats(origine, quantite)` (publique) : applique les
    dégâts au personnage le plus proche d'`origine`, via le même indice.
  Les deux fonctions publiques partagent la même fonction privée pour
  garantir qu'elles désignent toujours le même personnage par le même
  critère, même appelées séparément par `main.cpp` sur deux lignes
  consécutives.
- **`Monstre` riposte avec la même logique de portée/recharge que
  `Personnage`**, elle aussi dupliquée plutôt que partagée :
  `m_portee` (80 px), `m_degats` (10, le double d'un personnage, pour
  qu'un monstre seul reste dangereux face à 4 personnages),
  `m_tempsRecharge` (1 s, plus lent qu'un personnage), `m_recharge`.
  `mettreAJour(dt)` ne décrémente que la recharge (le monstre reste
  immobile) ; `frapper(cible)` a exactement le même contrat que
  `Personnage::frapper`. `Monstre` ne connaît toujours pas `Escouade`
  ni `Personnage` : il raisonne sur un simple `sf::Vector2f`, comme
  `Personnage::frapper` raisonne sans connaître `Monstre`.
- **`main.cpp` fait l'intermédiaire, en miroir de l'attaque des
  personnages (étape 17)** : pour chaque monstre, à chaque image,
  `monstre.mettreAJour(dt)` puis `escouade.recevoirDegats(origine,
  monstre.frapper(escouade.personnageLePlusProche(origine)))`, où
  `origine` est `monstre.getCentre()`. Les deux appels à `Escouade`
  (`personnageLePlusProche` pour donner la cible au monstre,
  `recevoirDegats` pour appliquer le résultat) utilisent la même
  `origine`, donc désignent forcément le même personnage.
- **Un monstre frappe qu'il soit lui-même attaqué ou non** : sa riposte
  ne dépend pas du verrou de cible (`idCible`) ni de l'attaque des
  personnages — uniquement de la distance et de sa propre recharge.
- **Limite connue** : un personnage à 0 point de vie ne meurt pas — il
  reste en formation et continue de recevoir des coups (voir la suite,
  étape 21, pour le fait qu'il ne frappe plus et n'est plus ciblé).
  Traiter la mort d'un personnage (retrait de la formation, etc.) est
  hors de portée de cette étape.

## Un monstre change de cible quand un personnage meurt (étape 21)

- **Le filtre est dans `Escouade::indicePersonnageLePlusProche`
  seulement** : elle ignore désormais les personnages dont
  `estVivant()` est faux. `personnageLePlusProche` et `recevoirDegats`
  n'ont pas changé de logique, ils héritent du filtre via cette
  fonction privée commune. `Monstre` n'est pas modifié : il continue de
  frapper un simple `sf::Vector2f` donné par `main.cpp`.
- **L'absence de personnage vivant est exprimée dans le type, pas par
  une valeur sentinelle** : `indicePersonnageLePlusProche` renvoie
  `std::optional<std::size_t>`, `personnageLePlusProche` renvoie
  `std::optional<sf::Vector2f>` (vide = aucun vivant),
  `recevoirDegats` ne fait rien si l'indice est vide.
- **C'est `Personnage::frapper` qui refuse de frapper quand il n'est
  pas vivant** (il connaît son propre état), pas `Escouade::attaquer` :
  elle ne filtre rien et continue d'appeler `frapper` sur les 4
  personnages sans distinction ; celui qui est mort renvoie simplement
  0 sans relancer sa recharge.
- **Dans `main.cpp`, la riposte des monstres devient conditionnelle** :
  `if (auto cible = escouade.personnageLePlusProche(origine))` avant
  d'appeler `monstre.frapper` et `escouade.recevoirDegats` ;
  `monstre.mettreAJour(dt)` reste appelée dans tous les cas, qu'il y
  ait un vivant ou non.
- **Un personnage mort reste en formation, se déplace et est dessiné
  comme avant** (barre de vie vide) — seule sa participation au combat
  change (il ne frappe plus, n'est plus ciblé). Retirer un personnage
  mort de la formation ou du vecteur, le griser, et la condition de
  défaite restent hors de portée de cette étape.

## Classe `Attaque`, partagée par `Personnage` et `Monstre` (étape 22)

- **Composition, pas héritage** : la duplication assumée depuis
  l'étape 20 (portée, dégâts, recharge, identiques dans leur structure
  mais pas dans leurs valeurs) est extraite dans une classe `Attaque`,
  possédée par valeur (`m_attaque`) par `Personnage` et par `Monstre`.
  Aucune classe de base commune, aucune fonction virtuelle : les deux
  classes restent par ailleurs indépendantes (points de vie, barre de
  vie, dessin — toujours dupliqués, non touchés par cette étape).
- **`Attaque` ne connaît pas la position de son propriétaire** : elle
  reçoit `origine` en paramètre de `estAPortee` et `frapper`, plutôt
  que de stocker elle-même un centre. Elle ne connaît ni `TileMap`, ni
  `Personnage`, ni `Monstre`, ni `Escouade`, et ne dessine rien.
- **Les trois valeurs (portée, dégâts, temps de recharge) restent des
  paramètres du constructeur** (`Attaque(float portee, int degats,
  float tempsRecharge)`), puisqu'elles diffèrent déjà entre les deux
  propriétaires : 80 px / 5 / 0,5 s pour `Personnage`, 80 px / 10 / 1 s
  pour `Monstre`.
- **La règle « un personnage mort ne frappe pas » (étape 21) reste dans
  `Personnage::frapper`**, avant la délégation à `m_attaque` : être
  vivant est l'affaire de `Personnage`, pas de son attaque. `Monstre`
  délègue entièrement à `m_attaque`, sans règle supplémentaire.
- **Interfaces publiques de `Personnage` et de `Monstre` inchangées** ;
  `Escouade` et `main.cpp` ne connaissent pas `Attaque` et n'ont pas été
  modifiés.
- **Tests** : `test_Attaque.cpp` couvre maintenant les six cas de
  recharge/portée (dégâts à portée, 0 hors de portée, 0 au second appel
  immédiat, 0 après une durée trop courte, dégâts de nouveau après une
  durée généreuse, un coup hors de portée ne déclenche pas la
  recharge). `test_Personnage.cpp` et `test_Monstre.cpp` ne gardent que
  les cas qui prouvent le branchement et les valeurs propres à chaque
  classe (dégâts à portée avec leur propre valeur, 0 hors de portée,
  nouveau coup après une durée généreuse — preuve que `mettreAJour`
  avance bien la recharge de `m_attaque` —, et pour `Personnage`
  seulement, 0 quand il n'est pas vivant).

## Les monstres détectent, poursuivent et abandonnent (étape 24)

- **`Monstre` perd `m_centre`/`m_taille` au profit d'un `Mobile`**
  (`m_mobile`, taille 30 × 30, vitesse 120 px/s — plus lent que les
  personnages à 240 px/s, pour qu'un groupe reste rattrapable). Une
  seule copie de la position et de la taille existe désormais (lue via
  `m_mobile.getCentre()`/`getTaille()`), là où `Escouade` et `Monstre`
  dupliquaient chacun ces données avant l'extraction de `Mobile` à
  l'étape 23.
- **`Monstre` ne connaît toujours ni `Escouade` ni `Personnage`** :
  c'est `main.cpp` qui lui donne sa proie, comme il lui donnait déjà la
  cible de `frapper` depuis l'étape 20. Nouvelle signature de
  `mettreAJour` : `(float dt, std::optional<sf::Vector2f> cible, const
  TileMap& carte)` — `cible` vide quand `Escouade::personnageLePlusProche`
  ne renvoie personne (aucun personnage vivant).
- **But du monstre : se mettre à portée pour frapper, pas rejoindre sa
  proie.** En poursuite, il avance vers la cible (`m_mobile.allerVers`,
  A* + lissage, comme l'ancre ou les personnages) tant qu'il n'est pas
  à portée de `m_attaque`, puis s'arrête sur place
  (`m_mobile.arreter()`) — même logique que `Escouade::attaquer`
  s'arrêtant une fois ses 4 personnages à portée (étape 17), mais pour
  un seul mobile plutôt que pour quatre.
- **État de poursuite avec hystérésis, plutôt qu'une réaction à la
  seule distance instantanée** : `m_enPoursuite` (faux à la création)
  passe à vrai dès que la cible entre dans `m_rayonDetection` (200 px),
  et ne repasse à faux que lorsqu'elle sort de `m_rayonAbandon`
  (400 px, plus large) ou disparaît (`cible` vide). Entre les deux
  rayons, l'état ne change pas : sans ce second seuil, un monstre et sa
  proie resteraient à une distance qui oscille autour d'un rayon
  unique, et le monstre s'arrêterait puis repartirait sans cesse. Hors
  poursuite, le monstre ne retourne pas à un poste : il reste où il
  s'est arrêté (limite acceptée, voir plus bas).
- **Toutes les distances (détection, abandon, portée d'attaque) sont à
  vol d'oiseau, centre à centre, et ignorent les murs** — la détection
  hérite de la même simplification que la portée (étape 17, limite
  connue) : un monstre peut détecter et poursuivre une proie à travers
  un mur, sans ligne de vue.
- **Dans `main.cpp`, la cible est lue avant le déplacement du monstre,
  son centre après** : pour chaque monstre,
  `escouade.personnageLePlusProche(m.getCentre())` est calculé sur la
  position du monstre *avant* `mettreAJour` (comme pour la riposte
  depuis l'étape 20), puis transmis à `m.mettreAJour(dt, cible, carte)`
  ; `m.getCentre()` est relu *après* pour la riposte, sur la position à
  jour — sinon la riposte frapperait depuis la position du tic
  précédent.
- **Nouveaux emplacements** : les trois monstres de l'étape 18 sont
  remplacés par six, répartis sur des centres de case de sol, hors de
  la salle de départ, pour que la poursuite ait la place de s'exprimer
  sans pousser l'escouade hors de la carte dès le début.
- **Limites connues** : un monstre qui abandonne reste sur place plutôt
  que de retourner à son poste (pas de patrouille) ; plusieurs monstres
  qui poursuivent la même cible peuvent se superposer (pas de collision
  entre monstres, ni entre un monstre et l'escouade/les personnages —
  cohérent avec l'absence de collision déjà actée pour un monstre
  immobile). Les deux sont hors de portée de cette étape.

## Statut du monstre et retour au poste (étape 25)

Corrige la première limite connue de l'étape 24 : un monstre qui
abandonne sa proie retourne maintenant à son poste au lieu de rester
sur place. L'occasion aussi de remplacer `m_enPoursuite` (un `bool`)
par un statut explicite, et de revoir ce qui déclenche l'abandon.

- **`enum class Etat { EnGarde, Poursuite, Retour }`, privé**, plutôt
  que des booléens : un type à trois valeurs rend impossible l'état
  incohérent « en poursuite et en retour à la fois », que deux `bool`
  indépendants (`m_enPoursuite`, un hypothétique `m_enRetour`)
  auraient permis d'exprimer par erreur. Même principe que `Tile` dans
  `TileMap`, mais encore plus local : `Etat` n'existe que pour
  `Monstre`, pas même visible depuis `Monstre.hpp` à l'extérieur de la
  classe.
- **« À portée, en train de frapper » n'est pas un quatrième statut** :
  ni en `Poursuite` ni ailleurs — c'est recalculé à chaque image à
  partir de la distance (`m_attaque.estAPortee`), exactement comme
  avant cette étape. Ajouter un statut pour ça aurait dupliqué une
  information déjà disponible par ailleurs, au risque de la désynchroniser.
- **`m_poste`, le centre donné au constructeur, jamais modifié** :
  l'ancre fixe à laquelle le monstre revient. Contrairement à
  `m_mobile.getCentre()` (qui bouge), `m_poste` ne change jamais après
  la construction.
- **Le territoire remplace la distance à la proie pour l'abandon** :
  `m_rayonAbandon` (étape 24, mesurait la distance monstre-proie) est
  renommé `m_rayonTerritoire` et change de sens — distance maximale
  entre le **monstre** et son **poste** (toujours 400 px). Un monstre
  peut désormais poursuivre une proie arbitrairement loin de son point
  de départ initial, tant que lui-même ne s'éloigne pas de plus de
  400 px de son poste ; avant, une proie qui s'enfuyait tout en restant
  proche du poste pouvait quand même déclencher un abandon si elle
  s'éloignait du *monstre* au-delà de 400 px — l'ancienne règle, jugée
  moins cohérente avec l'idée d'un territoire, a été entièrement
  supprimée (pas conservée en option).
- **Détection unique, dans une fonction privée `const`
  `proieDetectee(cible)`** : une proie est détectée si elle existe, si
  elle est à `<=` `m_rayonDetection` **du monstre** ET à `<=`
  `m_rayonTerritoire` **du poste**. La seconde condition est
  nécessaire : sans elle, un monstre qui vient de rentrer au bord de
  son territoire redétecterait aussitôt une proie restée juste à la
  frontière (puisqu'elle est probablement encore à moins de 200 px du
  monstre à cet instant), et le monstre tremblerait sur place entre
  `Poursuite` et `Retour` au lieu de rentrer franchement. Utilisée à
  l'identique en `EnGarde` (déclenche `Poursuite`) et en `Retour`
  (peut relancer `Poursuite`) — la même fonction, appelée aux deux
  mêmes endroits, garantit qu'un monstre en garde et un monstre qui
  rentre appliquent exactement la même règle, sans légère divergence
  possible entre deux copies du calcul.
- **Transitions, une seule par appel de `mettreAJour`**, dans un
  `switch` sur `m_etat` : `EnGarde` → `Poursuite` si `proieDetectee` ;
  `Poursuite` → `Retour` si plus de proie, ou si le monstre est à `>`
  `m_rayonTerritoire` de son poste ; `Retour` → `Poursuite` si
  `proieDetectee` (vérifié avant l'arrivée, de façon à ce qu'une proie
  qui redevient pertinente au moment même où le monstre atteint son
  poste reprenne le dessus) ; `Retour` → `EnGarde` sinon, une fois
  arrivé (`m_mobile.directionVoulue()` vide).
- **Détecter l'entrée dans un état, pas seulement l'état courant** :
  l'ordre `arreter()` puis `allerVers(m_poste, carte)` ne doit être
  donné qu'une seule fois, au moment où le monstre **devient**
  `Retour` — le redonner à chaque image écraserait inutilement le
  chemin déjà calculé par `Mobile` (et, avec le A*, recalculerait un
  chemin identique à chaque tic pour rien). La fonction compare l'état
  avant et après le `switch` (`etatPrecedent != m_etat`) pour ne
  déclencher cet ordre qu'à la transition elle-même. En `Poursuite`,
  à l'inverse, l'ordre (`arreter()` si à portée, sinon `allerVers`)
  est bien redonné à chaque image, comme avant l'étape 25 : la cible
  elle-même peut bouger d'une image à l'autre (c'est un personnage de
  l'escouade), contrairement au poste qui est fixe.
- **La riposte ne dépend pas du statut** : `frapper` n'a pas changé, et
  `main.cpp` n'a pas été modifié — un monstre en retour frappe toujours
  ce qui passe à sa portée, exactement comme avant cette étape.
  Interface publique de `Monstre` inchangée.
- **Limites connues** : un monstre qui rentre ignore une proie dès
  qu'elle sort de son territoire, même si elle reste proche de lui (le
  filtre `proieDetectee` l'exige par construction) ; pas de patrouille
  ni de récupération de points de vie pendant le retour ; chaque
  monstre garde son propre poste et territoire sans les partager ni se
  coordonner avec les autres monstres.

## Mort d'un personnage et défaite de l'escouade (étape 26)

Corrige la limite connue de l'étape 20 : un personnage à 0 point de
vie restait en formation, continuait de se déplacer et d'être dessiné
comme un vivant, et l'escouade restait pilotable même avec ses 4
personnages morts.

- **Un mort reste dans `m_personnages`, il n'est pas retiré** —
  contrairement à `Monstre`, où `main.cpp` fait `std::erase_if` sur le
  vecteur. La raison tient à `m_decalages` : l'indice `i` d'un
  personnage dans `m_personnages` désigne aussi son décalage de
  formation (`m_decalages[i]`, voir `placePersonnage`). Retirer un
  élément décalerait tous les indices suivants, et associerait chaque
  survivant au décalage d'un autre — `std::erase_if` n'est donc pas une
  option ici, sans changer aussi `m_decalages` en conséquence (hors de
  portée de cette étape). La formation ne se resserre donc pas : la
  place du mort reste simplement vide.
- **« Un mort ne bouge pas » vit dans `Personnage::mettreAJour`**,
  retour immédiat si `!estVivant()` — même endroit, même garde que
  « un mort ne frappe pas » dans `Personnage::frapper` depuis
  l'étape 21. `Escouade::mettreAJour` n'a pas été modifiée : elle
  continue d'appeler les 4 personnages sans distinction, c'est
  `Personnage` qui sait quoi faire de son propre état.
- **`Escouade::attaquer` ignore les morts pour décider de s'arrêter**
  (et non plus seulement pour frapper, comme depuis l'étape 21) :
  la condition d'approche ne porte que sur les personnages **vivants**.
  Sans ce changement, un mort immobile resté loin derrière — puisqu'il
  ne bouge plus — aurait empêché la condition « tous à portée » de
  jamais devenir vraie, et l'escouade aurait continué à avancer vers la
  cible indéfiniment. Si aucun personnage n'est vivant, « tous les
  vivants sont à portée » est vrai par vacuité (une quantification
  universelle sur un ensemble vide est toujours vraie) : l'escouade
  s'arrête, ce qui est le comportement voulu plutôt qu'un cas à exclure
  explicitement.
- **Apparence d'un mort, dans `Personnage::dessiner` seulement** : un
  carré gris (`sf::Color(100, 100, 100)`, une valeur fixe, pas un
  nouveau membre — `m_couleur` reste celle reçue au constructeur, juste
  ignorée) et aucune barre de vie (`dessiner` revient juste après avoir
  dessiné le carré). Aucun changement ailleurs : la place en formation,
  elle, reste déterminée par `Escouade::placePersonnage` comme pour un
  vivant (le mort continue d'en avoir une, simplement vide puisqu'il ne
  s'y déplace plus).
- **`Escouade::estVivante()` est une simple lecture** (vraie si au
  moins un personnage est vivant) : `Escouade` ne décide de rien
  elle-même en cas de défaite, elle se contente d'exposer l'état.
  C'est `main.cpp` qui choisit quoi en faire — même séparation que pour
  `idCible`, où `Monstre` ne sait pas lui-même s'il est visé.
- **`main.cpp` bloque les ordres du joueur sans arrêter le jeu** :
  tant qu'`estVivante()` est fausse, le clic qui poserait normalement
  le verrou (`idCible`) ne le pose pas, un verrou déjà posé retombe
  (ajouté à la même condition que la perte de focus ou du bouton), et
  `allerVers` n'est plus appelée. La mise à jour, la caméra, le dessin
  et la fermeture de la fenêtre continuent de tourner normalement :
  les monstres, par exemple, rentrent à leur poste comme n'importe
  quelle fois où ils perdent leur proie (`Escouade::personnageLePlusProche`
  renvoie vide dès que les 4 sont morts, sans code spécifique à la
  défaite dans `Monstre`).
- **Limites connues** : un personnage mort n'est jamais retiré de
  `m_personnages` ni remplacé ; la formation ne se resserre jamais ;
  pas de résurrection, de soin, ni de récupération de points de vie ;
  pas d'écran ou de texte de défaite, ni de moyen de recommencer la
  partie ; l'ancre ne s'arrête pas instantanément à la mort du dernier
  personnage — si elle avait un trajet en cours (lancé juste avant la
  défaite), elle le termine.

## Le verrou de combat devient un état d'`Escouade` (étape 27)

Avant cette étape, le verrou de cible (`idCible`) était une simple
variable locale de `main.cpp` : il retombait dès que le bouton de
souris était relâché, ce qui obligeait à le tenir enfoncé pendant tout
le combat. Le verrou devient un état d'`Escouade`, et un clic bref
suffit désormais à engager un combat qui continue seul.

- **`m_cibleVerrouillee` (`std::optional<int>`), trois fonctions
  minimales** : `verrouiller(idCible)`, `deverrouiller()`,
  `getCibleVerrouillee() const`. `Escouade` ne connaît toujours pas
  `Monstre` — elle ne stocke que son identifiant (`int`), un simple
  paramètre de constructeur dans `Monstre.hpp`, pas un type nommé à
  sortir dans un en-tête séparé.
- **`Escouade` ne décide de rien elle-même** : elle ne pose ni ne
  retire le verrou de sa propre initiative (ni dans `allerVers`, ni
  dans `mettreAJour`) — même séparation que pour `estVivante()` à
  l'étape 26. C'est `main.cpp` qui appelle `verrouiller`/`deverrouiller`
  en réponse aux événements de clic, et qui reste l'intermédiaire avec
  `Monstre` (retrouver le monstre à partir de l'identifiant, vérifier
  s'il est mort).
- **Le combat n'est plus lié à l'état du bouton de souris** : tant que
  `getCibleVerrouillee()` a une valeur, `main.cpp` appelle
  `escouade.attaquer(...)` à chaque image, que le bouton soit enfoncé
  ou non. Le verrou n'est retiré que dans trois cas : mort de la cible,
  clic sur le sol (retraite), ou clic sur un autre monstre (cible
  remplacée) — voir `docs/boucle-et-entrees.md` pour le détail de ces
  règles côté `main.cpp`.

## Portées d'attaque différentes selon le personnage (étape 28)

Jusqu'ici, les 4 personnages étaient interchangeables : même portée,
mêmes dégâts, même recharge, seule la couleur différait (et seulement
pour les distinguer à l'écran). Cette étape introduit deux rôles — deux
mêlées à l'avant, deux tireurs à l'arrière — en ne touchant qu'aux
données, sans ajouter de comportement nouveau.

- **Injection par le constructeur, pas de notion de « type »** :
  `Personnage(sf::Vector2f centre, sf::Color couleur, Attaque attaque)`
  reçoit désormais sa couleur *et* son attaque depuis l'extérieur,
  plutôt que de les fixer lui-même (comme c'était le cas depuis
  l'étape 17 pour l'attaque, et depuis l'étape 12 pour la couleur). Un
  personnage reste défini uniquement par ses données : aucune
  énumération, aucun champ « type » ou « classe » de personnage n'a été
  ajouté — deux personnages ayant la même `Attaque` et la même couleur
  seraient indiscernables, et c'est voulu.
- **C'est `Escouade` qui choisit les valeurs**, dans son constructeur,
  pour chacun des 4 personnages qu'elle crée — comme elle décidait déjà
  de leur position initiale (`placePersonnage`). Les valeurs (couleurs,
  portée des tireurs) sont des constantes nommées dans un espace de
  noms anonyme d'`Escouade.cpp` (`s_couleurMelee`, `s_couleurTireur`,
  `s_porteeTireur`), pas des littéraux répétés quatre fois.
- **Le rôle découle du décalage de formation, pas d'une liste séparée** :
  `m_decalages` vaut `{{25,-25}, {25,25}, {-25,-25}, {-25,25}}` pour une
  escouade orientée vers `+x` (voir plus haut) — les indices 0 et 1
  (`x = +25`, à l'avant) sont les mêlées, les indices 2 et 3 (`x = -25`,
  à l'arrière) les tireurs. Aucune nouvelle donnée n'a été ajoutée pour
  mémoriser « qui est quoi » : la place dans `m_personnages` suffit,
  exactement comme elle suffit déjà à associer chaque personnage à son
  décalage de formation.
- **Valeurs retenues** : mêlée, portée 80 px (inchangée depuis
  l'étape 17) ; tireur, portée 150 px (nouvelle constante). Dégâts (5)
  et recharge (0,5 s) identiques pour les 4 — seule la portée distingue
  les deux rôles à cette étape. Couleurs nettement différentes (rouge
  pour la mêlée, cyan pour le tireur), remplaçant les 4 couleurs
  distinctes de l'étape 12 (rouge, jaune, bleu, magenta).
- **Aucun changement de comportement de déplacement** : `attaquer` et
  `mettreAJour` ne sont pas modifiées par cette étape ; l'escouade
  continue de s'arrêter seulement quand les 4 personnages vivants sont
  à portée (règle de l'étape 17/26), ce qui amène les tireurs au
  contact comme les mêlées — corrigé par l'étape suivante.

## Un personnage à portée cesse de suivre sa place en formation (étape 29)

Limite de l'étape 28 : la règle d'arrêt de l'ancre ne change pas (tous
les vivants à portée), donc même avec une portée de 150 px, un tireur
continuait d'avancer avec le reste de la formation jusqu'au contact.
Cette étape ajoute une seconde règle, individuelle, qui s'applique
seulement en combat.

- **Une seule règle nouvelle, dans `Escouade::mettreAJour`** : pour
  chaque personnage vivant, si l'escouade est en combat (`m_cibleVerrouillee`
  posé) et que ce personnage est à portée de la cible
  (`estAPortee(*m_cibleCombat)`), son point à atteindre devient son
  propre centre (`getCentre()`) au lieu de `placePersonnage(i)` — il
  cesse donc d'avancer, sans qu'aucun code n'ait été ajouté à
  `Personnage` pour ça (il reçoit toujours un simple point en
  paramètre, exactement le principe retenu depuis l'étape 12). Dès que
  la cible n'est plus à portée (elle s'est éloignée) ou que le verrou
  tombe, le personnage recommence à suivre sa place normalement, au
  tic suivant.
- **`m_cibleCombat` (`std::optional<sf::Vector2f>`), le seul moyen pour
  `Escouade` de connaître la position de la cible** : mis à jour au
  début d'`attaquer(cible, carte)`, avant tout le reste de la fonction
  (approche/arrêt/frappe, inchangés). `Escouade` ne connaît toujours
  pas `Monstre` : elle ne stocke qu'un `sf::Vector2f`, exactement comme
  le paramètre `cible` qu'elle reçoit déjà depuis l'étape 17.
- **La règle exige le verrou, pas seulement une cible connue** :
  `enCombat` dans `mettreAJour` vaut `m_cibleVerrouillee.has_value() &&
  m_cibleCombat.has_value()`. Sans cette double condition, un appel
  isolé à `attaquer()` (en théorie possible sans verrou, même si
  `main.cpp` ne le fait jamais ainsi) figerait quand même les
  personnages à portée — contraire à la décision « le placement
  individuel est un comportement de combat », documentée aussi par un
  test unitaire dédié (voir `tests/test_Escouade.cpp`).
- **Pas de nouveau `Mobile`, pas de collision** : un personnage figé
  reste exactement là où il était ; aucune vérification de mur n'est
  nécessaire puisqu'il ne se déplace pas. Le chemin qui contourne les
  murs reste entièrement celui de l'ancre (`m_mobile`), comme avant
  cette étape.
- **Limites connues** : pas de placement « intelligent » (encerclement,
  écartement, ligne de tir) — un personnage à portée s'arrête
  exactement là où il se trouvait au moment d'entrer en portée, même si
  cette position est maladroite (par exemple collée à un autre
  personnage) ; pas de recul si un monstre s'approche d'un tireur déjà
  arrêté ; pas de collision entre personnages, ni entre un personnage
  et un monstre (limites héritées, inchangées par cette étape).

## Portée mesurée de bord à bord (étape 30)

Jusqu'ici, la portée d'une `Attaque` était comparée à la distance entre
les **centres** de l'attaquant et de sa cible. Conséquence observée avec
une petite portée : un personnage devait entrer dans le monstre pour le
frapper, et la même portée donnait un écart visible différent selon la
taille de la cible (un monstre de 30 px « absorbe » 15 px de portée, un
personnage de 20 px seulement 10). Désormais, la portée est l'écart
entre les **bords** : « portée 10 » signifie 10 px de vide entre les
deux, quelle que soit leur taille.

- **Le cercle comme forme** : identique sous tous les angles, donc
  compatible avec une future orientation des entités, sans calcul
  d'angle. Un rectangle orienté ou plusieurs cercles par entité ont été
  écartés : rien ne les justifie encore.
- **Une structure sans comportement + une fonction libre**, dans leur
  propre module (`Cercle.hpp`/`.cpp`), plutôt qu'une classe : un cercle
  n'a aucun invariant à protéger, et la distance bord à bord ne
  « appartient » à aucun des deux cercles plus qu'à l'autre — même
  raisonnement que `trouverChemin` ou `deplacerAvecGlissement`.
- **`distanceBordABord` jamais négative** : deux cercles qui se
  chevauchent sont à 0, pas à une distance négative. Sinon, une portée
  de 10 serait aussi « satisfaite » à −50 px, ce qui reste vrai mais
  rend le nombre renvoyé dénué de sens pour qui le lirait.
- **Rayon calculé, pas stocké** : `getCercle()` recalcule le rayon
  (demi-côté) à partir de la taille déjà stockée (`m_taille` pour
  `Personnage`, `m_mobile.getTaille()` pour `Monstre`) — une seule
  source de vérité pour la taille, comme depuis l'étape 24 pour la
  position du monstre.
- **Seules les portées d'attaque changent de mesure.** Tous les appels
  passent par `Attaque::estAPortee` : frappe des personnages, riposte
  des monstres, arrêt de l'ancre (`attaquer`), arrêt individuel en
  combat (étape 29, `m_cibleCombat` devient un `Cercle`) et arrêt du
  monstre en poursuite. Restent de centre à centre, volontairement :
  détection et territoire du monstre (`proieDetectee` ne lit que
  `cible->centre`), choix du personnage le plus proche
  (`indicePersonnageLePlusProche`), clic sur un monstre (`contient`),
  formation et lissage du chemin.
- **Pour transporter la taille de la cible, les signatures passent de
  `sf::Vector2f` à `Cercle`** plutôt que d'ajouter un paramètre `rayon`
  à côté de la position : `Escouade::attaquer(Cercle, carte)`,
  `Escouade::personnageLePlusProche` (renvoie un `Cercle`, dont le
  monstre lit `.centre` pour se déplacer), `Monstre::mettreAJour`
  (reçoit un `std::optional<Cercle>`), `Monstre::frapper(Cercle)`.
  `Escouade` et `Monstre` ne se connaissent toujours pas : ils
  n'échangent que des `Cercle`, comme ils n'échangeaient que des
  positions.
- **Valeurs retenues**, en constantes nommées pour les personnages
  (`s_porteeMelee`, `s_porteeTireur`) : mêlée 10 px, tireurs 120 px
  (presque la même position d'arrêt qu'avant, à l'œil). Monstres :
  ancienne portée moins les deux rayons, avec un minimum de 5 px —
  partie de 40 px (valeur d'essai en place juste avant l'étape, à la
  place des 80 px de l'étape 20), soit 40 − (10 + 15) = **15 px**.
- **La taille de la cible compte pour les mêlées, à cause de la
  formation** : un mêlée est toujours à 35,4 px du centre de l'ancre
  (décalage `(25, ±25)`), dont 25 px de côté par rapport à l'axe
  d'approche. Face à une cible de rayon nul, il ne passe jamais à moins
  d'environ 25 px de son centre, alors qu'il lui faudrait 20 px (portée
  10 + son rayon 10) : la condition « tous à portée » ne devient jamais
  vraie et l'ancre avance jusque sur la cible. Avec un vrai monstre
  (rayon 15), le seuil passe à 35 px et les mêlées l'atteignent. C'est
  pourquoi les tests de combat d'`Escouade` utilisent une cible de la
  taille d'un monstre (voir `docs/tests.md`). Une portée de mêlée de
  2 px laisse encore une marge (seuil de 27 px), mais faible.
- **Limites connues** : les entités restent dessinées comme des carrés,
  et les coins d'un carré dépassent de son cercle (de 4,1 px pour un
  personnage, 6,2 px pour un monstre) — face à face, l'écart visible
  est bien la portée, mais en diagonale (coin contre coin), une portée
  de mêlée de 10 px laisse à peine un écart. Toujours pas de collision :
  rien n'empêche deux entités de se chevaucher hors d'un arrêt à
  portée. La portée ignore toujours les murs (étape 17).

## Premier skill, déclenché au clavier (étape 31)

Le mêlée avant-gauche possède un skill : une attaque plus forte et plus
lente (25 dégâts, 5 s de recharge, même portée que sa mêlée),
déclenchée par la touche 1 sur la cible verrouillée.

- **Un skill est une seconde `Attaque`, pas une nouvelle classe** :
  portée, dégâts et recharge sont exactement ce qu'`Attaque` porte
  déjà. Aucun comportement propre au skill (effet de zone, état posé
  sur le monstre) ne justifie encore un type à part.
- **L'absence de skill est exprimée par `std::optional<Attaque>`**,
  reçu au constructeur comme l'attaque de base (injection, étape 28) :
  même principe que `personnageLePlusProche` (étape 21), plutôt qu'une
  `Attaque` « vide » à 0 dégâts qui ferait semblant d'exister. Le
  paramètre n'a pas de valeur par défaut : chaque création de
  `Personnage` dit explicitement s'il a un skill.
- **Le personnage qui a le skill est désigné par sa place**, comme les
  rôles de l'étape 28 : `Escouade` le donne au personnage 0 (mêlée
  avant-gauche), sans champ « type » ni indice mémorisé.
  `Escouade::declencherSkill` interroge donc les 4 personnages et
  additionne les résultats (ceux sans skill renvoient 0), exactement
  comme `attaquer` additionne les `frapper` : elle n'a jamais besoin de
  savoir lequel a un skill.
- **Le skill remplace l'attaque de base, pas l'inverse** : quand il
  part, `Personnage::declencherSkill` relance au complet la recharge de
  `m_attaque` (`Attaque::relancerRecharge`, seul ajout nécessaire à
  `Attaque` pour cette règle) ; la prochaine attaque de base attend
  donc une recharge entière. Une attaque de base, elle, ne touche pas
  la recharge du skill. La relance n'a lieu que si le skill part
  réellement (dégâts `> 0`) : une tentative refusée (hors de portée, en
  recharge) ne pénalise pas l'attaque de base.
- **Les refus sont répartis selon qui connaît l'information** : « pas
  de combat engagé » est vérifié par `Escouade` (elle seule porte le
  verrou) ; « pas de skill » et « mort » par `Personnage` (même endroit
  que la garde de `frapper`, étape 21) ; « hors de portée » et « en
  recharge » par `Attaque::frapper`, inchangée. Aucune mise en attente :
  un appui refusé est perdu.
- **La recharge du skill s'écoule en permanence**, dans
  `Personnage::mettreAJour`, à côté de celle de l'attaque de base — en
  combat ou non, y compris en retraite. Comme pour l'attaque de base,
  elle s'arrête pour un mort (retour immédiat, étape 26), ce qui est
  sans conséquence puisqu'un mort ne peut plus lancer le skill.
- **Même circuit que les frappes de base** : `main.cpp` demande à
  `Escouade`, qui demande au personnage, et `main.cpp` applique les
  dégâts au monstre verrouillé. `Escouade` et `Personnage` ne
  connaissent toujours pas `Monstre` (voir `docs/boucle-et-entrees.md`
  pour l'ordre skill puis `attaquer` dans une même image).
- **Barre de recharge** : dessinée par `Personnage` lui-même, comme sa
  barre de vie — orange, 3 px de haut, 1 px sous la barre de vie,
  largeur proportionnelle à `Attaque::getProgressionRecharge()` (pleine
  = prêt). Cette fraction est calculée par `Attaque`, qui seule connaît
  sa recharge et son temps de recharge, plutôt que d'exposer ces deux
  valeurs séparément. Pas de barre pour un personnage sans skill ni
  pour un mort.
- **Limites connues** : un seul skill, sur un seul personnage, une
  seule touche ; si c'est l'autre mêlée qui atteint la cible le
  premier, la touche ne fait rien tant que le mêlée au skill n'est pas
  à portée. Pas d'effet visuel au déclenchement, pas de HUD : seule la
  barre de recharge et la barre de vie du monstre montrent que le skill
  est parti.

## Les monstres se bloquent entre eux (étape 32)

Un monstre ne peut plus entrer dans un autre monstre : plusieurs
monstres qui poursuivent le même personnage ne se superposent plus.
Le mécanisme (voir `docs/carte-et-collision.md`) est écrit pour une liste
de cercles quelconque, afin de resservir aux étapes 33 et 34.

- **`Monstre` ne connaît pas les autres monstres** : `mettreAJour`
  reçoit `obstacles` (`std::vector<Cercle>`) et le transmet à
  `m_mobile`. C'est `main.cpp`, qui possède le vecteur, qui construit
  pour chaque monstre la liste des cercles des AUTRES (identifiant
  différent), à chaque image — même rôle d'intermédiaire que pour la
  cible de poursuite.
- **Mise à jour séquentielle** : chaque monstre voit les autres à leur
  position courante, déjà déplacés ou pas encore selon leur rang dans le
  vecteur. Deux monstres ne peuvent pas se chevaucher en bougeant dans
  la même image : le second à bouger voit déjà la nouvelle place du
  premier.
- **L'ancre et les personnages passent une liste vide** : ils traversent
  toujours les monstres, comme avant ; blocage dans les deux sens aux
  étapes 33 (monstres bloqués par les personnages vivants, voir
  ci-dessous) et 34 (personnages bloqués par les monstres).
- **Limite connue** : un monstre bloqué ne contourne pas, il glisse si
  l'angle le permet, sinon il attend que l'autre bouge.

## Les monstres bloqués par les personnages (étape 33)

Un monstre ne peut plus entrer dans un personnage vivant ni le
traverser : un mêlée à l'avant barre le passage aux tireurs derrière lui
(début du rôle de « tank »). Aucun changement de mécanisme : on ajoute
seulement des cercles à la liste d'obstacles de l'étape 32.

- **`Escouade::cerclesDesVivants()`** expose les cercles plutôt que de
  laisser `main.cpp` parcourir les personnages : `Escouade` reste seule à
  connaître `m_personnages`, et `Monstre` ne connaît toujours ni
  `Escouade` ni `Personnage` (`main.cpp` est l'intermédiaire).
- **Les morts ne bloquent pas** : un cadavre (carré gris) est franchissable,
  cohérent avec le fait qu'il n'est plus ciblé ni dangereux.
- **L'ancre n'est pas un obstacle** : c'est un point de contrôle abstrait,
  pas une entité de combat.
- **Calculé une fois par image**, après `escouade.mettreAJour` (positions
  à jour), avant la boucle des monstres, puis copié comme base de la liste
  de chaque monstre, où s'ajoutent les cercles des autres monstres.
- **Sens unique** : les personnages traversent encore les monstres (étape
  34), un chevauchement peut donc exister. La règle de `deplacementBloque`
  (ne refuser que ce qui rapproche) permet au monstre de s'en dégager.
