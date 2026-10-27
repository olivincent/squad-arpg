# Squad-ARPG — Aide-mémoire des objets et des maths du jeu

27 octobre 2026

## Légende

Chaque valeur porte un statut. **En jeu** : dans le code à la fin de l'étape 101. **Décidé** : tranché le 27 octobre, chiffres à tester. **Proposé** : pas encore tranché.

Les armes et protections de l'étape 101 sont des prototypes de test : tout ce qui est décidé ici les remplace. Les noms anglais sont ceux affichés à l'écran ; les noms français sont ceux du code et de `docs/`.

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

**Exemple** (avec l'épée longue prototype). Un guerrier à Might 13 frappe : 15 × 1,09 = 16. Contre l'infanterie (armure 3) : 13, ou 21 sur un critique (16 × 1,5 = 24, moins 3). Contre l'infanterie lourde (armure 10) : 6 seulement. La dague prototype (15 après Might) y fait 8 : sa pénétration de 30 % ramène l'armure à 7.

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

## Les prototypes en jeu

L'étape 101 a 7 armes et 12 protections aux valeurs fixes, faites pour tester ; elles seront toutes remplacées par les gabarits ci-dessous. Repère : l'épée longue prototype fait 15 dégâts toutes les 0,65 s, soit 23 dégâts/s, la valeur de référence des armes à une main.

## Gabarits d'armes (décidé, étape 102)

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

**Kits de départ** (niveau 1, qualité 50, communs) : guerrier épée à une main, paladin masse à une main (les deux avec bouclier rond et pièces de fer), prêtre sceptre, mage bâton, rôdeur arc.

Les tireurs gardent les mêmes dégâts/s que les autres pour l'instant, à ajuster en jouant.

## Gabarits d'armures (décidé, étape 103)

Une pièce d'armure a un **type** (tissu, léger, intermédiaire, lourd) et un **emplacement**. Toutes les valeurs sont des entiers : la table donne la base au niveau 1, et le niveau la multiplie, arrondie au plus proche.

```
armure de la pièce = arrondi(base × (1 + 0,25 × (niveau − 1)) × (0,85 + 0,30 × qualité))
```

**Base au niveau 1** (rapports 1 : 2 : 3,3 : 5, le tableau d'origine) :

| Emplacement | Tissu | Léger | Intermédiaire | Lourd |
| --- | --- | --- | --- | --- |
| Torse | 1 | 2 | 4 | 6 |
| Jambes | 1 | 1 | 2 | 3 |
| Tête | 1 | 1 | 2 | 2 |
| Gants | 0 | 1 | 1 | 2 |
| Bottes | 0 | 1 | 1 | 2 |
| Ensemble complet | 3 | 6 | 10 | 15 |
| Esquive de l'ensemble | — | — | −2 % | −8 % |

**Ensemble complet selon le niveau**, qualité moyenne. Au niveau 5, tout double exactement ; aux petits niveaux, l'arrondi tasse un peu le tissu, qui protège peu par choix.

| Niveau | Tissu | Léger | Intermédiaire | Lourd |
| --- | --- | --- | --- | --- |
| 1 | 3 | 6 | 10 | 15 |
| 3 | 6 | 11 | 16 | 23 |
| 5 | 6 | 12 | 20 | 30 |
| 10 | 9 | 19 | 33 | 51 |

**Malus d'esquive**, contre le contact et contre les projectiles, fixe à tout niveau : pièce lourde −3 % au torse, −2 % aux jambes, −1 % ailleurs ; pièce intermédiaire −1 % au torse et aux jambes. L'esquive totale ne descend jamais sous 0.

| Type | Seuils (6 au niveau 1), pente par niveau | Tendances d'affixes |
| --- | --- | --- |
| Tissu | aucun | of the Owl, of the Lion ; plus tard résistances, mana |
| Léger | Dex +2 | of Evasion, of the Fox, of Deflection |
| Intermédiaire | Con +1, Dex +1 | of the Bear, of Evasion |
| Lourd | Con +2 | Reinforced, of the Bear, of the Ox |

**Mouvement** : aucun malus. Des bonus de vitesse pourront venir quand chaque personnage aura sa propre vitesse en combat ; hors combat, l'escouade avance à la vitesse du plus lent.

## Boucliers (décidé, étape 103)

Un seul gabarit, à la *Titan Quest* : la chance de blocage et le montant absorbé grandissent tous les deux avec le niveau. La récupération de 3 s reste commune.

- Chance : 20 % au niveau 1, +1 point par niveau. Pas de plafond : la récupération et le montant fixe (un blocage n'annule pas le coup) suffisent à l'équilibre.
- Montant absorbé : 8 au niveau 1, × le facteur de niveau et la qualité. Armure : 1, × les mêmes facteurs.
- Seuil : Con, 6 au niveau 1, +2 par niveau.

| Niveau | Chance | Montant | Armure |
| --- | --- | --- | --- |
| 1 | 20 % | 8 | 1 |
| 5 | 24 % | 16 | 2 |
| 10 | 29 % | 26 | 3 |
| 20 | 39 % | 46 | 6 |

## Reliques de main gauche (décidé, étape 103)

Une nouvelle catégorie pour la main gauche, à la place du bouclier : le prêtre au sceptre ou le mage à la baguette y gagnent quelque chose. Une arme à deux mains la range dans le sac, comme un bouclier.

| Gabarit | Seuils (6 au niveau 1), pente | Effet de base (grandit avec le niveau) | Affinité ×3 |
| --- | --- | --- | --- |
| Orbe | Int +2 | +2 Int et +10 de capacité offensive | of the Owl |
| Relique sacrée | Res +2 | +2 Res et +10 de capacité offensive | of the Lion |

Un effet simple pour l'instant, à repenser quand l'Intellect et la Resolve auront un effet.

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

Aujourd'hui, l'affixe d'un objet Magic est tiré uniformément parmi ceux que sa catégorie permet. Les anneaux et colliers n'ont aucun préfixe, donc ne sont jamais Rare.

## Affinités d'affixes (décidé, étape 104)

La catégorie dit quels affixes ont un sens sur un objet ; le gabarit dit lesquels sont plus probables. Un bâton reste capable de tout, mais penche vers l'Intellect et le feu.

**Le but est une structure où l'on ajoute ou retire un concept sans rien réécrire.** Chaque affixe a un poids de base dans son catalogue ; chaque gabarit garde une courte liste de ses affinités, tout le reste vaut 1.

- Ajouter un affixe : une ligne dans le catalogue d'affixes, et, si l'on veut, une affinité dans quelques gabarits.
- Retirer un affixe : supprimer sa ligne ; le compilateur signale chaque gabarit qui le nommait encore.
- Ajouter un gabarit : une ligne dans le catalogue, sans affinité si aucune tendance n'est voulue.

```
poids d'un affixe = poids de base de l'affixe × affinité du gabarit   (1 par défaut, 0 = jamais)
```

**Exemple : un bâton Magic** (8 affixes permis sur une arme, of the Owl ×3, Blazing ×2) :

| Affixe | Poids | Chance avec affinités | Chance uniforme aujourd'hui |
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
| Relique sacrée | of the Lion | Hallowed |

Un affixe sans effet en jeu (régénération de mana) n'existe pas tant que le mana n'existe pas ; le tableau l'accueillera le jour venu.

## Exemple complet de génération (décidé)

Un monstre meurt au 3e étage et laisse une « Hand Axe of the Ox » de niveau 3. Voici chaque tirage, dans l'ordre fixe qui garde une graine reproductible.

1. **Rareté**, poids du niveau 3 : Common 60, Magic 31, Rare 9. Tirage : Magic.
2. **Gabarit**, uniforme parmi ceux qui peuvent être Magic. Tirage : hache à une main.
3. **Qualité**, entre 0 et 100. Tirage : 62, soit un facteur de 0,85 + 0,30 × 0,62 = 1,036.
4. **Dégâts de l'objet** : 20 × 1,5 × 1,036 = 31. Écart de ±25 % : chaque coup fera de 23 à 39.
5. **Seuil** : Might 6 au niveau 1 + 2 × 2 niveaux = Might 10.
6. **Affixe**, pondéré par les affinités de la hache (of the Ox ×2, étape 104) : 22 % de chances au lieu de 12,5 %. Tirage : of the Ox.
7. **Valeur**, plage du niveau 3 : 1 – 3 × 1,5 = 2 – 5. Tirage : 4.

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