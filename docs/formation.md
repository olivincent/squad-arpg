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
