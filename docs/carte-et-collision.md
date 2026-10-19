# Squad-ARPG — Carte et collision

## Carte, collision et dessin

- **Coordonnées du monde** : la carte et les entités sont positionnées en
  pixels de la carte, jamais en pixels de l'écran. C'est ce qui a permis
  d'ajouter la caméra sans modifier `TileMap`.
- **`enum class Tile { Floor, Wall, PorteFermee, PorteOuverte }`** : `enum class`
  plutôt que `enum` pour la portée (`Tile::Wall`) et l'absence de conversion
  implicite en `int`.
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

## Portes et objets interactifs (étape 70)

- **La carte est la seule source de vérité** pour « passe-t-on, voit-on » : une
  porte est un type de case, pas une entité à part. `estBloquante(Tile)` (vrai pour
  `Wall` et `PorteFermee`) remplace tous les tests `== Tile::Wall` ; collision, A*,
  lissage, repli, ligne de vue, détection des monstres et refus d'un point de skill
  de zone traitent donc une porte fermée comme un mur sans rien savoir des portes
  (la refactorisation a précédé l'ajout, sans changer de comportement).
- **Mutateurs nommés par l'intention**, pas de `setTile` : `placerPorte`
  (`Floor` → `PorteFermee`) et `ouvrirPorte` (`PorteFermee` → `PorteOuverte`), chacun
  avec une `assert`. Le plan texte reste de la géométrie pure : `Partie` ferme les
  cases de `Niveau::portes` à sa construction.
- **`ObjetInteractif`** = partie commune (`id`, `zone`) + `std::variant` de natures
  (une seule, `Porte`). Pas de classe de base virtuelle : l'effet a besoin du
  contexte de la `Partie`, l'objet n'est que des données et `Partie` fait le
  `std::visit` ; un coffre plus tard devra être traité à chaque `visit` (erreur de
  compilation sinon).
- **Point d'approche** : on ne vise jamais la porte elle-même, elle bloque et le A*
  refuserait l'ordre. Côté libre le plus proche de l'escouade, milieu des centres des
  cases du côté. Aucun côté libre : le clic compte comme un clic au sol.
- **Le groupe s'avance** : l'ancre va au point d'approche, les personnages suivent en
  formation ; la porte s'ouvre dès qu'**un** personnage vivant en est à 20 px bord à
  bord (`distanceCercleRectangle`, `Escouade::unVivantPresDe`), instantanément.
  `Escouade` ne connaît ni les portes ni les objets.
- **Interaction en cours** (`Partie::m_interactionEnCours`) : annulée par tout ordre
  de déplacement, clic au sol ou clic sur un monstre ; `Controleur` désarme le
  déplacement continu sur `Objet` (sinon le bouton tenu écraserait l'approche).
  Priorité au clic : monstre ciblable et visible, puis porte fermée dont une case au
  moins n'est pas inconnue (étape 72, `docs/vision.md`), puis sol. La vision et le brouillard
  n'ont eu qu'à lire `estBloquante`, comme prévu : une porte fermée arrête la vue, une
  porte ouverte la laisse passer.
- **Ouverture sur place et regard** (étape 71) : si un personnage vivant est déjà à
  portée au moment du clic, `Partie::cliquer` ouvre la porte tout de suite (comme
  les autres effets du clic), sans ordre de déplacement ni interaction mémorisée ;
  l'ancre ne bouge pas. Dans les deux cas `Partie` donne à `Escouade` le centre de
  la zone à regarder (`regarderVers`) pour que les mêlées entrent les premiers ;
  dans le cas de l'approche, l'appel suit l'`allerVers` de l'ordre dans
  `mettreAJour`, car `allerVers` efface le point. Voir `docs/formation.md`.
- **Limites** : l'escouade déjà en marche vers une autre destination au moment d'un clic
  sur une porte à portée la poursuit (la porte s'ouvre, le regard attend l'arrêt) ; les
  monstres n'ouvrent pas les portes ; une porte reste ouverte ; une
  porte d'une seule case est supposée horizontale ; pas de surbrillance au survol.

## Ligne de vue (étapes 44 puis 73)

- **Étape 44** : `TileMap::ligneDeVue(a, b)` répondait par `trajetLibre` avec un carré
  de 2 × 2 px balayé de `a` à `b` (segment centre à centre).
- **Étape 73** : elle a **quitté `TileMap`** pour devenir la fonction libre
  `ligneDeVue(carte, a, b)` de `ChampDeVision` (voir `docs/vision.md`). `TileMap` est la
  couche de base : elle ne doit pas dépendre d'un algorithme qui dépend d'elle.
  `trajetLibre` reste ici : il sert au déplacement, pas à la vue. La constante
  `s_coteLigneDeVue` a disparu avec l'ancienne implémentation.
- **Conséquences du passage aux cases** : la vue se décide d'une case à l'autre ; le
  comportement fin près d'un mur (1 px de marge, coin traversé en diagonale) n'existe plus.
  Un point dans un mur donne toujours faux.

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

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Portes** (étapes 70 et 71) : les couloirs de 2 cases sont fermés, à chaque jonction
  avec une salle (y compris la salle de départ), par une porte brune. Une porte
  fermée bloque comme un mur (déplacement, chemins, ligne de vue, détection des
  monstres, skill de zone). Un clic dessus (si aucun monstre ciblable ne s'y
  trouve) lève le verrou. Si un personnage vivant est déjà à 20 px (bord à bord)
  d'elle, elle s'ouvre tout de suite, sans déplacement ; sinon l'escouade
  s'avance devant elle, du côté libre le plus proche, et elle s'ouvre dès qu'un
  personnage vivant est à 20 px. Dans les deux cas la formation se tourne vers la
  porte (mêlées devant), jusqu'au prochain ordre ou verrouillage. Ouverte, elle
  s'affiche en sol avec un cadre brun et reste ouverte (un clic dessus
  vaut un clic au sol). Un ordre de déplacement (bouton tenu compris), un clic au
  sol ou sur un monstre annule l'approche ; les skills non. Les monstres
  n'ouvrent pas les portes. Voir `docs/carte-et-collision.md` et
  `docs/generation.md`.

### Limites connues

- Portes (étape 70) : seulement sur les ouvertures de la largeur du couloir
  (2 cases) ; pas de refermeture, de clé ni de porte verrouillée ; les monstres ne
  les ouvrent pas et ne contournent pas par un autre chemin ; un seul personnage
  n'ira pas ouvrir pendant que les autres attendent ; pas de surbrillance au
  survol ; une porte d'une seule case est supposée horizontale.

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `TileMap` (`monde/`) — l'environnement de jeu

Grille de cases de taille fixe, chaque case étant du sol ou un mur. Un
point hors de la carte est un mur. Construite à partir d'un plan texte
(`'#'` = mur, tout autre caractère = sol).

Données membres : `m_largeur`, `m_hauteur` (cases), `m_tailleCase`
(pixels), `m_cases` (`std::vector<Tile>`, `enum class Tile { Floor, Wall, PorteFermee, PorteOuverte }`). `bool estBloquante(Tile)` (fonction libre, vrai pour `Wall` et `PorteFermee`) est la seule façon de demander « passe-t-on, voit-on » : collision, A*, lissage, repli, champ de vision (`ChampDeVision`, donc aussi `ligneDeVue`) et skill de zone l'utilisent, aucun ne teste `Tile::Wall` directement.

| Fonction | Rôle |
|---|---|
| `TileMap(const std::vector<std::string>& plan, float tailleCase)` | Construit la carte à partir d'un plan texte ; `std::out_of_range` si le plan est vide, `std::invalid_argument` si les lignes n'ont pas toutes la même longueur |
| `Tile getTile(int x, int y) const` | Type de la case `(x, y)` ; hors de la carte = `Wall` |
| `int getLargeur() const` | Largeur de la carte, en cases |
| `int getHauteur() const` | Hauteur de la carte, en cases |
| `float getTailleCase() const` | Côté d'une case, en pixels (`coutDesObstacles` construit les rectangles des cases) ; `[[nodiscard]]` |
| `sf::Vector2i caseContenant(sf::Vector2f point) const` | Case (coordonnées de grille) contenant ce point, en pixels du monde |
| `sf::Vector2f centreDeCase(sf::Vector2i coordCase) const` | Centre de cette case, en pixels du monde |
| `void dessiner(sf::RenderTarget& rendu) const` | Dessine toutes les cases (porte fermée : case brune pleine ; porte ouverte : sol avec un cadre brun de 4 px) |
| `void placerPorte(sf::Vector2i coordCase)` | `Floor` → `PorteFermee` (`assert`) |
| `void ouvrirPorte(sf::Vector2i coordCase)` | `PorteFermee` → `PorteOuverte` (`assert`) ; une porte ouverte le reste |
| `bool chevaucheUnMur(const sf::FloatRect& rect) const` | Le rectangle (en pixels du monde) chevauche-t-il une case bloquante (mur ou porte fermée) ? |
| `bool trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const` | Le rectangle, translaté de sa position jusqu'à `position + deplacement`, ne chevauche-t-il un mur à aucun moment du trajet ? |

### `ObjetInteractif` (`monde/`) — les objets avec lesquels on interagit

`ObjetInteractif.hpp` / `.cpp` : des données et une fonction libre, sans état. Pas de classe de base virtuelle : l'effet d'une interaction a besoin du contexte de la `Partie`, qui visite `nature` avec `std::visit` et `Surcharges` (`Cible.hpp`). Voir `docs/carte-et-collision.md`.

`struct Porte { std::vector<sf::Vector2i> cases; }` ; `struct ObjetInteractif { int id; sf::FloatRect zone; std::variant<Porte> nature; }`.

| Fonction | Rôle |
|---|---|
| `ObjetInteractif(int identifiant, Porte porte, float tailleCase)` | Calcule `zone`, rectangle en pixels du monde qui couvre toutes les cases ; `assert` : au moins une case, toutes sur une même ligne ou une même colonne |
| `std::optional<sf::Vector2f> pointDApproche(const Porte&, const TileMap&, sf::Vector2f depuis)` | Porte horizontale (cases de même y) : côtés y−1 et y+1 ; verticale : x−1 et x+1. Le point d'un côté est le milieu des centres de ses cases ; seuls les côtés sans case bloquante sont gardés, le plus proche de `depuis` l'emporte ; vide si aucun. Une porte d'une seule case est traitée comme horizontale ; `[[nodiscard]]` |

### `Deplacement` (`monde/`) — collision et glissement partagés

Deux fonctions libres, sans état, utilisées par `Mobile::mettreAJour`.

| Fonction | Rôle |
|---|---|
| `bool deplacementBloque(const Cercle& avant, const Cercle& apres, const std::vector<Cercle>& obstacles)` | Vrai si, pour au moins un obstacle, `apres` chevauche l'obstacle (distance entre centres `<` somme des rayons) ET la distance entre centres a diminué par rapport à `avant` |
| `sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement, const std::vector<Cercle>& obstacles)` | Essaie le déplacement axe par axe (X puis Y) ; un essai est refusé s'il chevauche un mur ou si `deplacementBloque` est vrai. Si ni X ni Y n'ont passé et qu'un cercle a refusé, essaie un déplacement de même longueur le long de la tangente du cercle refusant le plus proche (côté de la projection du déplacement voulu, à gauche de celui-ci si elle est nulle), accepté seulement hors mur et si `deplacementBloque` est faux. Renvoie la nouvelle position du coin haut-gauche |
