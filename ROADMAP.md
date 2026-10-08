# Squad-ARPG — Roadmap

## Étapes terminées

Le détail de chaque étape (sous-étapes, limites connues, hors de portée)
est dans `docs/historique.md`.

- Étape 0 — Squelette de projet ✅
- Étape 1 — Carré qui bouge au clic ✅
- Étape 2 — Déplacement progressif ("tic" par "tic") ✅
- Étape 3 — Salle avec murs ✅ (remplacée en partie par l'étape 4)
- Étape 4 — Carte en grille (`TileMap`) ✅
- Étape 5 — Caméra centrée sur le personnage ✅
- Étape 6 — Classe `Escouade` ✅
- Étape 7 — Tests unitaires (Catch2) ✅
- Étape 8 — Module de recherche de chemin (A*) ✅
- Étape 9 — L'escouade suit le chemin du A* ✅
- Étape 10 — Tests unitaires de l'escouade ✅
- Étape 11 — Lissage du chemin ✅
- Étape 12 — Membres de l'escouade (`Personnage`, formation en carré) ✅
- Étape 13 — Déplacement continu tant que le bouton est enfoncé ✅
- Étape 14 — Rotation progressive de la formation ✅
- Étape 15 — Un monstre avec des points de vie ✅
- Étape 16 — Attaque continue et verrouillage de cible ✅
- Étape 17 — Portée d'attaque : les personnages frappent ✅
- Étape 18 — Plusieurs monstres, cible désignée par identifiant ✅
- Étape 19 — Indicateur visuel de la cible verrouillée ✅
- Étape 20 — Les monstres ripostent ✅
- Étape 21 — Un monstre change de cible quand un personnage meurt ✅
- Étape 22 — Classe `Attaque`, partagée par `Personnage` et `Monstre` ✅
- Étape 23 — Classe `Mobile` : le déplacement par chemin sort d'`Escouade` ✅
- Étape 24 — Les monstres détectent, poursuivent et abandonnent ✅
- Étape 25 — Statut du monstre et retour au poste ✅
- Étape 26 — Mort d'un personnage et défaite de l'escouade ✅
- Étape 27 — Un clic engage le combat ✅
- Étape 28 — Portées d'attaque différentes selon le personnage ✅
- Étape 29 — Placement individuel en combat ✅
- Étape 30 — Portée mesurée de bord à bord ✅
- Étape 31 — Premier skill, déclenché au clavier ✅
- Étape 32 — Les monstres se bloquent entre eux ✅
- Étape 33 — Les monstres bloqués par les personnages vivants ✅
- Étape 34 — Les personnages bloqués par les monstres ✅
- Étape 35 — Réorganisation de la documentation ✅
- Étape 36 — Classe Skill, skill de soin et cible générale ✅
- Étape 37 — Autodéfense : les personnages ripostent sans clic ✅
- Étape 38 — Places de formation toujours valides ✅

## Étape 39 — À définir

Pistes discutées :
- Donner un `Mobile` ou un chemin A* aux personnages, pour qu'un
  personnage resté derrière un coin le contourne
- Un mêlée fait un pas vers le monstre qui l'attaque
- L'escouade poursuit d'elle-même un monstre qui l'attaque (écartée pour
  l'instant)
- Indicateur visuel de la cible de chaque personnage
- Choisir sans verrou lequel de deux monstres à portée reçoit le skill
- Améliorer le A* pour qu'il tienne compte des entités (contournement
  d'un obstacle mobile)
- Blocage de l'ancre par les monstres, si le décalage ancre/personnages
  s'avère gênant
- Skills qui bousculent un adversaire ou permettent de se faufiler
- Skills pour les personnages 2 et 4 (touches 2 et 4), effets de skill
  (zone, état posé sur le monstre), soin de zone, résurrection, HUD ou
  icônes de skill
- Choix manuel de la cible d'un skill par le joueur
- Plusieurs skills en attente à la fois
- Patrouille, escouade de monstres, récupération de points de vie au
  retour d'un monstre
- Retirer un personnage mort de `m_personnages` et resserrer la
  formation, si l'avoir laissé en place s'avère gênant ; écran ou texte
  de défaite ; recommencer la partie
- Collision entre personnages, ou déplacement suivant l'arc de rotation
  plutôt qu'une ligne droite, pour éliminer le chevauchement résiduel
  lors d'un demi-tour rapide
- Factorisation des points de vie et de la barre de vie entre
  `Monstre` et `Personnage`, si la duplication devient gênante
- Effet visuel au coup ou à la mort d'un monstre ou d'un personnage
- Un lissage plus serré autour des coins (*funnel algorithm* ou Theta*),
  si le contournement actuel des piliers s'avère insuffisant
- Finitions : caméra bloquée aux bords de la carte, affichage fixe à
  l'écran, police committée dans le dépôt
- Limitation de fréquence des appels à `allerVers`/A*, si un problème de
  performance est mesuré sur une carte plus grande
- Ligne de vue pour la détection ou la portée
- Types de personnages, statistiques, armes, capacités, au-delà de la
  simple portée différenciée
- Placement « intelligent » en combat (encerclement, écartement, ligne
  de tir), recul des tireurs si un monstre s'approche d'eux
- Forme plus fidèle que le cercle unique (plusieurs cercles, rectangle
  orienté), orientation des entités

## État actuel (à mettre à jour à chaque session)

- **Outils** : conception dans Claude.ai, implémentation avec Claude Code
  dans VSCode (règles dans `CLAUDE.md`) ; dépôt Git `squad-arpg`, poussé
  sur GitHub ; SFML 3.0.2 intégrée à CMake.
- **Fenêtre et carte** : fenêtre 1200 × 800 ; carte de 59 × 40 cases de
  40 px (2360 × 1600 px), plusieurs pièces et couloirs, définie en texte
  dans `main.cpp`. La caméra (`sf::View`) est centrée sur l'escouade ;
  du noir apparaît au-delà des bords de la carte.
- **Escouade** : l'ancre (carré vert de 20 × 20, démarre en `(100, 100)`,
  180 px/s) suit le chemin calculé par le A* puis lissé (trajets directs en
  terrain dégagé, coins contournés au plus court), s'arrête exactement sur
  le point cliqué, est bloquée par les murs et glisse le long d'eux. Son
  déplacement vit dans `Mobile`, que `Monstre` utilise aussi.
- **Personnages** : 4 carrés de 20 × 20 (240 px/s) tiennent une formation
  en carré autour de l'ancre, bloqués par les murs et par les monstres. La
  formation pivote progressivement vers la direction de marche (360°/s,
  par le plus court côté). Les deux de devant sont des mêlées (rouges,
  portée 20 px), les deux de derrière des tireurs (cyan, portée 120 px) ;
  portées mesurées de bord à bord ; 5 dégâts, recharge 0,5 s ; 100
  points de vie et une barre de vie au-dessus d'eux.
- **Places de formation** : la place théorique de chaque personnage
  (ancre + décalage tourné) est ramenée vers l'ancre si le trajet en ligne
  droite de l'ancre à cette place n'est pas libre (`replierVersOrigine`) :
  près d'un mur, d'un coin ou d'un pilier, la formation se resserre
  d'elle-même. Un petit point jaune (repère provisoire, jusqu'à la fin de
  l'étape 39) marque la place de chaque personnage vivant.
- **Mort et défaite** : un personnage à 0 point de vie tombe sur place
  (carré gris, sans barre de vie), ne frappe plus, n'est plus ciblé et ne
  bloque plus personne ; il reste dans la formation, sa place est
  simplement vide. Une fois les 4 morts, plus aucun ordre du joueur n'est
  accepté, mais le jeu continue de tourner.
- **Entrées** : un clic sur le sol envoie l'escouade vers ce point ; tant
  que le bouton reste enfoncé (fenêtre ayant le focus, escouade vivante),
  l'ordre est redonné à chaque image vers le curseur. Un clic bref sur un
  monstre engage un combat qui continue seul, sans tenir le bouton ; la
  touches 1 à 4 déclenchent le skill du personnage de la même place.
- **Combat** : le monstre verrouillé est entouré d'un contour jaune.
  L'escouade s'approche en formation jusqu'à ce que tous ses personnages
  vivants soient à portée, puis s'arrête et chacun frappe selon sa propre
  recharge. En combat, un personnage déjà à portée reste sur place : les
  tireurs s'arrêtent à distance pendant que les mêlées vont jusqu'au
  contact. Le combat ne se rompt que par la mort de la cible, un clic sur
  le sol ou un clic sur un autre monstre.
- **Autodéfense** : sans clic, chaque personnage vivant frappe de
  lui-même un monstre qui se trouve à sa portée, à chaque image (à
  l'arrêt, en marche, en combat). Il choisit le monstre verrouillé s'il
  est à sa portée, sinon le plus proche à sa portée (bord à bord ;
  égalité : le premier de la liste). Deux personnages peuvent frapper
  deux monstres différents. Elle ne déplace personne : les mêlées ne
  frappent que ce qui est à 20 px d'eux, les tireurs à 120 px.
- **Skills** : deux, de classe `Skill` (type + `Attaque`). Touche 1 :
  skill de dégâts du mêlée avant-gauche (25 dégâts, recharge 5 s, portée
  de mêlée, 20 px), sur le monstre verrouillé ou, sans verrou, sur le
  monstre à portée que ce mêlée frapperait en autodéfense (alors sans
  marche ni mise en attente). Touche 3 : soin du tireur
  arrière-gauche (30 points de vie, recharge 8 s, portée 40 px), sur le
  vivant le plus blessé (soigneur inclus), permis hors combat. Touches 2
  et 4 : rien. Un skill hors de portée met son lanceur en marche vers la
  cible et part à l'arrivée (un seul skill en attente à la fois ; annulé
  par la mort du lanceur, celle de la cible ou son retour à pleine vie
  pour un soin, la rupture du combat pour des dégâts). Un skill remplace
  l'attaque de base à ce moment (il en relance la recharge). Une barre
  orange sous la barre de vie montre la recharge (pleine = prêt), qui ne
  démarre qu'au départ du skill.
- **Monstres** : six carrés rouges de 30 × 30 (120 px/s, 100 points de
  vie, barre de vie), à `(660, 380)`, `(500, 580)`, `(900, 220)`,
  `(1940, 180)`, `(1500, 540)` et `(1620, 540)`, possédés par
  `main.cpp` dans un `std::vector<Monstre>`. Un monstre détecte un
  personnage vivant à moins de 200 px de lui et de 400 px de son poste (sa
  position de départ), le poursuit (A* + lissage) et s'arrête à portée
  d'attaque (15 px bord à bord, 10 dégâts, recharge 1 s). Il abandonne
  quand lui-même s'éloigne de plus de 400 px de son poste, ou si la proie
  meurt, et rentre alors à son poste ; une proie qui revient dans le
  territoire relance la poursuite. Il frappe le personnage vivant le plus
  proche dès qu'il est à portée, attaqué ou non. Un monstre mort est
  retiré.
- **Blocage mutuel** : les monstres se bloquent entre eux et sont bloqués
  par les personnages vivants ; les personnages sont bloqués par les
  monstres. L'ancre ne bloque personne et n'est pas bloquée.
- **Tests** : logique de jeu dans `squad_core`, testée avec Catch2 3
  (`./build/tests.exe`, 219 tests : 32 sur `TileMap`, 17 sur `Chemin`, 69
  sur `Escouade`, 12 sur `Deplacement`, 27 sur `Monstre`, 23 sur
  `Personnage`, 11 sur `Attaque`, 5 sur `Skill`, 12 sur `Mobile`, 4 sur
  `Cercle`, 7 sur `Autodefense`).

**Limites connues** (acceptées pour l'instant) :
- Un personnage resté derrière un coin peut s'y accrocher : les places
  sont valides, mais il se déplace en ligne droite vers elles (étape 39).
  Deux places peuvent se replier presque au même endroit dans un passage
  étroit.
- Le A* et le lissage ignorent les entités : une entité bloquée ne
  contourne pas, elle glisse ou attend. Près des piliers, le contournement
  n'est pas toujours le plus court.
- Portée d'attaque, détection et territoire ignorent les murs.
- Lors d'un demi-tour rapide, deux personnages peuvent se chevaucher
  brièvement ; l'ancre traversant les monstres, elle (et la caméra) peut
  s'éloigner des personnages bloqués.
- Un personnage mort n'est jamais retiré et la formation ne se resserre
  pas ; pas d'écran de défaite.
- Un monstre qui rentre ignore une proie hors de son territoire ; pas de
  patrouille ni de récupération de points de vie ; pas de coordination
  entre monstres.
- En combat, un personnage s'arrête où il se trouve en entrant à portée,
  sans placement « intelligent ».
- Les entités sont dessinées en carrés mais mesurées en cercles : en
  diagonale, l'écart visible est plus petit que la portée (d'une dizaine
  de pixels).
- L'autodéfense ne déplace personne : un mêlée attaqué par un monstre
  qui s'arrête à 15 px de lui riposte sur place, mais un tireur ou un
  mêlée qui n'a rien à portée ne réagit pas ; elle ne regarde que la
  portée (pas de notion de « qui attaque qui »).
