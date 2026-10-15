# Squad-ARPG — Brainstorm

Oct 14, 2026 · @Olivier

## Mode d'emploi

Ce document garde en tête les idées à développer plus tard, classées par thème. Rien ici n'est une décision : une idée passe dans `ROADMAP.md` seulement quand on la choisit en discussion. Une fois réalisée et documentée dans les autres fichiers (`docs/`, `ARCHITECTURE.md`), elle est retirée d'ici.

Chaque idée suit le même gabarit : l'idée, ce qu'elle apporte au jeu, les options possibles, les questions ouvertes et ce qu'elle touche dans le code actuel. Le tableau de la dernière section donne le statut et les prérequis de chaque idée.

Statuts utilisés : **à explorer** (idée brute), **en discussion**, **écartée** (on garde la raison).

## 1. Rythme du combat

Objectif : casser la « rotation de skills » à la WoW pour que le combat ressemble à un vrai échange de coups, irrégulier. La recharge aléatoire et les temps d'exécution et d'activation sont faits (voir `docs/combat.md`).

*Pistes liées, pas encore retenues* : vitesse relative escouade/monstres comme levier contre le kiting, ralentissement de l'escouade engagée, attaque d'opportunité au désengagement, interruption de l'activation d'un skill par les dégâts reçus, skill en attente par personnage, barre de progression d'incantation, distribution de recharge « en cloche » plutôt qu'uniforme.

### 1.1 Coup double

**L'idée.** Une arme ou un passif (un voleur, par exemple) donne une chance de frapper deux fois au lieu d'une.

**Options.**

- Chance fixe par coup (15 %) ou statistique qui augmente avec l'équipement.
- Second coup à pleins dégâts, ou réduit (50 %) pour rester équilibré.
- Second coup immédiat, ou après un court délai visible à l'écran (plus lisible).

**Questions ouvertes.**

- Le second coup peut-il lui-même déclencher un coup double ? Proposition : non, pour éviter les chaînes.
- S'applique-t-il aussi aux skills, ou seulement à l'attaque de base ?

**Dans le code.** Demande une notion qui n'existe pas encore : un modificateur venant d'une arme ou d'un passif. C'est la première « statistique » du personnage, à concevoir avec la défense (section 2).

**Équilibrage.** Dégâts moyens par seconde = dégâts × (1 + chance de coup double) ÷ recharge moyenne. Utile pour comparer deux armes.

## 2. Mécanique de défense

L'armure à rendement décroissant est faite (voir `docs/combat.md`). Restent d'autres façons de réduire ou d'éviter un coup ; chacune ajoute une stat à équilibrer, et `ARCHITECTURE.md` demande d'en introduire une seule à la fois.

### 2.1 Les familles possibles

| Mécanisme | Fonctionnement | Avantage | Inconvénient |
| --- | --- | --- | --- |
| Esquive | chance d'éviter tout le coup | Ressenti « vrai combat », bon pour un voleur | Aléatoire, peut sembler injuste en série |
| Blocage au bouclier | chance de bloquer une partie du coup | Donne une identité au tank | Une stat de plus |
| Types de dégâts et résistances | physique, feu, froid, poison… chacun réduit par sa résistance | Profondeur à la Titan Quest, choix d'équipement | Beaucoup de contenu et d'équilibrage |

### 2.2 Lien avec l'escouade

Le blocage physique entre entités fait déjà du tank un mur. La défense en ferait aussi une **éponge** : c'est la combinaison des deux qui justifie de mettre le guerrier devant et le mage derrière.

### 2.3 Bouclier qui déséquilibre

Un bloc réussi a une chance d'allonger la prochaine recharge de l'attaquant. C'est une défense « active » qui ne réduit pas les dégâts mais le rythme adverse. Demande que la recharge d'un `Skill` puisse être modifiée de l'extérieur.

### 2.4 Types de dégâts dans le code

Avec les types de dégâts, `recevoirDegats` recevrait une petite structure `Degats { montant, type }` au lieu d'un entier.

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

Le donjon en salles et couloirs est en cours (étapes 59 à 61, voir `docs/generation.md`). Restent d'autres pistes.

**L'idée.** Deux générateurs, comme dans Diablo 3 : une carte extérieure avec des entrées de donjons, et des donjons générés. Diablo 3 n'est pas purement aléatoire : il assemble des morceaux dessinés à la main, ce qui garde un niveau de qualité constant.

### 4.1 Autres techniques pour les donjons

| Technique | Résultat typique | Difficulté |
| --- | --- | --- |
| Découpage BSP | Salles bien réparties, sans chevauchement | Faible à moyenne |
| Automate cellulaire | Grottes organiques | Faible |
| Assemblage de modules pré-faits (Diablo 3) | Qualité « faite main », variété contrôlée | Moyenne, demande de créer les modules |
| Wave Function Collapse | Motifs cohérents à partir d'exemples | Élevée |

Les salles non rectangulaires (rondes, en L, grottes) ne changeraient que `creuserSalle`.

### 4.2 Carte extérieure

- Carte fixe avec des points d'entrée de donjons tirés au hasard (le plus simple).
- Terrain généré par du bruit (Perlin) : plaines, forêts, rivières.
- Assemblage de grandes tuiles extérieures pré-faites, comme pour les donjons.

### 4.3 Ce que le générateur doit aussi décider

- Placement des monstres et de leurs territoires, aucun dans la salle de départ (étape 61).
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
| Classes bien définies, une maîtrise par personnage, synergies | Direction « chef d'orchestre » | Sera la source des passifs (1.1) et des défenses (2) |

## 6. Suivi

Les idées partagent des fondations. Le générateur aléatoire injecté (utile au coup double, à l'esquive et à la génération) existe depuis l'étape 56, et les `recevoirDegats` de `Personnage` et `Monstre` servent de point d'entrée unique des dégâts.

| Idée | Statut | Prérequis | Taille |
| --- | --- | --- | --- |
| 3. Gambits, niveau A (bascule auto) | à explorer | aucun | Moyenne |
| 1.1 Coup double | à explorer | notion de passif ou d'arme | Moyenne |
| 2.3 Bouclier qui déséquilibre | à explorer | blocage au bouclier, recharge modifiable | Petite |
| Esquive, blocage au bouclier | à explorer | stats par personnage | Petite |
| Types de dégâts et résistances | à explorer | classes définies | Grosse |
| 3. Gambits, niveaux B et C | à explorer | niveau A, état de l'escouade | Grosse |
| 4. Autres techniques de donjon, carte extérieure | à explorer | donjon en salles et couloirs | Moyenne |

L'ordre du tableau est une suggestion, pas un plan : chaque idée repasse par la discussion avant d'entrer dans `ROADMAP.md`.
