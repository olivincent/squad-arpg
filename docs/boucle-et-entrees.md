# Squad-ARPG — Boucle de jeu et entrées

## Boucle de jeu et déplacement

- **Boucle de jeu** : événements (fermeture de la fenêtre, et l'appui de
  souris qui pose ou retire le verrou de combat d'`Escouade`, ou arme le
  déplacement continu — seulement si l'escouade est vivante, depuis
  l'étape 26), attaque si la cible est verrouillée, retrait du verrou
  (mort de la cible seulement, depuis l'étape 27), retrait des monstres
  morts, entrée souris (`escouade.allerVers` si le bouton gauche est
  enfoncé et que le déplacement continu a été armé par l'événement
  d'appui — voir « Entrée souris et attaque » ci-dessous), mise à jour
  (`escouade.mettreAJour`, puis chaque monstre : détection/poursuite via
  son propre `Mobile` et riposte — voir « Riposte des monstres »
  ci-dessous), caméra, dessin. La défaite n'arrête jamais la boucle
  elle-même : seuls les ordres du joueur sont bloqués.
- **Déplacement à pas fixe** plutôt que proportionnel à la distance :
  vitesse constante, comme un personnage d'ARPG.
- **Delta time** (`sf::Clock::restart()`) : vitesse exprimée en pixels
  par seconde, pour qu'elle ne dépende pas de la cadence réelle.
  `setFramerateLimit(60)` est conservé pour limiter l'usage du CPU, mais
  la logique n'en dépend plus. L'horloge reste dans `main.cpp` ; `dt` est
  passé en paramètre à l'escouade.

## Entrée souris et attaque

- **Déplacement continu : lecture d'état, armée par l'événement
  d'appui** (étape 13, revu à l'étape 27) : `sf::Mouse::
  isButtonPressed(sf::Mouse::Button::Left)` est interrogé à chaque
  image, après la boucle d'événements et avant `escouade.mettreAJour`.
  Depuis l'étape 27, ce n'est plus suffisant à lui seul : une variable
  locale `deplacementContinu` (`bool`), mise à jour uniquement par
  l'événement `MouseButtonPressed`, dit si le bouton tenu doit
  effectivement produire un déplacement. Tant que le bouton reste
  enfoncé et que `deplacementContinu` est vrai, `escouade.allerVers` est
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
  `prochainId`, incrémentée à chaque création). Le verrou vit depuis
  l'étape 27 dans `Escouade` (`std::optional<int>`, vide = pas de
  verrou, voir `docs/formation.md`) : on y stocke l'identifiant du
  monstre visé, jamais un indice, un pointeur ou un itérateur —
  l'itérateur est recalculé à chaque image avec `std::find_if` et ne
  survit pas au-delà du bloc où il est utilisé (un monstre pourrait
  être retiré du vecteur entre deux images, invalidant tout itérateur
  gardé plus longtemps).
- **Un seul événement d'appui décide de tout** (étape 16, revu à
  l'étape 27) : l'événement `MouseButtonPressed` (bouton gauche) décide,
  une fois pour toutes à l'instant du clic, ce que fait ce clic — peu
  importe où le curseur se déplace ensuite tant que le bouton reste
  enfoncé (« si le bouton a été enfoncé sur un monstre, glisser le
  curseur sur le sol ne déclenche ni déplacement ni rupture du verrou »).
  Si le point cliqué est à l'intérieur d'un monstre du vecteur
  (`std::find_if`, étape 18 — plus de vérification `estVivant()` ici,
  voir plus bas) : `escouade.verrouiller(it->getId())` et
  `deplacementContinu = false`. Sinon (clic sur le sol) :
  `escouade.deverrouiller()` et `deplacementContinu = true`. Dans les
  deux cas, la condition `escouade.estVivante()` (étape 26) garde le
  clic : une escouade vaincue ne peut plus désigner de cible ni
  commencer un déplacement.
- **Retrait du verrou : seulement la mort de la cible, plus le bouton
  ni le focus (étape 27)** : après la boucle d'événements, le verrou ne
  retombe plus que si le monstre visé n'est plus trouvé dans le vecteur
  (`std::find_if` par `id`, mort donc retiré) ou vient de mourir.
  Relâcher le bouton ou perdre le focus ne rompt plus le combat — avant
  l'étape 27, ces deux conditions retiraient aussi le verrou, ce qui
  obligeait à tenir le bouton enfoncé pendant toute l'attaque. Un clic
  sur le sol ou sur un autre monstre rompt le combat **via** l'appui
  décrit au point précédent (`deverrouiller()` ou `verrouiller()` sur un
  autre identifiant), pas via cette condition de retrait. La position du
  curseur ne compte plus une fois le verrou posé — aucune zone de
  tolérance autour du monstre.
- **Attaque déplacée dans `Escouade` (étape 17)** : `main.cpp` ne gère
  plus de recharge lui-même. Tant que `escouade.getCibleVerrouillee()`
  a une valeur et que le monstre correspondant est trouvé dans le
  vecteur, `it->recevoirDegats(escouade.attaquer(it->getCentre(),
  carte))` est appelée à chaque image, que le bouton de souris soit
  enfoncé ou non (étape 27) ; `attaquer` décide si l'escouade doit
  encore s'approcher ou si elle peut s'arrêter et frapper (portée,
  dégâts et recharge par personnage — voir `docs/formation.md`), et
  renvoie la somme des dégâts à appliquer au monstre. Les variables
  locales `rechargeAttaque`, `tempsRecharge` et `degatsParClic` de
  l'étape 16 ont disparu de `main.cpp`.
- **Retrait des monstres morts** (étape 18) : `std::erase_if(monstres,
  ...)` enlève du vecteur tout monstre dont `estVivant()` est faux.
  Exécuté après le retrait du verrou et avant l'ordre de déplacement.
  À partir de là, l'invariant « tout monstre du vecteur est vivant »
  tient jusqu'au prochain appel d'`erase_if` — ce qui permet à la
  boucle de clic et à la boucle de dessin de ne plus tester
  `estVivant()` elles-mêmes.
- **Ordre précis après la boucle d'événements** (étape 27) : 1) attaquer
  si la cible est verrouillée (et retomber si le monstre n'est plus
  trouvé ou vient de mourir) ; 2) retirer les monstres morts ; 3) donner
  l'ordre de déplacement si le bouton est enfoncé, que
  `deplacementContinu` est vrai, et que l'escouade est vivante. L'étape
  de retrait du verrou liée au focus/bouton de l'étape 16 a disparu —
  elle n'a plus lieu d'être, puisque ni l'un ni l'autre ne rompt plus
  le combat.
- **Défaite de l'escouade : `estVivante()` consultée, jamais mise en
  cache** (étape 26) : `main.cpp` ne garde pas de variable locale
  séparée pour « l'escouade est vaincue » — il appelle
  `escouade.estVivante()` à chaque fois que la décision en dépend
  (clic, ordre de déplacement), comme il le fait déjà pour
  `window.hasFocus()`. Seuls les ordres du joueur sont bloqués ; la
  boucle continue de tourner normalement (mise à jour, caméra, dessin,
  fermeture de la fenêtre) — les monstres, par exemple, rentrent à leur
  poste comme n'importe quelle autre fois où ils perdent leur proie,
  sans code spécifique à la défaite dans `Monstre` (voir
  `docs/formation.md`).
- **Le verrou vit dans `Escouade` depuis l'étape 27** (voir
  `docs/formation.md`) ; `deplacementContinu` reste une simple variable
  locale à `main.cpp`, comme l'était le verrou avant cette étape — pas
  de nouvelle classe (`Combat`, `Minuteur`) pour les porter.
- **Texte de position retiré** (ainsi que la police `arial.ttf` qui ne
  servait qu'à lui) : plus rien n'affiche la position du curseur à
  l'écran.
- **Indicateur visuel de la cible verrouillée (étape 19)** : dans la
  boucle de dessin, chaque monstre reçoit
  `escouade.getCibleVerrouillee() == m.getId()` comme second argument de
  `dessiner` — une comparaison directe d'un `std::optional<int>` avec
  un `int`, sans tester `has_value()` au préalable (gère déjà le cas
  vide). Le verrou d'`Escouade` reste l'unique source de vérité sur le
  monstre visé ; `Monstre` ne stocke rien de cette information lui-même
  (voir `ARCHITECTURE.md`).
- **Détection/poursuite et riposte des monstres (étapes 20, 21, 24)** :
  après `escouade.mettreAJour(dt, carte)` et avant la caméra, une
  boucle sur `monstres` fait, pour chacun :
  1. `const auto cible = escouade.personnageLePlusProche(m.getCentre())`
     — le personnage **vivant** le plus proche du monstre, calculé sur
     sa position *avant* qu'il ne bouge ce tic-ci (vide si aucun
     personnage n'est vivant) ;
  2. `m.mettreAJour(dt, cible, carte)` — le monstre décide lui-même de
     poursuivre, de s'arrêter à portée ou d'abandonner (voir
     `docs/formation.md`) ;
  3. `const sf::Vector2f origine = m.getCentre()` — son centre **relu
     après** le déplacement, pour que la riposte parte de sa position à
     jour plutôt que de celle du tic précédent ;
  4. si `cible` a une valeur, `escouade.recevoirDegats(origine,
     m.frapper(*cible))`, sinon rien (riposte conditionnelle depuis
     l'étape 21, pour ne pas appeler `frapper` sur une cible absente).
  Indépendante du verrou de cible (`idCible`) et de la boucle d'attaque
  de l'escouade qui la précède dans l'image : un monstre poursuit et
  riposte dès qu'un personnage vivant est à sa portée, qu'il soit
  lui-même attaqué ou non. Voir `docs/formation.md` pour le détail côté
  `Escouade`/`Monstre`.

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
