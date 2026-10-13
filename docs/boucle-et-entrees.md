# Squad-ARPG — Boucle de jeu et entrées

`main.cpp` garde la fenêtre, la boucle de jeu, les événements, la caméra,
la carte (son plan) et les monstres. Il ne connaît ni `Chemin` ni
`Personnage`, et il est l'intermédiaire entre `Escouade` et `Monstre`, qui
ne se connaissent pas.

## Boucle de jeu

- **Ordre d'une image** :
  0. sortie de la visée si le lanceur ne peut plus lancer (mort, défaite) ;
  1. événements : fermeture ; appui de souris (pose ou retire le verrou
     de combat, arme le déplacement continu ; en visée, lance ou annule
     le sort) ; appui sur une touche 1 à 3 (demande le skill de cette
     place), 4 ou Échap (entre en visée ou en sort) ;
  2. liste des monstres vus (identifiant et cercle), lancer du skill de
     zone s'il y a un clic en visée, skill demandé s'il y a un appui, puis coups de l'escouade (`attaquer`, à chaque image,
     verrou ou non, tant que l'escouade a un vivant), appliqués monstre
     par monstre ; retrait du verrou si le monstre verrouillé est mort ;
  3. retrait des monstres morts ;
  4. ordre de déplacement continu ;
  5. mise à jour : liste des cercles des monstres, puis
     `escouade.mettreAJour`, puis `escouade.cerclesDesVivants()`, puis
     chaque monstre (cible, obstacles, mise à jour, riposte) ;
  6. caméra, dessin.
- **Déplacement à pas fixe** plutôt que proportionnel à la distance :
  vitesse constante, comme un personnage d'ARPG.
- **Delta time** (`sf::Clock::restart()`) : vitesse exprimée en pixels par
  seconde, pour qu'elle ne dépende pas de la cadence réelle.
  `setFramerateLimit(60)` limite l'usage du CPU, mais la logique n'en
  dépend pas. L'horloge reste dans `main.cpp` ; `dt` est passé en
  paramètre.
- **Aucune classe de plus** (`Combat`, `Minuteur`, `Camera`…) tant qu'il
  n'y a pas de comportement à y encapsuler : les variables locales
  `deplacementContinu`, `placeSkillDemandee`, `placeEnVisee`, `prochainId` et la vue `camera`
  suffisent.
- **Pas de texte de position ni de police chargée** : rien n'affiche la
  position du curseur à l'écran.

## Entrée souris

- **Un seul événement d'appui décide de tout** : l'événement
  `MouseButtonPressed` (bouton gauche) décide, à l'instant du clic, ce
  que fait ce clic — peu importe où le curseur se déplace ensuite tant
  que le bouton reste enfoncé. Si le point cliqué est dans un monstre
  (`std::find_if` sur `contient`) : `escouade.verrouiller(it->getId())`
  et `deplacementContinu = false`. Sinon (clic sur le sol) :
  `escouade.deverrouiller()` et `deplacementContinu = true`. La
  condition `escouade.estVivante()` garde le clic : une escouade vaincue
  ne peut plus désigner de cible ni commencer un déplacement.
- **Déplacement continu : lecture d'état, armée par l'événement
  d'appui** : `sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)` est
  interrogé à chaque image, après la boucle d'événements. Tant que le
  bouton reste enfoncé, que `deplacementContinu` est vrai et que
  l'escouade est vivante, `escouade.allerVers` est rappelée à chaque
  image avec la position courante du curseur — le même A* + lissage
  qu'un clic ponctuel, simplement redonné en continu. Un clic bref reste
  un simple aller-vers ponctuel. `deplacementContinu` est une variable
  locale mise à jour uniquement par l'événement d'appui : cliquer sur un
  monstre puis glisser le curseur sur le sol ne déclenche aucun
  déplacement.
- **`window.hasFocus()`** est une condition nécessaire, parce que
  `isButtonPressed` lit l'état global de la souris au niveau du système,
  même quand la fenêtre n'a pas le focus.
- **Position du curseur** : convertie avec `window.mapPixelToCoords(...)`
  après la boucle d'événements, donc avec la vue de l'image précédente —
  celle effectivement affichée au moment du clic.
- **Pas de limitation de fréquence** : un appel au A* par image, jugé
  négligeable sur la carte actuelle ; à revisiter seulement si un
  problème de performance est mesuré.

## Combat engagé

- **Le verrou vit dans `Escouade`** (voir `docs/formation.md`) ; on y
  stocke l'identifiant du monstre visé. Chaque image, `main.cpp` retrouve
  le monstre par `std::find_if` sur l'identifiant, l'itérateur étant
  recalculé à chaque fois et ne survivant pas au bloc : un monstre peut
  être retiré du vecteur entre deux images.
- **Le verrou ne se rompt que dans trois cas** : mort de la cible
  (monstre introuvable ou qui vient de mourir), clic sur le sol, clic sur
  un autre monstre (cible remplacée). Relâcher le bouton ou perdre le
  focus ne rompt pas le combat : un clic bref suffit à l'engager. La
  position du curseur ne compte plus une fois le verrou posé — aucune
  zone de tolérance autour du monstre.
- **L'attaque est dans `Escouade`** : `main.cpp` n'a aucune recharge ni
  dégât à lui. Il construit chaque image la liste des `MonstreVu`
  (identifiant et cercle), la passe à `escouade.attaquer(monstresVus,
  carte)` et applique chaque `Coup` renvoyé au monstre qui porte
  l'identifiant (lambda `appliquer`, `std::find_if`). `attaquer` est
  appelé à chaque image, bouton enfoncé ou non, **verrou ou non** :
  avec un verrou elle décide si l'escouade s'approche ou s'arrête ;
  dans tous les cas chaque personnage vivant frappe de lui-même un
  monstre à sa portée (autodéfense, voir `docs/combat.md`). Seule la
  mort du monstre verrouillé retire le verrou.
- **Retrait des monstres morts** : `std::erase_if(monstres, ...)` après
  le bloc d'attaque et avant l'ordre de déplacement. À partir de là,
  « tout monstre du vecteur est vivant » jusqu'au prochain appel, ce qui
  dispense la boucle de clic et celle de dessin de tester `estVivant()`.
- **Indicateur de cible** : chaque monstre reçoit
  `escouade.getCibleVerrouillee() == m.getId()` comme second argument de
  `dessiner` (voir `docs/monstre.md`).

## Entrée clavier : le skill

- **Événement d'appui, pas lecture d'état** : un skill est une action
  ponctuelle, un appui = une tentative. L'événement `KeyPressed` met dans
  la variable locale `placeSkillDemandee` (`std::optional<std::size_t>`)
  la place du lanceur, 1 à 4 pour les touches 1 à 4 ; elle est déclarée
  juste avant la boucle d'événements et donc remise à vide à chaque
  image. Si plusieurs touches sont appuyées dans la même image, la
  dernière gagne.
- **`window.setKeyRepeatEnabled(false)`** : par défaut une touche
  maintenue produit des `KeyPressed` répétés. Comme le skill se recharge
  en 5 s, une touche tenue assez longtemps l'aurait relancé ; désactiver
  la répétition garantit un seul appui par enfoncement.
- **Scancode plutôt que code de touche** : `sf::Keyboard::Scan::Num1`
  désigne la touche physique « 1 » de la rangée du haut, quelle que soit
  la disposition du clavier (en AZERTY elle donne « & » sans majuscule).
- **Skill demandé avant `attaquer`** : un skill qui part relance la
  recharge de base du personnage, qui ne frappe donc pas une seconde
  fois dans la même image (voir `docs/combat.md`). `declencherSkill` est
  appelée même sans verrou : le soin marche sans combat, et le skill de
  dégâts vise alors le monstre que son lanceur frapperait en
  autodéfense. `main.cpp` lui passe la liste des monstres vus, et
  `Escouade` retrouve le monstre verrouillé elle-même. Les coups
  renvoyés sont appliqués comme ceux d'`attaquer`.

## Mode visée : le skill de zone (étape 42)

- **État d'interface, pas état de jeu** : `placeEnVisee`
  (`std::optional<std::size_t>`, vide = mode normal, sinon la place du
  lanceur) vit dans `main.cpp`. `Escouade` ne connaît ni la souris ni le
  mode ; elle expose `skillEstPret`, `rayonZoneDuSkill` et
  `declencherSkillDeZone`.
- **Entrée** : appui sur 4 avec le skill prêt ; sinon ignoré. **Sorties
  sans lancer** : clic droit, Échap, nouvel appui sur 4, mort du lanceur
  ou défaite (testée en début d'image par `!skillEstPret`). Aucune
  recharge consommée.
- **Lancer** : uniquement sur l'événement `MouseButtonPressed` gauche
  reçu pendant la visée, jamais sur l'état du bouton. Le point est
  converti en coordonnées du monde. Le clic lance le sort même sur un
  monstre : il ne verrouille pas, ne déverrouille pas, ne déplace pas
  l'escouade (la branche normale n'est pas exécutée en visée). Après le
  lancer, réussi ou refusé, on sort de la visée. Les coups sont appliqués
  par `appliquer`, avant le skill demandé au clavier et avant `attaquer`.
- **Bouton déjà enfoncé** : entrer en visée met `deplacementContinu` à
  `false` ; il ne repasse à `true` que sur un nouvel appui au sol. Donc ni
  pendant la visée ni après le lancer l'escouade ne suit le curseur
  tant que le bouton n'a pas été relâché puis réappuyé. Le drapeau
  existant joue le rôle d'un « attendre le relâchement » : pas de
  booléen de plus.
- **Visuel** : un `sf::CircleShape` (contour jaune, sans remplissage, au
  rayon exact de la zone, origine au centre) est créé dans la boucle de
  dessin sous le curseur, en coordonnées du monde. Rien à l'impact.
- **Touches 1 à 3** restent actives pendant la visée (non spécifié).

## Défaite de l'escouade

- **`estVivante()` est consultée, jamais mise en cache** : `main.cpp` ne
  garde pas de variable locale « l'escouade est vaincue », il l'appelle
  quand une décision en dépend (clic, ordre de déplacement), comme pour
  `window.hasFocus()`.
- **Seuls les ordres du joueur sont bloqués** : la boucle continue de
  tourner normalement (mise à jour, caméra, dessin, fermeture de la
  fenêtre). Les monstres, par exemple, rentrent à leur poste sans code
  propre à la défaite (voir `docs/formation.md`).

## Les monstres dans la boucle

- **Obstacles pour les personnages** : avant `escouade.mettreAJour`, la
  liste des cercles de tous les monstres (à leur position de l'image
  précédente) est construite et passée en paramètre.
- **Obstacles pour les monstres** : après `escouade.mettreAJour`, les
  cercles des personnages vivants sont calculés une fois
  (`cerclesDesVivants`) ; chaque monstre en part et y ajoute les cercles
  des autres monstres (identifiant différent). Voir « Qui bloque qui »
  dans `docs/combat.md`.
- **Pour chaque monstre, dans l'ordre** :
  1. `const auto cible = escouade.proieDuMonstre(m.getId(), m.getCentre())`
     — le cercle du provocateur si ce monstre est provoqué (voir
     « Provocation » dans `docs/combat.md`), sinon du personnage
     **vivant** le plus proche, calculé sur la
     position du monstre *avant* qu'il ne bouge (vide si aucun
     personnage n'est vivant) ;
  2. `m.mettreAJour(dt, cible, carte, obstacles)` — le monstre décide
     seul de poursuivre, de s'arrêter à portée ou de rentrer (voir
     `docs/monstre.md`) ;
  3. `const sf::Vector2f origine = m.getCentre()` — son centre **relu
     après** le déplacement, pour que la riposte parte de la position à
     jour ;
  4. si `cible` a une valeur, `escouade.recevoirDegats(origine,
     m.frapper(*cible))`, sinon rien.
- **Indépendant du verrou de cible** : un monstre poursuit et riposte dès
  qu'un personnage vivant est à sa portée, qu'il soit lui-même attaqué ou
  non.

## Caméra

- **`sf::View` dans `main.cpp`** : une simple variable `camera`, copiée de
  `window.getDefaultView()` (taille de la fenêtre). Pas de classe `Camera`
  tant qu'il n'y a pas de comportement à y encapsuler.
- **Toujours centrée sur l'escouade** : à chaque image, après la mise à
  jour et avant le dessin, `camera.setCenter(escouade.getCentre())` puis
  `window.setView(camera)`.
- **Conséquence acceptée** : près des bords de la carte, la caméra montre
  du noir au-delà des murs. Bloquer la caméra aux bords reste une option.
  L'ancre traversant les monstres, la caméra (centrée sur elle) peut
  s'éloigner des personnages bloqués.
