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

## Suivi de chemin par l'escouade

- **Conversions pixels ↔ cases regroupées dans `TileMap`** (seule à
  connaître `m_tailleCase`) : `caseContenant` (comme `chevaucheUnMur`,
  avec `std::floor` pour rester correct en négatif) et `centreDeCase`.
- **Les points de passage sont des positions du centre de l'escouade**,
  pas de son coin haut-gauche. `mettreAJour` calcule donc la direction et
  la distance depuis `getCentre()`, mais applique le déplacement obtenu
  (un simple vecteur de translation) à `m_position` — une translation
  déplace un rectangle rigide tout entier, inutile de la recalculer
  séparément pour le centre et le coin.
- **`m_chemin` + `m_indexChemin`** plutôt qu'un seul point cible : avancer
  l'indice évite un `erase(begin())` à chaque point atteint (coûteux, car
  il décale tous les éléments suivants en mémoire). Le chemin déjà
  parcouru reste en mémoire jusqu'au prochain `allerVers`.
- **L'appel au A* est dans `Escouade::allerVers`**, pas dans `main.cpp`,
  avec la carte passée en paramètre comme pour `mettreAJour` (jamais
  gardée en membre).
- **Construction du chemin suivi** à partir du résultat de
  `trouverChemin` (qui travaille en cases) : la case de départ est
  ignorée (l'escouade est déjà dessus, la viser d'abord la ferait
  reculer vers le centre de sa propre case) ; les cases intermédiaires
  sont converties en centres de case ; le dernier point est remplacé par
  le point exact cliqué, pour que l'escouade s'arrête précisément dessus
  plutôt qu'au centre de la case d'arrivée.
- **Chemin vide = ordre ignoré** : `allerVers` ne touche à `m_chemin`
  qu'après avoir confirmé que `trouverChemin` a renvoyé un résultat non
  vide ; un clic sur un mur ou une zone inaccessible n'interrompt donc
  pas le trajet en cours.
- **Collision et glissement conservés tels quels**, comme filet de
  sécurité : l'escouade peut partir d'une position décentrée dans sa case
  ou viser un point proche d'un mur (notamment le point exact cliqué).
- **Chemin lissé avant d'être stocké** (étape 11) : entre la
  construction du chemin en pixels et son affectation à `m_chemin`,
  `allerVers` le passe par `Chemin::lisserChemin`, avec `getCentre()`
  comme point de départ et `m_taille` comme taille de rectangle. Voir
  « Lissage de chemin » ci-dessous. L'ancien zigzag près des coins de
  case (viser un centre de case presque aligné avec l'escouade sur un
  seul axe) est résolu par ce lissage.

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
- **Branchement dans `Escouade::allerVers`** : le chemin en pixels est
  construit comme avant l'étape 11 (centres de case, puis point exact
  cliqué), puis lissé avant d'être stocké dans `m_chemin`. Le reste
  d'`Escouade` (`mettreAJour`, collision, glissement, chemin vide = ordre
  ignoré) n'a pas changé.
- **Limite connue** : `lisserChemin` ne fait que retirer des points de la
  liste donnée par le A* (centres de case) ; il ne peut pas en ajouter ni
  en déplacer. Autour d'un pilier, dès qu'un saut direct est bloqué par
  le mur, l'escouade retombe sur le centre de la case suivante tel quel,
  plutôt que sur un point plus proche du coin du pilier qui
  raccourcirait encore le trajet — elle frôle donc moins les murs qu'un
  lissage « idéal » (type *funnel algorithm* ou Theta*) ne le ferait,
  mais elle ne reste jamais coincée. Accepté pour l'instant.
