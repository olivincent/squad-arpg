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
- Étape 54 — Défaite et recommencer la partie ✅
- Étape 55 — Armure à rendement décroissant ✅
- Étape 56 — Générateur aléatoire injecté et recharge aléatoire ✅
- Étape 57 — Temps d'exécution des attaques : on s'immobilise pour frapper ✅
- Étape 58 — Temps d'activation des skills (touches 1 à 4) et action engagée par personnage ✅

- Étape 59 — Génération procédurale : les salles ✅
- Étape 60 — Génération procédurale : les couloirs ✅
- Étape 61 — Génération procédurale : monstres, touche R et F5 ✅
- Étape 62 — Ligne de vue des monstres et nettoyage de Skill ✅
- Étape 63 — Objectif et victoire ✅
- Étape 64 — Groupes de monstres : alerte du groupe ✅
- Étape 65 — Menu de départ, options d'affichage et sauvegarde des réglages ✅
- Étape 66 — Proie et cible de frappe des monstres ✅
- Étape 67 — Places d'attaque autour de la cible et séparation des personnages ✅
- Étape 68 — A* qui contourne les obstacles mobiles ✅
- Étape 69 — Retour au poste : invulnérabilité, régénération et territoire élargi ✅
- Étape 70 — Portes et premier objet interactif ✅
- Étape 71 — Correction des portes : ouvrir sur place et faire face à la porte ✅
- Étape 72 — Vision des personnages et brouillard de guerre ✅
- Étape 73 — La ligne de vue passe par le shadowcasting symétrique ✅ (avec le garde-fou `contientUnCoinDiagonal`)
- Étape 74 — Escouade de 3 personnages, skills désignés par identifiant ✅
- Étape 75 — Barre de raccourcis : 12 emplacements, touches et clic ✅
- Étape 76 — Spécialités, catalogue et choix des 3 classes ✅
- Étape 77 — Zones de contrôle et attaque d'opportunité ✅
- Étape 78 — Dégagement des tireurs ✅
- Étape 79 — Dégâts flottants : une file d'événements de combat ✅
- Étape 80 — Filet de sécurité et corrections locales (suite de l'audit) ✅

## Plan après l'audit (« hybride », décidé à l'étape 80)

L'audit de `docs/audit.md` est traité en trois temps : deux étapes de refactorisation, puis chaque
refactorisation restante comme **première sous-étape de la fonctionnalité qui en a besoin**. Les
fonctionnalités prévues sont donc décalées après l'étape 82.
- **Étape 80** (faite) : build vérifiable (presets, CTest, avertissements partout, clang-format et
  clang-tidy), tests indépendants du catalogue, corrections locales (C1, C4, D6, K2, V1 partie 1, L1).
- **Étape 81** : dossiers (option B de l'audit, par domaine), P1 (`Reglages` dépend du modèle de la
  barre, `Controleur` dans la mauvaise couche), K1 (documentation chargée à chaque session), K3
  (`docs/historique.md` arrêté à l'étape 73).
- **Étape 82** : alléger `Escouade` : R1 (six responsabilités) avec R2 (couplage temporel entre
  `attaquer` et `mettreAJour`), D1 (vitalité répétée dans `Personnage` et `Monstre`), D3 (fonctions
  publiques appelées seulement par les tests). R2 doit **supprimer `m_cibleCombat` en tant que
  membre** : il n'est jamais remis à zéro et peut dater d'un ancien combat (à l'étape 80, la marche vers
  la cible d'un skill de dégâts en attente a dû exiger un verrou pour ne pas le lire à tort). Décision
  sur R5 (identifiants et places) à la fin de l'étape.
- **Refactorisations rattachées à une fonctionnalité**, en première sous-étape de celle-ci :
  - R4 (skills : le type décide partout) et C2 (constructeurs à longues listes) avant les buffs, les
    soins dans le temps ou le mana. Le mode de cible « Soi » de R4 doit lever une limite de l'étape 80 :
    la provocation, sans cible, ne peut pas être mise en attente quand son lanceur est occupé ;
  - E1 (valeurs dispersées, `Monstre` en littéraux) avant les types de monstres ;
  - R3 (`Partie` accumule plusieurs rôles), D2 (indices de grille à la main), C3 (conversions de
    signe) et D5 (petites duplications de `Partie`) avant les coffres, les leviers ou la carte
    extérieure ;
  - M1 (`dessiner` dans le noyau), R6 (`EcranJeu`), D4 (écrans de menu), P2 (en-tête parapluie des
    écrans) et T4 (trous de couverture) au premier ajout visuel important.

**Décisions reportées** :
- V1 partie 2 (recalcul moins fréquent du chemin des monstres) : à réévaluer avec la carte extérieure
  ou des hordes (la partie 1 a déjà divisé le coût par 7 à 13) ;
- R5 (identifiants et places) : à la fin de l'étape 82.

## Prochaine étape (à choisir, après l'étape 82)

Étape 83 envisagée, par la même file d'événements que l'étape 79 : le nom du skill lancé au-dessus du
lanceur et « Provoqué » au-dessus des monstres provoqués (un type d'événement de plus, un style de plus
dans `TextesFlottants` ; voir `docs/brainstorm.md`, section 5 ter).

Des gambits pour choisir le comportement d'un personnage engagé (rester et tirer, se dégager, passer
en mêlée : le dégagement de l'étape 78 en est la première règle, écrite à un seul endroit, `degagementVoulu`)
et des skills qui permettent de se faufiler sans pénalité (voir `docs/brainstorm.md`).

Pistes issues de l'étape 76 : arbres de skills par classe (on ne peut pas tout prendre, pour
monter une même classe de façons différentes ; c'est ce qui différenciera deux personnages de la
même classe), niveaux, description des classes et infobulles à l'écran de choix, mémoire de la
dernière composition entre deux lancements (dans `reglages.ini`). Autres pistes issues de l'étape 75 (voir
`docs/brainstorm.md`, section 5 bis) : écran « Configurer les touches » dans les options (préréglage
classique 1 à 0, - et =, barre en ligne droite ; sauvegarde des touches dans `reglages.ini`), barre
d'auras seulement cliquable (deuxième instance de `BarreDeRaccourcis`, valeur de plus dans `Barre`), skills
à bascule, interface pour placer ses skills (glisser-déposer, skills appris en montant de niveau),
survol et infobulles sur la barre. Sinon, à préciser avec Claude.ai (voir `docs/brainstorm.md`, dont l'idée de « stratégie
d'attaque » par type de monstre, qui se branchera sur `coutDesObstacles` : la liste
de cercles passée changera selon le type d'entité).

Autres pistes discutées (issues de `docs/audit.md` et des étapes précédentes) :
- Passe de nettoyage des coins diagonaux dans `genererNiveau` (creuser en sol le mur du haut ou
  de gauche de chaque damier de 2 × 2 cases, après les couloirs et avant `placerPortes`, puis
  revérifier la connexité) : non écrite parce que le test sur les graines 1 à 50 passe, mais
  145 graines sur 20 000 (0,7 %) contiennent le motif ; à décider (voir `docs/vision.md`)
- Skill en attente par personnage (aujourd'hui un seul pour l'escouade)
- Interruption de l'activation d'un skill par les dégâts reçus
- Laisse prolongée par les dégâts (un monstre touché qui dépasse son
  territoire ne rentre pas aussitôt) : à étudier avec les autres leviers contre
  le kiting, voir `docs/monstre.md`
- Vitesse relative escouade/monstres comme levier contre le kiting
- Ralentissement de l'escouade engagée
- Classe `Arme` qui fournit l'intervalle de recharge de l'attaque de base
  (aujourd'hui écrit dans `Escouade.cpp` et `Monstre.cpp`)
- Le monstre touché se jette sur l'auteur du coup (mémoire de
  l'attaquant), effet visuel d'alerte, alerte par proximité ou par le bruit
  d'un groupe à l'autre, chef de groupe, retour au poste en groupe
- Effet visuel à l'impact du skill de zone
- Indicateur visuel du monstre provoqué
- L'escouade poursuit d'elle-même un monstre qui l'attaque (écartée pour
  l'instant)
- Indicateur visuel de la cible de chaque personnage
- Choisir sans verrou lequel de deux monstres à portée reçoit le skill
- Blocage de l'ancre par les monstres, si le décalage ancre/personnages
  s'avère gênant
- Skills qui bousculent un adversaire ou permettent de se faufiler
- Autres effets de skill (étourdissement, ralentissement, dégâts de zone
  dans le temps ou aux alliés), soin de zone, résurrection
- Choix manuel de la cible d'un skill par le joueur
- Plusieurs skills en attente à la fois
- Patrouille, escouade de monstres, retour au poste en groupe
- Retirer un personnage mort de `m_personnages` et resserrer la
  formation, si l'avoir laissé en place s'avère gênant
- Sortie ou escalier comme objectif, étages
- Déplacement suivant l'arc de rotation plutôt qu'une ligne droite
  (la séparation douce de l'étape 67 écarte déjà les chevauchements)
- Effet visuel au coup ou à la mort d'un monstre ou d'un personnage
- Un lissage plus serré autour des coins (*funnel algorithm* ou Theta*),
  si le contournement actuel des piliers s'avère insuffisant
- Finitions : caméra bloquée aux bords de la carte
- Limitation de fréquence des appels à `allerVers`/A*, si un problème de
  performance est mesuré sur une carte plus grande
- Types de personnages, statistiques, armes, capacités, au-delà de la
  simple portée différenciée
- Ligne de tir dégagée, recul des tireurs si un monstre s'approche d'eux ;
  places d'attaque pour les monstres qui encerclent un personnage
- Forme plus fidèle que le cercle unique (plusieurs cercles, rectangle
  orienté), orientation des entités

## État actuel (à mettre à jour à chaque session)

- **Outils** : conception dans Claude.ai, implémentation avec Claude Code
  dans VSCode (règles dans `CLAUDE.md`) ; dépôt Git `squad-arpg`, poussé
  sur GitHub ; SFML 3.0.2 intégrée à CMake.
- **Menu et réglages** (étape 65) : au lancement, un menu à la souris
  (« Commencer partie », « Options », « Quitter ») ; « Commencer partie »
  ouvre le choix de l'escouade (voir « Choix de l'escouade »), puis lance une partie (nouvelle carte,
  graine dans la console). « Options » :
  mode fenêtré ou plein écran (à la résolution du bureau) et, en fenêtré,
  résolution parmi 1200 × 800, 1280 × 720, 1600 × 900 et 1920 × 1080 (celles
  qui tiennent dans le bureau) ; chaque changement s'applique tout de suite
  (la fenêtre est recréée) et est écrit dans `reglages.ini` à côté de
  `main.exe` (ignoré par Git ; absent ou invalide = 1200 × 800 fenêtré). Le
  monde visible fait toujours 800 px de haut (la largeur suit le format de la
  fenêtre) ; la barre de skills, le compteur, l'écran de fin et les menus
  suivent la même règle. Fenêtre sans redimensionnement à la main. Voir
  `docs/ecrans.md`.
- **Choix de l'escouade** (étape 76) : « Commencer partie » ouvre un écran à la souris. À gauche, un
  bouton par classe du catalogue (guerrier, prêtre, mage, paladin, rôdeur) ; à droite, trois cases
  « 1. Guerrier » / « 1. — » et les boutons « Commencer » (grisé tant que la sélection n'est pas
  complète) et « Retour » (menu). Cliquer une classe remplit la case suivante (la même classe peut
  être prise 1 à 3 fois ; une 4ᵉ ne fait rien), cliquer une case remplie la vide et les suivantes
  remontent. L'ordre de sélection donne les places 1 à 3, donc les rangées de la barre de raccourcis
  et la formation (selon le rang de chaque classe). La 2ᵉ et la 3ᵉ occurrence d'une même classe ont
  la couleur de la classe assombrie de 30 % et de 55 % ; la case montre déjà cette couleur. La
  sélection reste dans l'écran de choix : R après une défaite ou une victoire y revient avec la même
  sélection ; F5 en jeu garde la composition. **Classes** (`Specialite`, données, pas de
  hiérarchie) : *paladin* (devant, orange, armure 50, attaque de mêlée ; `FrappeSacree` : 20 dégâts,
  20 px, recharge 5 s, activation 0,3 s ; `ImpositionDesMains` : soin de 25, 20 px, recharge 10 s,
  activation 0,5 s) et *rôdeur* (derrière, vert, armure 0, attaque de tireur ; `TirPercant` : 25
  dégâts, 200 px, recharge 6 s, activation 0,5 s ; `PluieDeFleches` : zone de 12 dégâts, rayon 45 px,
  portée 320 px, recharge 8 s, activation 0,8 s) ; valeurs provisoires, à ajuster après test. Les
  icônes de la barre montrent un sigle de deux lettres par skill (FP, PR, SO, EX, FS, IM, TP, PF) et,
  dans leur bande de couleur, la place du lanceur (1 à 3). Voir `docs/ecrans.md`, `docs/formation.md`
  et `docs/combat.md`.
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
- **Zones de contrôle et attaque d'opportunité** (étape 77) : une entité qui a une zone de contrôle
  (20 px bord à bord pour le guerrier, le paladin et tout monstre ; 0 pour le prêtre, le mage et le
  rôdeur, qui peuvent être engagés mais n'engagent personne) engage ce qui est dans sa zone. Quand B
  sort de la zone de A en s'écartant au moins autant que A s'écarte de B, A le frappe gratuitement :
  coup instantané, dégâts de son attaque de base (armure de la cible appliquée), sans toucher à sa
  recharge ni à son coup en cours, sans ligne de vue. Pas de coup si A engage encore quelqu'un à la
  fin de l'image ; au plus un coup par attaquant et par image (le fuyard le plus proche) ; un monstre
  qui rentre à son poste n'est ni attaquant ni cible. Dans les deux sens (monstre → personnage,
  personnage → monstre). Module pur `Engagement`, orchestré par `Partie` (deux photos par image, avant
  et après le déplacement). F3 dessine un cercle fin magenta autour de chaque entité qui a une zone.
  Un monstre tué par un coup gratuit est retiré et le verrou levé. Voir `docs/combat.md`,
  `docs/monstre.md` et `docs/boucle-et-entrees.md`.
- **Dégagement des tireurs** (étape 78) : un personnage sans zone de contrôle (prêtre, mage, rôdeur)
  engagé par un ou plusieurs monstres se dégage vers sa place d'attaque (anneau autour du plus proche
  d'entre eux, à 90 % de la portée de son attaque) dès que CHAQUE monstre qui l'engage a un autre
  personnage dans sa zone : il part sans coup gratuit, puisque ce monstre engage encore quelqu'un à la
  fin de l'image. Seul face à un monstre, il reste et tire. Conditions en plus : ancre à l'arrêt, aucun
  skill en attente pour lui, aucune action en exécution (un coup en cours part d'abord). Pendant le
  dégagement il ne frappe pas et ne s'arrête pas à portée ; fin à l'arrivée, si le monstre disparaît ou
  rentre à son poste, si l'ancre bouge, si un skill est mis en attente ou engagé, ou s'il est encore
  engagé alors que le porteur est parti. Priorité de `pointVoulu` : exécution > skill en attente >
  dégagement > verrou > agresseur > formation. Règle pure `monstreDontSeDegager` (`Engagement`), état
  et fin dans `Escouade::degagementVoulu`. Aucun affichage nouveau (F3 montre le point visé). Voir
  `docs/combat.md` et `docs/formation.md`.
- **Dégâts flottants** (étape 79) : au-dessus de chaque entité montent, 0,8 s durant, les dégâts réellement
  subis (après armure) et les soins reçus. Blanc pour les dégâts sur un monstre, rouge sur un personnage,
  vert précédé de « + » pour un soin, orange plus gros (20 px au lieu de 16) suivi de « ! » pour un coup
  gratuit ; contour noir ; le texte monte de 30 px et s'efface dans la seconde moitié de sa vie. Plusieurs
  textes sur la même cible dans la même image s'empilent de 14 px. Rien pour un montant de 0 : monstre qui
  rentre à son poste (invulnérable), soin sur un allié plein. Dans le brouillard, rien n'est montré (F6 :
  tout). Les textes finissent leur animation après une défaite ou une victoire. Mécanisme : `Partie`
  inscrit chaque dégât et chaque soin appliqué dans une file d'`EvenementDeCombat` (données), qu'`EcranJeu`
  prend après chaque image pour nourrir `TextesFlottants` (logique pure, dessin dans la vue du monde) ;
  les fonctions qui appliquent dégâts et soins renvoient le montant réellement appliqué. Voir
  `docs/combat.md`, `docs/boucle-et-entrees.md`.
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
- **Fenêtre et carte** : fenêtre 1200 × 800 par défaut ; carte de 59 × 40 cases de
  40 px (2360 × 1600 px). Depuis l'étape 59, `EcranJeu` joue sur un niveau
  généré (`genererNiveau` : salles rectangulaires reliées par des couloirs en L de
  2 cases de large, escouade au centre de la première salle, 1 à 3
  monstres par autre salle, même graine que la partie, voir
  `docs/generation.md`). F5 (à tout moment, touche
  de débogage) et « Commencer » (écran de choix) génèrent une nouvelle carte avec une
  nouvelle graine, affichée dans la console ; R (après une défaite ou une
  victoire) ramène à l'écran de choix de l'escouade ; la carte faite à la main
  (`creerNiveau()` dans `Niveau.cpp`) reste le terrain des tests. La caméra (`sf::View`) est centrée
  sur l'escouade ; du noir apparaît au-delà des bords de la carte.
- **Architecture de la boucle** (étapes 52, 54 et 65) : `main.cpp` ne fait
  que lancer l'`Application`, qui possède la fenêtre, la police, les
  `Reglages` et une pile d'écrans (`EcranMenu`, `EcranOptions`, `EcranJeu`,
  patron State ; un écran renvoie une `Transition` que l'`Application`
  applique). `EcranJeu` garde la caméra et traduit les événements SFML en appels au
  `Controleur` (mode visée, déplacement continu, aperçu de visée ; type valeur, il reçoit la `Partie` à chaque appel) ; la
  `Partie` (carte, escouade, monstres) porte les règles de jeu et
  l'intermédiaire entre `Escouade` et `Monstre`, avec l'ordre d'une image
  conservé ; `Niveau` porte les données. Voir `docs/boucle-et-entrees.md`.
- **Barre de raccourcis** (étape 75) : en bas au centre de la fenêtre, fixe à l'écran (vue de
  l'interface, dessinée en dernier), une grille de 4 × 3 icônes carrées de 48 px (4 px d'écart,
  10 px de marge), les 12 emplacements de la `BarreDeRaccourcis` (propriété d'`EcranJeu`, ni
  `Partie` ni `Escouade` ne la connaissent). Chaque emplacement pointe vers un skill de n'importe
  quel personnage (une `AdresseSkill`) ; disposition par défaut : une rangée par personnage, dans
  l'ordre des places (rangée du haut : frappe puissante et provocation du guerrier ; milieu : soin
  du prêtre ; bas : explosion du mage), les autres cadres vides. Touches (scancodes, table dans
  `Reglages::touchesDesRaccourcis`, ni lue ni écrite dans `reglages.ini`) : Q W E R / A S D F /
  Z X C V, donc Q = frappe, W = provocation, A = soin, Z = explosion ; les touches 1 à 4 ne font
  plus rien. Une icône montre le sigle du skill (deux lettres : FP, PR, SO, EX…), le nom de sa touche dans un coin
  et une bande de la couleur du personnage lanceur ; un emplacement vide est un cadre sombre avec
  le nom de sa touche. **Clic** : cliquer une icône fait la même chose que sa touche
  (`Controleur::activerEmplacement`), avec la règle « l'interface d'abord » : le clic est testé
  sur la barre avant le monde et consommé (pas de verrou, de porte, de déplacement ni de point
  d'impact de visée) ; un appui sur la barre ne lance pas le déplacement continu et désarme
  celui d'un clic au sol précédent, mais un déplacement commencé au sol continue quand le
  curseur passe sur la barre. Sur l'écran de fin, R ramène au choix de l'escouade et les raccourcis (touche et
  clic) sont ignorés ; en jeu, R est l'emplacement 3. Voir `docs/boucle-et-entrees.md`.
  Pendant la recharge, un quartier noir semi-transparent suit les
  bords du carré (zone claire grandissante depuis midi, sens horaire) et
  le temps restant en secondes entières s'affiche au centre. Icône grisée,
  sans quartier ni chiffre, quand le lanceur est mort ; contour jaune sur
  l'icône du skill en cours de visée, contour blanc pendant son activation. Police DejaVu Sans dans
  `assets/fonts/`, copiée à côté de `main.exe` par CMake ; le jeu refuse
  de démarrer si elle manque.
- **Qualité du code** (étapes 49 et 80) : `squad_core`, `main` et `tests` se compilent avec
  `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` sans aucun avertissement, dans les deux presets
  (pas de `-Werror`) ; `[[nodiscard]]` sur tout retour d'une fonction sans effet de bord ou qui
  signale un échec (règle écrite dans `CLAUDE.md`) ; `assert` devant les déréférencements qui
  reposent sur un invariant ; `Cercle` passé par valeur ; constantes en `s_`. **Build** (étape 80) :
  presets `debug` (assertions de la bibliothèque standard, UBSan) et `jeu` (Release), CTest,
  `.clang-format` (sans reformatage du code existant) et `.clang-tidy` (59 avertissements restants,
  aucun corrigé hors de `[[nodiscard]]` et des accès à un `optional` ; décompte dans `docs/audit.md`).
  `dt` borné à 1/20 s dans `Application` : après un gel, le jeu ralentit au lieu de faire traverser
  les murs. `coutDesObstacles` ne visite plus que le rectangle englobant de chaque obstacle (même
  résultat ; 24 monstres en poursuite : 50 ms → 3,9 ms par image en debug, 2,35 → 0,34 ms en jeu).
  Voir `docs/build.md`. Barre de vie
  des personnages et des monstres dessinée par le module `BarreDeVie`.
  Points de vie portés par la classe `PointsDeVie` (étape 53), possédée par
  `Personnage` et `Monstre`. `Skill` n'a plus que la machine à états
  (`commencer` / `avancer` / `annuler`) : `declencher` et
  `declencherSansCible` ont été supprimées à l'étape 62.
- **Escouade** : l'ancre (carré vert de 20 × 20, démarre au centre de la première salle
  sur la carte générée, en `(100, 100)` seulement avec `creerNiveau()`,
  180 px/s) suit le chemin calculé par le A* puis lissé (trajets directs en
  terrain dégagé, coins contournés au plus court), s'arrête exactement sur
  le point cliqué, est bloquée par les murs et glisse le long d'eux. Son
  déplacement vit dans `Mobile`, que `Monstre` utilise aussi.
- **Temps d'exécution** (étape 57) : toute attaque de base a un temps
  d'exécution, 0,15 s pour les personnages, 0,3 s pour les monstres,
  pendant lequel l'attaquant est immobile. Pour engager un coup il faut
  être vivant, immobile (`Mobile::estEnMouvement` faux), avoir l'attaque
  prête et une cible à portée et en vue : on ne frappe plus en marchant.
  Le coup engagé part toujours à la fin de l'exécution, même si la cible a
  bougé. Il est annulé, sans recharge consommée, par la mort de l'auteur ou
  de la cible ; pour un personnage aussi par un ordre de déplacement (clic,
  déplacement continu, clic sur un autre monstre) ou par un de ses skills
  (qui relance la recharge de l'attaque de base). Un monstre va toujours au
  bout de son coup, même si une provocation change sa proie. La recharge
  démarre au départ du coup. Le carré de l'entité s'éclaircit de 50 % pendant
  l'exécution (le contour jaune du monstre verrouillé est inchangé). Les
  skills ont leur propre temps d'activation (voir
  « Skills »). Voir `docs/combat.md`.
- **Aléatoire** (étape 56) : `Partie` possède un `Generateur`
  (`std::mt19937`, `Aleatoire.hpp`) initialisé par une graine que `main.cpp`
  tire avec `std::random_device` et affiche dans la console (au démarrage
  et à chaque R) ; il est passé par référence jusqu'à `Skill`. La recharge
  d'un `Skill` est un intervalle `[min, max]` tiré au déclenchement (aucun
  tirage si `min == max`) : attaques de base des mêlées 0,3–0,7 s, des
  tireurs 0,4–0,6 s, des monstres 0,7–1,3 s ; skills des personnages
  fixes. Voir `docs/combat.md`.
- **Armure** (étape 55) : `appliquerArmure` (module `Defense`) réduit les
  dégâts reçus, même formule pour tous : réduction = armure / (armure +
  100), arrondi au plus proche, au moins 1 par coup positif. Mêlées 50
  (un tiers de moins), tireurs 0, monstres 20 (un sixième de moins). Le
  soin n'est pas réduit. Rien n'est affiché à l'écran.
- **Personnages** (étape 74 : trois, venus d'une `Composition` injectée à `Escouade` par
  `Partie`, `composer` des classes choisies) : 3 carrés de 20 × 20 (240 px/s) tiennent une
  formation en triangle autour de l'ancre, bloqués par les murs et par les monstres.
  Chacun possède un `Mobile` : en ligne droite quand le trajet vers son
  point est libre, par un chemin A* lissé quand un mur est dans le chemin
  (recalculé seulement si la cible s'éloigne de plus d'une case de la fin
  du chemin) ; un personnage resté derrière un coin ou un pilier le
  contourne donc. La
  formation pivote progressivement vers la direction de marche (360°/s,
  par le plus court côté). Devant, le guerrier (rouge, mêlée, portée 20 px) ; derrière, le prêtre
  (jaune pâle) et le mage (violet), des tireurs (portée 120 px) ;
  portées mesurées de bord à bord ; 5 dégâts, recharge 0,3–0,7 s (mêlées) ou 0,4–0,6 s (tireurs) ; 100
  points de vie et une barre de vie au-dessus d'eux.
- **Places de formation** : la place théorique de chaque personnage
  (ancre + décalage tourné) est ramenée vers l'ancre si le trajet en ligne
  droite de l'ancre à cette place n'est pas libre (`replierVersOrigine`) :
  près d'un mur, d'un coin ou d'un pilier, la formation se resserre
  d'elle-même.
- **Places d'attaque et séparation** (étape 67) : un personnage qui approche un
  monstre (combat verrouillé, agresseur, skill de dégâts en attente) marche vers
  une **place d'attaque** sur un anneau autour du monstre (rayon du monstre + rayon
  du personnage + 90 % de la portée), distincte de celle des autres : la première
  face à l'escouade, puis alternance gauche/droite, hors mur, en vue du monstre,
  à plus de 34 px des places retenues et des personnages immobiles ; l'angle est
  gardé tant qu'il vise le même monstre. Il s'arrête toujours dès que le monstre
  est à portée et en vue. Les places de formation sont elles aussi distinctes
  (écartées à 34 px près d'un mur). Une **séparation douce** (120 px/s, plus lente
  que la marche) écarte les personnages à moins de 34 px ; elle ne pousse pas un
  personnage en exécution. Chaque personnage a un contour sombre de 1 px. Débogage :
  F3 affiche le point visé et le chemin restant de chaque personnage (cyan) et le
  chemin restant de chaque monstre (rouge) et rend semi-transparent tout monstre
  qui rentre à son poste (étape 69), F4 cache le carré vert de l'ancre.
  Voir `docs/formation.md`.
- **Chemins qui contournent les entités** (étape 68) : le A* ajoute un coût
  (8) aux cases touchées par un obstacle agrandi du rayon du mobile, le lissage
  et la ligne droite d'un personnage évitent les mêmes cercles ; un passage sans
  autre issue reste franchissable. Les monstres contournent les autres monstres
  (pas les personnages : on les frappe) ; chaque personnage contourne les monstres
  qu'il n'approche pas (pas les autres personnages, pas l'ancre). Un mobile
  bloqué depuis 0,3 s recalcule son chemin (le propriétaire, pas `Mobile`). Les
  places de formation ne sont plus dans un monstre. Contre un cercle, le glissement
  essaie aussi la tangente (un pas de côté ; la règle de blocage est inchangée).
  Voir `docs/chemin.md` et `docs/carte-et-collision.md`.
- **Mort et défaite** : un personnage à 0 point de vie tombe sur place
  (carré gris, sans barre de vie), ne frappe plus, n'est plus ciblé et ne
  bloque plus personne ; il reste dans la formation, sa place est
  simplement vide. Une fois tous morts (`Partie::estPerdue`), le monde se
  fige, un écran de défaite (voile sombre, « Défaite », « Appuyez sur R pour
  revenir au choix de l'escouade ») s'affiche et la touche R ramène à l'écran de choix (étapes 65 et 76) ;
  « Commencer » crée alors une nouvelle `Partie` (nouvelle graine), avec la même sélection.
- **Objectif et victoire** (étape 63) : la partie est gagnée quand il ne
  reste aucun monstre (`Partie::estGagnee`, faux si la partie est perdue : la
  défaite l'emporte si les derniers monstres et personnages meurent dans la
  même image). Le monde se fige comme après la défaite (même sortie
  anticipée de `mettreAJour`), l'écran de fin affiche « Victoire » (module
  `EcranDeFin`, titre en paramètre, partagé avec « Défaite ») et R ramène au
  choix de l'escouade, après une victoire comme après une défaite (F5 inchangée). Un
  compteur « Monstres : N » (`Partie::monstresRestants`), blanc à contour
  noir, est fixe en haut à gauche de l'écran (vue de l'interface).
- **Entrées** : un clic sur le sol envoie l'escouade vers ce point ; tant
  que le bouton reste enfoncé (fenêtre ayant le focus, escouade vivante),
  l'ordre est redonné à chaque image vers le curseur. Un clic bref sur un
  monstre engage un combat qui continue seul, sans tenir le bouton ; les
  raccourcis de la barre (clavier ou clic, voir « Barre de raccourcis ») déclenchent le
  skill de leur emplacement ; un skill de zone entre en mode visée (voir « Skills »).
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
- **Skills** : quatre, de classe `Skill` (type, portée, puissance, recharge, rayon de zone, temps d'activation).
  **Étape 74** : un skill est désigné par une `AdresseSkill` (place du personnage et `IdSkill`
  stable : `FrappePuissante`, `Provocation`, `Soin`, `Explosion`) ; un personnage connaît une liste de
  skills, une seule action engagée à la fois (engager un second skill pendant l'activation d'un autre est
  refusé), recharges indépendantes. **Étape 75** : les skills se lancent par la barre
  de raccourcis (par défaut Q → frappe du guerrier, W → sa provocation, A → soin du prêtre, Z → explosion
  du mage, au clavier ou au clic ; ci-dessous, « touche 2 », « touche 3 » et « touche 4 » désignent W, A et Z) ; la
  visée dépend du type `Zone` du skill. Frappe (Q) :
  skill de dégâts du guerrier (25 dégâts, recharge 5 s, portée
  de mêlée, 20 px), sur le monstre verrouillé ou, sans verrou, sur le
  monstre à portée que ce guerrier frapperait en autodéfense (alors sans
  marche ni mise en attente). Touche 3 : soin du prêtre
  (30 points de vie, recharge 8 s, portée 40 px), sur le
  vivant le plus blessé (soigneur inclus), permis hors combat. Touche 2 :
  provocation du guerrier (zone de 150 px depuis son centre,
  4 s, recharge 10 s) : tous les monstres dans la zone le prennent pour
  proie au lieu du personnage le plus proche, sans dégâts ni attente
  (consommée même s'il n'y a aucun monstre dans la zone) ; elle prend fin à
  l'écoulement de la durée, à la mort du provocateur ou à celle du
  monstre. Touche 4 : skill de zone du mage (15 dégâts, recharge 8 s,
  zone de 60 px, portée 250 px) en deux temps : 4 fait entrer en
  mode visée (un cercle jaune de 60 px suit le curseur, un cercle blanc
  montre la portée autour du lanceur), puis un clic gauche lance le sort
  au point visé (même sur un monstre : pas de verrou). À portée (bord à
  bord) et en vue (ligne de vue entre le lanceur et le point, sans mur),
  son activation commence tout de suite ; hors portée ou derrière un mur, le lanceur
  quitte sa place, marche vers le point (contournant le mur) et commence
  l'activation au point fixé au clic dès qu'il est à portée et en vue (un point
  dans un mur est refusé, sans recharge ; seule sa mort l'annule : ni verrou, ni clic, ni ordre de
  déplacement). Le sort touche tous les monstres dont le bord est dans le
  cercle au moment du départ et qui sont en vue du point d'impact (centre
  à centre, il ne traverse pas les murs), et coûte sa recharge même si la zone est vide. Clic
  droit, Échap ou 4 annulent sans recharge ; le déplacement continu est
  suspendu pendant la visée. Un skill de dégâts ou de soin hors de portée met son lanceur en marche vers la
  cible et commence son activation à l'arrivée (un seul skill en attente à
  la fois ; annulé par la mort du lanceur, celle de la cible ou son retour
  à pleine vie pour un soin, la rupture du combat pour des dégâts). Une
  recharge, qui ne démarre qu'au départ du skill, est montrée par la barre de skills (la
  barre orange sous les personnages a été supprimée à l'étape 74).
  **Temps d'activation** (étape 58) : chaque skill passe par la même machine
  à états que l'attaque de base : dégâts 0,3 s, provocation 0,2 s, soin
  0,6 s, zone 0,8 s. L'appui est un ordre : il n'exige pas d'être immobile,
  mais le lanceur s'arrête sur place pendant toute l'activation (son carré
  s'éclaircit, l'icône a un contour blanc), puis l'effet part, évalué à ce
  moment (monstres dans la zone, cible du coup ou du soin). Le coup de base
  engagé est annulé au début et sa recharge est relancée au départ.
  Pendant l'activation le skill n'est pas prêt : un nouvel appui est refusé
  et la touche 4 n'entre pas en visée ; les skills d'autres personnages
  restent permis (plusieurs activations simultanées). Seuls la mort du
  lanceur, celle du monstre visé (dégâts) ou celle de l'allié visé (soin)
  l'annulent, sans recharge ; un ordre de déplacement, un clic au sol ou un
  verrouillage ne l'annulent pas. Pas de revérification de portée ni de vue
  au départ. **Lanceur occupé** (étape 80) : un skill demandé pendant l'activation d'un autre skill du
  même personnage (guerrier : provocation puis frappe ; paladin ; rôdeur) n'est plus perdu : il est mis
  en attente et part dès que le lanceur est libre ; un skill déjà en attente le reste si sa cible
  devient atteignable pendant cette activation. Sans verrou, une frappe ainsi en attente part si le
  monstre est encore atteint quand le lanceur est libre, sinon elle est abandonnée, sans recharge.
  Voir `docs/combat.md`.
- **Monstres** : des carrés rouges de 30 × 30 (160 px/s, 100 points de
  vie, armure 20, barre de vie). Sur la carte générée : 1 à 3 par salle sauf
  la première, au centre d'une case intérieure (une case de marge contre
  les murs), à au moins 2 cases les uns des autres et à 400 px ou plus du
  départ (rejet, 10 tentatives par monstre, abandonné sinon : une salle
  peut en avoir moins ou aucun). Sur la carte à la main (tests) : six, à
  `(660, 380)`, `(500, 580)`, `(900, 220)`,
  `(1940, 180)`, `(1500, 540)` et `(1620, 540)`. Possédés par
  `Partie` dans un `std::vector<Monstre>`. Un monstre détecte un
  personnage vivant à moins de 200 px de lui et de 600 px de son poste (sa
  position de départ) ET en vue (`ligneDeVue`, en cases) ;
  il le poursuit (A* + lissage) sans que la perte de vue y mette fin, et
  s'arrête dès qu'il peut l'atteindre : à portée d'attaque (15 px bord à
  bord, 10 dégâts, recharge 0,7–1,3 s) ET en vue (`Monstre::peutAtteindre`). Un monstre qui reçoit des dégâts (> 0) entre en poursuite de sa
  proie habituelle (provocateur, sinon le plus proche) même hors de sa
  détection, s'il est lui-même dans son territoire ; un coup reçu hors
  du territoire ne le réveille pas, et ne sert qu'une fois. Il abandonne
  quand lui-même s'éloigne de plus de 600 px de son poste, ou si la proie
  meurt, et rentre alors à son poste. **Retour au poste** (étape 69) : jusqu'à
  son arrivée, il ne réagit à rien (ni détection, ni alerte, ni dégâts, ni
  alerte de groupe), n'engage aucun coup, est **invulnérable**, **regagne la moitié de
  ses points de vie maximum par seconde** et n'est plus une cible (ni autodéfense,
  ni verrou, ni provocation, ni skill ; un clic sur lui compte comme un clic au
  sol), tout en continuant de bloquer et d'être contourné. Cela supprime
  l'exploit où des tireurs touchaient un monstre qui faisait des allers-retours
  à la frontière de son territoire. Sa proie (le personnage vivant le plus
  proche, ou le provocateur) est celle qu'il poursuit ; il frappe sa **cible de
  frappe** (étape 66) : la proie si elle est à portée ET en vue, sinon le
  personnage atteignable le plus proche (un monstre provoqué ne frappe que son
  provocateur), attaqué ou non, et s'arrête dès qu'il en a une ; un monstre touché
  s'active même sans voir son agresseur. Un monstre mort est
  retiré. **Groupes** (étape 64) : chaque monstre a un identifiant de
  groupe opaque (`Monstre::getGroupe()`), fixé à la construction : sur la
  carte générée, un groupe par salle peuplée ; sur la carte à la main, un
  groupe par monstre sauf les deux de `(1500, 540)` et `(1620, 540)`, qui
  partagent le leur. Quand un monstre passe en poursuite (depuis « en garde »),
  `Partie` appelle `Monstre::alerter()` sur les autres
  membres de son groupe : même règle que le réveil par les dégâts (proie
  existante ET monstre dans son territoire, sinon rien). Les alertés
  commencent à bouger à l'image suivante, et sont tous alertés d'un coup
  (pas de chaîne vers un autre groupe) ; le retour au poste reste
  individuel. Un monstre touché par un skill de zone de loin alerte donc
  toute sa salle.
- **Blocage mutuel** : les monstres se bloquent entre eux et sont bloqués
  par les personnages vivants ; les personnages sont bloqués par les
  monstres. L'ancre ne bloque personne et n'est pas bloquée.
- **Tests** : logique de jeu dans `squad_core`, testée avec Catch2 3
  (`ctest --preset debug`, ou `./build/debug/tests.exe`, 893 tests : 27 sur `Engagement`, 23 sur `ChampDeVision`, 8 sur `Vision`, 5 sur `ObjetInteractif`, 15 sur `Reglages`, 3 sur `TailleDeVue`, 6 sur `ListeDeBoutons`, 4 sur `EcranMenu`, 12 sur `EcranOptions`, 61 sur `GenerateurDonjon`, 36 sur `TileMap`, 28 sur `Chemin`, 226
  sur `Escouade` en onze fichiers par thème (déplacement, combat, quatre fichiers de skills par type plus un commun, agresseurs, vue, places, dégagement),
  7 sur `Composition`, 16 sur `Specialite`, 9 sur `SelectionDEscouade`, 11 sur `EcranChoixEscouade`, 11 sur `Encerclement`, 16 sur `Deplacement`, 78 sur `Monstre` en quatre fichiers (base, états, retour, engagement),
  64 sur `Personnage`, 24 sur `Skill`, 24 sur `Mobile`, 4 sur `Eclaircir`, 10 sur `Cercle`, 13
  sur `Autodefense`, 8 sur `BarreDeSkills`, 11 sur `BarreDeRaccourcis`, 3 sur `ClicInterface`, 4 sur `EntiteVue`, 1 sur
  `BarreDeVie`, 11 sur `PointsDeVie`, 6 sur `Defense`, 9 sur `Provocations`, 8 sur `Agresseurs`, 66 sur `Partie` en sept fichiers (clics, skills, monstres, fin, portes, zones de contrôle, événements), 16 sur `Controleur`, 9 sur `TextesFlottants`). Aides communes dans
  `tests/aides.hpp` (base), `tests/aides_escouade.hpp` (fixtures d'`Escouade` et de `Partie`) et
  `tests/aides_monstre.hpp` ; noms de tests sans accents (`./build/debug/tests.exe "*a portee*"`
  filtre ; ctest en dépend) ; rotation de la formation et `provoquer` à une place invalide couverts.
  **Les tests ne dépendent pas du catalogue** (étape 80) : la composition par défaut des tests
  (`compositionDeTest()` : guerrier, prêtre, mage de test ; `uneEscouade`, `unePartie`) est construite
  avec des valeurs figées (`s_degatsBaseDeTest`, `s_porteeMeleeDeTest`…), les valeurs non injectables
  se lisent par les accesseurs ; changer une valeur d'équilibrage du jeu ne fait plus échouer que les
  tests dont c'est le sujet (seul `test_Specialite.cpp` lit les valeurs du catalogue).
  `uneEscouadeADeuxGuerriers` et `uneEscouadeAuSoinCourt` servent aux cas qui ont besoin de deux mêlées
  ou d'un allié hors de portée du soin. Voir `docs/tests.md`.

**Limites connues** (acceptées pour l'instant) :
- Vision (étapes 72 et 73) : un monstre verrouillé qui sort de la vision garde le verrou ; la
  ligne de vue ne regarde pas le rayon de vision (320 px) : un monstre voilé peut détecter et
  frapper, et le skill de zone peut toucher des monstres voilés ; le skill de zone peut être
  visé dans l'inconnu ; pas de mémoire des monstres vus ; le compteur « Monstres : N » compte
  aussi les monstres cachés ; deux cases libres en diagonale entre deux murs qui se touchent se
  voient (algorithme de Ford), ce que le générateur n'empêche pas (145 graines sur 20 000, soit
  0,7 %, contiennent un tel coin ; le test sur les graines 1 à 50 n'en trouve aucun) ;
  `ligneDeVue` n'a pas de cache (aucun ralentissement mesuré). Voir `docs/vision.md`.
- Menus (étape 65) : souris seulement ; « Options » et « Commencer partie »
  n'existent qu'au menu de départ (pas de pause ni de retour au menu en
  cours de partie) ; une résolution enregistrée plus grande que le bureau
  ouvre une fenêtre trop grande (les options la corrigent) ; `reglages.ini`
  est cherché dans le dossier courant, comme la police. Voir `docs/ecrans.md`.
- Portes (étape 70) : seulement sur les ouvertures de la largeur du couloir
  (2 cases) ; pas de refermeture, de clé ni de porte verrouillée ; les monstres ne
  les ouvrent pas et ne contournent pas par un autre chemin ; un seul personnage
  n'ira pas ouvrir pendant que les autres attendent ; pas de surbrillance au
  survol ; une porte d'une seule case est supposée horizontale.
- Si le générateur ne place aucun monstre (très rare), la partie est gagnée
  dès le départ ; F5 génère une autre carte (le générateur n'est pas
  modifié).
- Couloirs générés (étape 60) : un couloir peut traverser une autre salle
  ou longer un mur, et les couloirs ne forment pas de boucle (arbre).
  Aucun problème de déplacement observé dans les couloirs de 2 cases.
- Choix de l'escouade (étape 76) : souris seulement ; pas de description des classes ni d'infobulle ;
  la sélection n'est pas gardée entre deux lancements du jeu ; pas d'arbre de skills ni de niveaux
  (deux personnages de la même classe ne diffèrent que par leur couleur) ; le paladin et le rôdeur
  ont des valeurs provisoires ; au plus trois fois la même classe (3 places).
- Pas d'interruption de l'activation d'un skill par les dégâts reçus ; un
  seul skill en attente pour toute l'escouade (depuis l'étape 80, un skill attend aussi que son
  lanceur ait fini l'activation d'un autre de ses skills : pendant ce temps, l'ordre de skill d'un
  autre personnage est refusé) ; la provocation, sans cible, ne peut pas attendre : demandée pendant
  l'activation d'un autre skill du guerrier, elle est refusée (le mode de cible « Soi » de R4 doit
  lever cette limite) ; pas de barre de progression d'incantation ; pas de revérification de portée
  ni de vue au départ du skill.
- Barre de raccourcis (étape 75) : elle ne montre pas le skill en attente ; les touches ne sont
  ni configurables ni sauvegardées (table dans `Reglages`, pas dans `reglages.ini`) ; une seule
  barre, pas de survol, d'infobulle ni de clic droit sur une icône, pas d'interface pour placer
  les skills ; une 4ᵉ place n'a pas de rangée dans la disposition par défaut, et un 5ᵉ skill d'un
  personnage n'est pas placé. Sa position se recalcule à chaque image depuis la vue d'interface.
  La police se charge par chemin relatif : le jeu doit être lancé depuis le dossier de `main.exe`.
- Le skill de zone n'a pas d'effet visuel à
  l'impact et ne touche pas les alliés ; son lanceur, bloqué par un
  monstre ou face à un point inaccessible, peut rester en attente.
- Un personnage dont le A* ne trouve aucun chemin garde la ligne droite.
  Les chemins contournent les monstres (étape 68), mais pas les autres
  personnages ni l'ancre ; un passage sans autre issue est traversé au prix du
  coût ajouté, l'entité attend alors derrière le monstre ; contre un cercle le
  glissement tangent n'est qu'un pas de côté. Près des piliers, le contournement
  n'est pas toujours le plus court.
- Le territoire des monstres reste une distance pure (il ignore les
  murs) ; une fois en poursuite, un monstre ne perd pas sa proie en la
  perdant de vue (il la suit par le A* jusqu'à l'abandon habituel) ; la
  ligne de vue se décide en cases (étape 73), d'un centre à l'autre.
- Zones de contrôle (étape 77) : un seul coup gratuit par attaquant et par image, sans délai de
  réaction (peut sembler généreux) ; un pas de côté de A qui rompt le contact compte comme une fuite
  de B (égalité des projections : B est le fuyard) ; toute la poussée de la séparation douce compte
  comme déplacement ; un tireur au contact d'un monstre seul, ou de plusieurs dont un n'est pas tenu,
  qui recule reçoit le coup gratuit (l'étape 78 ne le dégage que si chaque monstre est tenu) ; pas de
  skill qui annule la pénalité ; pas d'effet visuel propre au coup gratuit (la barre de vie suffit).
- Dégagement (étape 78) : un tireur bloqué pendant son recul n'en sort que si la règle change
  (`Personnage` n'expose pas `estBloque`) ; l'arrivée est mesurée à 0,01 px de la place (une poussée de
  la séparation douce peut la retarder) ; une fois arrivé, sans verrou ni agresseur connu, les règles
  habituelles le ramènent vers sa place de formation (en partie réelle le monstre frappe le guerrier et
  devient agresseur : le tireur reste) ; une action déclenchée par le joueur interrompt le recul, que la
  règle relance ensuite si le tireur est toujours engagé et tenu ; un monstre qui change de cible peut
  relancer le dégagement.
- Dégâts flottants (étape 79) : le nombre est le montant après armure, sans plafond par les points de
  vie restants (le coup qui tue affiche son plein montant) ; pas d'« Immunisé » sur un monstre qui
  rentre, pas de critique ni de « raté » ; pas de nom de skill ni de « Provoqué » (étape 83) ; la taille
  du texte est fixe en pixels du monde, donc elle suit le zoom de la vue ; la visibilité d'un événement
  est jugée à la fin de l'image où il a lieu ; deux textes de la même cible ajoutés dans des images
  différentes mais proches dans le temps peuvent se superposer en partie.
- Provocation stricte (étape 66) : un monstre provoqué ne frappe que son
  provocateur, même s'il est collé à un autre personnage ; si le provocateur
  est caché, il ne frappe personne.
- Un coup engagé part sans revérifier portée ni vue ; seule la marche
  empêche d'engager (pas de ralentissement de l'escouade engagée).
- Lors d'un demi-tour rapide, deux personnages peuvent se croiser brièvement
  (la séparation douce les écarte en moins d'une seconde) ; l'ancre traversant
  les monstres, elle (et la caméra) peut s'éloigner des personnages bloqués.
- Un personnage mort n'est jamais retiré et la formation ne se resserre
  pas.
- Un monstre qui rentre ignore tout jusqu'à son poste, même une proie collée à
  lui (voulu, étape 69) ; un monstre touché juste avant d'abandonner ne prolonge
  pas sa laisse ; la régénération ne soigne presque rien si le retour est très
  court ; pas de patrouille ; pas de coordination entre monstres.
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
