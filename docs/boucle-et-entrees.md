# Squad-ARPG — Boucle de jeu et entrées

## Boucle de jeu et déplacement

- **Boucle de jeu** : événements (fermeture de la fenêtre, et
  verrouillage de cible sur un clic qui touche le monstre), recharge et
  attaque, retrait du verrou, entrée souris (`escouade.allerVers` si le
  bouton gauche est enfoncé et la cible non verrouillée — voir « Entrée
  souris et attaque » ci-dessous), mise à jour (`escouade.mettreAJour`),
  dessin.
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
- **Verrouillage de cible : événement, pas d'état** (étape 16) :
  l'événement `MouseButtonPressed` (bouton gauche) sert uniquement à
  **poser** le verrou (`cibleVerrouillee = true`) si le monstre est
  vivant et que le point cliqué le contient — il n'inflige plus de
  dégâts lui-même. Nécessaire en tant qu'événement (pas lecture d'état)
  parce qu'il ne doit se déclencher qu'à l'instant du clic, une fois,
  même si le bouton reste ensuite enfoncé.
- **Retrait du verrou : lecture d'état, pas d'événement
  `MouseButtonReleased`** : après la boucle d'événements, le verrou
  retombe si la fenêtre n'a plus le focus, si le bouton gauche n'est
  plus enfoncé, ou si le monstre est mort. La position du curseur ne
  compte plus une fois le verrou posé — aucune zone de tolérance autour
  du monstre.
- **Recharge et attaque continue** : `rechargeAttaque` (`float`,
  secondes) est décrémentée de `dt` à chaque image, sans descendre sous
  zéro — le même `dt` que celui passé à `escouade.mettreAJour`. Un coup
  (`monstre.recevoirDegats(degatsParClic)`) n'est permis que si la cible
  est verrouillée **et** que la recharge est à zéro ; après un coup, la
  recharge repart à `tempsRecharge` (0,5 s). Conséquence : cliquer très
  vite ne frappe pas plus souvent que tenir le bouton, puisque seul le
  temps réel écoulé fait descendre la recharge.
- **Ordre précis après la boucle d'événements** : 1) diminuer la
  recharge ; 2) attaquer si verrou posé et recharge à zéro ; 3) retirer
  le verrou si nécessaire ; 4) donner l'ordre de déplacement si le
  bouton est enfoncé et la cible non verrouillée. L'attaque (2) passe
  **avant** le retrait du verrou (3) pour qu'un clic très bref, posé et
  retiré dans la même image, compte quand même pour un coup ; et si ce
  coup tue le monstre, l'étape 3 fait retomber le verrou aussitôt,
  permettant à l'étape 4 de redonner un ordre de déplacement vers le
  curseur dès cette même image.
- **`degatsParClic` (20) et `tempsRecharge` (0,5 s)** : constantes
  locales à `main.cpp` — des propriétés de l'attaque, pas du monstre.
  Le verrou (`cibleVerrouillee`) et la recharge (`rechargeAttaque`) sont
  eux aussi de simples variables locales ; pas de nouvelle classe
  (`Combat`, `Attaque`, `Minuteur`) pour les porter.
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
