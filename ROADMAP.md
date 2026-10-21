# Squad-ARPG — Roadmap

Version courte, importée par `CLAUDE.md` à chaque session. Ailleurs :
- étapes terminées, liste et détail : `docs/historique.md` ;
- état détaillé et limites connues de chaque sujet : section « État et limites à la fin de
  l'étape 80 » (et « Limites connues ») du doc de `docs/` du sujet (tableau à la fin
  d'`ARCHITECTURE.md`).

## Étapes terminées

Étapes 0 à 85 ✅. Les dernières :
- Étape 81 — Dossiers et documentation ✅
- Étape 82 — `Escouade` allégée : `Vitalite`, `Formation`, `PlacesDAttaque`, `Intention` ✅
- Étape 83 — Inventaire et équipement : le modèle, sans interface ✅
- Étape 84 — Inventaire : l'écran en lecture (avec R6, D4, P2) ✅
- Étape 85 — Inventaire : le glisser-déposer ✅

## État actuel (résumé)

- **Outils** : conception dans Claude.ai, implémentation avec Claude Code dans VSCode (règles dans
  `CLAUDE.md`) ; dépôt Git `squad-arpg`, poussé sur GitHub ; SFML 3.0.2 intégrée à CMake.
- **Code** (étape 85) : 69 modules rangés en 10 dossiers par domaine (`base`, `rendu`, `monde`,
  `combat`, `objets`, `escouade`, `monstres`, `jeu`, `interface`, `app`), `#include` à chemin complet, règles
  de dépendance vérifiées (`ARCHITECTURE.md`) ; tests en miroir. Presets `debug` et `jeu`, 0
  avertissement, 1 069 tests verts sous CTest, 41 avertissements `clang-tidy` (`docs/build.md`,
  `docs/audit.md`). `Escouade` n'est plus que l'assemblage : `Escouade.cpp` de 1 223 à 1 067 lignes. Les tests ne dépendent pas du catalogue des classes (`docs/tests.md`).
- **Écrans** : menu à la souris, options d'affichage (fenêtré ou plein écran, résolutions,
  `reglages.ini`), choix de 3 classes parmi 5 (guerrier, prêtre, mage, paladin, rôdeur ; doublons
  permis), puis la partie ; R après une victoire ou une défaite ramène au choix. La pile d'écrans
  dessine sous un écran qui laisse voir dessous (`laisseVoirDessous`) ; le `clear` est dans
  l'`Application` (`docs/ecrans.md`).
- **Carte** : donjon généré à chaque partie (salles, couloirs en L de 2 cases, portes à chaque
  jonction, 1 à 3 monstres par salle sauf la première, graine affichée), F5 = nouvelle carte ;
  carte à la main pour les tests (`docs/generation.md`, `docs/carte-et-collision.md`).
- **Vision** : brouillard de guerre (inconnu, exploré, visible) par shadowcasting symétrique, la
  même règle que la ligne de vue du combat ; F6 le coupe à l'affichage (`docs/vision.md`).
- **Objets** : 10 emplacements par personnage, un sac de 40 cases pour l'escouade, un catalogue de
  10 objets ; l'armure d'un personnage est la somme de son équipement (guerrier et paladin partent
  équipés : 50 ; les autres : 0) ; `ramasser` et `deplacerObjet` sont transactionnelles. La
  touche I ouvre l'inventaire par-dessus la partie figée : équipement des 3 personnages, sac de 40
  cases, infobulle au survol, et glisser-déposer des objets (sac, emplacements, entre personnages ;
  échange si la destination est occupée ; destinations valides en surbrillance) (`docs/objets.md`,
  `docs/ecrans.md`).
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
  débogage ; une touche devient une commande par une fonction pure, `commandePourTouche`
  (`docs/boucle-et-entrees.md`).

## Plan après l'audit (« hybride », décidé à l'étape 80)

L'audit de `docs/audit.md` est traité en trois temps : deux étapes de refactorisation, puis chaque
refactorisation restante comme **première sous-étape de la fonctionnalité qui en a besoin**. Les
fonctionnalités prévues sont donc décalées après l'étape 82.
- **Étape 80** (faite) : build vérifiable, tests indépendants du catalogue, corrections locales
  (C1, C4, D6, K2, V1 partie 1, L1).
- **Étape 81** (faite) : dossiers (option B de l'audit), P1, K1, K3.
- **Étape 82** (faite) : alléger `Escouade` : R1 (six responsabilités) avec R2 (couplage
  temporel entre `attaquer` et `mettreAJour`), D1 (vitalité répétée dans `Personnage` et
  `Monstre`), D3 (fonctions publiques appelées seulement par les tests). Les extractions
  (`Formation`, `PlacesDAttaque`, choix du point visé) sont nées dans `escouade/`. Cinq parties,
  chacune avec son prompt et ses tests verts :
  - **82.1** (faite) : D1, `Vitalite` (points de vie et armure, possédée par `Personnage` et
    `Monstre`) et cercle commun (`cercleDuRectangle`, `Mobile::getCercle`) ;
  - **82.2** (faite) : `Formation` (possédée par `Escouade`, ne connaît pas `Personnage`), la
    séparation douce en fonction pure (`escouade/Separation`, un état `Absent`/`Fixe`/`Libre` par
    personnage, calcul simultané comme avant) et `s_ecartEntrePersonnages` dans un en-tête neutre ;
    927 tests, 59 avertissements `clang-tidy`, `Escouade.cpp` de 1 223 à 1 112 lignes ;
  - **82.3** (faite) : `Formation` à appel unique (`mettreAJour` reçoit la direction de marche,
    `choisirOrientation` devient privée) ; `PlacesDAttaque` (possédée par `Escouade`, ne connaît pas
    `Personnage` ; `Approche` en initialisation désignée, « aucune place libre » en `std::optional`),
    mécanisme d'oubli des places déplacé tel quel (`retirer`, `retenir`, `cibleApprochee` provisoire),
    décrit dans `docs/formation.md` pour 82.4 ; 940 tests, 60 avertissements `clang-tidy`,
    `Escouade.cpp` de 1 112 à 1 069 lignes ;
  - **82.4** (faite) : choix du point visé par intentions, avec R2. Module `escouade/Intention` :
    trois intentions (`std::variant` : rester sur place, aller au point, approcher un monstre), une
    `SituationDuPersonnage` calculée avant par `Escouade`, et `choisirIntention`, fonction pure qui ne
    fait que classer les règles (l'ordre 0 à 5 n'a pas changé) ; `Escouade` résout l'intention en
    point (place d'attaque par `PlacesDAttaque`) et applique la règle d'oubli, écrite une fois ;
    `monstresAContourner` se déduit de l'intention ; `PlacesDAttaque` : `placeRetenue`, `peutPlacer`
    (le dégagement est confirmé avant la décision, sans drapeau) et `oublierSaufAutourDe`. R2 :
    `m_cibleCombat` et `m_monstreAPortee` supprimés, calculs locaux sur les monstres ciblables, que
    `mettreAJour` reçoit dans une `VueDesMonstres` ; l'ordre `attaquer` / `mettreAJour` n'est plus une
    règle de correction. Trois écarts de comportement acceptés, chacun fixé par un test
    (`docs/formation.md`, « Intentions ») ; 962 tests, 60 avertissements `clang-tidy`, `Escouade.cpp`
    de 1 069 à 1 086 lignes ;
  - **82.5** (faite) : R2 terminé : `Agresseurs` ne garde que l'identifiant et la minuterie, la
    position est lue dans les ciblables (plus aucun canal entre `attaquer` et `mettreAJour`) ;
    `cerclesAContourner`, fonction pure d'`Intention`, dont la signature garantit que le monstre
    approché n'est jamais à contourner ; D3 : `Escouade::recevoirDegats` retirée (aide `infliger`),
    `VueDesMonstres::deControle` sans valeur par défaut, les autres accesseurs réservés aux tests
    gardés avec leur raison (`docs/formation.md`, « Bilan de l'étape 82 ») ; 5 tests en double
    retirés ; marques `NOLINT` justifiées (59 → 41 avertissements `clang-tidy`) ; l'exigence d'un
    verrou pour marcher vers la cible d'un skill de dégâts en attente est gardée comme règle de jeu
    (étape 37) ; R5 rattachée à R4 ; 962 tests, `Escouade.cpp` de 1 223 à 1 067 lignes au total de
    l'étape (42 fonctions publiques au départ, 41 à la fin).
- **Refactorisations rattachées à une fonctionnalité**, en première sous-étape de celle-ci :
  - R4 (skills : le type décide partout), **avec R5** (types forts pour les places et les identifiants :
    les deux touchent `AdresseSkill`, `Cible`, `EntiteVue` et les identifiants des intentions, on ne les
    modifie qu'une fois) et C2 (constructeurs à longues listes) avant les buffs, les
    soins dans le temps ou le mana. Le mode de cible « Soi » de R4 doit lever une limite de l'étape 80 :
    la provocation, sans cible, ne peut pas être mise en attente quand son lanceur est occupé ;
  - E1 (valeurs dispersées, `Monstre` en littéraux) avant les types de monstres ;
  - R3 (`Partie` accumule plusieurs rôles), D2 (indices de grille à la main), C3 (conversions de
    signe) et D5 (petites duplications de `Partie`) avant les coffres, les leviers ou la carte
    extérieure ;
  - R6, D4 et P2 (faits à l'étape 84, en première sous-étape de l'écran d'inventaire) ;
  - M1 (`dessiner` dans le noyau) et T4 (trous de couverture) au premier ajout visuel important dans
    le monde, après R4. M1 lève les exceptions transitoires aux règles de dépendance
    (`ARCHITECTURE.md`) et permet une bibliothèque par dossier.

**Limite connue (étape 82.5)** : le contournement des monstres évite leur cercle, pas leur zone de
contrôle : un personnage peut la frôler et subir l'attaque d'opportunité. Piste : la « stratégie
d'attaque » par type de monstre (`docs/brainstorm.md`, section 4 bis), qui se branchera sur
`coutDesObstacles` (`docs/formation.md`, « Intentions »).

**Décisions reportées** :
- V1 partie 2 (recalcul moins fréquent du chemin des monstres) : à réévaluer avec la carte extérieure
  ou des hordes (la partie 1 a déjà divisé le coût par 7 à 13) ;
- R5 (identifiants et places) : décidé en 82.5, rattachée à R4 (voir ci-dessus).

## Étape 86 (prochaine) : nom du skill lancé et « Provoqué »

(Anciennement étape 83.) Le nom du skill lancé au-dessus du lanceur et « Provoqué » au-dessus des
monstres provoqués, par la même file d'événements que l'étape 79 : un type d'événement de plus
(`EvenementDeCombat`, produit par `Partie`) et un style de plus dans `TextesFlottants`. Aujourd'hui la
file ne porte que les dégâts et les soins réellement appliqués. Les autres usages de la file
(« Immunisé », sons, journal de combat, statistiques…) sont des idées, pas des décisions :
`docs/brainstorm.md`, section 5 ter.

## Fonctionnalités prévues (à choisir avec Claude.ai)

- Gambits pour choisir le comportement d'un personnage engagé (rester et tirer, se dégager, passer
  en mêlée ; ils se brancheront sur `choisirIntention` depuis l'étape 82.4, dont le dégagement de
  l'étape 78 est la règle 2) et skills qui permettent de se faufiler sans pénalité
  (`docs/brainstorm.md`).
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
