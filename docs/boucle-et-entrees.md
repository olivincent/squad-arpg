# Squad-ARPG — Boucle de jeu et entrées

## Boucle de jeu et déplacement

- **Boucle de jeu** : événements (fermeture de la fenêtre, et
  verrouillage de cible sur un clic qui touche un monstre), attaque si
  la cible est verrouillée, retrait du verrou, retrait des monstres
  morts, entrée souris (`escouade.allerVers` si le bouton gauche est
  enfoncé et la cible non verrouillée — voir « Entrée souris et
  attaque » ci-dessous), mise à jour (`escouade.mettreAJour`), dessin.
- **Déplacement à pas fixe** plutôt que proportionnel à la distance :
  vitesse constante, comme un personnage d'ARPG.
- **Delta time** (`sf::Clock::restart()`) : vitesse exprimée en pixels
  par seconde, pour qu'elle ne dépende pas de la cadence réelle.
  `setFramerateLimit(60)` est conservé pour limiter l'usage du CPU, mais
  la logique n'en dépend plus. L'horloge reste dans `main.cpp` ; `dt` est
  passé en paramètre à l'escouade.

## Entrée souris et attaque

- **Déplacement : lecture d'état** (étape 13) : `sf::Mouse::
  isButtonPressed(sf::Mouse::Button::Left)` est interrogé à chaque
  image, après la boucle d'événements et avant `escouade.mettreAJour`.
  Tant que le bouton reste enfoncé (et que la cible n'est pas
  verrouillée sur le monstre, voir ci-dessous), `escouade.allerVers` est
  rappelée à chaque image avec la position courante du curseur — pas de
  second mode de déplacement « en direction du curseur », c'est le même
  A* + lissage qu'un clic ponctuel, simplement redonné en continu.
- **`window.hasFocus()`** : condition nécessaire, parce que
  `isButtonPressed` lit l'état global de la souris au niveau du système,
  même quand la fenêtre du jeu n'a pas le focus.
- **Pas de limitation de fréquence** : un appel au A* par image, jugé
  négligeable sur la carte actuelle ; à revisiter seulement si un
  problème de performance est mesuré.
- **Plusieurs monstres, cible désignée par identifiant** (étape 18) :
  `main.cpp` possède `std::vector<Monstre> monstres` ; chaque `Monstre`
  porte un `m_id` (`int`) attribué par `main.cpp` (variable locale
  `prochainId`, incrémentée à chaque création). Le verrou devient
  `std::optional<int> idCible` (vide = pas de verrou) : on stocke
  l'identifiant du monstre visé, jamais un indice, un pointeur ou un
  itérateur — l'itérateur est recalculé à chaque image avec
  `std::find_if` et ne survit pas au-delà du bloc où il est utilisé
  (un monstre pourrait être retiré du vecteur entre deux images,
  invalidant tout itérateur gardé plus longtemps).
- **Verrouillage de cible : événement, pas d'état** (étape 16) :
  l'événement `MouseButtonPressed` (bouton gauche) sert uniquement à
  **poser** le verrou (`idCible = ...`) sur le premier monstre du
  vecteur dont le point cliqué est à l'intérieur (`std::find_if`,
  étape 18 — plus de vérification `estVivant()` ici, voir plus bas) —
  il n'inflige plus de dégâts lui-même. Nécessaire en tant qu'événement
  (pas lecture d'état) parce qu'il ne doit se déclencher qu'à l'instant
  du clic, une fois, même si le bouton reste ensuite enfoncé.
- **Retrait du verrou : lecture d'état, pas d'événement
  `MouseButtonReleased`** : après la boucle d'événements, le verrou
  retombe si la fenêtre n'a plus le focus, si le bouton gauche n'est
  plus enfoncé, ou si le monstre visé n'est plus trouvé dans le vecteur
  (`std::find_if` par `id`, mort donc retiré) ou vient de mourir.
  La position du curseur ne compte plus une fois le verrou posé —
  aucune zone de tolérance autour du monstre.
- **Attaque déplacée dans `Escouade` (étape 17)** : `main.cpp` ne gère
  plus de recharge lui-même. Tant que la cible est verrouillée et
  trouvée dans le vecteur, `it->recevoirDegats(escouade.attaquer(
  it->getCentre(), carte))` est appelée à chaque image ; `attaquer`
  décide si l'escouade doit encore s'approcher ou si elle peut
  s'arrêter et frapper (portée, dégâts et recharge par personnage —
  voir `docs/formation.md`), et renvoie la somme des dégâts à appliquer
  au monstre. Les variables locales `rechargeAttaque`, `tempsRecharge`
  et `degatsParClic` de l'étape 16 ont disparu de `main.cpp`.
- **Retrait des monstres morts** (étape 18) : `std::erase_if(monstres,
  ...)` enlève du vecteur tout monstre dont `estVivant()` est faux.
  Exécuté après le retrait du verrou et avant l'ordre de déplacement.
  À partir de là, l'invariant « tout monstre du vecteur est vivant »
  tient jusqu'au prochain appel d'`erase_if` — ce qui permet à la
  boucle de clic et à la boucle de dessin de ne plus tester
  `estVivant()` elles-mêmes.
- **Ordre précis après la boucle d'événements** : 1) attaquer si la
  cible est verrouillée (et retomber si le monstre n'est plus trouvé) ;
  2) retirer le verrou si la fenêtre n'a plus le focus ou le bouton
  n'est plus enfoncé ; 3) retirer les monstres morts ; 4) donner l'ordre
  de déplacement si le bouton est enfoncé et la cible non verrouillée.
  L'attaque (1) passe **avant** le retrait du verrou de focus/bouton (2)
  pour qu'un clic très bref, posé et retiré dans la même image, compte
  quand même pour une tentative de coup ; et si ce coup tue le monstre,
  l'étape 1 fait déjà retomber le verrou, permettant à l'étape 4 de
  redonner un ordre de déplacement vers le curseur dès cette même
  image.
- **Le verrou (`idCible`) reste une simple variable locale** à
  `main.cpp` ; pas de nouvelle classe (`Combat`, `Minuteur`) pour le
  porter.
- **Texte de position retiré** (ainsi que la police `arial.ttf` qui ne
  servait qu'à lui) : plus rien n'affiche la position du curseur à
  l'écran.

## Caméra

- **`sf::View` dans `main.cpp`** : une simple variable `camera`, copiée de
  `window.getDefaultView()` (taille de la fenêtre). Pas de classe `Camera`
  tant qu'il n'y a pas de comportement à y encapsuler.
- **Toujours centrée sur l'escouade** : à chaque image, après la mise à
  jour et avant le dessin, `camera.setCenter(escouade.getCentre())` puis
  `window.setView(camera)`.
- **Conséquence acceptée** : près des bords de la carte, la caméra montre
  du noir au-delà des murs. Bloquer la caméra aux bords de la carte reste
  une option pour plus tard.
- **Position du curseur** : convertie avec `window.mapPixelToCoords(...)`
  après la boucle d'événements, donc avec la vue de l'image précédente —
  celle effectivement affichée à l'écran au moment de la lecture — ce
  qui est voulu (même raisonnement que pour l'ancien clic, voir
  « Entrée souris et attaque » ci-dessous).
