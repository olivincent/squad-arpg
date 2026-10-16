# Squad-ARPG — Vision et brouillard de guerre

Étape 72. Ce que le joueur voit dépend de ce que voient ses personnages vivants. Trois
modules : `ChampDeVision` (l'algorithme), `Vision` (la connaissance du joueur) et, dans
`Partie`, les règles de clic et le dessin.

## `ChampDeVision` : shadowcasting symétrique

- **Algorithme** : *symmetric shadowcasting* d'Albert Ford. Quatre quadrants, des rangées
  de cases de plus en plus loin de l'origine, un intervalle de pentes [début, fin] qui
  se rétrécit quand un mur coupe la lumière. Le principe est commenté en tête de
  `ChampDeVision.cpp`.
- **Pourquoi symétrique** : à l'étape 73, `TileMap::ligneDeVue` (attaques, skills,
  détection des monstres) passera par ce module. « A voit B » doit alors valoir « B voit
  A », sinon un monstre pourrait frapper un personnage qu'il ne voit pas, ou
  l'inverse. Le shadowcasting classique n'est pas symétrique ; la variante de Ford l'est
  en n'acceptant une case libre que si son **centre** est dans l'intervalle.
- **Mur et porte fermée** : une case bloquante (`estBloquante`) qui reçoit la lumière est
  révélée et l'arrête. On voit la face d'un mur ou d'une porte fermée, jamais ce qui est
  derrière. Une porte ouverte est du sol.
- **Pentes en fractions d'entiers** (`Pente`, produits croisés), jamais en `float` : un
  arrondi ferait différer deux calculs « identiques » vus des deux bouts et casserait la
  symétrie. Les divisions arrondies vers le bas sont écrites à la main (`/` arrondit vers
  zéro en C++).
- **Rayon** : en cases, rangées jusqu'à la profondeur `rayon`, et une case n'est révélée que
  si la distance au carré entre les centres est `<= rayon²` (entiers). La lumière continue
  de se propager au-delà du cercle dans la rangée : seule la révélation est coupée.
- **Résultat** : `std::vector<sf::Vector2i>`, doublons permis (les quadrants partagent leurs
  axes). Une case hors de la carte est un mur (elle bloque) mais n'est jamais renvoyée ;
  l'origine n'est donc pas renvoyée si elle est hors carte (cas qui n'arrive pas en jeu).
- **Test de symétrie** : pour toute paire de cases non bloquantes à distance `<= rayon`,
  sur un plan à piliers et sur trois cartes générées (portes une sur deux ouvertes). Le test
  compte aussi les paires qui ne se voient pas, pour prouver qu'il éprouve quelque chose.
  Vérifié par mutation : sans le critère du centre, il échoue.

## `Vision` : la connaissance, pas le monde

- **Séparée de la `TileMap`** : la carte est la vérité (ce qui est vrai du monde) ; `Vision`
  est ce que le joueur en sait. Elle copie largeur, hauteur et taille de case, et ne garde
  qu'un `std::vector<EtatCase>` (`Inconnue`, `Exploree`, `Visible`) indexé comme la carte.
  Elle ne connaît ni `Personnage`, ni `Escouade`, ni `Monstre` : elle reçoit des `Cercle`
  (centre et rayon de vision en pixels).
- **`mettreAJour`** : tout `Visible` devient `Exploree`, puis le champ de chaque source
  redevient `Visible`. Une case explorée ne redevient jamais inconnue. Rayon en cases =
  rayon / taille de case, arrondi vers le bas (320 px = 8 cases).
- **`estVisible(point)`** prend des pixels du monde : un monstre est visible si la case qui
  contient son centre l'est. Un monstre à cheval sur une case visible et une case cachée
  suit donc son centre.
- **Dessin** : un seul `sf::VertexArray` de triangles (6 sommets par case voilée),
  reconstruit à chaque appel. Noir opaque sur l'inconnu, noir d'alpha 150 sur l'exploré,
  rien sur le visible. Sur 2360 cases c'est négligeable ; la reconstruction seulement quand
  un personnage change de case est écartée tant qu'aucun ralentissement n'est mesuré.

## Rayon de vision des personnages

- `Personnage::m_rayonVision` (pixels), fourni par `Escouade` à la construction comme
  l'armure : 320 px pour les quatre (`s_rayonVision` dans `Escouade.cpp`). La structure
  permet des rayons différents par personnage ; les valeurs restent identiques.
- `Escouade::cerclesDeVision()` : un `Cercle` (centre du personnage, rayon de vision) par
  personnage **vivant**. La vision de l'escouade est l'union des champs ; un mort ne voit
  plus.

## Dans `Partie`

- **`m_vision`**, mise à jour à la fin du constructeur (la salle de départ est visible dès la
  première image) et à la **fin** de `mettreAJour`, après le déplacement de l'escouade et
  des monstres, pour que le dessin corresponde aux positions. Une partie perdue ou gagnée est
  figée et ne la met plus à jour. Une nouvelle partie crée une `Vision` neuve.
- **Règles de clic** (seules règles de jeu qui lisent la vision) :
  - un monstre non visible n'est pas candidat ; le clic tombe sur la porte ou le sol, comme
    si le monstre n'était pas là ;
  - une porte n'est cliquable que si au moins une de ses cases n'est pas `Inconnue` ; sinon
    le clic compte comme un clic au sol ;
  - un clic au sol dans l'inconnu reste permis.
- **Inchangés, volontairement** : autodéfense, skills (le skill de zone peut être visé dans
  l'inconnu), détection et frappe des monstres, verrou déjà posé. Ils continuent d'utiliser
  `ligneDeVue`. C'est l'étape 73.
- **Dessin** : carte, monstres visibles, escouade, voile de la `Vision` ; puis, dans
  `EcranJeu`, l'aperçu de visée, les traces de débogage (F3) et l'interface. Un monstre non
  visible n'est pas dessiné du tout (ni carré, ni barre de vie, ni contour jaune).
- **F6** (`OptionsDeDebogage::brouillard`, vrai au départ, remis par `nouvelleCarte`) :
  brouillard coupé = ni voile, ni monstre caché. Visuelle **seulement** : les règles de clic
  s'appliquent toujours, ce qui permet de voir où est un monstre qu'on ne peut pas cliquer.
  Le paramètre `brouillard` de `Partie::dessiner` s'ajoute à `afficherAncre`.
- **Pas d'accesseur `getVision()`** : les tests passent par le comportement (un monstre
  voisin est cliquable avant tout `mettreAJour` ; un monstre éloigné ou derrière un mur ne
  l'est pas ; il le devient quand l'escouade s'approche).

## Limites connues

- Un monstre verrouillé qui sort de la vision garde le verrou : l'escouade le poursuit sans
  le voir.
- Les monstres n'ont ni vision ni mémoire propres ; pas de « fantôme » de la dernière
  position vue d'un monstre.
- La vision n'intervient pas dans les attaques, skills et détections (étape 73) : le skill
  de zone peut toucher des monstres voilés, un monstre voilé peut détecter et frapper, et
  la ligne de vue centre à centre ne coïncide pas exactement avec le champ de vision.
- Le compteur « Monstres : N » révèle le nombre total de monstres, vus ou non.
- Bords durs, pas de dégradé ni de lumière ; pas de mini-carte ; la caméra ne dézoome pas.
