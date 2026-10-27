# Squad-ARPG — Aide-mémoire des objets et des maths du jeu

27 octobre 2026

## Légende

Chaque valeur porte un statut. **En jeu** : dans le code à la fin de l'étape 104 (tout ce qui a été fait aux étapes 102 à 104 : gabarits, familles de butin, affinités d'affixes). **Décidé** : tranché, pas encore dans le code (les poids de base des affixes). **Proposé** : pas encore tranché. Les chiffres « en jeu » restent à tester en jouant.

Les armes et protections de l'étape 101 étaient des prototypes de test : les étapes 102 (armes) et 103 (armures, boucliers, reliques, bijoux) les ont toutes remplacées. Les noms anglais sont ceux affichés à l'écran ; les noms français sont ceux du code et de `docs/`.

## Les cinq caractéristiques

Trois caractéristiques ont un effet en jeu ; le Savoir et la Volonté existent, s'affichent et servent de seuils, mais n'ont encore aucun effet. Valeur de référence : 10 (ni bonus ni malus).

| Code / écran | Effet en jeu | Effet prévu | Seuils typiques |
| --- | --- | --- | --- |
| Puissance / Might | +3 % de dégâts et de soins par point au-dessus de 10 | — | Armes lourdes |
| Endurance / Constitution | +6 points de vie par point | Résistance au contrôle physique | Armures lourdes, boucliers |
| Adresse / Dexterity | +5 de capacités offensive et défensive par point | — | Dague, arc |
| Savoir / Intellect | aucun | Taille des zones (sur l'aire), durée des effets | Bâton |
| Volonté / Resolve | aucun | Résistance au contrôle magique et à l'interruption | Sceptre |

**Bases de classe et croissance** (à tester). Somme 50 pour chaque classe.

| Classe | Might | Con | Dex | Int | Res | +1 par niveau, en alternance |
| --- | --- | --- | --- | --- | --- | --- |
| Guerrier (Warrior) | 12 | 14 | 10 | 6 | 8 | Might, puis Con |
| Paladin | 11 | 13 | 8 | 7 | 11 | Might, puis Con |
| Prêtre (Priest) | 10 | 10 | 8 | 10 | 12 | Might, puis Res |
| Mage | 10 | 8 | 8 | 14 | 10 | Might, puis Int |
| Rôdeur (Ranger) | 10 | 10 | 14 | 8 | 8 | Dex, puis Might |

Chaque niveau au-delà du premier donne aussi **3 points libres** par personnage (à tester), dépensés de façon définitive dans l'inventaire. Base d'un personnage = classe + croissance + points dépensés, toujours recalculée.

## Formules dérivées

Toutes les valeurs dérivées sont recalculées en entier à chaque changement, jamais ajustées pas à pas.

**Points de vie maximum** (environ 300 au départ) :

```
PV max = 240 + 6 × Endurance + 8 × (niveau − 1)
```

**Montant effectif** d'un dégât ou d'un soin, un arrondi par portion, jamais négatif. Le coefficient de Puissance vaut 1,0 pour l'attaque de base et tous les skills de dégâts et de soin :

```
montant × (1 + Σ coef × 0,03 × (caractéristique − 10))
```

**Capacités offensive et défensive** d'un personnage (100 à Adresse 10 ; monstre ordinaire : 100 et 100) :

```
capacité = 50 + 5 × Adresse
```

**Expérience cumulée** pour atteindre le niveau n (k = 21, à tester) ; repères : niveau 2 à 42, niveau 3 à 126, niveau 5 à 420, niveau 10 à 1 890 :

```
XP(n) = k × n × (n − 1)
```

**Monstres par étage** au-delà du premier : points de vie +25 %, dégâts +15 %, capacités +10, expérience +25 % (à tester).

## Le déroulement d'un coup

Un coup passe par six étapes, toujours dans cet ordre (`resoudreCoup`). Un coup est un ensemble de portions par élément ; chaque étape travaille sans arrondir.

1. **Esquive.** Chance = esquive contre le contact ou contre les projectiles, plafond 50 %. Une zone ne s'esquive jamais. Esquivé : le coup s'arrête (« Dodged »).
2. **Critique.** r = capacité offensive ÷ capacité défensive. Chance = min(0,05 × r², 50 %) ; multiplicateur = 1,5 + 0,25 × (r − 1), entre ×1,5 et ×2. Il multiplie toutes les portions.
3. **Blocage.** Seulement contre le contact ou un projectile qui a du physique. Retire un montant fixe de la portion physique, puis 3 s de récupération. Tout absorbé : le coup s'arrête (« Blocked »).
4. **Armure**, sur la portion physique : armure effective = armure × (1 − pénétration), pénétration plafonnée à 75 % ; subi = max(D − armure effective, 20 % de D).
5. **Résistances**, sur chaque portion : portion × (1 − résistance), résistance ramenée entre −100 % et 80 %.
6. **Arrondi** : un seul par portion, au plus proche ; un coup qui passe fait au moins 1.

**Repères du critique** (à tester) :

| Rapport r | Exemple | Chance | Multiplicateur |
| --- | --- | --- | --- |
| 0,5 | Adresse 0 contre 100 | 1,25 % | ×1,5 |
| 1 | capacités égales | 5 % | ×1,5 |
| 1,2 | rôdeur (Adresse 14) contre un monstre | 7,2 % | ×1,55 |
| 2 | double capacité | 20 % | ×1,75 |
| 3 | triple capacité | 45 % | ×2 |

**Exemple** (avec l'épée longue prototype de l'étape 101, gardée pour montrer le calcul ; l'épée à une main de l'étape 102 fait 13 – 17 au niveau 1). Un guerrier à Might 13 frappe : 15 × 1,09 = 16. Contre l'infanterie (armure 3) : 13, ou 21 sur un critique (16 × 1,5 = 24, moins 3). Contre l'infanterie lourde (armure 10) : 6 seulement. La dague prototype (15 après Might) y fait 8 : sa pénétration de 30 % ramène l'armure à 7.

## Éléments et résistances

Trois éléments sont en jeu ; les autres entreront dans le code avec leur premier skill ou monstre. L'armure et le bouclier ne touchent que le physique ; chaque élément, physique compris, a sa résistance en %.

| Élément | Statut | Source aujourd'hui | Règle propre | Couleur du nombre |
| --- | --- | --- | --- | --- |
| Physique | En jeu | Armes, monstres | Armure, bouclier, pénétration | Blanc sur un monstre, rouge sur un personnage |
| Feu | En jeu | Explosion, préfixe Blazing | — | Orange rougeâtre |
| Sacré | En jeu | Frappe sacrée, préfixe Hallowed | — | Or pâle |
| Froid, foudre | Prévu | — | — | — |
| Poison, nécrotique | Prévu | — | — | — |
| Saignement | Prévu | — | Sa propre résistance ; dans le temps, ignore l'armure | — |
| Psychique / arcane | Piste | — | À revoir avec les classes | — |

**Résistances** (à tester) : plafond 80 %, vulnérabilité jusqu'à −100 % (coup doublé). Personnages : 0 partout, aucun objet n'en donne encore. Monstres : 0, sauf la créature monstrueuse, 40 % contre le feu.

## Les prototypes

Les sept armes et les douze protections de l'étape 101, aux valeurs fixes, ont été remplacées : les armes par treize gabarits (étape 102), les protections par vingt pièces d'armure, un Shield, deux reliques et quatre bijoux calculés (étape 103). Repère historique : l'épée longue prototype faisait 15 dégâts toutes les 0,65 s, soit 23 dégâts/s, la valeur de référence des armes à une main.

## Gabarits d'armes (en jeu, étape 102)

Un gabarit par type d'arme fixe son identité ; le niveau ne multiplie que les dégâts. Une dague de niveau 80 est aussi rapide qu'une dague de niveau 1, elle frappe seulement plus fort.

**Grandissent avec le niveau d'objet** : les dégâts et les seuils. **Ne grandissent jamais** : la vitesse, la portée, la nature, les mains, la pénétration et l'écart par coup. Le facteur de niveau est celui des affixes et des points de vie des monstres par étage :

```
dégâts moyens de l'objet = référence du gabarit × (1 + 0,25 × (niveau − 1)) × (0,85 + 0,30 × qualité)
```

Deux hasards distincts. La **qualité** (0 à 100) est tirée une fois, à la création, et n'est pas affichée. L'**écart par coup** est tiré à chaque coup, entre le minimum et le maximum affichés ; son pourcentage reste le même à tout niveau (une épée de 40 à ±15 % frappe de 34 à 46). Les **dégâts/s** se calculent sur la moyenne des coups.

**Trois réglages, pas cinquante nombres.** Dégâts moyens de référence = dégâts/s visés (23 à une main, 30 à deux mains) × cycle moyen (exécution + recharge moyenne). Vitesse : très rapide 0,50 s, rapide 0,65 s, moyen 0,85 s, lent 1,15 s, très lent 1,60 s. Écart : très serré ±10 %, serré ±15 %, moyen ±25 %, large ±40 %.

| Gabarit (écran, sigle) | Mains | Nature, portée | Exécution | Recharge | Moyenne niv. 1 | Écart | Coup niv. 1 | Pénétration | Seuils, pente par niveau |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Dague (Dagger, DA) | 1 | contact, 20 px | 0,15 s | 0,20 – 0,50 s | 10 | ±10 % | 9 – 11 | 15 % | Dex +2 |
| Épée à une main (Sword, SW) | 1 | contact, 20 px | 0,15 s | 0,30 – 0,70 s | 15 | ±15 % | 13 – 17 | — | Might +2 |
| Hache à une main (Hand Axe, HA) | 1 | contact, 20 px | 0,15 s | 0,40 – 1,00 s | 20 | ±25 % | 15 – 25 | — | Might +2 |
| Masse à une main (Mace, MA) | 1 | contact, 20 px | 0,15 s | 0,60 – 1,40 s | 26 | ±40 % | 16 – 36 | — | Might +2 |
| Lance (Spear, SP) | 1 | contact, 35 px | 0,15 s | 0,70 – 1,45 s | 28 | ±25 % | 21 – 35 | 30 % | Dex +2, Might +1 |
| Épée à deux mains (Greatsword, GS) | 2 | contact, 20 px | 0,30 s | 0,50 – 1,20 s | 35 | ±15 % | 30 – 40 | — | Might +2, Dex +1 |
| Hache à deux mains (Great Axe, GA) | 2 | contact, 20 px | 0,30 s | 0,80 – 1,80 s | 48 | ±25 % | 36 – 60 | — | Might +2 |
| Masse à deux mains (Maul, MU) | 2 | contact, 20 px | 0,30 s | 0,80 – 1,80 s | 48 | ±40 % | 29 – 67 | — | Might +2, Con +1 |
| Baguette (Wand, WA) | 1 | projectile, 120 px | 0,15 s | 0,30 – 0,70 s | 15 | ±15 % | 13 – 17 | — | Int +2 |
| Sceptre (Scepter, SC) | 1 | projectile, 120 px | 0,15 s | 0,40 – 1,00 s | 20 | ±15 % | 17 – 23 | — | Res +2 |
| Bâton (Staff, ST) | 2 | projectile, 120 px | 0,30 s | 0,55 – 0,85 s | 30 | ±25 % | 23 – 38 | — | Int +2 |
| Arc (Bow, BO) | 2 | projectile, 120 px | 0,30 s | 0,35 – 0,55 s | 23 | ±25 % | 17 – 29 | 15 % | Dex +2 |
| Arbalète (Crossbow, CB) | 2 | projectile, 140 px | 0,30 s | 0,55 – 0,85 s | 30 | ±15 % | 26 – 35 | 30 % | Dex +2, Might +1 |

Dans chaque famille, l'épée est régulière, la hache moyenne, la masse imprévisible. Contre l'armure par soustraction, les gros coups perdent moins : la masse devient l'arme naturelle contre l'infanterie lourde. L'allonge de 35 px laisse une lance au second rang frapper par-dessus le tank ; sa zone de contrôle reste celle du contact (20 px).

**Seuils** : 6 au niveau 1 pour toute caractéristique exigée, puis + la pente × (niveau − 1). Tout le monde peut donc porter tout l'équipement de niveau 1 (la plus petite valeur de base d'une classe est 6) ; une arme de niveau 5 exige 14 dans sa caractéristique principale. C'est ce qui limite l'équipement, à la place des classes.

**Skills** : la frappe puissante et la frappe sacrée exigent une arme de contact (n'importe laquelle) ; le tir perçant et la pluie de flèches, un arc ou une arbalète.

**Kits de départ** (niveau 1, qualité 50, communs) : guerrier épée à une main, paladin masse à une main, prêtre sceptre, mage bâton, rôdeur arc ; l'armure, le bouclier et la relique sont dans les sections suivantes.

Les tireurs gardent les mêmes dégâts/s que les autres pour l'instant, à ajuster en jouant.

## Gabarits d'armures (en jeu, étape 103)

Une pièce d'armure a un **type** (tissu, léger, intermédiaire, lourd) et un **emplacement**. Toutes les valeurs sont des entiers : la table donne la base au niveau 1, et le niveau la multiplie, arrondie au plus proche. Le facteur de qualité est celui des armes, écrit une fois (`facteurDeQualite`), et une seule fonction, `effetsDeBase`, calcule ce qu'un objet apporte par lui-même.

```
armure de la pièce = arrondi(base × (1 + 0,25 × (niveau − 1)) × (0,85 + 0,30 × qualité / 100))
```

**Base au niveau 1** (chaque type bat le précédent sur chaque pièce, aucune pièce à 0) :

| Emplacement | Tissu | Léger | Intermédiaire | Lourd |
| --- | --- | --- | --- | --- |
| Torse | 2 | 3 | 5 | 7 |
| Jambes | 1 | 2 | 3 | 5 |
| Tête | 1 | 2 | 3 | 4 |
| Gants | 1 | 2 | 3 | 4 |
| Bottes | 1 | 2 | 3 | 4 |
| Ensemble complet | 6 | 11 | 17 | 24 |
| Esquive de l'ensemble | — | — | −2 % | −8 % |

Aucune pièce n'a de bonus de caractéristique de base. **Ensemble complet selon le niveau**, qualité moyenne (au niveau 5, tout double exactement ; l'arrondi tasse un peu le tissu aux petits niveaux) :

| Niveau | Tissu | Léger | Intermédiaire | Lourd |
| --- | --- | --- | --- | --- |
| 1 | 6 | 11 | 17 | 24 |
| 3 | 11 | 17 | 28 | 37 |
| 5 | 12 | 22 | 34 | 48 |
| 10 | 19 | 38 | 56 | 78 |

**Malus d'esquive**, contre le contact et contre les projectiles, fixe à tout niveau et à toute qualité : pièce lourde −3 % au torse, −2 % aux jambes, −1 % à la tête, aux gants et aux bottes ; pièce intermédiaire −1 % au torse et aux jambes. Il passe par les statistiques de combat, jamais par les caractéristiques. L'esquive qui joue ne descend jamais sous 0 ; un objet inactif (sous son seuil) n'a aucun effet, malus compris.

| Type | Seuils (6 au niveau 1), pente par niveau | Niveau d'apparition | Tendances d'affixes (étape 104) |
| --- | --- | --- | --- |
| Tissu | aucun | 1 | of the Owl, of the Lion ; plus tard résistances, mana |
| Léger | Dex +2 | 1 | of Evasion, of the Fox, of Deflection |
| Intermédiaire | Con +1, Dex +1 | 2 | of the Bear, of Evasion |
| Lourd | Con +2 | 3 | Reinforced, of the Bear, of the Ox |

**Niveau d'apparition** (en jeu) : un entier du gabarit, 1 par défaut ; `genererObjet` ne tire que les modèles dont le niveau d'apparition est au plus celui de l'objet : d'abord une famille de butin, puis un gabarit uniformément parmi les éligibles de cette famille (« Familles de butin » ci-dessous). Ainsi, au 1er étage on ne trouve que du tissu et du léger, des pièces Chain dès le 2e, des pièces Plate dès le 3e.

**Noms à l'écran** : Robe, Cloth Leggings, Hood, Cloth Gloves, Slippers ; Leather Jerkin, Leather Leggings, Leather Cap, Leather Gloves, Leather Boots ; Chain Mail, Chain Leggings, Chain Coif, Chain Gloves, Chain Boots ; Plate Armor, Plate Greaves, Great Helm, Gauntlets, Sabatons.

**Kits de départ** (niveau 1, qualité 50, communs) : une seule pièce d'armure, le torse. Guerrier, paladin et rôdeur : Leather Jerkin (armure 3) ; prêtre et mage : Robe (armure 2). Main gauche : Shield pour le guerrier et le paladin, Holy Relic pour le prêtre. Aucune autre pièce d'armure au départ.

**Infobulle** : nom, catégorie, type d'armure (« Heavy Armor »), « Item Level N », « Armor: N », malus « Dodge: −3 % » (en rouge), bonus de base, affixes, seuils.

**Mouvement** : aucun malus. Des bonus de vitesse pourront venir quand chaque personnage aura sa propre vitesse en combat ; hors combat, l'escouade avance à la vitesse du plus lent.

## Boucliers (en jeu, étape 103)

Un seul gabarit, « Shield », à la *Titan Quest* : la chance de blocage et le montant absorbé grandissent tous les deux avec le niveau. La récupération de 3 s reste commune.

- Chance : 20 % au niveau 1, +1 point par niveau (le niveau seul, pas la qualité). Pas de plafond : la récupération et le montant fixe (un blocage n'annule pas le coup) suffisent à l'équilibre.
- Montant absorbé : 8 au niveau 1, × le facteur de niveau et la qualité. Armure : 1, × les mêmes facteurs.
- Seuil : Con, 6 au niveau 1, +2 par niveau. « of Blocking » s'ajoute comme avant.
- Infobulle : « Block: 20 % » et « Absorbs: 8 ».

| Niveau | Chance | Montant | Armure |
| --- | --- | --- | --- |
| 1 | 20 % | 8 | 1 |
| 5 | 24 % | 16 | 2 |
| 10 | 29 % | 26 | 3 |
| 20 | 39 % | 46 | 6 |

## Reliques de main gauche (en jeu, étape 103)

Une catégorie pour la main gauche, à la place du bouclier : le prêtre au sceptre ou le mage à la baguette y gagnent quelque chose. La main gauche accepte un bouclier ou une relique ; une arme à deux mains range l'objet de la main gauche dans le sac, quelle que soit sa catégorie.

| Gabarit | Seuils (6 au niveau 1), pente | Effet de base (grandit avec le niveau et la qualité) | Affinité ×3 (étape 104) |
| --- | --- | --- | --- |
| Orbe (Orb) | Int +2 | +2 Int et +10 de capacité offensive | of the Owl |
| Relique sacrée (Holy Relic) | Res +2 | +2 Res et +10 de capacité offensive | of the Lion |

La capacité offensive est un bonus de statistiques de combat de l'équipement (pas une caractéristique). Affixes permis : les suffixes de caractéristique seulement (aucun préfixe, donc jamais Rare, comme les bijoux). Un effet simple pour l'instant, à repenser quand l'Intellect et la Resolve auront un effet.

## Bijoux (en jeu, étape 103)

Les quatre bijoux suivent la même règle que les autres objets (`base × facteurDeNiveau × facteurDeQualite`), sans seuil : Ring of Protection et Necklace of Protection armure 1, Ring of Might Puissance +3, Necklace of Constitution Endurance +3 (valeurs au niveau 1).

## Raretés et affixes en jeu

Un objet a au plus un préfixe et un suffixe, chacun avec sa valeur tirée, gardée dans l'objet. Le niveau d'objet (= la profondeur) agrandit les plages et rend Magic et Rare plus fréquents.

| Rareté | Couleur | Affixes | Poids niv. 1 | Poids niv. 5 | Par niveau |
| --- | --- | --- | --- | --- | --- |
| Common | blanc | aucun | 70 | 50 | −5, plancher 20 |
| Magic | jaune | un préfixe ou un suffixe | 25 | 37 | +3 |
| Rare | vert | un préfixe et un suffixe | 5 | 13 | +2 |

Les plages sont multipliées par 1 + 0,25 × (niveau − 1), arrondies au plus proche ; au niveau 5, elles doublent.

| Affixe (écran) | Position | Effet | Plage niv. 1 | Plage niv. 5 | Catégories permises |
| --- | --- | --- | --- | --- | --- |
| Blazing | préfixe | feu ajouté à l'attaque de base | 3 – 8 | 6 – 16 | armes |
| Hallowed | préfixe | sacré ajouté à l'attaque de base | 3 – 8 | 6 – 16 | armes |
| Keen | préfixe | pénétration d'armure | 10 – 20 % | 20 – 40 % | armes |
| Reinforced | préfixe | armure | 2 – 5 | 4 – 10 | casque, armure, jambières, gants, bottes, bouclier |
| of the Ox / Bear / Fox / Owl / Lion | suffixe | Might / Con / Dex / Int / Res | 1 – 3 | 2 – 6 | toutes |
| of Evasion | suffixe | esquive contre le contact | 2 – 4 % | 4 – 8 % | bottes, armure |
| of Deflection | suffixe | esquive contre les projectiles | 2 – 4 % | 4 – 8 % | casque, bouclier |
| of Blocking | suffixe | chance de blocage | 3 – 7 % | 6 – 14 % | bouclier |

L'affixe d'un objet Magic est tiré parmi ceux que sa catégorie permet, pondéré par les affinités de son gabarit (« Affinités d'affixes » ci-dessous). Les anneaux et colliers n'ont aucun préfixe, donc ne sont jamais Rare.

## Familles de butin (en jeu, étape 104)

Le modèle d'un objet se tire en deux temps : une **famille**, puis un gabarit dans cette famille. Sans cela, l'armure (vingt pièces sur quarante modèles) ferait la moitié du butin, et davantage en débloquant le Chain (niveau 2) et le Plate (niveau 3).

| Famille | Catégories | Poids |
| --- | --- | --- |
| Arme | armes | 35 |
| Armure | casque, armure, jambières, gants, bottes | 35 |
| Main gauche | bouclier, relique | 15 |
| Bijou | anneau, collier | 15 |

- La famille se **déduit** de la catégorie (`familleDe`) ; elle n'est stockée dans aucun gabarit.
- **Éligibilité** : un gabarit est éligible s'il est apparu à ce niveau et peut porter la rareté tirée. Une famille est éligible si elle a au moins un gabarit éligible. Le tirage de famille se fait parmi les familles éligibles, selon leurs poids ; le gabarit est ensuite tiré **uniformément** parmi les éligibles de la famille (pas de poids par gabarit).
- **Même proportions à toute profondeur** : débloquer des gabarits ne change pas la part des familles. Au niveau 1 comme au niveau 5, un objet Common est une arme, une armure, une main gauche ou un bijou dans les proportions 35 / 35 / 15 / 15.
- **Rare** : les bijoux (pas de préfixe) et les reliques ne peuvent pas l'être ; il reste, pour la main gauche, le seul Shield. Les familles éligibles d'un Rare sont donc Arme, Armure et Main gauche, soit 41 / 41 / 18 %.
- Ordre des tirages : rareté, famille, gabarit, qualité, affixes.
- Valeurs « à tester ». Un poids par gabarit dans une famille viendra si un gabarit doit être plus rare.

## Affinités d'affixes (en jeu, étape 104)

La catégorie dit quels affixes ont un sens sur un objet ; le gabarit dit lesquels sont plus probables. Un bâton reste capable de tout, mais penche vers l'Intellect et le feu.

**Le but est une structure où l'on ajoute ou retire un concept sans rien réécrire.** Chaque gabarit garde une courte liste de ses affinités (`DefinitionObjet::affinites`, des `AffiniteDAffixe { affixe, multiplicateur }`), tout le reste vaut 1. Les cinq pièces d'un type d'armure partagent la liste de leur type (`affinitesDuType`). Un multiplicateur est d'au moins 2.

- Ajouter un affixe : une ligne dans le catalogue d'affixes, et, si l'on veut, une affinité dans quelques gabarits.
- Retirer un affixe : supprimer sa ligne ; le compilateur signale chaque gabarit qui le nommait encore.
- Ajouter un gabarit : une ligne dans le catalogue, sans affinité si aucune tendance n'est voulue.

```
poids d'un affixe = affinité du gabarit   (1 par défaut)
```

**Poids de base des affixes : pas encore en jeu.** Ils seraient tous égaux aujourd'hui ; ils viendront avec le premier affixe qu'on voudra plus rare (poids de base × affinité).

**Une affinité pour un affixe que la catégorie ne permet pas est sans effet** : la catégorie décide de ce qui est permis, l'affinité ne fait que pondérer parmi les permis (of Deflection n'est permis que sur casque et bouclier : sur des gants légers, l'affinité du type léger ne joue pas). **Magic** : un seul tirage pondéré parmi tous les affixes permis, préfixes et suffixes confondus ; **Rare** : le préfixe et le suffixe sont chacun pondérés parmi ceux de leur position.

**Exemple : un bâton Magic** (8 affixes permis sur une arme, of the Owl ×3, Blazing ×2) :

| Affixe | Poids | Chance avec affinités | Chance uniforme (avant l'étape 104) |
| --- | --- | --- | --- |
| of the Owl (Int) | 3 | 27,3 % | 12,5 % |
| Blazing (feu) | 2 | 18,2 % | 12,5 % |
| Hallowed, Keen, of the Ox, of the Bear, of the Fox, of the Lion | 1 chacun | 9,1 % chacun | 12,5 % |

Le tirage reste un seul tirage pondéré, comme les raretés : choisir d'abord « bonus de dégâts », puis l'élément, revient au même avec des poids.

| Gabarit | Affinité ×3 | Affinité ×2 |
| --- | --- | --- |
| Dague | Keen | of the Fox |
| Épée à une main | — | of the Ox, of the Fox |
| Hache (une ou deux mains) | — | of the Ox |
| Masse (une ou deux mains) | Hallowed | of the Ox |
| Lance | Keen | of the Fox |
| Épée à deux mains | of the Ox | of the Fox |
| Baguette | of the Owl | Blazing |
| Sceptre | of the Lion | Hallowed |
| Bâton | of the Owl | Blazing ; plus tard, régénération de mana |
| Arc | of the Fox | Keen |
| Arbalète | Keen | of the Ox |
| Armure de tissu | of the Owl, of the Lion | — |
| Armure légère | of Evasion | of the Fox, of Deflection |
| Armure intermédiaire | — | of the Bear, of Evasion |
| Armure lourde | Reinforced | of the Bear, of the Ox |
| Bouclier | of Blocking | Reinforced |
| Orbe | of the Owl | — |
| Relique sacrée | of the Lion | — (pas de Hallowed : une relique ne porte pas de préfixe) |
| Bijoux | — | — |

Un affixe sans effet en jeu (régénération de mana) n'existe pas tant que le mana n'existe pas ; le tableau l'accueillera le jour venu.

## Exemple complet de génération (en jeu)

Un monstre meurt au 3e étage et laisse une « Hand Axe of the Ox » de niveau 3. Voici chaque tirage, dans l'ordre fixe qui garde une graine reproductible.

1. **Rareté**, poids du niveau 3 : Common 60, Magic 31, Rare 9. Tirage : Magic.
2. **Famille**, poids Arme 35, Armure 35, Main gauche 15, Bijou 15 (toutes éligibles pour un Magic). Tirage : Arme.
3. **Gabarit**, uniforme parmi les armes (toutes peuvent être Magic). Tirage : hache à une main.
4. **Qualité**, entre 0 et 100. Tirage : 62, soit un facteur de 0,85 + 0,30 × 0,62 = 1,036.
5. **Dégâts de l'objet** : 20 × 1,5 × 1,036 = 31. Écart de ±25 % : chaque coup fera de 23 à 39.
6. **Seuil** : Might 6 au niveau 1 + 2 × 2 niveaux = Might 10.
7. **Affixe**, pondéré par les affinités de la hache (of the Ox ×2) : 2 sur 9, soit 22 % de chances au lieu de 12,5 %. Tirage : of the Ox.
8. **Valeur**, plage du niveau 3 : 1 – 3 × 1,5 = 2 – 5. Tirage : 4.

**Ce que l'objet garde**. Tout le reste se recalcule depuis le gabarit :

```cpp
Objet{ .modele = IdObjet::HacheAUneMain, .rarete = Rarete::Magique,
       .niveau = 3, .qualite = 62,
       .suffixe = Affixe{ IdAffixe::DuBoeuf, 4 } }
```

**Ce que le joueur voit** dans l'infobulle :

```
Hand Axe of the Ox          (jaune)
Weapon
Item Level 3
Damage: 23–39
Cooldown: 0.4 to 1.0 s
Range: 20 px, Melee
Might: +4
Requires Might 10
```
