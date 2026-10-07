# Squad-ARPG — Formation

## Escouade

- **État de jeu séparé de l'affichage** : la position, la taille et la
  cible sont des données de la classe ; le `sf::RectangleShape` n'existe
  que dans `dessiner`. Même modèle que `TileMap`.
- **Dépendance à `TileMap` le temps d'un appel** : la carte est passée en
  `const TileMap&` à `mettreAJour` plutôt que gardée en membre — pas de
  question de durée de vie, et l'escouade ne peut pas modifier la carte.
- **Déclaration anticipée** : `Escouade.hpp` contient `class TileMap;`
  (une référence ne demande pas la définition complète) ; l'include de
  `TileMap.hpp` est dans `Escouade.cpp`. Les fichiers qui incluent
  `Escouade.hpp` ne dépendent donc pas de `TileMap.hpp`.
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
