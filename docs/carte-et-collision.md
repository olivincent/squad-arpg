# Squad-ARPG — Carte et collision

## Carte, collision et dessin

- **Coordonnées du monde** : la carte et les entités sont positionnées en
  pixels de la carte, jamais en pixels de l'écran. C'est ce qui a permis
  d'ajouter la caméra sans modifier `TileMap`.
- **`enum class Tile { Floor, Wall }`** : `enum class` plutôt que `enum`
  pour la portée (`Tile::Wall`) et l'absence de conversion implicite en
  `int`.
- **Stockage 1D** : la case `(x, y)` est à l'indice `y * largeur + x`,
  calculé par la fonction privée `indice(x, y)`. Plus simple et contigu en
  mémoire qu'un vecteur de vecteurs.
- **Ordre de déclaration des membres** : `m_cases` est déclaré après
  `m_largeur` et `m_hauteur`, car il est initialisé à partir d'eux dans la
  liste d'initialisation du constructeur.
- **Hors de la carte = mur** : `getTile` vérifie les bornes et renvoie
  `Tile::Wall` pour toute case hors limites (pas d'accès indéfini au
  vecteur, et la carte est infranchissable à ses bords).
- **Collision rectangle/grille** : on ne parcourt que les cases couvertes
  par le rectangle. Bord gauche/haut : `std::floor` (correct aussi pour
  les coordonnées négatives, contrairement à la troncature d'un
  `static_cast<int>`). Bord droit/bas : `std::ceil(...) - 1`, pour qu'un
  rectangle qui *touche* une case sans la *chevaucher* ne soit pas en
  collision (même convention que `sf::Rect::findIntersection`).
- **Glissement le long des murs** : le déplacement est testé axe par axe,
  X puis Y, l'essai Y partant de la position obtenue après l'essai X.
  Conséquence acceptée pour l'instant : en glissant, l'entité ne garde
  que la composante du déplacement parallèle au mur et avance donc plus
  lentement.
- **Dessin de la carte** : un seul `sf::RectangleShape` repositionné et
  recoloré pour chaque case, avec un contour de -1 px pour rendre la
  grille visible. Environ 2400 appels à `draw` par image avec la carte
  actuelle (`sf::VertexArray` envisageable si ça devient un problème).
  Les fonctions `dessiner` prennent un `sf::RenderTarget&` (classe de base
  de `sf::RenderWindow`) pour ne pas être couplées à la fenêtre.

## Ligne de vue (étape 44)

- **`TileMap::ligneDeVue(a, b)`** : vrai si le segment de `a` à `b` ne
  traverse aucun mur. C'est une question posée à la carte, à côté de
  `trajetLibre`.
- **Implémentation** : `trajetLibre` avec un carré de 2 × 2 px centré sur
  `a` et déplacé de `b - a` (constante `COTE_LIGNE_DE_VUE` dans
  `TileMap.cpp`). Le carré n'est pas de taille nulle : avec la convention
  « toucher n'est pas chevaucher », un rectangle vide ne chevauche jamais
  rien. Le nom cache le « comment » : un parcours de grille (DDA) pourra le
  remplacer sans toucher aux appelants.
- **Conséquences** : un point d'arrivée dans un mur donne faux ; un segment
  qui passe par le coin où deux murs se touchent en diagonale est bloqué ;
  un segment à 1 px d'un mur est libre. Si `a` est à moins de 1 px d'un
  mur, le carré le touche et la vue est refusée (limite acceptée).
- **Utilisée** pour l'instant seulement par le skill de zone (voir
  `docs/combat.md`).

## Déplacement avec glissement et obstacles

`deplacerAvecGlissement` est une fonction libre, pas une classe : elle n'a
aucun état à conserver. Elle est partagée par `Mobile` et `Personnage`,
qui n'ont ainsi à dépendre ni l'un de l'autre ni d'`Escouade`, et sans
dupliquer le code. Elle essaie le déplacement axe par axe (voir le
glissement ci-dessus) et refuse un essai d'axe qui chevauche un mur ou qui
mène à un chevauchement avec un cercle obstacle (`deplacementBloque`).

- **Blocage, pas séparation ni poussée** : même schéma que pour les murs
  (essai axe par axe, l'autre axe peut passer : glissement). Rien ne
  déplace jamais un obstacle.
- **On ne refuse que ce qui rapproche** : un essai est bloqué si, après,
  les cercles se chevauchent (distance strictement inférieure à la somme
  des rayons) ET que la distance entre centres a diminué. Deux entités
  déjà superposées peuvent donc toujours s'éloigner, au lieu de rester
  collées pour toujours. Se toucher exactement n'est pas un
  chevauchement, même convention que pour les murs.
- **Murs en rectangle, entités en cercle** : les murs restent testés
  avec le rectangle de l'entité ; le cercle (centre du rectangle, rayon
  = demi-largeur) ne sert qu'aux obstacles, par cohérence avec
  `getCercle()`.
- **Une liste de cercles, pas de types** : celui qui bouge reçoit ses
  obstacles en paramètre ; `Deplacement` ne sait pas d'où ils viennent
  (monstres ou personnages). Aucune dépendance entre modules. Qui reçoit
  quelle liste est décrit dans `docs/combat.md`.
- **Glissement tangent (étape 68 bis)** : l'essai axe par axe ne suffit pas
  contre un cercle. La surface d'un mur est parallèle à un axe, donc l'autre axe
  passe toujours ; celle d'un cercle ne l'est pas : un mobile calé contre lui,
  poussé droit dessus (ou presque), voit les essais X et Y se rapprocher de ses
  centres l'un comme l'autre, et reste coincé pour toujours (constaté : un
  tireur dont la place entrait dans un monstre). Si **ni X ni Y n'ont
  passé** et qu'**au moins un cercle a refusé** un essai, `deplacerAvecGlissement`
  essaie donc un déplacement **de même longueur que le déplacement voulu** le
  long de la **tangente du cercle refusant le plus proche** (perpendiculaire à
  la droite des centres). Côté : celui où la projection du déplacement voulu
  sur la tangente est positive ; projection nulle (poussée exactement droit
  contre le cercle) : à gauche du déplacement voulu, ce qui est déterministe.
  Il n'est accepté que s'il ne chevauche pas un mur et si `deplacementBloque`
  est faux : **la règle de blocage ne change pas** (jamais rapprocher les
  centres d'un obstacle chevauché), on essaie seulement un mouvement permis de
  plus. S'éloigner étant toujours permis, la tangente (qui éloigne du cercle
  considéré) ne peut être refusée que par un mur ou un autre cercle. Un mobile
  poussé exactement droit contre un cercle fait un pas de côté, puis les essais
  d'axe le ramènent vers sa cible : seul un chemin A* qui contourne le cercle
  (`docs/chemin.md`) le fait passer de l'autre côté ; la tangente le décale
  surtout hors du coin où il était calé. Les tests de collision existants
  passent sans modification (`Un obstacle droit devant bloque…` : la tangente à
  gauche y sort de la carte, donc d'un mur) ; seul un test de `Mobile`, qui
  comparait la position à l'identique après une longue poussée, compare
  maintenant à 0,5 px près.
- **Limites connues** : parcours de toutes les paires sans structure
  d'accélération ; contre un cercle, le glissement tangent est un pas de côté,
  pas un contournement : c'est le A* qui contourne (`docs/chemin.md`).

## Plan texte et validation

- **Format** : `std::vector<std::string>`, une chaîne par ligne de cases.
  Largeur = longueur des lignes, hauteur = nombre de lignes.
- **Validation** : `plan.at(0)` lance `std::out_of_range` si le plan est
  vide ; une ligne de longueur différente lance `std::invalid_argument`
  avec le numéro de la ligne. Ces exceptions ne sont pas attrapées : le
  programme s'arrête avec un message clair, ce qui est voulu pour détecter
  une erreur de saisie dans le plan.
- Le plan est pour l'instant défini directement dans `main.cpp`.
