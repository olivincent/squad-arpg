# Squad-ARPG — Classes, skills et mathématiques du combat

## Mode d'emploi

Document de référence issu des discussions de conception (octobre 2026). Il sert de base aux prochaines discussions sur les classes, les skills et le combat. Ce n'est pas de la conception technique : aucune classe C++, aucune formule définitive.

Chaque point porte un statut :

- **Retenu** : Olivier l'a décidé.
- **Proposé** : recommandé en discussion, pas encore confirmé.
- **À tester** : décidé dans le principe, valeur à ajuster en jouant.
- **Ouvert** : question à trancher.

Quand un point entre dans `ROADMAP.md`, sa conception détaillée se fait en discussion.

---

## 1. Vision

- **Retenu.** Monde très fantasy, style *Donjons et Dragons*.
- **Retenu.** Inspirations de gameplay : *World of Warcraft*, *Titan Quest*, *Dawn of War 2*. Inspirations de classes : WoW, D&D / *Baldur's Gate 3*, Diablo, Titan Quest, *Age of Conan*.
- **Retenu.** Un combat **plus lent et stratégique** : un « simulateur de combat » escouade contre monstres, avec peu d'ennemis qui comptent chacun. Pas un jeu à la *Diablo 4* où les monstres meurent par milliers en un coup.
- **Retenu.** Pas de pause tactique : elle casse le rythme (presque du tour par tour) et gênerait un éventuel multijoueur.
- **Principe.** Un combat long n'ennuie pas s'il est long ; il ennuie quand le joueur n'a rien à décider. Ce qui compte, c'est la **densité de décisions**. Sources de décisions visées :
  - des ennemis qui annoncent leurs coups (temps d'activation visibles) : interrompre, protéger, s'écarter ou encaisser ;
  - des compositions ennemies (escouade contre escouade) : qui tuer en premier ;
  - des fenêtres de combo entre personnages (debuff, puis gros coup) ;
  - des ressources et des recharges longues : le « quand » compte autant que le « quoi » ;
  - le positionnement (ligne de front, zones de contrôle, attaque d'opportunité).
- **Principe.** Le joueur ne contrôle jamais un individu, mais il a toujours une réponse.
- **Proposé.** Penser l'équilibrage en **temps pour tuer** plutôt qu'en points de vie. Points de départ : monstre ordinaire 5 à 10 s pour l'escouade, élite 20 à 40 s, boss 2 à 3 min.

## 2. Le temps

- **Proposé.** Une **vitesse de jeu** dans les options (lente, normale, rapide), à la *StarCraft* : un réglage de confort en solo, fixé par l'hôte en multijoueur.
- **Proposé.** La manipulation du temps comme **identité de classe** (mage du temps, couleur bleue) : elle touche des entités, pas l'horloge du jeu, donc reste compatible avec le multijoueur. Pistes : *Lenteur* (zone qui ralentit déplacement, attaques et activations ennemies), *Hâte* (un allié agit plus vite), *Stase* (un ennemi figé mais invulnérable), *Retour en arrière* (un allié revient à ses PV d'il y a quelques secondes).
- Note de structure : la manipulation du temps appellera une **échelle de temps par entité** (multiplicateur de son `dt`), plutôt que de modifier séparément chaque vitesse et chaque recharge.

## 3. Les couleurs

- **Retenu.** Les couleurs (inspirées de *Magic: The Gathering*) sont un **guide pour concevoir les skills**, pas un lien dur entre une classe et une couleur. Le code ne connaît pas les couleurs : une spécialisation « penche » vers une ou deux couleurs dans sa conception.
- **Retenu.** Point de départ (d'autres couleurs possibles) :

| Couleur | Identité | Mécaniques | Faiblesse voulue (proposé) |
|---|---|---|---|
| **Blanc** | Sacré, ordre, protection | Soin direct, buffs, armure | Peu de dégâts bruts, peu de contrôle |
| **Noir** | Ombre, sacrifice | Dégâts de vitalité, vol de vie, debuffs (warlock, chevalier de la mort) | Peu de protection pour les alliés |
| **Rouge** | Feu, terre, fureur | Dégâts directs, feu, terre | Aucun soin, peu de défense |
| **Vert** | Nature, forêt | Archers, poison, soin dans le temps | Peu de magie instantanée, contrôle limité |
| **Bleu** | Esprit, tempête | Protection magique, instantané, froid et foudre, contrôle, psychique | Dégâts directs faibles, fragile |

- **Proposé.** Ce qu'une couleur ne sait **pas** faire compte autant que ce qu'elle fait : c'est le manque qui force la synergie entre les 3 personnages.
- **Proposé.** Les mots-clés de *Magic* comme mécaniques nommées et réutilisables (le joueur les apprend une fois) :

| Mot-clé | Transposition |
|---|---|
| Lien de vie | Vol de vie (% des dégâts rendus en PV) |
| Contact mortel | Le poison exécute une cible sous X % de PV |
| Célérité | Pas de temps d'activation au premier usage |
| Flash | Instantané, utilisable pendant une autre action |
| Initiative | Temps d'exécution réduit |
| Piétinement | Les dégâts excédentaires débordent sur l'ennemi derrière |
| Vigilance | Attaquer n'immobilise pas |
| Défense talismanique | Le premier sort ennemi est annulé |
| Indestructible | Ne tombe pas sous 1 PV pendant X s |
| Menace | Doit être engagé par 2 ennemis pour être bloqué |
| Portée | Peut toucher les volants |

## 4. Les classes et la progression

- **Retenu.** Une seule classe par personnage (3 personnages, donc déjà 3 classes : plus serait trop de combinaisons). Une même classe peut être prise jusqu'à 3 fois (étape 76).
- **Retenu (dans le principe).** Une **classe de base qu'on spécialise ensuite**, par exemple un mage qui devient mage du feu ou mage du givre.
- **Retenu.** Progression à la *Titan Quest* : en montant le niveau de sa classe, on débloque des skills et on choisit lesquels prendre. On ne peut pas tout prendre, c'est ce qui rend les personnages uniques.
- **Retenu.** Pas d'arbre géant à la *Path of Exile* (intimidant, on s'y perd). La classe garde le joueur dans une lignée cohérente.
- **Proposé.** Combinaison : une barre de maîtrise à la *Titan Quest* (paliers de skills) et une fourche de spécialisation unique et définitive à un certain niveau (à la WoW / BG3). 5 classes × 2 spécialisations donnent 10 profils et 220 escouades possibles avec doublons.

## 5. Les caractéristiques

- **Retenu.** Des caractéristiques **par effet** (à la *Pillars of Eternity*), pas une stat de dégâts par classe : le joueur doit vraiment hésiter où mettre ses points.
- **Retenu.** Des noms **immersifs**, avec une infobulle qui donne les chiffres exacts.
- **Retenu.** Peu de caractéristiques (6, c'est trop).
- **Retenu.** La **vitesse** n'est pas une caractéristique (difficile à contrôler, casse le jeu). Elle vient de l'arme (intervalle de recharge propre à chaque type d'arme), de l'équipement (réduction de recharge, vitesse) et des skills (*Hâte*).
- **Proposé (base de travail).** Trois caractéristiques, chacune avec un effet offensif et un effet défensif, pour que toutes servent à toutes les classes :

| Caractéristique | Offensif | Défensif | En bref |
|---|---|---|---|
| **Vigueur** | Force des dégâts **et** des soins | Points de vie | Frapper fort |
| **Adresse** | Capacité offensive (critiques) | Capacité défensive | Frapper juste |
| **Esprit** | Taille des zones, durée des effets infligés | Résistance au contrôle et à l'interruption | Frapper large |

- **Retenu.** Trois sources : une **petite croissance automatique orientée par la spécialisation** (un mage du feu gagne plutôt de la Vigueur, un mage du givre plutôt de l'Esprit), l'**essentiel en points libres**, et des **bonus d'équipement**.
- **Retenu.** Redistribution des points **payante, seulement dans les refuges** (village, campement), jamais en pleine quête.
- **Retenu.** Seuils d'équipement à la *Titan Quest* (une arme lourde demande de la Vigueur, un bâton runique de l'Esprit).

## 6. Le déroulement d'un coup

**Proposé.** L'ordre compte autant que les nombres :

1. **Peut-il agir ?** Recharge écoulée, pas sous contrôle, activation terminée.
2. **Le coup est-il évité ?** Esquive, puis blocage (ouvert).
3. **Est-ce un critique ?** Capacité offensive contre capacité défensive.
4. **Dégâts de départ.** Arme ou skill, modifiés par la Vigueur.
5. **Absorption et réduction.** Barrière ou Égide (section 9), armure (physique) ou résistance (éléments).
6. **Effets secondaires.** Vol de vie, effets dans le temps, contrôle.

## 7. Toucher, esquive et critique

- **Proposé.** Presque tout touche : pas de jet de toucher qui rate. La variété vient des critiques et de l'esquive.
- **Retenu.** Esquive en pourcentages, à la *Titan Quest* : une contre la mêlée, une contre les projectiles.
- **Proposé.** Un plafond à l'esquive (par exemple 50 %). Plus tard, une distribution pseudo-aléatoire (à la *Dota 2*) contre les séries d'esquives.
- **Proposé.** Les projectiles magiques (boule de feu) peuvent être esquivés comme une flèche. Les zones ne s'esquivent pas en pourcentage : elles s'évitent en se déplaçant (section 10).
- **Retenu (dans le principe).** Le critique dépend de la **relation** entre la capacité offensive de l'attaquant et la capacité défensive du défenseur (à la *Titan Quest* / *Grim Dawn*), pas d'un pourcentage fixe de l'attaquant. Plus l'écart est grand, plus le critique est fréquent et fort.

## 8. Armure, pénétration et résistances

- **Retenu.** Un modèle **mixte** soustraction / pourcentage, avec de la **pénétration d'armure**.
- **Retenu.** Une réduction ne ramène **jamais** un coup à zéro. Seuls deux événements l'annulent, et ils sont affichés : le **blocage au bouclier** (« Bloqué ») et l'**esquive** (« Esquivé »).
- **Proposé.** Modèle M1 : `dégâts = max(D − armure, D × plancher)`. L'armure retire un montant fixe par coup, le coup garde toujours au moins un pourcentage. Lisible (« retire 15 par coup, jamais plus de 80 % ») et à effets émergents : la dague s'effondre contre l'infanterie lourde, l'épée à deux mains tient ; une nuée ne fait presque rien au tank, donc elle cherche à le contourner. Il remplacerait le modèle de l'étape 55 (pourcentage à rendement décroissant, K = 100), avec le même plancher à 1.
  - Alternative écartée pour l'instant : M2 à la *Path of Exile*, `réduction = A / (A + 5 × D)`, plus douce mais moins lisible.
- **À tester.** Plancher de départ : 20 %. C'est lui qui décide si une nuée est « inoffensive » ou « gênante » contre un tank.
- **Proposé.** Pénétration d'armure **en %** (ignore X % de l'armure) : identité des armes de spécialiste (estoc, dague de miséricorde, arbalète, pic de guerre).
- **Retenu.** Les effets dans le temps (saignement, poison…) **ignorent l'armure** ; seules les résistances s'appliquent. Le saignement devient une réponse naturelle à l'infanterie lourde.
- **Proposé.** L'armure ne protège **que contre le physique** (intuition d'Olivier). Chaque élément a sa **résistance en %**, plafonnée (par exemple 75 %). Une **pénétration de résistance** pourra exister pour les sorts, gagnée par l'équipement ou les skills, pas propre à toute la magie.
- **Principe.** On ne peut pas se protéger contre tout, et c'est voulu :
  - l'équipement a des profils (plate : beaucoup d'armure, peu de résistances ; robe enchantée : l'inverse ; cuir : entre les deux) ;
  - les couleurs ont un rôle défensif (blanc : armure ; bleu : protection magique) ;
  - les ennemis ont un profil (nid de chamans du feu, chevaliers en plate) ;
  - les points de vie restent la défense universelle.

## 9. Barrières (à la *Divinity: Original Sin 2*)

- **Retenu.** Garder le principe de *Divinity 2* : des réserves de protection physique et magique en plus des points de vie.
- **Retenu.** Les intégrer comme **effets temporaires** donnés par des skills et un peu d'équipement, pas comme système de base pour tout le monde (qui doublerait nos défenses et pousse, dans *Divinity 2*, aux groupes tout physiques ou tout magiques). Noms de travail : **Égide** (physique, blanc) et **Barrière** (magique, bleu). Exemple d'équipement : une amulette rare donnant une petite Barrière qui se recharge hors combat.
- **Proposé.** Garder la règle forte de *Divinity 2* : **tant qu'une réserve tient, le contrôle de son type ne passe pas.** Le combat devient « percer la défense, puis contrôler ».
- Note : « armure » désigne déjà la réduction par soustraction ; ces réserves ont besoin d'un autre nom.

## 10. Les zones et l'escouade

**Le problème (Olivier).** Le joueur ne contrôle pas les personnages individuellement. Si un sort de zone vise le mage à l'arrière, il faudrait bouger toute l'escouade pour l'éviter.

**Retenu.** Trois réponses combinées comme principes de conception des monstres :

- **a. Les grosses zones visent le groupe** et s'évitent en déplaçant l'escouade entière, comme les grenades de *Dawn of War 2* : larges, bien annoncées (cercle au sol pendant l'activation du monstre).
- **b. Les petites zones sont évitées par réflexe** : un personnage dans une zone annoncée fait un pas pour en sortir, dans la limite de sa laisse autour de sa place en formation (étape 40). Le réflexe pourrait dépendre d'un trait de classe (un voleur réagit vite, un chevalier en plate pas du tout).
- **c. Les sorts dangereux ciblés sur un personnage se gèrent sans bouger** : interrompre le lanceur, protéger la cible (Barrière, Égide), ou encaisser et soigner.
- **d. Plus tard, si le besoin s'en fait sentir :** un ordre de formation resserrée ou dispersée. Dispersée, elle encaisse moins les zones, mais les auras touchent moins de monde et la ligne de front tient moins bien.

## 11. Catégories d'ennemis (à la *Warhammer 40k*)

- **Retenu (dans le principe).** Des catégories d'ennemis : **nuée**, **infanterie**, **infanterie lourde**, **créature monstrueuse** (peut-être **commandant** pour les boss). Par exemple, des attaques au couteau rapides et peu puissantes, très bonnes contre une nuée, moins contre l'infanterie lourde.
- **Proposé.** Faire **émerger** ces effets des règles plutôt que d'une grande table de multiplicateurs à la *Dawn of War 2* :
  - l'armure par soustraction (section 8) rend les petits coups faibles contre l'infanterie lourde ;
  - une nuée a peu de PV : les gros coups y gaspillent leurs dégâts en excédent, pas les petits ;
  - une créature monstrueuse est immunisée au recul et à l'étourdissement, et ne peut pas être bloquée par un seul personnage.
  - Une petite table de multiplicateurs seulement pour les cas que les règles n'expliquent pas.
- Deux axes indépendants : la **catégorie** dit quel corps a la cible ; les **résistances** disent de quoi est faite l'attaque.

## 12. La grammaire des skills

**Proposé.** Un skill se décrit par cinq axes indépendants :

1. **Ciblage** : soi, un allié, un ennemi, un point au sol, une zone autour du lanceur, un cône, une ligne, une chaîne, un projectile.
2. **Déclenchement** : activation (interruptible), instantané, canalisé, bascule (aura), passif, réaction (« quand je suis touché », « quand un allié tombe sous 30 % », lien direct avec les gambits).
3. **Durée de l'effet** : immédiat, dans le temps (DoT, HoT), persistant au sol, posé sur une cible, permanent tant que la source existe.
4. **Effet** : dégâts (typés), soin, absorption (Barrière, Égide), modification de statistique, contrôle, déplacement (charge, recul, téléportation), invocation, menace.
5. **Coût** : recharge, mana, ressource de classe, ou points de vie (identité du noir).

Note de structure : un skill deviendra probablement une **composition** (un ciblage, un déclenchement, une liste d'effets) plutôt qu'un `enum` et un `switch` qui grossissent à chaque nouveau skill. Extension connue, à accueillir le moment venu.

Points de conception associés (**proposés**) :

- **Contrôle de foule, du plus doux au plus fort** : ralentir le déplacement ; ralentir attaques et recharges ; enraciner ; silence ou désarmement ; interrompre ; étourdir ; peur (attention aux attaques d'opportunité de l'étape 77) ; recul et attraction. Une règle contre l'enchaînement infini : rendement décroissant, immunité temporaire ou boss résistants.
- **L'interruption** est peut-être le skill le plus important pour le rôle de « chef d'orchestre » : manuel, lisible, à fort impact, à condition que les monstres lancent des sorts avec une activation visible.
- **Les auras** récompensent une escouade serrée en formation, et créent une tension avec la distance des personnages arrière. Elles **réservent** une partie du mana (à la *Diablo 2* / *Path of Exile*) au lieu de le dépenser.
- **Empilement des buffs** : un même effet ne s'empile pas, il est rafraîchi ; des effets différents s'additionnent (règle de WoW). Important avec les doublons de classe.
- **Les formes de soin ont chacune un rôle** : direct (réagit à l'urgence), dans le temps (anticipe, s'automatise bien), absorption (choix de timing), vol de vie (oblige à attaquer).
- **Instantané contre activation** : l'instantané compense par des nombres plus faibles ou une recharge plus longue ; l'activation peut être interrompue.

## 13. Inventaire des statistiques de combat

« Existe » : déjà dans le code.

**Côté attaquant**

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Dégâts de base | Montant de départ du coup | Arme, skill | Oui (fixe par skill) |
| Type de dégâts | Physique, feu, froid, foudre, poison, sacré, vitalité… | Arme, skill | |
| Intervalle de recharge | Temps entre deux coups (min-max) | Arme, skill | Oui |
| Temps d'exécution / d'activation | Durée d'immobilité de l'attaquant | Arme, skill | Oui |
| Portée | Distance bord à bord | Arme, skill | Oui |
| Capacité offensive | Chance et force du critique | Adresse, équipement | |
| Multiplicateur de critique | Force du critique | Équipement, skills | |
| Pénétration d'armure | Ignore un % de l'armure | Arme, équipement | |
| Pénétration de résistance | Ignore une part de la résistance | Équipement, skills | |
| Puissance de soin | Montant soigné | Skill, Vigueur | Oui (fixe) |
| Taille de zone | Rayon d'un skill de zone | Skill, Esprit | Oui (fixe) |
| Durée des effets | Buffs, debuffs, DoT, contrôle infligé | Skill, Esprit | |
| Vol de vie | % des dégâts rendus en PV | Équipement, skills | |
| Réduction de recharge, vitesse | Accélère les actions | Équipement, *Hâte* | |

**Côté défenseur**

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Points de vie | Survie | Vigueur, niveau, équipement | Oui |
| Régénération | PV regagnés dans le temps | Équipement, skills | |
| Armure | Réduit le physique | Équipement | Oui (en %, K = 100, à remplacer) |
| Résistances | Une par élément, en % plafonné | Équipement, skills | |
| Esquive contre la mêlée | % d'éviter un coup au contact | Équipement, skills (plafond) | |
| Esquive contre les projectiles | % d'éviter un tir | Équipement, skills (plafond) | |
| Capacité défensive | Réduit le critique subi | Adresse, équipement | |
| Résistance au contrôle | Réduit la durée du contrôle subi | Esprit, équipement | |
| Résistance à l'interruption | Chance de garder son activation | Esprit | |
| Égide / Barrière | Réserve temporaire, bloque le contrôle de son type | Skills, équipement | |
| Blocage (ouvert) | Chance, montant bloqué, récupération du bouclier | Bouclier | |

**Ressources et déplacement**

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Mana max et régénération | Budget de skills, surtout hors combat | Niveau, équipement | |
| Coût d'un skill | Mana, ou PV pour le noir | Skill | |
| Vitesse de déplacement | Poursuite, fuite, placement | Classe, équipement, *Lenteur* / *Hâte* | Oui |
| Zone de contrôle | Déclenche l'attaque d'opportunité | Classe (mêlée) | Oui |

**Propriétés de la cible**

| Propriété | Rôle | Existe |
|---|---|---|
| Catégorie | Nuée, infanterie, infanterie lourde, créature monstrueuse | |
| Taille (cercle) | Portée bord à bord, blocage, encerclement | Oui |
| Immunités | Ex. pas de recul pour une créature monstrueuse | |

## 14. Questions ouvertes

1. **Le blocage au bouclier** existe-t-il en plus de l'esquive ? Chance, montant, récupération ; « bouclier qui déséquilibre » (voir `docs/brainstorm.md`).
2. **Le niveau** entre-t-il dans les formules (pénalité selon l'écart, à la *Diablo 2* / WoW), ou un monstre plus fort l'est-il seulement par ses nombres ?
3. **Les plafonds** : esquive, résistances, réduction de recharge ; lesquels, et à combien ?
4. **Égide et Barrière** absorbent-elles avant ou après l'armure et les résistances ?
5. **La liste des éléments** : physique, feu, froid, foudre, poison, sacré, vitalité, psychique ? Le saignement est-il un type ou un effet ?
6. **La ressource** : un mana pour tous, ou une ressource par style (rage, PV, mana) ?
7. **La magie peut-elle critiquer ?** (Dans *Titan Quest*, non.)
8. **Les classes de départ** : quelles classes de base, quelles spécialisations, quels paliers ?
9. **L'automatisation** : quels skills partent seuls (gambits, `docs/brainstorm.md`), lesquels restent au clavier ?
10. **Un bonus pour une escouade d'une seule couleur** ? (Moins pertinent depuis que les couleurs ne sont qu'un guide de conception.)

## Ordre de discussion proposé

1. Finir les maths du combat (questions 1 à 7).
2. Les catégories d'ennemis, qui découlent du modèle d'armure.
3. La progression des classes : barre de maîtrise, paliers, fourche de spécialisation.
4. Les skills de chaque classe, conçus avec les couleurs comme boussole.
