# Squad-ARPG — Vision et brouillard de guerre

Étape 72. Ce que le joueur voit dépend de ce que voient ses personnages vivants. Trois
modules : `ChampDeVision` (l'algorithme), `Vision` (la connaissance du joueur) et, dans
`Partie`, les règles de clic et le dessin.

## `ChampDeVision` : shadowcasting symétrique

- **Algorithme** : *symmetric shadowcasting* d'Albert Ford. Quatre quadrants, des rangées
  de cases de plus en plus loin de l'origine, un intervalle de pentes [début, fin] qui
  se rétrécit quand un mur coupe la lumière. Le principe est commenté en tête de
  `ChampDeVision.cpp`.
- **Pourquoi symétrique** : depuis l'étape 73, `ligneDeVue` (attaques, skills, détection des
  monstres) passe par ce module. « A voit B » doit valoir « B voit A », sinon un monstre
  pourrait frapper un personnage qu'il ne voit pas, ou l'inverse. Le shadowcasting classique n'est pas symétrique ; la variante de Ford l'est
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

## Une seule ligne de vue (étape 73)

- **Un seul balayage** : la classe `Balayage` (patron `template <typename Visiteur>`) contient
  les quadrants, les rangées, les pentes et le critère du centre, **en un seul exemplaire**. Elle
  appelle un visiteur pour chaque case révélée dans la carte ; si le visiteur renvoie vrai, le
  balayage s'arrête. Les trois fonctions publiques ne diffèrent que par le visiteur et la
  profondeur maximale :
  - `champDeVision(carte, origine, rayon)` : visiteur qui collecte les cases à distance
    `<= rayon` (entiers) ; profondeur max = `rayon` ; comportement et tests de l'étape 72
    inchangés ;
  - `estVisibleDepuis(carte, origine, cible)` : visiteur qui cherche la `cible` et arrête
    dès qu'il la trouve ; profondeur max = profondeur de la cible dans le quadrant ; seuls
    les quadrants qui la contiennent sont balayés. Une cible sur une diagonale (|colonne| =
    profondeur) est dans deux quadrants : vrai si l'un des deux la révèle, ce qui donne
    exactement l'union de `champDeVision` ;
  - `ligneDeVue(carte, a, b)` (pixels du monde) : faux si la case de `a` ou de `b` est
    bloquante (hors carte = mur) ; sinon `estVisibleDepuis(caseContenant(a), caseContenant(b))`.
- **Visiteur plutôt que copier le balayage** : une seconde copie du critère de révélation
  pourrait diverger de la première, et c'est justement l'équivalence entre vision du joueur
  et ligne de vue des combattants qu'on veut garantir. Un patron plutôt que `std::function` :
  l'appel est inlinable, et `ligneDeVue` est appelée plusieurs fois par image et par monstre.
- **Pourquoi hors de `TileMap`** : `TileMap` est la couche de base ; elle ne doit pas dépendre
  d'un algorithme qui dépend d'elle. `TileMap::ligneDeVue` et sa constante ont été supprimées.
- **Pas de limite de rayon dans `estVisibleDepuis`** : la portée est vérifiée ailleurs (bord à
  bord). Le rayon de vision des personnages (320 px) n'intervient donc pas dans le combat.
- **Granularité** : la vue se décide en cases. Deux points d'une même case se voient toujours ;
  un point dans un mur ou une porte fermée ne voit rien et n'est vu de personne (règle
  d'avant, conservée). Le comportement fin près d'un mur (1 px de marge, coin traversé en
  diagonale) change.
- **Deux murs en diagonale** : deux cases libres en diagonale entre deux murs qui se touchent
  **se voient**. Le sol en diagonale est à la pente exactement 1 (ou −1), son centre est dans
  l'intervalle (bornes incluses), et le mur qui précède n'a pas encore rétréci l'intervalle de
  ce côté. C'est ce que donne l'algorithme de Ford, conservé sans retouche ; l'ancien segment
  centre à centre était bloqué. Épinglé par un test. Effet visible en jeu : on peut viser en
  biais à travers un coin « en escalier ».
- **Coins diagonaux : réglés dans la carte, pas dans l'algorithme** (décision d'après l'étape 73).
  Le motif qui pose problème est un *damier* : un carré de 2 × 2 cases dont deux cases en
  diagonale sont du sol et les deux autres des murs. On n'y passe pas (deux murs qui se
  touchent par un coin arrêtent le déplacement), mais on y voit. Corriger `ChampDeVision` pour
  que la lumière ne passe pas serait possible, mais risquerait la symétrie « A voit B ⇔ B voit
  A », qui est ce que l'étape 73 garantit ; mieux vaut empêcher le motif d'exister. La fonction
  pure `contientUnCoinDiagonal(plan)` de `GenerateurDonjon` le détecte (voir
  `docs/generation.md`).
- **Le motif apparaît-il ? Oui, rarement** : le test de propriété sur les graines 1 à 50 passe
  (aucun coin diagonal), donc **aucune passe de nettoyage n'a été écrite**, comme convenu. Mais un
  sondage de 20 000 graines, hors des tests, en trouve **145 (0,7 %)**, la première étant la 391,
  chacune avec exactement un motif : deux masses de mur qui se touchent par un coin, avec du sol
  sur l'autre diagonale. Le test 1 à 50 est donc un échantillon, pas une garantie ; si le motif
  doit être exclu pour de bon, il faut soit étendre le test, soit écrire la passe de nettoyage
  (creuser en sol le mur du haut ou de gauche de chaque motif, après les couloirs et avant
  `placerPortes`, jusqu'à ce qu'il n'y en ait plus, puis revérifier la connexité).
- **Pas de cache** : voir `docs/combat.md`.
- **Tests** : équivalence `estVisibleDepuis` ⇔ `champDeVision` et symétrie, sur le plan à
  piliers et trois cartes générées ; voir `docs/tests.md`.

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
  plus. L'escouade compte quatre personnages depuis l'étape 106 (`s_tailleDeLEscouade`, `docs/formation.md`) : l'union est de quatre
  champs au plus, sans rien changer à l'algorithme (la vision lit les cercles de l'escouade, quel que soit leur nombre).

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
- **Combat** : autodéfense, skills, détection et frappe des monstres utilisent `ligneDeVue`,
  donc le même shadowcasting que le dessin du brouillard (étape 73, voir plus bas). Le skill de
  zone peut encore être visé dans l'inconnu, et le verrou déjà posé n'est pas rompu.
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
- Le skill de zone peut toucher des monstres voilés (hors du rayon de vision, mais en vue
  du point d'impact), et un monstre voilé peut détecter et frapper : la ligne de vue ne
  regarde pas le rayon de vision (320 px), seulement les murs ; la portée est vérifiée à
  part.
- Le compteur « Monstres : N » révèle le nombre total de monstres, vus ou non.
- Bords durs, pas de dégradé ni de lumière ; pas de mini-carte ; la caméra ne dézoome pas.

## État et limites à la fin de l'étape 80

Repris tels quels de la section « État actuel » de `ROADMAP.md` à l'étape 81 (K1), pour ce
sujet. `ROADMAP.md` n'en garde qu'un résumé. Les sections précédentes de ce document peuvent dire la
même chose avec plus de détail.

### État

- **Vision et brouillard de guerre** (étape 72) : chaque case est inconnue, explorée ou
  visible (`Vision`, possédée par `Partie`). Les personnages vivants voient à 320 px (8 cases)
  chacun ; la vision de l'escouade est l'union de leurs champs, calculés par un shadowcasting
  symétrique (`ChampDeVision`) : un mur ou une porte fermée est vu mais arrête la vue, une porte
  ouverte la laisse passer. Un voile noir opaque couvre l'inconnu, un voile sombre (alpha 150)
  l'exploré (les zones visitées restent affichées). Un monstre hors de vue n'est pas dessiné
  (ni barre de vie, ni contour) et n'est pas cliquable (le clic déplace l'escouade) ; une
  porte dont aucune case n'est connue non plus. Le combat (autodéfense, skills, détection des
  monstres, verrou posé) utilise `ligneDeVue`, qui passe depuis l'étape 73 par le même
  shadowcasting (voir « Ligne de vue »). F6 (débogage) coupe
  le brouillard (visuel seulement : la règle de clic reste) ; F5 le remet. Voir `docs/vision.md`.
- **Ligne de vue** : un personnage ne frappe, ne soigne et ne provoque
  que ce qu'il voit (à portée ET `ligneDeVue`, étape 73, en cases,
  règle unique `Escouade::peutAtteindre`) : autodéfense, combat verrouillé
  (l'ancre continue d'avancer tant que tous les vivants ne voient pas la
  cible), agresseurs (qui ne voit pas son agresseur va vers lui), skills
  de dégâts, soin et zone. Un skill de dégâts ou de soin à portée mais
  sans vue est mis en attente et le lanceur marche ; sans verrou, un
  monstre caché n'est pas candidat. La provocation ne touche que les
  monstres en vue (recharge consommée dans tous les cas). **Étape 73** :
  `ligneDeVue(carte, a, b)`, fonction libre de `ChampDeVision` (elle a quitté
  `TileMap`), répond par le même shadowcasting symétrique que le brouillard :
  ce qu'un personnage voit est exactement ce qu'il peut viser, et « A voit B »
  équivaut à « B voit A », monstres compris (détection, frappe, places
  d'attaque, provocation, skill de zone). La vue se décide d'une case à
  l'autre : deux points d'une même case se voient, un point dans un mur ne
  voit rien. Voir `docs/vision.md` et `docs/combat.md`.

### Limites connues

- Vision (étapes 72 et 73) : un monstre verrouillé qui sort de la vision garde le verrou ; la
  ligne de vue ne regarde pas le rayon de vision (320 px) : un monstre voilé peut détecter et
  frapper, et le skill de zone peut toucher des monstres voilés ; le skill de zone peut être
  visé dans l'inconnu ; pas de mémoire des monstres vus ; le compteur « Monstres : N » compte
  aussi les monstres cachés ; deux cases libres en diagonale entre deux murs qui se touchent se
  voient (algorithme de Ford), ce que le générateur n'empêche pas (145 graines sur 20 000, soit
  0,7 %, contiennent un tel coin ; le test sur les graines 1 à 50 n'en trouve aucun) ;
  `ligneDeVue` n'a pas de cache (aucun ralentissement mesuré). Voir `docs/vision.md`.

## Référence des modules

Repris tel quel d'`ARCHITECTURE.md` à l'étape 81 (K1, « déplacer d'abord ») : données
membres et interfaces publiques des modules de ce sujet. Le dossier de `src/` de chaque module est
indiqué dans son titre. `ARCHITECTURE.md` n'en garde qu'une ligne par module.

### `ChampDeVision` (`monde/`) — les cases qu'un observateur voit

`ChampDeVision.hpp` / `.cpp` : trois fonctions libres pures, sans état, sans SFML graphique. Shadowcasting symétrique (Albert Ford), pentes en fractions d'entiers. Un seul balayage (classe `Balayage`, patron à visiteur) sert `champDeVision`, `estVisibleDepuis` et `ligneDeVue`. Utilisé par `Vision::mettreAJour` et, via `ligneDeVue`, par toutes les règles « en vue » : `Escouade::peutAtteindre`, provocation, skill de zone, places d'attaque, `Monstre::peutAtteindre` et détection des monstres. Voir `docs/vision.md`.

| Fonction | Rôle |
|---|---|
| `std::vector<sf::Vector2i> champDeVision(const TileMap& carte, sf::Vector2i origine, int rayon)` | Cases vues depuis la case `origine`, à distance (entre centres) `<= rayon` cases. Une case bloquante (`estBloquante`) qui reçoit la lumière est révélée et l'arrête ; une case libre n'est révélée que si son centre est dans l'intervalle de pentes (symétrie : A voit B ⇔ B voit A pour deux cases libres). Hors carte = mur, jamais renvoyé ; l'origine est renvoyée si elle est dans la carte ; doublons permis ; `[[nodiscard]]` |
| `bool estVisibleDepuis(const TileMap& carte, sf::Vector2i origine, sf::Vector2i cible)` | La case `cible` serait-elle révélée par `champDeVision` depuis `origine` avec un rayon assez grand ? Même critère, mais ne balaie que les quadrants qui contiennent la cible (une cible sur une diagonale est dans deux quadrants : vrai si l'un la révèle), jusqu'à sa rangée, et s'arrête dès qu'elle est révélée. Pas de limite de rayon (la portée est vérifiée ailleurs). `origine == cible` : vrai ; cible hors carte : faux ; symétrique pour deux cases libres ; `[[nodiscard]]` |
| `bool ligneDeVue(const TileMap& carte, sf::Vector2f a, sf::Vector2f b)` | Le point `a` voit-il le point `b` (pixels du monde) ? Faux si la case de `a` ou celle de `b` est bloquante (mur, porte fermée, hors carte) ; sinon `estVisibleDepuis` entre leurs cases. Se décide en cases : deux points d'une même case se voient toujours ; symétrique ; `[[nodiscard]]` |

### `Vision` (`monde/`) — ce que le joueur sait de la carte

`Vision.hpp` / `.cpp`. Possédée par valeur par `Partie`. Ne connaît ni `Personnage`, ni `Escouade`, ni `Monstre` : elle reçoit des `Cercle`. `enum class EtatCase { Inconnue, Exploree, Visible }`.

Données membres : `m_largeur`, `m_hauteur`, `m_tailleCase` (copiées de la carte), `m_cases` (`std::vector<EtatCase>`, même indexage que `TileMap`).

| Fonction | Rôle |
|---|---|
| `explicit Vision(const TileMap& carte)` | Tout est `Inconnue` |
| `void mettreAJour(const TileMap& carte, const std::vector<Cercle>& sources)` | Tout `Visible` devient `Exploree`, puis pour chaque source (centre et rayon en pixels) le `champDeVision` depuis la case du centre, rayon en cases = rayon / taille de case arrondi vers le bas, devient `Visible`. Une case explorée ne redevient jamais inconnue ; `assert` : mêmes dimensions que la carte de la construction |
| `EtatCase etat(sf::Vector2i coordCase) const` | Hors carte = `Inconnue` ; `[[nodiscard]]` |
| `bool estVisible(sf::Vector2f point) const` | La case qui contient ce point (pixels du monde) est-elle `Visible` ? ; `[[nodiscard]]` |
| `void dessiner(sf::RenderTarget& rendu) const` | Un seul `sf::VertexArray` (`Triangles`, 6 sommets par case voilée), reconstruit à chaque appel : noir opaque sur l'inconnu, noir d'alpha 150 sur l'exploré, rien sur le visible ; à dessiner avec la vue du monde |
