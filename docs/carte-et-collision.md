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
- **Limites connues** : parcours de toutes les paires sans structure
  d'accélération ; le A* ignore les entités, donc une entité bloquée ne
  contourne pas, elle glisse si l'angle le permet, sinon elle attend.

## Plan texte et validation

- **Format** : `std::vector<std::string>`, une chaîne par ligne de cases.
  Largeur = longueur des lignes, hauteur = nombre de lignes.
- **Validation** : `plan.at(0)` lance `std::out_of_range` si le plan est
  vide ; une ligne de longueur différente lance `std::invalid_argument`
  avec le numéro de la ligne. Ces exceptions ne sont pas attrapées : le
  programme s'arrête avec un message clair, ce qui est voulu pour détecter
  une erreur de saisie dans le plan.
- Le plan est pour l'instant défini directement dans `main.cpp`.
