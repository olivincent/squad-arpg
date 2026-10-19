# Squad-ARPG — Roadmap

Version courte, importée par `CLAUDE.md` à chaque session. Ailleurs :
- étapes terminées, liste et détail : `docs/historique.md` ;
- état détaillé et limites connues de chaque sujet : section « État et limites à la fin de
  l'étape 80 » (et « Limites connues ») du doc de `docs/` du sujet (tableau à la fin
  d'`ARCHITECTURE.md`).

## Étapes terminées

Étapes 0 à 81 ✅. Les dernières :
- Étape 79 — Dégâts flottants : une file d'événements de combat ✅
- Étape 80 — Filet de sécurité et corrections locales (suite de l'audit) ✅
- Étape 81 — Dossiers et documentation ✅

## État actuel (résumé)

- **Outils** : conception dans Claude.ai, implémentation avec Claude Code dans VSCode (règles dans
  `CLAUDE.md`) ; dépôt Git `squad-arpg`, poussé sur GitHub ; SFML 3.0.2 intégrée à CMake.
- **Code** (étape 81) : 57 modules rangés en 9 dossiers par domaine (`base`, `rendu`, `monde`,
  `combat`, `escouade`, `monstres`, `jeu`, `interface`, `app`), `#include` à chemin complet, règles
  de dépendance vérifiées (`ARCHITECTURE.md`) ; tests en miroir. Presets `debug` et `jeu`, 0
  avertissement, 893 tests verts sous CTest, 59 avertissements `clang-tidy` (`docs/build.md`,
  `docs/audit.md`). Les tests ne dépendent pas du catalogue des classes (`docs/tests.md`).
- **Écrans** : menu à la souris, options d'affichage (fenêtré ou plein écran, résolutions,
  `reglages.ini`), choix de 3 classes parmi 5 (guerrier, prêtre, mage, paladin, rôdeur ; doublons
  permis), puis la partie ; R après une victoire ou une défaite ramène au choix (`docs/ecrans.md`).
- **Carte** : donjon généré à chaque partie (salles, couloirs en L de 2 cases, portes à chaque
  jonction, 1 à 3 monstres par salle sauf la première, graine affichée), F5 = nouvelle carte ;
  carte à la main pour les tests (`docs/generation.md`, `docs/carte-et-collision.md`).
- **Vision** : brouillard de guerre (inconnu, exploré, visible) par shadowcasting symétrique, la
  même règle que la ligne de vue du combat ; F6 le coupe à l'affichage (`docs/vision.md`).
- **Escouade** : une ancre suivie par 3 personnages en formation (rangs avant et arrière), qui
  pivote et se resserre près des murs ; places d'attaque autour des monstres, séparation douce,
  chemins qui contournent les monstres, dégagement des tireurs (`docs/formation.md`, `docs/chemin.md`).
- **Combat** : verrou par clic, autodéfense, agresseurs, temps d'exécution des coups, armure à
  rendement décroissant, recharges aléatoires, zones de contrôle et attaques d'opportunité ; skills
  (dégâts, soin, provocation, zone avec visée) avec temps d'activation et un skill en attente pour
  l'escouade ; dégâts et soins flottants (`docs/combat.md`).
- **Monstres** : détection en vue, poursuite, frappe de la proie ou du plus proche atteignable,
  alerte du groupe, retour au poste invulnérable avec régénération (`docs/monstre.md`).
- **Interface** : barre de raccourcis de 12 emplacements (Q W E R / A S D F / Z X C V, ou clic ;
  « l'interface d'abord »), compteur de monstres, écran de victoire ou de défaite ; F3 et F4 de
  débogage (`docs/boucle-et-entrees.md`).

## Plan après l'audit (« hybride », décidé à l'étape 80)

L'audit de `docs/audit.md` est traité en trois temps : deux étapes de refactorisation, puis chaque
refactorisation restante comme **première sous-étape de la fonctionnalité qui en a besoin**. Les
fonctionnalités prévues sont donc décalées après l'étape 82.
- **Étape 80** (faite) : build vérifiable, tests indépendants du catalogue, corrections locales
  (C1, C4, D6, K2, V1 partie 1, L1).
- **Étape 81** (faite) : dossiers (option B de l'audit), P1, K1, K3.
- **Étape 82** (prochaine) : alléger `Escouade` : R1 (six responsabilités) avec R2 (couplage
  temporel entre `attaquer` et `mettreAJour`), D1 (vitalité répétée dans `Personnage` et
  `Monstre`), D3 (fonctions publiques appelées seulement par les tests). R2 doit **supprimer
  `m_cibleCombat` en tant que membre** : il n'est jamais remis à zéro et peut dater d'un ancien
  combat (à l'étape 80, la marche vers la cible d'un skill de dégâts en attente a dû exiger un
  verrou pour ne pas le lire à tort). Décision sur R5 (identifiants et places) à la fin de
  l'étape. Les extractions (`Formation`, `PlacesDAttaque`, choix du point visé) naissent dans
  `escouade/`.
  Découpage en cinq parties, chacune avec son prompt, tests verts à la fin de chacune, interface
  publique d'`Escouade` inchangée jusqu'à 82.5 :
  - **82.1** (faite) : D1, `Vitalite` (points de vie et armure, possédée par `Personnage` et
    `Monstre`) et cercle commun (`cercleDuRectangle`, `Mobile::getCercle`) ;
  - **82.2** : `Formation`, et la séparation douce en fonction pure à part (`escouade/Separation`),
    qui reçoit les centres et les drapeaux « poussable » et renvoie une poussée par personnage ;
    `Formation` ne connaît pas `Personnage` ;
  - **82.3** : `PlacesDAttaque` ;
  - **82.4** : choix du point visé par intentions, avec R2. Il renvoie une intention
    (`std::variant` : rester sur place, aller au point, approcher un monstre avec une portée),
    calculée par une fonction pure à partir d'une situation ; `Escouade` la traduit en point (place
    d'attaque par `PlacesDAttaque`) ; `monstresAContourner` se déduit de l'intention ; `Escouade` dit
    à `PlacesDAttaque` quel monstre chaque personnage approche, les autres places sont oubliées ; le
    dégagement reçoit un drapeau « dégagement possible » calculé avant ;
  - **82.5** : D3, bilan, décision sur R5.
- **Refactorisations rattachées à une fonctionnalité**, en première sous-étape de celle-ci :
  - R4 (skills : le type décide partout) et C2 (constructeurs à longues listes) avant les buffs, les
    soins dans le temps ou le mana. Le mode de cible « Soi » de R4 doit lever une limite de l'étape 80 :
    la provocation, sans cible, ne peut pas être mise en attente quand son lanceur est occupé ;
  - E1 (valeurs dispersées, `Monstre` en littéraux) avant les types de monstres ;
  - R3 (`Partie` accumule plusieurs rôles), D2 (indices de grille à la main), C3 (conversions de
    signe) et D5 (petites duplications de `Partie`) avant les coffres, les leviers ou la carte
    extérieure ;
  - M1 (`dessiner` dans le noyau), R6 (`EcranJeu`), D4 (écrans de menu), P2 (en-tête parapluie des
    écrans) et T4 (trous de couverture) au premier ajout visuel important. M1 lève les exceptions
    transitoires aux règles de dépendance (`ARCHITECTURE.md`) et permet une bibliothèque par dossier.

**Décisions reportées** :
- V1 partie 2 (recalcul moins fréquent du chemin des monstres) : à réévaluer avec la carte extérieure
  ou des hordes (la partie 1 a déjà divisé le coût par 7 à 13) ;
- R5 (identifiants et places) : à la fin de l'étape 82.

## Étape 83 (envisagée, après l'étape 82)

Le nom du skill lancé au-dessus du lanceur et « Provoqué » au-dessus des monstres provoqués, par la
même file d'événements que l'étape 79 : un type d'événement de plus (`EvenementDeCombat`, produit
par `Partie`) et un style de plus dans `TextesFlottants`. Aujourd'hui la file ne porte que les
dégâts et les soins réellement appliqués. Les autres usages de la file (« Immunisé », sons, journal
de combat, statistiques…) sont des idées, pas des décisions : `docs/brainstorm.md`, section 5 ter.

## Fonctionnalités prévues (à choisir avec Claude.ai)

- Gambits pour choisir le comportement d'un personnage engagé (rester et tirer, se dégager, passer
  en mêlée ; le dégagement de l'étape 78 en est la première règle, écrite à un seul endroit,
  `degagementVoulu`) et skills qui permettent de se faufiler sans pénalité (`docs/brainstorm.md`).
- Issues de l'étape 76 : arbres de skills par classe (on ne peut pas tout prendre, pour monter une
  même classe de façons différentes ; c'est ce qui différenciera deux personnages de la même
  classe), niveaux, description des classes et infobulles à l'écran de choix, mémoire de la
  dernière composition dans `reglages.ini`.
- Issues de l'étape 75 (`docs/brainstorm.md`, 5 bis) : écran « Configurer les touches » (préréglage
  1 à 0, - et =, barre en ligne droite, touches sauvegardées), barre d'auras seulement cliquable
  (deuxième `BarreDeRaccourcis`, valeur de plus dans `Barre`), skills à bascule, interface pour
  placer ses skills, survol et infobulles sur la barre.
- « Stratégie d'attaque » par type de monstre, qui se branchera sur `coutDesObstacles` : la liste
  de cercles passée changera selon le type d'entité (`docs/brainstorm.md`).

Autres pistes discutées (issues de `docs/audit.md` et des étapes précédentes) :
- Passe de nettoyage des coins diagonaux dans `genererNiveau` : à décider (`docs/vision.md`)
- Skill en attente par personnage (aujourd'hui un seul pour l'escouade) ; plusieurs à la fois
- Interruption de l'activation d'un skill par les dégâts reçus
- Laisse prolongée par les dégâts (`docs/monstre.md`), vitesse relative escouade/monstres et
  ralentissement de l'escouade engagée, comme leviers contre le kiting
- Classe `Arme` qui fournit l'intervalle de recharge de l'attaque de base (aujourd'hui écrit dans
  `Escouade.cpp` et `Monstre.cpp`)
- Monstre touché qui se jette sur l'auteur du coup, effet visuel d'alerte, alerte par proximité ou
  par le bruit, chef de groupe, retour au poste en groupe, patrouille, escouade de monstres
- Effets visuels : impact du skill de zone, monstre provoqué, cible de chaque personnage, coup ou
  mort d'une entité
- L'escouade poursuit d'elle-même un monstre qui l'attaque (écartée pour l'instant)
- Choisir sans verrou lequel de deux monstres à portée reçoit le skill ; choix manuel de la cible
- Blocage de l'ancre par les monstres, si le décalage ancre/personnages s'avère gênant
- Skills qui bousculent un adversaire ou permettent de se faufiler
- Autres effets de skill (étourdissement, ralentissement, dégâts de zone dans le temps ou aux
  alliés), soin de zone, résurrection
- Retirer un personnage mort de `m_personnages` et resserrer la formation, si c'est gênant
- Sortie ou escalier comme objectif, étages
- Déplacement suivant l'arc de rotation plutôt qu'une ligne droite (la séparation douce de
  l'étape 67 écarte déjà les chevauchements)
- Lissage plus serré autour des coins (*funnel algorithm* ou Theta*), si le contournement actuel
  des piliers s'avère insuffisant
- Finitions : caméra bloquée aux bords de la carte
- Limitation de fréquence des appels à `allerVers`/A*, si un problème de performance est mesuré
  sur une carte plus grande
- Types de personnages, statistiques, armes, capacités, au-delà de la simple portée différenciée
- Ligne de tir dégagée, recul des tireurs si un monstre s'approche d'eux ; places d'attaque pour
  les monstres qui encerclent un personnage
- Forme plus fidèle que le cercle unique (plusieurs cercles, rectangle orienté), orientation des
  entités
