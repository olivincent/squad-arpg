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

## Suivi de chemin par `Mobile`

`Mobile` porte le déplacement par chemin : position, chemin, vitesse,
A* + lissage, collision/glissement. Elle est possédée par valeur par
`Escouade` (`m_mobile`) et par `Monstre` — composition, pas héritage :
aucune classe de base commune, aucune fonction virtuelle. Elle ne connaît
ni `Escouade`, ni `Personnage`, ni `Monstre`, ni `Attaque`, et ne dessine
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

