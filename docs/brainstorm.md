# Squad-ARPG — Brainstorm

Oct 14, 2026 · @Olivier

## Mode d'emploi

Ce document rassemble les idées à explorer, classées par thème. Rien ici n'est une décision : une idée passe dans `ROADMAP.md` seulement quand on la choisit en discussion.

Chaque idée suit le même gabarit : l'idée, ce qu'elle apporte au jeu, les options possibles, les questions ouvertes et ce qu'elle touche dans le code actuel. Le tableau de la dernière section donne le statut et les prérequis de chaque idée.

Statuts utilisés : **à explorer** (idée brute), **en discussion**, **retenue** (inscrite dans `ROADMAP.md`), **écartée** (on garde la raison).

## 1. Rythme du combat

Objectif commun : casser la « rotation de skills » à la WoW pour que le combat ressemble à un vrai échange de coups, irrégulier.

### 1.1 Recharge aléatoire

**L'idée.** Au lieu d'une recharge fixe (un coup toutes les 1,5 s), on tire une nouvelle recharge à chaque utilisation, par exemple entre 1,0 et 2,0 s.

**Options de tirage.**

| Option | Exemple | Ressenti |
| --- | --- | --- |
| Uniforme entre min et max | 1,0 à 2,0 s | Très irrégulier, les extrêmes sont aussi fréquents que le milieu |
| Base ± variation en % | 1,5 s ± 20 % | Facile à régler par skill avec un seul nombre |
| Distribution « en cloche » bornée | centrée sur 1,5 s | Surtout près de la moyenne, rares coups très rapides ou très lents |

**Questions ouvertes.**

- Aléa pour tout, ou seulement pour les attaques de base ? Un skill déclenché par le joueur avec une recharge imprévisible peut frustrer : il veut planifier son prochain soin.
- Tirer la recharge au lancement (la barre de skills affiche alors un temps exact) plutôt qu'à la fin.
- Lien avec la défense : un bouclier qui a une chance d'allonger la recharge de l'attaquant (voir 2.4).

**Dans le code.** `Skill` stockerait une recharge de base et une variation au lieu d'une valeur fixe. Le tirage demande un générateur aléatoire.

*Étape 56 (retenue) : tirage uniforme dans un intervalle `[min, max]` (et non base ± %), tiré au déclenchement, seulement pour les attaques de base (mêlées 0,3–0,7 s, tireurs 0,4–0,6 s, monstres 0,7–1,3 s, mêmes moyennes qu'avant) ; les skills des touches 1 à 4 restent fixes (`min == max`, aucun tirage). Générateur `std::mt19937` possédé par `Partie`, passé par référence ; graine tirée par `std::random_device` et affichée dans la console. La distribution en cloche reste à explorer.*

**Notion C++ : `<random>`.** On utilise `std::mt19937` avec une distribution (`std::uniform_real_distribution`), jamais `rand()`. Le générateur doit être fourni de l'extérieur (injection) avec une graine fixe dans les tests, sinon les tests de combat deviennent non reproductibles.

### 1.1 bis Temps d'exécution (étape 57, retenue)

Chaque attaque de base a un temps d'exécution pendant lequel l'attaquant est immobile : pour engager un coup il faut être immobile, et le coup engagé part à la fin de l'exécution (0,15 s pour un personnage, 0,3 s pour un monstre), même si la cible a bougé. Annulation sans recharge par la mort de la cible ou de l'attaquant, et, pour un personnage, par un ordre du joueur ou un skill. Indice visuel : le carré s'éclaircit. Voir `docs/combat.md`.

*Pistes liées, non retenues pour l'instant* : vitesse relative escouade/monstres comme levier contre le kiting, ralentissement de l'escouade engagée, attaque d'opportunité au désengagement, temps d'activation des skills 1 à 4 (retenu, voir 1.1 ter).

### 1.1 ter Temps d'activation des skills (étape 58, retenue)

Les skills des touches 1 à 4 passent par la même machine à états que l'attaque de base : engagés, ils s'activent (dégâts 0,3 s, provocation 0,2 s, soin 0,6 s, zone 0,8 s) pendant que le lanceur reste sur place, puis l'effet part, évalué à ce moment. L'appui est un ordre (il n'exige pas d'être immobile) ; seuls la mort du lanceur ou de la cible annulent l'activation, sans recharge. Un ordre de déplacement, un clic au sol ou un verrouillage ne l'annulent pas : le skill est l'ordre le plus récent. Indices visuels : le carré s'éclaircit, l'icône a un contour blanc. Voir `docs/combat.md`.

*Pistes liées, non retenues pour l'instant* : interruption de l'activation par les dégâts reçus, skill en attente par personnage, barre de progression d'incantation.

### 1.2 Coup double

**L'idée.** Une arme ou un passif (un voleur, par exemple) donne une chance de frapper deux fois au lieu d'une.

**Options.**

- Chance fixe par coup (15 %) ou statistique qui augmente avec l'équipement.
- Second coup à pleins dégâts, ou réduit (50 %) pour rester équilibré.
- Second coup immédiat, ou après un court délai visible à l'écran (plus lisible).

**Questions ouvertes.**

- Le second coup peut-il lui-même déclencher un coup double ? Proposition : non, pour éviter les chaînes.
- S'applique-t-il aussi aux skills, ou seulement à l'attaque de base ?

**Dans le code.** Demande une notion qui n'existe pas encore : un modificateur venant d'une arme ou d'un passif. C'est la première « statistique » du personnage, à concevoir avec la défense (section 2).

**Équilibrage.** Avec ces deux idées, les dégâts moyens par seconde = dégâts × (1 + chance de coup double) ÷ recharge moyenne. Utile pour comparer deux armes.

## 2. Mécanique de défense

Question de fond : comment un coup reçu est-il réduit ou évité ? Les mécanismes ci-dessous se combinent, mais chacun ajoute une stat à équilibrer. Principe d'`ARCHITECTURE.md` : en introduire un seul à la fois.

### 2.1 Les familles possibles

| Mécanisme | Fonctionnement | Avantage | Inconvénient |
| --- | --- | --- | --- |
| Armure plate | dégâts − armure | Très simple, lisible | Les petits coups tombent à 0 ; l'armure devient vite toute-puissante |
| Armure en % | dégâts × (1 − réduction) | Simple, jamais 0 | Proche de 100 % = invincible, il faut un plafond |
| Armure à rendement décroissant | réduction = armure ÷ (armure + K) | Chaque point compte, jamais 100 % | Moins intuitive pour le joueur |
| Esquive | chance d'éviter tout le coup | Ressenti « vrai combat », bon pour un voleur | Aléatoire, peut sembler injuste en série |
| Blocage au bouclier | chance de bloquer une partie du coup | Donne une identité au tank | Une stat de plus |
| Types de dégâts et résistances | physique, feu, froid, poison… chacun réduit par sa résistance | Profondeur à la Titan Quest, choix d'équipement | Beaucoup de contenu et d'équilibrage |

### 2.2 Recommandation de départ

Commencer par **l'armure à rendement décroissant**, symétrique pour personnages et monstres. Elle règle les défauts des deux autres armures et ne demande aucun aléa. L'esquive ou le blocage viendraient ensuite, puis les types de dégâts quand il y aura plusieurs classes.

### 2.3 Lien avec l'escouade

Le blocage physique (étapes 32 à 34) fait déjà du tank un mur. La défense en ferait aussi un **éponge** : c'est la combinaison des deux qui justifie de mettre le guerrier devant et le mage derrière.

### 2.4 Bouclier qui déséquilibre

Idée liée à 1.1 : un bloc réussi a une chance d'allonger la prochaine recharge de l'attaquant. C'est une défense « active » qui ne réduit pas les dégâts mais le rythme adverse. Demande que la recharge d'un `Skill` puisse être modifiée de l'extérieur.

### 2.5 Dans le code

*Étape 55 : l'armure à rendement décroissant est faite (`appliquerArmure`, module `Defense`, K = 100) ; elle s'applique dans `Personnage::recevoirDegats` et `Monstre::recevoirDegats`, qui servent de point d'entrée. Le texte ci-dessous reste valable pour les types de dégâts.*

Aujourd'hui, les dégâts sont soustraits directement aux points de vie. Avant toute défense, il faut **un seul point d'entrée** pour recevoir un coup (une fonction comme `subirDegats`), où la formule viendra se loger. Avec les types de dégâts, ce point recevrait une petite structure `Degats { montant, type }` au lieu d'un entier.

**Questions ouvertes.** L'armure vient-elle de l'équipement ou de la classe ? Les monstres ont-ils les mêmes mécanismes ?

## 3. Automatisation de l'escouade (gambits)

**L'idée.** Laisser le joueur décider quels skills partent tout seuls, à la manière des gambits de Final Fantasy XII : une liste ordonnée de règles « condition → action », évaluées de haut en bas, la première valide s'exécute.

**Pourquoi c'est central.** C'est la réponse directe à la question restée ouverte à l'étape 36 : comment gérer 4 personnages sans micro-gestion ni jeu trop automatique. L'autodéfense (étape 37) est déjà une première règle codée en dur.

### 3.1 Trois niveaux d'ambition

| Niveau | Ce que le joueur règle | Exemple | Coût |
| --- | --- | --- | --- |
| A. Bascule auto | un interrupteur par skill | « Coup puissant : auto » = lancé dès qu'une cible est à portée | Petit |
| B. Condition par skill | un seuil simple | « Soin : auto si un allié < 50 % PV » | Moyen |
| C. Liste de gambits | plusieurs règles ordonnées par personnage | 1. allié < 30 % → soin ; 2. ennemi qui attaque le mage → provocation ; 3. sinon attaque | Gros (logique + interface) |

**Recommandation.** Commencer par A, qui se branche sur les skills existants, puis passer à B si le besoin se fait sentir. C est l'aboutissement, mais son interface de configuration est un projet en soi.

### 3.2 Tension de design

La direction « chef d'orchestre » veut que le joueur garde un rôle. Si tout est automatisé, il regarde le combat. Piste : les skills de base passent en auto, les skills à forte valeur (zone, provocation) restent au clavier.

### 3.3 Dans le code

- Une règle = une condition + une action. Pour commencer, la condition peut être un `enum class` + un paramètre (comme le type de skill de l'étape 36), plutôt que des lambdas ou de l'héritage.
- Les règles sont évaluées à chaque mise à jour du personnage, après les ordres du joueur.

**Notion de génie logiciel.** C'est le patron « règle » (ou système de production) : séparer *quoi décider* (les données des règles) de *comment l'exécuter* (le moteur qui les évalue). Les règles deviennent alors configurables sans toucher au code.

**Questions ouvertes.** Un ordre du joueur suspend-il les règles, et combien de temps ? Les règles sont-elles propres à chaque personnage ou à sa classe ?

## 4. Génération procédurale

**L'idée.** Deux générateurs, comme dans Diablo 3 : une carte extérieure avec des entrées de donjons, et des donjons générés. Diablo 3 n'est pas purement aléatoire : il assemble des morceaux dessinés à la main, ce qui garde un niveau de qualité constant.

### 4.1 Techniques pour les donjons

| Technique | Résultat typique | Difficulté |
| --- | --- | --- |
| Salles placées au hasard + couloirs | Donjon classique, salles rectangulaires | Faible (le A\* existant peut tracer les couloirs) |
| Découpage BSP | Salles bien réparties, sans chevauchement | Faible à moyenne |
| Automate cellulaire | Grottes organiques | Faible |
| Assemblage de modules pré-faits (Diablo 3) | Qualité « faite main », variété contrôlée | Moyenne, demande de créer les modules |
| Wave Function Collapse | Motifs cohérents à partir d'exemples | Élevée |

### 4.2 Carte extérieure

- Carte fixe avec des points d'entrée de donjons tirés au hasard (le plus simple).
- Terrain généré par du bruit (Perlin) : plaines, forêts, rivières.
- Assemblage de grandes tuiles extérieures pré-faites, comme pour les donjons.

### 4.3 Recommandation de départ

Commencer par un **donjon en salles + couloirs** qui remplit la `TileMap` existante, avec une **graine** : la même graine redonne le même donjon, ce qui rend les bogues reproductibles et les tests possibles. Le même générateur aléatoire injecté qu'en 1.1 sert ici.

### 4.4 Ce que le générateur doit aussi décider

- Placement des monstres et de leurs territoires (étape 25), aucun dans la salle de départ.
- Validité : toutes les salles reliées, vérifiable avec le A\*.
- Taille de carte raisonnable pour garder le A\* rapide.

**Lien avec Unreal Engine.** Un générateur écrit comme une logique pure sur une grille, sans SFML, se transporterait facilement vers un autre moteur. C'est un argument pour le garder séparé du rendu.

## 5. Idées notées en cours de route

Idées lancées pendant les étapes précédentes, pas encore dans `ROADMAP.md`.

| Idée | Origine | Note |
| --- | --- | --- |
| État de l'escouade (`enum` : en combat, en mouvement, immobile, au repos) | Étape 40 | Prérequis naturel de la mana et des gambits |
| Mana qui se régénère hors combat | Étape 40 | Dépend de l'état « en combat » |
| Caméra qui zoome pour garder le groupe à l'écran | Réflexion de gameplay (étape 26) |  |
| Brouillard de guerre | Réflexion de gameplay (étape 26) | Pousse à garder le groupe uni |
| Skills pour bousculer un adversaire ou se faufiler (voleur) | Étape 31 | Exceptions au blocage entre entités |
| A\* qui tient compte des entités | Étape 34 | Moins de personnages coincés derrière un monstre |
| Choisir soi-même la cible d'un soin | Étape 36 | Aujourd'hui, le plus faible est choisi |
| Passage à Unreal Engine pour la 3D | Réflexion du 10 octobre | Décision lointaine, garder la logique séparée du rendu |
| Classes bien définies, une maîtrise par personnage, synergies | Direction « chef d'orchestre » | Sera la source des passifs (1.2) et des défenses (2) |

## 6. Suivi et ordre suggéré

Plusieurs idées partagent des fondations : un **générateur aléatoire injecté** (1.1, 1.2, esquive, génération) et un **point d'entrée unique des dégâts** (défense, coup double). Les poser d'abord évite de refaire le travail.

| Idée | Statut | Prérequis | Taille |
| --- | --- | --- | --- |
| Générateur aléatoire injecté (graine) | **retenue** (étape 56) | aucun | Petite |
| 1.1 Recharge aléatoire | **retenue** (étape 56) | générateur aléatoire | Petite |
| Point d'entrée unique des dégâts | à explorer | aucun | Petite |
| 2. Armure à rendement décroissant | **retenue** (étape 55) | point d'entrée des dégâts (les `recevoirDegats` de `Personnage` et `Monstre` en tiennent lieu) | Petite |
| 3. Gambits, niveau A (bascule auto) | à explorer | aucun | Moyenne |
| 1.2 Coup double | à explorer | générateur, notion de passif ou d'arme | Moyenne |
| 2.4 Bouclier qui déséquilibre | à explorer | 1.1, blocage au bouclier | Petite |
| 4. Donjon salles + couloirs | à explorer | générateur aléatoire | Moyenne |
| Types de dégâts et résistances | à explorer | armure, classes définies | Grosse |
| 3. Gambits, niveaux B et C | à explorer | niveau A, état de l'escouade | Grosse |

L'ordre du tableau est une suggestion, pas un plan : chaque idée repasse par la discussion avant d'entrer dans `ROADMAP.md`.
