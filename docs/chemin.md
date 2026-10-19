# Squad-ARPG — Chemin

## Recherche de chemin (A*)

- **Fonction libre `trouverChemin`**, pas de classe : aucun état à
  conserver entre deux appels.
- **File de priorité** : `std::priority_queue<Noeud, std::vector<Noeud>,
  ComparateurNoeud>`. `std::priority_queue` sort toujours le plus **grand**
  élément selon son comparateur ; `ComparateurNoeud` inverse l'ordre
  (`a.coutTotal > b.coutTotal`) pour que ce soit le plus **petit** coût
  total qui sorte en premier.
- **Heuristique** : distance de Manhattan entre la case et l'arrivée,
  ignorant les murs. Elle ne surestime jamais la distance réelle
  (admissible), ce qui garantit que l'A* trouve le plus court chemin ; son
  rôle est de diriger l'exploration vers l'arrivée plutôt que dans toutes
  les directions comme le ferait un Dijkstra sans heuristique.
- **Entrées périmées** : `std::priority_queue` ne permet pas de mettre à
  jour une entrée existante. Quand un meilleur chemin est trouvé vers une
  case déjà dans la file, une nouvelle entrée est ajoutée sans retirer
  l'ancienne. Le tableau `cout` (coût réel actuellement connu par case)
  fait autorité : à la sortie de la file, une entrée dont le coût
  parcouru dépasse `cout[case]` est ignorée (suppression paresseuse).
- **Tableaux 1D** `cout` et `parent`, indexés `y * largeur + x` comme
  `TileMap::m_cases`, dimensionnés via `getLargeur()`/`getHauteur()`.
- **Reconstruction** : remontée des `parent` depuis l'arrivée jusqu'au
  départ, puis `std::reverse`.
- **Chemin vide** si départ ou arrivée est un mur (ou hors carte, que
  `getTile` traite comme un mur) ou si l'arrivée est inaccessible ; ce
  test précède le raccourci départ == arrivée, pour qu'une case murée ne
  renvoie jamais un chemin d'une case.
- **Travaille en cases** (coordonnées de grille, `sf::Vector2i`), jamais
  en pixels, contrairement aux entités, qui travaillent en pixels
  (`sf::Vector2f`).
- **Déclaration anticipée** : `Chemin.hpp` contient `class TileMap;` ;
  l'include de `TileMap.hpp` est dans `Chemin.cpp`.
- **Tout l'interne de l'algorithme** (structure de nœud, comparateur de la
  file de priorité, heuristique) est dans un espace de noms anonyme de
  `Chemin.cpp` : rien n'apparaît dans `Chemin.hpp`.
- **Deux niveaux** : `trouverChemin` raisonne en cases, `lisserChemin` en
  pixels. `main.cpp` ne connaît pas ce module : seul `Mobile::allerVers`
  l'utilise.

## Contourner les obstacles mobiles (étape 68)

Jusqu'à l'étape 67, le A* et le lissage ne voyaient que les murs : une entité
bloquée par une autre glissait ou attendait. Depuis l'étape 68, un chemin peut
contourner une **liste de cercles** (`aContourner`). Ce n'est pas la liste des
obstacles **bloquants** de `Mobile::mettreAJour` (collision, inchangée) : un
monstre est bloqué par les personnages mais ne les contourne pas (un
personnage sur son chemin est quelqu'un à frapper).

- **Coût ajouté, pas interdiction** : `trouverChemin` reçoit une
  `FonctionDeCout` (`std::function<int(sf::Vector2i)>`, vide par défaut = aucun
  coût, comportement d'avant). Entrer dans une case coûte `1 + cout(case)`.
  Une case occupée coûte `s_penaliteCaseOccupee` = 8 de plus (constante de
  `Mobile.cpp`) : un détour d'environ 8 cases est préféré à la traversée,
  au-delà le chemin passe quand même, au lieu de ne pas exister (un couloir
  bouché par un monstre reste franchissable). L'heuristique de Manhattan reste
  admissible : le coût ajouté n'est jamais négatif.
- **Case occupée = somme de Minkowski** : le rectangle de la case chevauche
  (distance strictement inférieure) le cercle de l'obstacle **agrandi du rayon
  du mobile** qui cherche son chemin (demi-taille : 10 px pour un personnage,
  15 px pour un monstre). On raisonne ainsi sur un mobile réduit à un point
  qui évite des disques plus gros, ce qui revient au même qu'un mobile de
  taille réelle évitant les disques d'origine. Toucher sans chevaucher n'est
  pas occuper (même convention que partout).
- **Départ et arrivée jamais pénalisés** : `cout` n'est pas appelée pour eux.
  Un mobile déjà collé à un obstacle doit pouvoir en partir, et un but à côté
  d'un obstacle (la place d'attaque, la place de formation) doit rester
  atteignable.
- **`coutDesObstacles(carte, obstacles, rayonMobile, penalite)`** construit
  cette fonction : elle précalcule une fois les cases occupées
  (`std::vector<bool>` de la taille de la carte) et renvoie une lambda qui **capture le tableau par
  valeur**. Depuis l'étape 80 (audit V1, partie 1), elle ne teste, pour chaque obstacle, que les
  cases de son **rectangle englobant** (agrandi de `rayonMobile`, borné à la carte, plus une case de
  marge de chaque côté pour que l'arrondi de la division n'écarte aucune case) : une vingtaine de
  cases au lieu des 2 360 de la carte. Le test exact de chaque case est inchangé : le résultat est
  identique (vérifié hors du dépôt contre l'ancien parcours sur 47 millions de cases). Un monstre en
  poursuite appelle `allerVers` à chaque image, avec les autres monstres à contourner : le coût
  était en N² sur la carte entière. Mesures (24 monstres en poursuite, une image) : 50 ms → 3,9 ms
  dans le preset `debug`, 2,35 ms → 0,34 ms dans le preset `jeu` ; `allerVers` seul (23 obstacles) :
  2,06 ms → 0,15 ms et 0,10 ms → 0,014 ms. Capturer par référence aurait laissé une référence pendante dès
  que la fonction de construction est quittée ; capturer par valeur copie le
  tableau (quelques milliers de booléens) et rend la fonction autonome. C'est
  le point de branchement de la future « stratégie d'attaque » de
  `docs/brainstorm.md` : la liste de cercles passée changera selon le type
  d'entité.
- **Lissage** : `lisserChemin` reçoit la même liste ; un raccourci est refusé
  si le segment passe à moins de (rayon de l'obstacle + rayon du mobile) du
  centre d'un obstacle. Distance point-segment : `distancePointSegment`
  (module `Cercle`, projection sur le segment ramenée dans `[0, 1]`).
  `traverseUnObstacle` (module `Chemin`) en fait un test réutilisable. Liste
  vide = comportement d'avant.
- **Ligne droite d'un personnage** : seulement si le trajet est libre de murs
  (règle d'avant) ET ne passe pas près d'un obstacle à contourner (même test
  que le lissage) ; sinon A* lissé avec le coût.
- **Détection du blocage** (`Mobile::estBloque`, définition unique comme
  `estEnMouvement`) : à chaque `mettreAJour`, s'il a un point à atteindre
  (`estEnMouvement`) et a parcouru moins de 10 % de `vitesse × dt`
  (`s_fractionDePasBloque`), un compteur augmente de `dt`, sinon il revient à
  0. Bloqué à partir de 0,3 s (`s_tempsAvantBlocage`). `allerVers`,
  `allerToutDroit` et `arreter` remettent le compteur à 0 ; `allerVers` le fait
  même quand le A* ne trouve rien, pour qu'un mobile coincé ne relance pas le
  A* à chaque image. Un mobile arrêté n'est jamais bloqué.
- **Recalcul par le propriétaire, pas par `Mobile`** (correction de la décision
  notée dans `ROADMAP.md`) : `Mobile` ne connaît que ses obstacles bloquants,
  pas ceux à contourner ; seul le propriétaire sait quelle liste passer.
  `Personnage` recalcule quand `estBloque()` (en plus de la règle « cible
  éloignée de plus d'une case »), `Monstre` aussi en retour au poste (en
  poursuite, il recalcule déjà à chaque image). Le compteur étant remis à 0,
  un mobile durablement coincé recalcule toutes les 0,3 s, pas à chaque image.
- **Qui contourne quoi** : voir `docs/monstre.md` (les monstres : les autres
  monstres) et `docs/formation.md` (les personnages : les monstres qu'ils
  n'approchent pas). L'ancre ne contourne rien. Les listes sont construites par
  `Partie` et `Escouade` (`docs/boucle-et-entrees.md`).
- **Débogage** : F3 trace le chemin restant (`Mobile::cheminRestant`,
  `dessinerTraceDeChemin`) : cyan pour les personnages, rouge pour les monstres.
- **Limites connues** : le lissage ne fait que retirer des points (voir
  « Lissage de chemin ») et le coût est en cases : un point de passage au
  centre d'une case peut rogner un peu le disque d'un obstacle quand le
  mobile part de sa frontière ; c'est le glissement tangent
  (`docs/carte-et-collision.md`) qui le sort de là. Le A* est recalculé en
  entier à chaque appel ; aucune limitation de fréquence pour les monstres (audit V1, partie 2 :
  décision reportée, à réévaluer avec la carte extérieure ou des hordes).

## Suivi de chemin par `Mobile`

`Mobile` porte le déplacement par chemin : position, chemin, vitesse,
A* + lissage, collision/glissement. Elle est possédée par valeur par
`Escouade` (`m_mobile`), par `Personnage` et par `Monstre` — composition, pas héritage :
aucune classe de base commune, aucune fonction virtuelle. Elle ne connaît
ni `Escouade`, ni `Personnage`, ni `Monstre`, ni `Skill`, et ne dessine
rien ; elle connaît `TileMap`, `Chemin` et `Deplacement`.

- **Interface en centres** : le constructeur et `getCentre` raisonnent en
  centre, pas en coin haut-gauche (la formation et les cibles
  raisonnent en centre). En interne, `m_position` est le coin haut-gauche,
  ce que `deplacerAvecGlissement` attend.
- **Conversions pixels ↔ cases regroupées dans `TileMap`** (seule à
  connaître `m_tailleCase`) : `caseContenant` (comme `chevaucheUnMur`,
  avec `std::floor` pour rester correct en négatif) et `centreDeCase`.
- **Les points de passage sont des positions du centre du mobile**, pas de
  son coin haut-gauche. `mettreAJour` calcule donc la direction et la
  distance depuis `getCentre()`, mais applique le déplacement obtenu (un
  simple vecteur de translation) à `m_position` — une translation déplace
  un rectangle rigide tout entier, inutile de la recalculer séparément
  pour le centre et le coin.
- **`m_chemin` + `m_indexChemin`** plutôt qu'un seul point cible : avancer
  l'indice évite un `erase(begin())` à chaque point atteint (coûteux, car
  il décale tous les éléments suivants en mémoire). Le chemin déjà
  parcouru reste en mémoire jusqu'au prochain `allerVers`.
- **L'appel au A* est dans `Mobile::allerVers`**, avec la carte passée en
  paramètre comme pour `mettreAJour` (jamais gardée en membre).
- **Construction du chemin suivi** à partir du résultat de `trouverChemin`
  : la case de départ est ignorée (le mobile est déjà dessus, la viser
  d'abord le ferait reculer vers le centre de sa propre case) ; les cases
  intermédiaires sont converties en centres de case ; le dernier point
  est remplacé par le point exact cliqué, pour que le mobile s'arrête
  précisément dessus plutôt qu'au centre de la case d'arrivée. Le chemin
  est ensuite lissé par `lisserChemin` (voir plus bas), avec `getCentre()`
  comme point de départ et la taille du mobile comme taille de rectangle.
  Ce lissage évite le zigzag qu'on aurait en visant des centres de case
  presque alignés sur un seul axe.
- **`allerVers(destination, carte, aContourner = {})`** : le coût du A* est `coutDesObstacles(carte, aContourner, m_taille.x / 2, s_penaliteCaseOccupee)` et le lissage reçoit la même liste (voir « Contourner les obstacles mobiles »).
- **`estBloque()` et `cheminRestant()`** : voir « Contourner les obstacles mobiles ». `cheminRestant()` renvoie une copie des points de `m_indexChemin` à la fin.
- **Chemin vide = ordre ignoré** : `allerVers` ne touche à `m_chemin`
  qu'après avoir confirmé que `trouverChemin` a renvoyé un résultat non
  vide ; un clic sur un mur ou une zone inaccessible n'interrompt donc
  pas le trajet en cours.
- **`arreter()`** réduit le chemin au centre actuel : le mobile cesse
  d'avancer sans reculer. Utilisé par `Escouade::attaquer` et par
  `Monstre`.
- **Collision et glissement conservés comme filet de sécurité** : le
  mobile peut partir d'une position décentrée dans sa case ou viser un
  point proche d'un mur (notamment le point exact cliqué).
  `mettreAJour(dt, carte, obstacles)` passe aussi les cercles obstacles à
  `deplacerAvecGlissement` (voir `docs/carte-et-collision.md`).
- **`allerToutDroit(destination)`** (étape 39) : remplace le chemin par
  ce seul point, sans A* ni carte. Un `Personnage` en a besoin pour
  avancer en ligne droite vers un point qui change à chaque image :
  `allerVers` ferait un A* à chaque image et ignorerait un point dans un
  mur. `mettreAJour` traite le chemin d'un seul point comme les autres
  (arrivée exacte, glissement).
- **`directionVoulue()`, le point de passage vers `Escouade`** : `Mobile`
  ne connaît pas l'orientation de la formation (affaire d'`Escouade`),
  mais `Escouade` a besoin, pour l'orienter, de la même direction que
  celle que `mettreAJour` utilise en interne, lue au même moment du tic
  (avant l'essai de collision, sur les positions du *début* du tic).
  `directionVoulue()` renvoie cette direction (vide si plus aucun point à
  atteindre, ou si la distance est nulle) sans modifier d'état ;
  `Escouade::mettreAJour` l'appelle avant `m_mobile.mettreAJour`.
  `Monstre` s'en sert aussi pour savoir qu'il est arrivé à son poste.

## Lissage de chemin

- **`TileMap::trajetLibre(rect, deplacement)`** : vrai si le rectangle,
  translaté de sa position jusqu'à `position + deplacement`, ne chevauche
  un mur à aucun moment du trajet. Même convention que `chevaucheUnMur`
  (toucher sans chevaucher n'est pas une collision ; hors carte = mur).
  Calcul exact, pas d'échantillonnage.
  - **Cas horizontal, vertical ou nul** (une composante de `deplacement`
    nulle) : la zone balayée est le rectangle qui englobe la position de
    départ et celle d'arrivée ; un seul appel à `chevaucheUnMur` suffit.
  - **Cas oblique** : balayage par colonnes de cases, paramétré par `t`
    entre 0 (départ) et 1 (arrivée). Pour chaque colonne couverte par le
    mouvement, deux instants (`t1`, `t2`) marquent quand le bord arrière
    du rectangle atteint le bord gauche de la colonne, et quand son bord
    avant atteint son bord droit. `tEntree = min(t1, t2)` et
    `tSortie = max(t1, t2)` (bornés à `[0, 1]`) donnent la bonne paire
    entrée/sortie quel que soit le sens du déplacement, puisque diviser
    par une composante négative de `deplacement` inverse l'ordre de `t1`
    et `t2` — sans `min`/`max`, il faudrait une branche selon le signe.
    Une composante nulle est exclue de ce cas parce que les formules de
    `t1`/`t2` divisent par elle (division par zéro indéfinie), d'où la
    séparation avec le cas horizontal/vertical.
  - Pour chaque colonne avec une fenêtre `[tEntree, tSortie]` non vide,
    la plage de hauteur balayée pendant cette fenêtre (`haut`/`bas`) donne
    les rangées à vérifier avec `getTile`.
  - Un rectangle peut « accrocher » le coin d'un mur même quand la ligne
    reliant les CENTRES du rectangle à chaque instant ne traverse jamais
    la case du mur — vérifié par un test dédié (voir Tests ci-dessous) :
    le rectangle a une taille, pas seulement un centre.
- **`Chemin::lisserChemin(carte, depart, points, taille)`** : algorithme
  glouton. L'ancre part de `depart` ; à chaque tour, on cherche, **en
  partant de la fin** de `points`, le plus lointain atteignable depuis
  l'ancre (`trajetLibre` avec le rectangle `taille` centré sur l'ancre) —
  s'arrêter au premier trouvé donne directement le plus lointain. Ce
  point devient la nouvelle ancre, et on continue avec les points qui le
  suivent. Si aucun point restant n'est atteignable, le prochain point
  de la liste est gardé tel quel, sans être testé : la collision dans
  `mettreAJour` reste le filet de sécurité.
- **Branchement dans `Mobile::allerVers`** : le chemin en pixels (centres
  de case, puis point exact cliqué) est lissé avant d'être stocké dans
  `m_chemin`.
- **Limite connue** : `lisserChemin` ne fait que retirer des points de la
  liste donnée par le A* (centres de case) ; il ne peut pas en ajouter ni
  en déplacer. Autour d'un pilier, dès qu'un saut direct est bloqué par
  le mur, le mobile retombe sur le centre de la case suivante tel quel,
  plutôt que sur un point plus proche du coin du pilier qui
  raccourcirait encore le trajet — il frôle donc moins les murs qu'un
  lissage « idéal » (type *funnel algorithm* ou Theta*) ne le ferait,
  mais il ne reste jamais coincé. Accepté pour l'instant.

## Repli d'un point vers une origine

- **`Chemin::replierVersOrigine(carte, origine, cible, taille)`** : si
  `trajetLibre` dit que le rectangle `taille` centré sur `origine` peut
  aller jusqu'à `cible` en ligne droite, `cible` est renvoyée telle
  quelle. Sinon, dichotomie sur la fraction du segment : `libre` (0 au
  départ, donc l'origine) et `bloque` (1), 6 itérations
  (`s_iterationsRepli`) ; le milieu devient `libre` si son trajet est
  libre, `bloque` sinon. Renvoie `origine + segment * libre`.
- **Pourquoi la dichotomie est correcte** : le trajet vers un point plus
  proche est un début du trajet vers un point plus lointain ; si un
  point du segment est atteignable, tous les précédents le sont. Le
  prédicat « trajet libre jusqu'à la fraction t » est donc monotone.
- **Pourquoi 6 itérations** : précision de 1/64 de la longueur du segment
  (moins d'un pixel pour une place de formation à ~35 px) ; si même le
  point le plus proche n'est pas libre, `libre` reste 0 et le résultat
  est l'origine, sans cas particulier.
- **Pourquoi `trajetLibre` et non `chevaucheUnMur` sur la cible** : une
  cible peut être libre mais cachée derrière un coin de pilier ; seul le
  trajet exact le détecte.
- **Limites connues** : ne cherche pas la case libre la plus proche, ne
  tient pas compte des entités ; le résultat est toujours sur le segment.

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Chemins qui contournent les entités** (étape 68) : le A* ajoute un coût
  (8) aux cases touchées par un obstacle agrandi du rayon du mobile, le lissage
  et la ligne droite d'un personnage évitent les mêmes cercles ; un passage sans
  autre issue reste franchissable. Les monstres contournent les autres monstres
  (pas les personnages : on les frappe) ; chaque personnage contourne les monstres
  qu'il n'approche pas (pas les autres personnages, pas l'ancre). Un mobile
  bloqué depuis 0,3 s recalcule son chemin (le propriétaire, pas `Mobile`). Les
  places de formation ne sont plus dans un monstre. Contre un cercle, le glissement
  essaie aussi la tangente (un pas de côté ; la règle de blocage est inchangée).
  Voir `docs/chemin.md` et `docs/carte-et-collision.md`.

### Limites connues

- Un personnage dont le A* ne trouve aucun chemin garde la ligne droite.
  Les chemins contournent les monstres (étape 68), mais pas les autres
  personnages ni l'ancre ; un passage sans autre issue est traversé au prix du
  coût ajouté, l'entité attend alors derrière le monstre ; contre un cercle le
  glissement tangent n'est qu'un pas de côté. Près des piliers, le contournement
  n'est pas toujours le plus court.

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `Mobile` (`monde/`) — déplacement par chemin

Suit un chemin calculé par le A* puis lissé, avec collision contre les
murs et les cercles obstacles. Possédé par valeur par `Escouade`, `Personnage` et
`Monstre`. Ne connaît ni `Escouade`, ni `Personnage`, ni `Monstre`, ni
`Skill`, et ne dessine rien.

Données membres : `m_position` (coin haut-gauche), `m_taille`, `m_chemin`
(`std::vector<sf::Vector2f>`), `m_indexChemin`, `m_vitesse`, `m_enMouvement` (`bool`), `m_tempsBloque` (`float`, secondes consécutives sans avancer vraiment).

| Fonction | Rôle |
|---|---|
| `Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)` | Crée le mobile à ce centre, cette taille, cette vitesse ; immobile |
| `void allerVers(sf::Vector2f destination, const TileMap& carte, const std::vector<Cercle>& aContourner = {})` | Calcule un chemin vers la destination (A* avec `coutDesObstacles` du rayon du mobile et de `s_penaliteCaseOccupee` = 8, puis lissé en évitant `aContourner`) et le remplace comme trajet à suivre ; ordre ignoré si aucun chemin n'existe. Remet le compteur de blocage à 0 dans tous les cas |
| `void allerToutDroit(sf::Vector2f destination)` | Remplace le chemin par ce seul point, sans A* : le mobile avance en ligne droite (collision et glissement) ; remet le compteur de blocage à 0 |
| `void arreter()` | Réduit le chemin au centre actuel, pour cesser d'avancer sans reculer ; `estEnMouvement()` devient faux, le compteur de blocage est remis à 0 |
| `void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Avance vers le point courant du chemin (collision avec les murs et les cercles obstacles, glissement) |
| `void decaler(sf::Vector2f decalage, const TileMap& carte, const std::vector<Cercle>& obstacles)` | Pousse le mobile (mêmes collisions et même glissement), sans toucher au chemin ni à `estEnMouvement()` ; utilisée par la séparation des personnages |
| `sf::Vector2f getCentre() const` | Centre du mobile |
| `sf::Vector2f getTaille() const` | Taille du mobile |
| `bool estEnMouvement() const` | Définition unique de « en mouvement » : au dernier tic, le point courant du chemin était à plus de 0,01 px, que le mobile ait pu avancer ou non (bloqué = en mouvement ; arrivé à ce tic = encore en mouvement). Faux avant le premier tic et après `arreter()` ; `[[nodiscard]]` |
| `bool estBloque() const` | Définition unique de « bloqué » : à chaque `mettreAJour`, s'il a un point à atteindre (`estEnMouvement`) et a parcouru moins de 10 % de `vitesse × dt`, un compteur augmente de `dt`, sinon il revient à 0 ; vrai à partir de 0,3 s (`s_tempsAvantBlocage`). Jamais vrai à l'arrêt ; `[[nodiscard]]`. Le recalcul du chemin est l'affaire du propriétaire |
| `std::vector<sf::Vector2f> cheminRestant() const` | Points du chemin pas encore atteints, le prochain en premier (débogage F3 et tests) ; `[[nodiscard]]` |
| `std::optional<sf::Vector2f> directionVoulue() const` | Vecteur du centre vers le point courant du chemin ; vide s'il n'y a plus de point à atteindre ou si la distance est nulle |

### `Chemin` (`monde/`) — recherche de chemin (A*)

Fonctions libres, sans état, plus `using FonctionDeCout = std::function<int(sf::Vector2i)>` (coût **ajouté** pour entrer dans une case, en plus de 1 par pas ; fonction vide = aucun coût). `trouverChemin` raisonne en cases, les autres en pixels. `trouverChemin`, `coutDesObstacles` et `lisserChemin` sont utilisées par `Mobile::allerVers`, `traverseUnObstacle` aussi par `Personnage::mettreAJour`, `replierVersOrigine` par `Escouade::mettreAJour`. Voir `docs/chemin.md`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee, const FonctionDeCout& cout = {})` | Plus court chemin entre deux cases (4 directions, coût 1 par pas plus `cout(case)`, jamais appelée pour le départ ni l'arrivée ; heuristique de Manhattan, toujours admissible) ; inclut le départ et l'arrivée ; vecteur vide si départ/arrivée est un mur (ou hors carte) ou si l'arrivée est inaccessible |
| `FonctionDeCout coutDesObstacles(const TileMap& carte, const std::vector<Cercle>& obstacles, float rayonMobile, int penalite)` | `penalite` pour une case dont le rectangle chevauche (strictement) un obstacle agrandi de `rayonMobile` ; 0 ailleurs et hors carte. Cases occupées précalculées une fois (`std::vector<bool>`), en ne visitant pour chaque obstacle que les cases de son rectangle englobant (agrandi, borné à la carte, une case de marge), copiées dans la lambda renvoyée (aucune référence pendante) ; fonction vide si `obstacles` est vide ; `[[nodiscard]]` |
| `bool traverseUnObstacle(sf::Vector2f a, sf::Vector2f b, const std::vector<Cercle>& obstacles, float rayonMobile)` | Le segment passe-t-il à moins de (rayon de l'obstacle + `rayonMobile`) du centre d'un obstacle ? ; `[[nodiscard]]` |
| `std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Vector2f taille, const std::vector<Cercle>& aContourner = {})` | Retire de `points` (positions du centre d'un rectangle `taille`, en pixels) les points de passage inutiles ; un raccourci est refusé s'il traverse un des `aContourner` (`traverseUnObstacle`, rayon du mobile = `taille.x / 2`) ; `depart` n'en fait pas partie dans le résultat |
| `sf::Vector2f replierVersOrigine(const TileMap& carte, sf::Vector2f origine, sf::Vector2f cible, sf::Vector2f taille)` | `cible` si le rectangle `taille` centré sur `origine` peut aller en ligne droite jusqu'à elle sans toucher un mur ; sinon le point libre le plus éloigné de `origine` sur le segment (dichotomie, 6 itérations) ; `origine` si aucun point n'est libre |

### `TraceDeChemin` (`rendu/`) — le chemin restant, pour le débogage

Une fonction libre, sans état, utilisée par `Escouade::dessinerPointsVises` et `Monstre::dessinerChemin` (F3).

| Fonction | Rôle |
|---|---|
| `void dessinerTraceDeChemin(sf::RenderTarget& rendu, sf::Vector2f depart, const std::vector<sf::Vector2f>& points, sf::Color couleur)` | Ligne brisée (`LineStrip`) de `depart` à travers `points`, dans l'ordre |
