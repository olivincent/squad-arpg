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
- Étape 39 — Personnages autonomes dans leur chemin ✅
- Étape 40 — Les personnages vont vers leurs agresseurs ✅
- Étape 41 — Skill de provocation (touche 2) ✅
- Étape 42 — Rayon de zone des skills, skill de zone ciblé au sol (touche 4) ✅

- Étape 43 — Portée de lancement du skill de zone, provocation toujours consommée ✅
- Étape 44 — Ligne de vue pour le skill de zone ✅
- Étape 45 — Fusion d'`Attaque` dans `Skill` (refactorisation pure) ✅
- Étape 46 — Ligne de vue pour les attaques de base et les autres skills ✅
- Étape 47 — Un monstre touché s'active ✅
- Étape 48 — Barre de skills à l'écran (HUD) ✅
- Étape 49 — Hygiène et sûreté du code (refactorisation pure) ✅
- Étape 50 — Réorganisation des tests (refactorisation pure) ✅
- Étape 51 — Alléger Escouade : Provocations, Agresseurs, découpage (refactorisation pure) ✅
- Étape 52 — Alléger main.cpp : Niveau, Partie et Controleur (refactorisation pure) ✅
- Étape 53 — Type PointsDeVie et Skill::declencherSansCible (refactorisation pure) ✅

## Étape 54 — défaite et recommencer la partie

À préciser dans Claude.ai (écran ou texte de défaite, recommencer la partie).

Autres pistes discutées (issues de `docs/audit.md` et des étapes précédentes) :
- Le monstre touché se jette sur l'auteur du coup (mémoire de
  l'attaquant), aggro sociale des voisins, effet visuel d'alerte
- Effet visuel à l'impact du skill de zone
- Indicateur visuel du monstre provoqué
- L'escouade poursuit d'elle-même un monstre qui l'attaque (écartée pour
  l'instant)
- Indicateur visuel de la cible de chaque personnage
- Choisir sans verrou lequel de deux monstres à portée reçoit le skill
- Améliorer le A* pour qu'il tienne compte des entités (contournement
  d'un obstacle mobile)
- Blocage de l'ancre par les monstres, si le décalage ancre/personnages
  s'avère gênant
- Skills qui bousculent un adversaire ou permettent de se faufiler
- Autres effets de skill (étourdissement, ralentissement, dégâts de zone
  dans le temps ou aux alliés), soin de zone, résurrection
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
- Effet visuel au coup ou à la mort d'un monstre ou d'un personnage
- Un lissage plus serré autour des coins (*funnel algorithm* ou Theta*),
  si le contournement actuel des piliers s'avère insuffisant
- Finitions : caméra bloquée aux bords de la carte
- Limitation de fréquence des appels à `allerVers`/A*, si un problème de
  performance est mesuré sur une carte plus grande
- Ligne de vue pour la détection des monstres et leur riposte
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
  dans `Niveau.cpp` (`creerNiveau()`). La caméra (`sf::View`) est centrée
  sur l'escouade ; du noir apparaît au-delà des bords de la carte.
- **Architecture de la boucle** (étape 52) : `main.cpp` (88 lignes) garde la
  fenêtre, la police, la caméra et traduit les événements SFML en appels au
  `Controleur` (mode visée, déplacement continu, aperçu de visée) ; la
  `Partie` (carte, escouade, monstres) porte les règles de jeu et
  l'intermédiaire entre `Escouade` et `Monstre`, avec l'ordre d'une image
  conservé ; `Niveau` porte les données. Voir `docs/boucle-et-entrees.md`.
- **Barre de skills** : en bas de la fenêtre, fixe à l'écran (vue par
  défaut, dessinée en dernier), 4 icônes carrées de 56 px dans l'ordre des
  touches 1 à 4, avec la lettre du skill (D, P, S, Z) et le numéro de
  touche. Pendant la recharge, un quartier noir semi-transparent suit les
  bords du carré (zone claire grandissante depuis midi, sens horaire) et
  le temps restant en secondes entières s'affiche au centre. Icône grisée,
  sans quartier ni chiffre, quand le lanceur est mort ; contour jaune sur
  l'icône du skill en cours de visée. Police DejaVu Sans dans
  `assets/fonts/`, copiée à côté de `main.exe` par CMake ; le jeu refuse
  de démarrer si elle manque.
- **Qualité du code** (étape 49) : `squad_core` et `main` se compilent avec
  `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` sans aucun avertissement
  (pas de `-Werror`, pas sur `tests`) ; `[[nodiscard]]` sur les retours à ne
  pas ignorer ; `Cercle` passé par valeur ; constantes en `s_`. Barre de vie
  des personnages et des monstres dessinée par le module `BarreDeVie`.
  Points de vie portés par la classe `PointsDeVie` (étape 53), possédée par
  `Personnage` et `Monstre` ; provocation et zone par
  `Skill::declencherSansCible`.
- **Escouade** : l'ancre (carré vert de 20 × 20, démarre en `(100, 100)`,
  180 px/s) suit le chemin calculé par le A* puis lissé (trajets directs en
  terrain dégagé, coins contournés au plus court), s'arrête exactement sur
  le point cliqué, est bloquée par les murs et glisse le long d'eux. Son
  déplacement vit dans `Mobile`, que `Monstre` utilise aussi.
- **Personnages** : 4 carrés de 20 × 20 (240 px/s) tiennent une formation
  en carré autour de l'ancre, bloqués par les murs et par les monstres.
  Chacun possède un `Mobile` : en ligne droite quand le trajet vers son
  point est libre, par un chemin A* lissé quand un mur est dans le chemin
  (recalculé seulement si la cible s'éloigne de plus d'une case de la fin
  du chemin) ; un personnage resté derrière un coin ou un pilier le
  contourne donc. La
  formation pivote progressivement vers la direction de marche (360°/s,
  par le plus court côté). Les deux de devant sont des mêlées (rouges,
  portée 20 px), les deux de derrière des tireurs (cyan, portée 120 px) ;
  portées mesurées de bord à bord ; 5 dégâts, recharge 0,5 s ; 100
  points de vie et une barre de vie au-dessus d'eux.
- **Places de formation** : la place théorique de chaque personnage
  (ancre + décalage tourné) est ramenée vers l'ancre si le trajet en ligne
  droite de l'ancre à cette place n'est pas libre (`replierVersOrigine`) :
  près d'un mur, d'un coin ou d'un pilier, la formation se resserre
  d'elle-même.
- **Mort et défaite** : un personnage à 0 point de vie tombe sur place
  (carré gris, sans barre de vie), ne frappe plus, n'est plus ciblé et ne
  bloque plus personne ; il reste dans la formation, sa place est
  simplement vide. Une fois les 4 morts, plus aucun ordre du joueur n'est
  accepté, mais le jeu continue de tourner.
- **Entrées** : un clic sur le sol envoie l'escouade vers ce point ; tant
  que le bouton reste enfoncé (fenêtre ayant le focus, escouade vivante),
  l'ordre est redonné à chaque image vers le curseur. Un clic bref sur un
  monstre engage un combat qui continue seul, sans tenir le bouton ; la
  touches 1 à 3 déclenchent le skill du personnage de la même place ; la
  touche 4 entre en mode visée (voir « Skills »).
- **Ligne de vue** : un personnage ne frappe, ne soigne et ne provoque
  que ce qu'il voit (à portée ET `TileMap::ligneDeVue` centre à centre,
  règle unique `Escouade::peutAtteindre`) : autodéfense, combat verrouillé
  (l'ancre continue d'avancer tant que tous les vivants ne voient pas la
  cible), agresseurs (qui ne voit pas son agresseur va vers lui), skills
  de dégâts, soin et zone. Un skill de dégâts ou de soin à portée mais
  sans vue est mis en attente et le lanceur marche ; sans verrou, un
  monstre caché n'est pas candidat. La provocation ne touche que les
  monstres en vue (recharge consommée dans tous les cas).
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
- **Agresseurs** : `Escouade` retient les monstres qui viennent de la
  frapper (3 s sans nouveau coup, ou mort du monstre). Quand l'ancre est
  à l'arrêt, un personnage vivant sans monstre à portée quitte sa place
  pour aller vers l'agresseur le plus proche, à condition que celui-ci
  soit à 100 px ou moins de sa place ; avec un monstre déjà à portée, il
  reste sur place. L'ordre du joueur (ancre en mouvement), le skill en
  attente et le verrou (personnage à portée du monstre verrouillé) passent
  avant.
- **Skills** : trois, de classe `Skill` (type, portée, puissance, recharge, rayon de zone). Touche 1 :
  skill de dégâts du mêlée avant-gauche (25 dégâts, recharge 5 s, portée
  de mêlée, 20 px), sur le monstre verrouillé ou, sans verrou, sur le
  monstre à portée que ce mêlée frapperait en autodéfense (alors sans
  marche ni mise en attente). Touche 3 : soin du tireur
  arrière-gauche (30 points de vie, recharge 8 s, portée 40 px), sur le
  vivant le plus blessé (soigneur inclus), permis hors combat. Touche 2 :
  provocation du mêlée avant-droit (zone de 150 px depuis son centre,
  4 s, recharge 10 s) : tous les monstres dans la zone le prennent pour
  proie au lieu du personnage le plus proche, sans dégâts ni attente
  (consommée même s'il n'y a aucun monstre dans la zone) ; elle prend fin à
  l'écoulement de la durée, à la mort du provocateur ou à celle du
  monstre. Touche 4 : skill de zone du tireur arrière-droit (15 dégâts, recharge 8 s,
  zone de 60 px, portée 250 px) en deux temps : 4 fait entrer en
  mode visée (un cercle jaune de 60 px suit le curseur, un cercle blanc
  montre la portée autour du lanceur), puis un clic gauche lance le sort
  au point visé (même sur un monstre : pas de verrou). À portée (bord à
  bord) et en vue (ligne de vue entre le lanceur et le point, sans mur),
  il part tout de suite ; hors portée ou derrière un mur, le lanceur
  quitte sa place, marche vers le point (contournant le mur) et lance le
  sort au point fixé au clic dès qu'il est à portée et en vue (un point
  dans un mur est refusé, sans recharge ; seule sa mort l'annule : ni verrou, ni clic, ni ordre de
  déplacement). Le sort touche tous les monstres dont le bord est dans le
  cercle au moment du départ et qui sont en vue du point d'impact (centre
  à centre, il ne traverse pas les murs), et coûte sa recharge même si la zone est vide. Clic
  droit, Échap ou 4 annulent sans recharge ; le déplacement continu est
  suspendu pendant la visée. Un skill de dégâts ou de soin hors de portée met son lanceur en marche vers la
  cible et part à l'arrivée (un seul skill en attente à la fois ; annulé
  par la mort du lanceur, celle de la cible ou son retour à pleine vie
  pour un soin, la rupture du combat pour des dégâts). Un skill remplace
  l'attaque de base à ce moment (il en relance la recharge). Une barre
  orange sous la barre de vie montre la recharge (pleine = prêt), qui ne
  démarre qu'au départ du skill.
- **Monstres** : six carrés rouges de 30 × 30 (120 px/s, 100 points de
  vie, barre de vie), à `(660, 380)`, `(500, 580)`, `(900, 220)`,
  `(1940, 180)`, `(1500, 540)` et `(1620, 540)`, possédés par
  `Partie` dans un `std::vector<Monstre>`. Un monstre détecte un
  personnage vivant à moins de 200 px de lui et de 400 px de son poste (sa
  position de départ), le poursuit (A* + lissage) et s'arrête à portée
  d'attaque (15 px bord à bord, 10 dégâts, recharge 1 s). Un monstre qui reçoit des dégâts (> 0) entre en poursuite de sa
  proie habituelle (provocateur, sinon le plus proche) même hors de sa
  détection, s'il est lui-même dans son territoire ; un coup reçu hors
  du territoire ne le réveille pas, et ne sert qu'une fois. Il abandonne
  quand lui-même s'éloigne de plus de 400 px de son poste, ou si la proie
  meurt, et rentre alors à son poste ; une proie qui revient dans le
  territoire relance la poursuite. Il frappe le personnage vivant le plus
  proche dès qu'il est à portée, attaqué ou non. Un monstre mort est
  retiré.
- **Blocage mutuel** : les monstres se bloquent entre eux et sont bloqués
  par les personnages vivants ; les personnages sont bloqués par les
  monstres. L'ancre ne bloque personne et n'est pas bloquée.
- **Tests** : logique de jeu dans `squad_core`, testée avec Catch2 3
  (`./build/tests.exe`, 377 tests : 38 sur `TileMap`, 17 sur `Chemin`, 144
  sur `Escouade` en cinq fichiers par thème — 22 déplacement, 17 combat, 61
  skills, 26 agresseurs, 18 vue —, 12 sur `Deplacement`, 34 sur `Monstre`,
  31 sur `Personnage`, 19 sur `Skill`, 14 sur `Mobile`, 4 sur `Cercle`, 13
  sur `Autodefense`, 6 sur `BarreDeSkills`, 4 sur `MonstreVu`, 1 sur
  `BarreDeVie`, 8 sur `PointsDeVie`, 9 sur `Provocations`, 8 sur `Agresseurs`, 8 sur `Partie`, 7 sur `Controleur`). Aides communes dans `tests/aides.hpp` ; noms de tests sans
  accents (`./build/tests.exe "*a portee*"` filtre) ; rotation de la
  formation et `provoquer` à une place invalide couverts.

**Limites connues** (acceptées pour l'instant) :
- La barre de skills ne montre pas le skill en attente et n'est pas
  cliquable ; sa position suit la taille initiale de la fenêtre (pas de
  gestion du redimensionnement). La police se charge par chemin relatif :
  le jeu doit être lancé depuis le dossier de `main.exe`.
- Le skill de zone n'a pas d'effet visuel à
  l'impact et ne touche pas les alliés ; son lanceur, bloqué par un
  monstre ou face à un point inaccessible, peut rester en attente.
- Deux places peuvent se replier presque au même endroit dans un passage
  étroit.
- Le chemin d'un personnage ignore les monstres : bloqué par un monstre,
  il glisse ou attend. Un personnage dont le A* ne trouve aucun chemin
  garde la ligne droite.
- Le A* et le lissage ignorent les entités : une entité bloquée ne
  contourne pas, elle glisse ou attend. Près des piliers, le contournement
  n'est pas toujours le plus court.
- La détection, le territoire et la riposte des monstres ignorent les
  murs ; la ligne de vue des personnages est mesurée centre à centre.
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
  portée (pas de notion de « qui attaque qui »). Le déplacement vers
  un agresseur (voir « Agresseurs ») s'y ajoute, mais seulement dans la
  laisse de 100 px et ancre à l'arrêt.
