# Squad-ARPG — Roadmap

Version courte, importée par `CLAUDE.md` à chaque session. Ailleurs :
- étapes terminées, liste et détail : `docs/historique.md` ;
- état détaillé et limites connues de chaque sujet : section « État et limites à la fin de
  l'étape 80 » (et « Limites connues ») du doc de `docs/` du sujet (tableau à la fin
  d'`ARCHITECTURE.md`).

## Étapes terminées

Étapes 0 à 99 ✅. Les dernières :
- Étape 81 — Dossiers et documentation ✅
- Étape 82 — `Escouade` allégée : `Vitalite`, `Formation`, `PlacesDAttaque`, `Intention` ✅
- Étape 83 — Inventaire et équipement : le modèle, sans interface ✅
- Étape 84 — Inventaire : l'écran en lecture (avec R6, D4, P2) ✅
- Étape 85 — Inventaire : le glisser-déposer ✅
- Étape 86 — Nom du skill lancé et « Provoqué » ✅
- Étape 87 — Moteur du coup : échelle × 10, point d'entrée unique, armure M1 ✅
- Étape 88 — Caractéristiques : cinq valeurs, Endurance vers PV, Puissance vers dégâts et soins ✅
- Étape 89 — Le critique : statistiques de combat, capacités offensive et défensive, soins critiques (avec E1) ✅
- Étape 90 — Esquive et blocage : nature des attaques, esquive plafonnée, bouclier, textes « Esquivé » et « Bloqué » ✅
- Étape 91 — Les armes : l'attaque de base et la zone de contrôle viennent de l'arme, échelle × 3, deux mains, skills qui exigent une arme ✅
- Étape 92 — Seuils d'équipement, objets inactifs, pénétration d'armure (la dague) ✅
- Étape 93 — Les types de monstres : nuée, infanterie, infanterie lourde, créature monstrueuse, groupes en données ✅
- Étape 94 — Types de dégâts et résistances : le coup en portions (physique, feu, sacré), l'explosion en feu, la frappe sacrée, premier skill de frappe ✅
- Étape 95 — Les objets ont leurs propres données (rareté, préfixe et suffixe) ; le texte du jeu passe à l'anglais ✅
- Étape 96 — La génération aléatoire des objets : plages de valeurs des affixes, poids des raretés, `genererObjet`, F7 tire au hasard ✅
- Étape 97 — Le butin au sol : les monstres lâchent des objets, ils se voient et se ramassent, on peut les jeter ✅
- Étape 98 — Les étages : l'escalier, l'expédition (équipement et sac gardés), des monstres plus forts avec la profondeur, la victoire au 5e étage ✅
- Étape 99 — L'expérience et les niveaux : un niveau d'escouade, la croissance automatique de deux caractéristiques par classe, les points de vie par niveau ✅

## État actuel (résumé)

- **Outils** : conception dans Claude.ai, implémentation avec Claude Code dans VSCode (règles dans
  `CLAUDE.md`) ; dépôt Git `squad-arpg`, poussé sur GitHub ; SFML 3.0.2 intégrée à CMake.
- **Code** (étape 99) : 101 modules rangés en 10 dossiers par domaine (`base`, `rendu`, `monde`,
  `combat`, `objets`, `escouade`, `monstres`, `jeu`, `interface`, `app`), `#include` à chemin complet, règles
  de dépendance vérifiées (`ARCHITECTURE.md`) ; tests en miroir. Presets `debug` et `jeu`, 0
  avertissement, 1 661 tests verts sous CTest, 39 avertissements `clang-tidy` avant l'étape 98 (non recomptés ; aucun nouveau dans les fichiers des étapes 98 et 99 : `docs/build.md`,
  `docs/audit.md`). `Escouade` n'est plus que l'assemblage : `Escouade.cpp` de 1 223 à 1 067 lignes. Les tests ne dépendent pas du catalogue des classes (`docs/tests.md`). **Tout le texte affiché est en anglais** (identifiants, commentaires et documentation restent en français ; pas de système de traduction : `docs/glossaire.md` est la référence des noms du jeu). Un commit par sous-étape, fait par Claude Code (`CLAUDE.md`).
- **Écrans** : menu à la souris, options d'affichage (fenêtré ou plein écran, résolutions,
  `reglages.ini`), choix de 3 classes parmi 5 (guerrier, prêtre, mage, paladin, rôdeur ; doublons
  permis), puis la partie ; R après une victoire ou une défaite ramène au choix. La pile d'écrans
  dessine sous un écran qui laisse voir dessous (`laisseVoirDessous`) ; le `clear` est dans
  l'`Application` (`docs/ecrans.md`).
- **Étages et expédition** (étape 98) : une partie est **un étage** d'une `Expedition` (profondeur de 1 à 5, graine, les trois
  personnages avec leur classe et leur équipement, le sac, l'expérience totale) ; l'expédition est ce qui survit d'un
  étage à l'autre, et rien d'autre. Un **escalier** (une case du terrain, dessinée en marches, cachée sous l'inconnu comme le sol)
  est placé dans la salle la plus éloignée du départ, celle de la créature, jamais sous elle ; cliquer dessus (après un monstre
  et un objet au sol, avant une porte) envoie l'escouade le prendre, et quand l'ancre arrive à moins de 30 px **la partie suivante
  se construit** : nouvelle carte (graine dérivée de celle de l'expédition et de la profondeur), personnages **à pleine vie, morts
  compris**, même équipement et même sac, monstres **renforcés** (points de vie +25 %, dégâts +15 %, capacités +10 par étage au-delà du
  premier). On peut descendre à tout moment, même en plein combat. **La victoire est de prendre l'escalier du 5e étage** (tuer
  les monstres ne la donne plus) ; R ramène au choix après une victoire ou une défaite, et « Start » ou F5 fondent une nouvelle
  expédition au 1er étage (équipement de départ, **sac vide** depuis l'étape 99, niveau 1). « Depth N / 5 » s'affiche avec le
  compteur de monstres ; F8 descend d'un étage tout de suite (`docs/boucle-et-entrees.md`, `docs/generation.md`, `docs/monstre.md`).
- **Expérience et niveaux** (étape 99) : **un seul niveau pour toute l'escouade**. L'expédition ne garde que l'**expérience
  totale** ; le niveau s'en déduit (`niveauPour`, `combat/Progression`), il n'est jamais stocké. **Toute mort de monstre rapporte son
  expérience** (une donnée de `DefinitionDeMonstre` : nuée 3, infanterie 10, infanterie lourde 25, créature 100, majorée de 25 % par
  étage au-delà du premier), quel que soit le tueur, même si des personnages sont morts ; elle survit à la descente. La courbe est
  `k × n × (n − 1)` avec **`k = 21`** (calibré : tous les monstres tués, niveau 3 à 4 après le 1er étage, 10 à 11 après le 5e).
  **Chaque niveau donne +1 point, en alternance dans les deux caractéristiques de la classe** (guerrier et paladin : Might puis
  Constitution ; prêtre : Might puis Resolve ; mage : Might puis Intellect ; rôdeur : Dexterity puis Might) et **8 points de vie
  maximum** (`240 + 6 × Endurance + 8 × (niveau − 1)`). Les caractéristiques de base sont **calculées** (classe plus croissance),
  plus copiées dans l'expédition. Une montée en cours d'étage recalcule tout par le chemin de l'équipement : un vivant gagne autant
  de points de vie que son maximum, un mort reste mort, plusieurs niveaux d'un coup s'appliquent tous, un seuil d'équipement
  atteint remet l'objet en service. En haut de l'écran : « Level N » et une barre d'expérience, « Level up! » flottant, « Level N »
  dans l'inventaire ; **F9** donne le niveau suivant. L'Intellect et la Resolve n'ont encore aucun effet (`docs/classes-et-combat.md`,
  « L'expérience et les niveaux »).
- **Carte** : donjon généré à chaque partie (salles, couloirs en L de 2 cases, portes à chaque
  jonction, un groupe de monstres par salle sauf la première, un escalier, graine affichée), F5 = nouvelle expédition ;
  carte à la main pour les tests (`docs/generation.md`, `docs/carte-et-collision.md`).
- **Vision** : brouillard de guerre (inconnu, exploré, visible) par shadowcasting symétrique, la
  même règle que la ligne de vue du combat ; F6 le coupe à l'affichage (`docs/vision.md`).
- **Objets** : 10 emplacements par personnage, un sac de 40 cases pour l'escouade, un catalogue de
  19 objets dont 7 armes (épée longue, masse, dague, épée à deux mains, sceptre, bâton, arc) ; l'armure d'un
  personnage est la somme de son équipement (guerrier et paladin partent équipés : 9 ; les autres : 0) ;
  **l'attaque de base et la zone de contrôle viennent de l'arme** de la main droite (les mains nues sans arme ;
  contact : zone de 20 px, projectile : aucune), changer d'arme relance une recharge complète ; une arme à deux
  mains exige la main gauche vide et range ce qui la gêne dans le sac (sac plein : refus) ; **les objets ont des
  seuils de caractéristiques** (impossible d'équiper sous le seuil ; un objet porté dont le seuil n'est plus atteint
  est inactif : porté, sans effet, encadré en rouge) et la dague perce 30 % de l'armure ; chaque personnage a
  cinq caractéristiques (base de sa classe plus bonus de l'équipement : Puissance, Endurance, Adresse, Savoir,
  Volonté), dont l'Endurance donne des points de vie (base commune de 240 plus 6 par point : environ 300 au
  départ, échelle × 3 de l'étape 91) et la Puissance augmente dégâts et soins (3 % par point au-dessus de 10),
  l'Adresse donne les capacités offensive et défensive (`50 + 5 × Adresse`) ; des objets donnent aussi un
  bonus de combat (bottes de cuir : esquive contre la mêlée ; casque de cuir : esquive contre les projectiles) et
  le bouclier rond un blocage (30 %, 12 points, récupération commune de 3 s) ; **un objet porte sa rareté et au plus un
  préfixe et un suffixe, avec leur valeur** (Common : aucun ; Magic : un ; Rare : un préfixe et un suffixe ; douze
  affixes : feu, sacré, pénétration, armure, une caractéristique, esquives, blocage ; nom composé « Keen Bow of the Fox »,
  couleur de rareté blanc, jaune, vert) dont les effets entrent dans les totaux de l'équipement ; **`genererObjet` tire un
  objet valide au hasard** (rareté 70 / 25 / 5, un modèle qui peut la porter, affixes et valeurs dans la plage de chaque
  affixe ; les anneaux et colliers, sans préfixe, ne sont jamais Rare ; Reinforced va aussi sur les gants et les bottes) ;
  `ramasser`, `ramasserTout` (F7 : quatre objets tirés au hasard, tout ou rien, sans tirage en cas de refus) et
  `deplacerObjet` sont transactionnelles ; **les objets existent aussi au sol** (étape 97) : un monstre qui meurt laisse,
  selon sa définition, des objets tirés par `genererButin` (nuée 5 %, infanterie 20 %, infanterie lourde 35 %, un objet ;
  créature monstrueuse 100 %, trois objets), posés sur sa case ou les cases libres voisines ; un losange de la couleur de la
  rareté les montre (caché sous l'inconnu, assombri sous l'exploré), leur nom composé s'affiche au survol, un clic gauche
  envoie l'escouade les ramasser (un monstre sous le curseur passe avant ; l'ancre à moins de 30 px les met dans le sac ; sac
  plein : « Bag full », l'objet reste au sol), et lâcher un objet du sac ou d'un emplacement hors du panneau de l'inventaire le
  jette près de l'ancre (le panneau est réduit depuis l'étape 98 : le jeu reste visible tout autour, le sac tient sur 2 rangées de
  20 cases ; un objet évite la case de l'escalier ; les objets au sol ne descendent pas avec l'escouade). La
  touche I ouvre l'inventaire par-dessus la partie figée : équipement des 3 personnages, sac de 40
  cases, caractéristiques, points de vie, capacités, blocage et esquives de chaque personnage, infobulle au survol, et glisser-déposer des objets (sac, emplacements, entre personnages ;
  échange si la destination est occupée ; destinations valides en surbrillance) (`docs/objets.md`,
  `docs/ecrans.md`).
- **Escouade** : une ancre suivie par 3 personnages en formation (rangs avant et arrière), qui
  pivote et se resserre près des murs ; places d'attaque autour des monstres, séparation douce,
  chemins qui contournent les monstres, dégagement des tireurs (`docs/formation.md`, `docs/chemin.md`).
- **Combat** : verrou par clic, attaque de base et zone de contrôle de l'arme, skills qui exigent un type d'arme
  (refusés sans elle, icône grisée), autodéfense, agresseurs, temps d'exécution des coups, armure à
  armure par soustraction avec plancher, recharges aléatoires, zones de contrôle et attaques d'opportunité ; skills
  (dégâts, soin, provocation, zone avec visée) avec temps d'activation et un skill en attente pour
  l'escouade ; critiques (dégâts et soins, des deux camps, selon le rapport des capacités offensive et
  défensive) ; esquive (plafond de 50 %, selon la nature de l'attaque : contact, projectile ou zone, qui ne
  s'esquive ni ne se bloque) et blocage au bouclier (chance, montant fixe, récupération, de la seule portion
  physique) ; **un coup est un ensemble de portions par élément** (physique, feu, sacré), chacune réduite par sa propre
  défense : l'armure pour le physique, une résistance en pour cent (plafond de 80 %, vulnérabilités jusqu'à −100 %) pour
  chaque élément, un seul arrondi par portion ; l'explosion du mage est du feu, la frappe sacrée du paladin est le premier
  skill de frappe (l'arme plus un bonus sacré de 45) ; **l'attaque de base est typée** (l'arme est physique, ses
  affixes y ajoutent du feu ou du sacré, la Puissance joue sur chaque portion, et la frappe sacrée prend toutes les
  portions de l'arme) ; dégâts (orange pour le feu, doré pour le sacré), soins, critiques, « Dodged », « Blocked »,
  nom du skill lancé et « Taunted » flottants (`docs/combat.md`).
- **Monstres** : détection en vue, poursuite, frappe de la proie ou du plus proche atteignable,
  alerte du groupe, retour au poste invulnérable avec régénération ; **quatre catégories** (nuée de petits
  monstres, infanterie, infanterie lourde, créature monstrueuse), chacune avec ses valeurs dans une
  `DefinitionDeMonstre` du catalogue (`docs/monstre.md`), sa couleur et sa taille ; le `Niveau` dit la catégorie de
  chaque monstre (`CategorieDeMonstre`). Le générateur peuple chaque salle sauf la première d'un groupe dont la
  composition est une donnée (nuée de 5 à 7 en grappe, 1 à 3 infanteries, ou 1 à 2 infanteries avec 1 infanterie
  lourde ; poids 30, 40, 30), plus une créature monstrueuse seule dans la salle la plus éloignée du départ
  (`docs/generation.md`). La créature n'a que ses valeurs (rayon réduit à 16, le chemin ne tenant pas compte du
  rayon contre les murs) ; ses règles propres (immunités, blocage) et l'escouade de monstres viendront plus tard.
- **Interface** : barre de raccourcis de 12 emplacements (Q W E R / A S D F / Z X C V, ou clic ;
  « l'interface d'abord »), compteur de monstres, écran de victoire ou de défaite ; F3 à F9 de
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
  - C2 (constructeurs à longues listes) : fait à l'étape 88.1 pour `Skill` et `Personnage`
    (`DefinitionDeSkill`, `DefinitionPersonnage` en initialisation désignée) ;
  - E1 (valeurs dispersées, `Monstre` en littéraux) : fait à l'étape 89.1 pour `Monstre`
    (`DefinitionDeMonstre`, `docs/audit.md`) ;
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

## Plan en cinq phases (étapes 87 à 101), décidé avec Olivier

Les mathématiques du combat sont conçues dans `docs/classes-et-combat.md`. Pour chaque étape à venir,
ce qui reste à décider **en discussion** avant son prompt :
- **Phase 1, moteur du coup**
  - 87 ✅ : résolution en étapes (`resoudreCoup`), armure M1, échelle × 10 des points de vie.
  - 88 ✅ : cinq caractéristiques (base de classe + équipement), Endurance → points de vie, Puissance →
    dégâts et soins, visibles dans l'inventaire.
  - 89 ✅ : critique (E1 en première sous-étape) : `StatistiquesDeCombat` (capacités `50 + 5 × Adresse`),
    chance et multiplicateur par le rapport des capacités, la magie et les soins critiquent, « Critique ! »
    à l'écran, capacités dans l'inventaire.
  - 90 ✅ : esquive et blocage : nature explicite des attaques, esquive plafonnée à 50 %, bonus de combat des
    objets, bouclier rond (30 %, 12 points depuis l'échelle × 3, récupération commune de 3 s), « Esquivé » et
    « Bloqué » à l'écran.
- **Phase 2, armes**
  - 91 ✅ : armes ; l'attaque de base et la zone de contrôle viennent de l'arme, échelle × 3, base de points de
    vie commune, deux mains (rangement automatique), skills qui exigent un type d'arme, blocage propriété du
    bouclier, infobulles (modèle C : pas de restriction par classe).
  - 92 ✅ : seuils d'équipement (impossible d'équiper sous le seuil, calcul à une seule fonction de décision), objets
    inactifs (point fixe), affichage en rouge, pénétration d'armure en pour cent (la dague, 30 %, plafond de 75 %).
- **Phase 3, monstres et éléments**
  - 93 ✅ : types de monstres (nuée, infanterie, infanterie lourde, créature monstrueuse), catalogue de
    définitions, catégorie dans le `Niveau`, groupes en données, créature dans la salle la plus éloignée,
    mesure de performance (inchangée à 0,04 ms par image).
  - 94 ✅ : coups en portions typées (physique, feu, sacré), résistances, blocage du seul physique, explosion en feu,
    créature monstrueuse à 40 % de résistance au feu, frappe sacrée en skill de frappe, nombres flottants colorés.
- **Phase 4, butin** (renumérotée à l'étape 95)
  - 95 ✅ : le modèle et l'affichage des objets : rareté (Common, Magic, Rare), préfixe et suffixe avec leur valeur, douze
    affixes qui ont un effet, nom composé, couleurs, infobulle, F7 ; le jeu en anglais, `docs/glossaire.md`.
  - 96 ✅ : la génération aléatoire des objets : plages de valeurs des affixes, poids des raretés (70, 25, 5), `genererObjet`,
    F7 tire quatre objets au hasard.
  - 97 ✅ : le butin au sol : chance et nombre d'objets dans `DefinitionDeMonstre`, `genererButin`, `ObjetAuSol`, losange et
    nom au survol, ramassage au clic, « Bag full », jeter en lâchant hors de l'inventaire. **La phase 4 est terminée.**
- **Phase 5, progression** (décidée à l'étape 98)
  - 98 ✅ : les étages : l'escalier, l'`Expedition` (ce qui survit d'un étage à l'autre), des monstres renforcés par la
    profondeur, la victoire au 5e étage ; en première sous-étape les dettes de l'étape 97 (panneau d'inventaire réduit,
    `ResultatClic` séparé, `ObjetsAuSol`, `EcranInventaire` sans la `Partie`).
  - 99 ✅ : l'expérience et les niveaux : en première sous-étape trois corrections de l'étape 98 (sac vide au départ, `renforcer`
    sur toutes les portions des dégâts, une seule source pour l'équipement) ; l'expérience des monstres ; l'expérience totale de
    l'expédition et `niveauPour` (`k = 21`) ; la croissance automatique par classe, les points de vie par niveau, la montée de
    niveau en cours d'étage ; l'affichage (en-tête, « Level up! », inventaire) et F9.
  - 100 : les points libres (à discuter avec Claude.ai) ;
  - 101 : le niveau d'objet.

## Étape 100 (prochaine) : les points libres et leur écran, à discuter avec Claude.ai

L'étape 99 (l'expérience et les niveaux) est faite ; la phase 5 continue par les points libres (100) et le niveau d'objet (101).
L'étape 100 ajoutera les points libres dépensés au calcul des caractéristiques de base (classe plus croissance du niveau, plus
points libres), qui est déjà une fonction pure. Le Savoir et la Volonté n'ont encore aucun effet (les paires de croissance du prêtre
et du mage deviendront justes quand ils serviront). L'`Expedition` grandira avec les points libres (`jeu/Expedition`) et c'est
elle qu'on sauvegardera un jour.

**Pistes issues de l'étape 99** (étapes à part, plus tard) :
- **Un niveau par personnage** si un recrutement arrive (aujourd'hui un seul niveau, déduit de l'expérience totale de l'escouade).
- **Un niveau maximum** (aujourd'hui aucun : la courbe est quadratique, les points de vie linéaires).
- **L'expérience refusée aux morts** (aujourd'hui toute mort de monstre rapporte à l'escouade, même si des personnages sont morts).
- **Ajuster en jouant** `k` (21), l'expérience de chaque catégorie, les 8 points de vie par niveau et les paires de croissance :
  tous « à tester ». Le calibrage de `k` laisse le 1er étage à 3 ou 4 (`docs/classes-et-combat.md`).

**Pistes issues de l'étape 98** (étapes à part, plus tard) :
- **Les refuges** : un endroit sûr où l'on reprend son souffle entre deux étages (soin, sac, achat).
- **La résurrection et le personnage à terre** : aujourd'hui un mort revient à pleine vie à l'étage suivant, sans rien coûter.
- **Conserver les points de vie en descendant** (aujourd'hui l'escouade arrive soignée sur chaque étage).
- **Interdire de descendre en combat** si la fuite par l'escalier s'avère trop facile (aujourd'hui on descend à tout moment,
  même en plein combat : voulu).
- **Des étages qui changent de taille ou de thème** (aujourd'hui tous les étages ont les mêmes paramètres de génération).
- **La sauvegarde de l'`Expedition`**.
- **Le butin qui dépend de la profondeur**, et le niveau d'objet (101).
- **Ajuster en jouant** les facteurs de renforcement (+25 % de points de vie, +15 % de dégâts, +10 de capacités par étage), la
  profondeur maximale (5) et la distance de descente (30 px) : tous « à tester ».

**Pistes issues de l'étape 97** (étapes à part, plus tard) :
- **Des tables de butin par monstre** : quels modèles tombent de quel monstre (aujourd'hui chaque objet est un `genererObjet`).
- **Une rareté minimale**, par exemple pour la créature monstrueuse (aujourd'hui 70 / 25 / 5 pour tous les objets).
- **Montrer tous les noms au sol** en maintenant une touche (Alt) : aujourd'hui le nom ne s'affiche qu'au survol.
- **Le ramassage automatique** ou une touche « tout ramasser ».
- **Ajuster en jouant** les chances de butin (valeurs « à tester ») et la distance de ramassage (30 px).
- L'or, la monnaie, la vente, les coffres, la disparition des objets avec le temps, la sauvegarde, le tri du sac et la
  comparaison d'objets restent hors de portée.

**Pistes issues de l'étape 96** (étapes à part, plus tard) :
- **Un préfixe pour les bijoux**, pour qu'ils puissent être Rare (aujourd'hui `peutPorterLaRarete` les écarte).
- **Des poids par affixe** (aujourd'hui tous égaux) et **des affixes plus forts sur les objets Rare** (même plage que Magic).
- **Le niveau d'objet**, qui ferait varier les plages et les chances de rareté (question 2 de `docs/classes-et-combat.md`).
- **Des distributions portables** si un jour la graine doit donner les mêmes objets partout (`std::uniform_int_distribution`
  n'est pas défini par la norme : `docs/objets.md`, « Génération aléatoire »).
- **Des affixes négatifs (maudits)** : ils demanderaient de revoir le point fixe des objets actifs, qui suppose des bonus
  positifs ou nuls (`objetValide` exige déjà une valeur strictement positive).

**Pistes issues de l'étape 95** (étapes à part, plus tard) :
- **Les résistances sur les objets** : un affixe de résistance (le champ existe depuis l'étape 94, dans
  `StatistiquesDeCombat`, additionné par `bonusDeCombatTotal`) et son affichage dans l'inventaire ; sans intérêt tant que les
  monstres ne frappent qu'en physique.
- **Les objets uniques** (un nom propre, des affixes fixes) et **les raretés supérieures** (plus de deux affixes) : le type
  `Objet` n'a que deux places d'affixe, il faudra une liste.
- **Le niveau d'objet**, et des affixes qui changent les seuils.
- **Une icône par objet** à la place des sigles, et le nom composé sur les emplacements.
- **Les textes d'un autre pays** : s'il faut un jour une deuxième langue, les textes sont dispersés dans les `switch` des
  `Affichage*` et dans les catalogues ; `docs/glossaire.md` en dresse la liste.

**Pistes issues de l'étape 94** (étapes à part, plus tard) :
- **Les éléments sans source** : froid, foudre, poison, nécrotique, saignement s'ajoutent à `Element` avec leur premier
  skill ou monstre (le saignement est un type de dégâts, `docs/classes-et-combat.md`, section 8) ; Psychique / Arcane est
  à revoir en créant les classes.
- **La pénétration de résistance** (par l'équipement ou des skills), et les bonus de pénétration d'armure.
- **Un bouclier anti-magie**, qui bloquerait aussi une part des portions élémentaires.
- **La frappe puissante et le tir perçant en skills de frappe** (la structure de la frappe sacrée, avec un bonus
  physique).
- **Les effets dans le temps** (saignement, poison) qui ignorent l'armure, et les immunités.

**Pistes issues de l'étape 93** (étapes à part, plus tard) :
- **La vraie escouade de monstres** (ancre, formation, places d'attaque autour de leur cible, cible commune) : elle
  demandera d'abord de déplacer `Formation` et `PlacesDAttaque` hors d'`escouade/` (`monstres/` ne peut pas les
  inclure). Les compositions de groupe (`monde/CompositionDeGroupe.hpp`) l'attendent ; une nuée n'est aujourd'hui
  qu'un groupe de petits monstres qui s'alertent ensemble.
- **Les règles propres de la créature monstrueuse** : immunités au recul et à l'étourdissement, ne pas être
  bloquée par un seul personnage (`docs/classes-et-combat.md`, section 11).
- **Un chemin qui tient compte du rayon contre les murs**, pour rétablir le rayon de 30 px de la créature (16
  aujourd'hui : un chemin par les centres des cases la bloque dans les couloirs de 2 cases, `docs/monstre.md`).
- Des comportements différents par catégorie (stratégie d'attaque, `docs/brainstorm.md`, 4 bis) et des poids et
  nombres de groupes à ajuster en jouant.

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
