# Squad-ARPG — Glossaire (concept → texte à l'écran)

Référence de tous les noms que le joueur voit. **Le texte affiché est en anglais** (décidé à l'étape 95) ;
le code (identifiants), les commentaires, la documentation et les messages de commit restent en français.
Il n'y a pas de système de traduction : les textes sont écrits là où ils s'affichent (les `switch` de
`interface/Affichage*.cpp`, le catalogue des objets, celui des classes, `TextesFlottants`, les écrans). Ce
fichier dit où chercher et fixe le vocabulaire ; la documentation cite les textes du jeu en anglais.

Conventions du texte affiché : deux-points collé au mot (« Damage: 12 »), point décimal (« 0.6 s »), le
signe `%` et l'unité `px` séparés du nombre par une espace (« 30 % », « 20 px »), signe moins `−` (U+2212)
dans « Blocked −40 ».

## Caractéristiques (`interface/AffichageCaracteristiques.cpp`)

| Concept | Texte | Identifiant |
|---|---|---|
| Puissance | Might | `Caracteristique::Puissance` |
| Endurance | Constitution | `Caracteristique::Endurance` |
| Adresse | Dexterity | `Caracteristique::Adresse` |
| Savoir | Intellect | `Caracteristique::Savoir` |
| Volonté | Resolve | `Caracteristique::Volonte` |

Points libres (étape 100, `interface/AffichageCaracteristiques.cpp`, `AffichageNiveau.cpp`, `EcranInventaire.cpp`) :

| Concept | Texte |
|---|---|
| Points à répartir d'un personnage (brouillon déduit) | Points: 3 |
| Points en attente dans une caractéristique | (+2), écrit après la valeur : « Might 15 (+3) (+2) » |
| Boutons d'une caractéristique | +, − |
| Boutons du brouillon d'un personnage | Apply, Reset |
| Signal en haut de l'écran | Points to spend |
| Effet de Might | Might: +1% weapon damage per point |
| Effet de Constitution | Constitution: +6 health and +0.3% Stun and Slow resistance per point |
| Effet de Dexterity | Dexterity: +5 offensive and defensive capability per point |
| Effet d'Intellect | Intellect: +1% spell damage, +0.3% area and +0.3% duration per point |
| Effet de Resolve | Resolve: +3 energy, +1% healing and +0.3% Charm, Fear and Disruption resistance per point |

Les nombres des effets sont lus dans les constantes du code (`s_bonusParPoint`, `s_pvParPointDEndurance`, `s_capaciteParPointDAdresse`, `s_energieParPointDeVolonte`, `s_resistanceAuControleParPoint`, `s_bonusDAireParPoint`, `s_bonusDeDureeParPoint`).

## Éléments (pas encore affichés : textes à venir avec les résistances dans l'inventaire)

Huit éléments depuis l'étape 109 (`docs/classes-et-combat.md`, section 13). Le saignement n'est plus un élément : c'est du physique dans le temps.

| Concept | Texte | Identifiant |
|---|---|---|
| Physique | Physical | `Element::Physique` |
| Feu | Fire | `Element::Feu` |
| Sacré | Holy | `Element::Sacre` |
| Froid | Cold | pas encore dans `Element` |
| Foudre | Lightning | pas encore dans `Element` |
| Arcane | Arcane | pas encore dans `Element` |
| Nécrotique | Necrotic | pas encore dans `Element` |
| Poison | Poison | pas encore dans `Element` |

## Liste cible des statistiques (étape 109, noms de travail)

Noms anglais de la liste cible (`docs/classes-et-combat.md`, section 13). « Affiché aujourd'hui » dit le texte que le jeu montre déjà quand il
diffère du nom cible : ces textes ne changent pas à l'étape 109 (documentation seulement).

| Concept | Texte | Identifiant ou texte affiché aujourd'hui |
|---|---|---|
| Capacité offensive | Offensive Ability | `StatistiquesDeCombat::capaciteOffensive` ; affiché tel quel |
| Capacité défensive | Defensive Ability | `StatistiquesDeCombat::capaciteDefensive` ; affiché tel quel |
| Multiplicateur de critique | Crit Damage | `multiplicateurDeCritique` (fonction du rapport des capacités, pas encore une statistique) |
| Armure | Armor | `armure` (objets, `Defenses`) ; affiché tel quel |
| Pénétration d'armure | Armor Penetration | `Attaque::penetration` ; affiché « Penetration » |
| Esquive contre le contact | Dodge | `StatistiquesDeCombat::esquiveContact` ; affiché « Melee Dodge » (inventaire), « Dodge vs Melee » (objets) |
| Esquive contre les projectiles | Deflect | `StatistiquesDeCombat::esquiveProjectile` ; affiché « Projectile Dodge », « Dodge vs Projectiles » |
| Chance de blocage | Block Chance | `StatistiquesDeCombat::chanceDeBlocage` ; affiché « Block » |
| Montant bloqué | Block Amount | `StatistiquesDeCombat::montantBloque` ; affiché « Absorbs » ou entre parenthèses |
| Récupération du blocage | Block Recovery | `s_recuperationDuBlocage` (une constante, pas une statistique) |
| Réserve d'absorption du physique | Aegis | n'existe pas (anciennement « Égide ») |
| Réserve d'absorption du non-physique | Ward | n'existe pas (anciennement « Barrière ») |
| Résistance à un élément | Fire Resistance, Holy Resistance… (un nom par élément) | `StatistiquesDeCombat::resistances` (`ParElement<int>`) ; pas encore affichées |
| Résistance à l'étourdissement, au ralentissement | Stun Resistance, Slow Resistance | `StatistiquesDeCombat::resistancesAuControle` (`ParEtat<float>`, étape 110) ; affichées dans l'inventaire, « Stun Resistance 3.0 % » (une décimale, signe moins typographique sous zéro) |
| Résistance au charme, à la peur, à la perturbation | Charm Resistance, Fear Resistance, Disruption Resistance | idem (étape 110), « Charm Resistance −1.5 % » |
| États de contrôle physique | Stun, Slow | `EtatDeControle` (étape 110) ; aucun état n'est encore infligé |
| États de contrôle magique | Charm, Fear, Disruption (empêche d'utiliser les skills) | idem |
| Points de vie maximum | Max Life | `pointsDeVieMax` ; affiché « HP » |
| Régénération de vie | Life Regen | n'existe pas |
| Vol de vie, vie par mort | Life Steal, Life per Kill | n'existent pas |
| Soins reçus | Healing Received | n'existe pas |
| Énergie maximum | Max Energy | `energieMax` ; affiché « Energy » |
| Régénération d'énergie | Energy Regen | `regenerationDEnergie` ; affiché « Energy Regen » |
| Vol d'énergie | Energy Steal | n'existe pas |
| Vitesse de déplacement | Movement Speed | `Mobile::m_vitesse` (fixe) ; pas affichée |
| Vitesse d'attaque (modificateur, 100 % de base) | Attack Speed | n'existe pas (la vitesse de base vient de l'arme) |
| Vitesse d'incantation | Cast Speed | n'existe pas |
| Recharge des skills (%) | Skill Recharge | n'existe pas |
| Durée des effets (%) | Effect Duration | n'existe pas |
| Taille de zone (%) | Area Size | n'existe pas |

## Raretés d'objet

| Concept | Texte |
|---|---|
| Commun | Common |
| Magique | Magic |
| Rare | Rare |

## Affixes (`objets/CatalogueAffixes.cpp`, lignes d'infobulle dans `interface/AffichageObjets.cpp`)

Le nom affiché d'un objet est `[préfixe] modèle [suffixe]` (« Blazing Mace », « Keen Bow of the Fox »).

| Concept | Texte | Position | Ligne d'infobulle |
|---|---|---|---|
| Flamboyant | Blazing | préfixe | Fire Damage: +N |
| Sanctifié | Hallowed | préfixe | Holy Damage: +N |
| Acéré | Keen | préfixe | Penetration: +N % |
| Renforcé | Reinforced | préfixe | Armor: +N |
| du Bœuf | of the Ox | suffixe | Might: +N |
| de l'Ours | of the Bear | suffixe | Constitution: +N |
| du Renard | of the Fox | suffixe | Dexterity: +N |
| de la Chouette | of the Owl | suffixe | Intellect: +N |
| du Lion | of the Lion | suffixe | Resolve: +N |
| d'Esquive | of Evasion | suffixe | Dodge vs Melee: +N % |
| de Déviation | of Deflection | suffixe | Dodge vs Projectiles: +N % |
| de Blocage | of Blocking | suffixe | Block: +N % |
| de Renouveau (régénération d'énergie) | of Replenishment | suffixe | +N % Energy Regeneration |

## Classes (`escouade/Specialite.cpp`)

| Concept | Texte |
|---|---|
| Guerrier | Warrior |
| Prêtre | Priest |
| Mage | Mage |
| Paladin | Paladin |
| Rôdeur | Ranger |

## Catégories de monstres (pas affichées ; `monde/CategorieDeMonstre.hpp`)

| Concept | Texte |
|---|---|
| Nuée | Swarm |
| Infanterie | Infantry |
| Infanterie lourde | Heavy Infantry |
| Créature monstrueuse | Monstrous Creature |

## Skills (`interface/AffichageSkills.cpp`, sigles dans `interface/BarreDeSkills.cpp`)

| Concept | Texte | Sigle |
|---|---|---|
| Frappe puissante | Power Strike | PS |
| Provocation | Taunt | TA |
| Soin | Heal | HE |
| Explosion | Explosion | EX |
| Frappe sacrée | Holy Strike | HS |
| Imposition des mains | Lay on Hands | LH |
| Tir perçant | Piercing Shot | PC |
| Pluie de flèches | Rain of Arrows | RA |
| Bénédiction | Blessing | BL |

## Effets actifs (`interface/AffichageEffets.cpp`)

| Concept | Texte |
|---|---|
| Titre du bloc de l'inventaire | Active effects |
| Une ligne : skill, valeur, secondes restantes (arrondies vers le haut) | « Blessing: +20 % Armor (10 s) », « Power Strike: −25 % Armor (8 s) » (vert pour un bonus, rouge pour un malus) |
| Statistique modifiée (armure) | Armor (aussi « Armor: N » en tête de la colonne de l'inventaire, effets actifs compris) |

## Textes flottants (`interface/TextesFlottants.cpp`)

| Concept | Texte |
|---|---|
| Critique | `Critical!` à la suite du nombre (« 12 Critical! ») |
| Esquivé | Dodged |
| Bloqué | Blocked (« Blocked −40 » si le blocage est partiel) |
| Provoqué | Taunted |
| Sac plein (l'objet visé reste au sol) | Bag full |
| Niveau atteint (au-dessus de l'escouade) | Level up! |
| Skill refusé faute d'énergie (au-dessus du lanceur) | Not enough energy (bleu violacé) |
| Coup gratuit (attaque d'opportunité) | un `!` après le nombre (« 7! ») |

## Emplacements et catégories d'objet (`interface/AffichageObjets.cpp`)

| Emplacement | Texte | Catégorie | Texte |
|---|---|---|---|
| Tête | Head | Casque | Helmet |
| Torse | Chest | Armure | Body Armor |
| Jambes | Legs | Jambières | Leggings |
| Gants | Hands | Gants | Gloves |
| Bottes | Feet | Bottes | Boots |
| Main droite | Main Hand | Bouclier | Shield |
| Main gauche | Off Hand | Relique | Relic |
| Collier | Necklace | Collier | Necklace |
| Anneau 1, Anneau 2 | Ring 1, Ring 2 | Anneau | Ring |
| | | Arme | Weapon |

## Objets (`objets/CatalogueObjets.cpp`, sigles dans `interface/AffichageObjets.cpp`)

| Concept | Texte | Sigle |
|---|---|---|
| Robe | Robe | RO |
| Jambières de tissu | Cloth Leggings | CL |
| Capuchon | Hood | HD |
| Gants de tissu | Cloth Gloves | CG |
| Chaussons | Slippers | SL |
| Justaucorps de cuir | Leather Jerkin | LJ |
| Jambières de cuir | Leather Leggings | LL |
| Calotte de cuir | Leather Cap | LC |
| Gants de cuir | Leather Gloves | LG |
| Bottes de cuir | Leather Boots | LB |
| Cotte de mailles | Chain Mail | CM |
| Jambières de mailles | Chain Leggings | CE |
| Coiffe de mailles | Chain Coif | CF |
| Gants de mailles | Chain Gloves | CH |
| Bottes de mailles | Chain Boots | CT |
| Armure de plaques | Plate Armor | PA |
| Jambières de plaques | Plate Greaves | PG |
| Heaume intégral | Great Helm | GH |
| Gantelets | Gauntlets | GT |
| Solerets | Sabatons | SB |
| Bouclier | Shield | SH |
| Orbe | Orb | OR |
| Relique sacrée | Holy Relic | HR |
| Collier de protection | Necklace of Protection | NP |
| Anneau de protection | Ring of Protection | RP |
| Anneau de puissance | Ring of Might | RM |
| Collier d'endurance | Necklace of Constitution | NC |
| Dague | Dagger | DA |
| Épée à une main | Sword | SW |
| Hache à une main | Hand Axe | HA |
| Masse à une main | Mace | MA |
| Lance | Spear | SP |
| Épée à deux mains | Greatsword | GS |
| Hache à deux mains | Great Axe | GA |
| Masse à deux mains | Maul | MU |
| Baguette | Wand | WA |
| Sceptre | Scepter | SC |
| Bâton | Staff | ST |
| Arc | Bow | BO |
| Arbalète | Crossbow | CB |

## Lignes d'infobulle et statistiques (`interface/AffichageObjets.cpp`, `AffichageStatistiques.cpp`)

| Concept | Texte |
|---|---|
| Armure d'un objet | Armor (« Armor: 10 » ; avec les affixes, « Armor: +3 » sur une ligne à part) |
| Type d'une pièce d'armure (sous la catégorie) | Cloth Armor, Light Armor, Medium Armor, Heavy Armor |
| Malus d'esquive d'une pièce (rouge) | Dodge (« Dodge: −3 % » quand les deux esquives sont égales), sinon Dodge vs Melee / Dodge vs Projectiles ; le signe moins est U+2212 |
| Niveau de l'objet (sur tous les objets, sous la catégorie) | Item Level (« Item Level 3 ») ; la qualité n'est pas affichée |
| Dégâts, recharge, portée | Damage (« Damage: 13–17 » : les bornes de la plage, avant la Puissance ; un seul nombre sans écart), Cooldown (« 0.3 to 0.7 s »), Range (« 20 px ») |
| Nature de l'attaque : contact, projectile, zone | Melee, Projectile, Area |
| Pénétration d'armure | Penetration (« Penetration: 30 % ») |
| Deux mains | Two-handed |
| Seuil de caractéristique | Requires Might 11 |
| Objet inactif | Inactive: requirement not met |
| Blocage d'un bouclier (infobulle, deux lignes) | Block: 20 %, Absorbs: 8 |
| Bonus de capacité offensive d'un objet | Offensive Ability: +10 |
| Capacité offensive, défensive | Offensive Ability, Defensive Ability |
| Blocage (inventaire) | Block 30 % (12) |
| Esquive mêlée, esquive projectile (inventaire) | Melee Dodge, Projectile Dodge |
| Bonus d'esquive d'un objet | Dodge vs Melee, Dodge vs Projectiles |
| Points de vie | HP |
| Énergie du personnage (inventaire) | Energy (« Energy 87 / 100 ») |
| Régénération d'énergie (inventaire ; une décimale) | Energy Regen (« Energy Regen 2.3/s ») |
| Résistance à un état de contrôle (inventaire ; un pour cent à une décimale) | Stun Resistance, Slow Resistance, Charm Resistance, Fear Resistance, Disruption Resistance (« Stun Resistance 3.0 % ») |

## Touches de débogage

| Touche | Effet |
|---|---|
| F3 | points visés, chemins, zones de contrôle |
| F4 | carré de l'ancre |
| F5 | nouvelle carte |
| F6 | brouillard de guerre (visuel) |
| F7 | quatre objets tirés au hasard dans le sac (tout ou rien) |
| F8 | prend l'escalier d'où l'on est : descend d'un étage (au dernier étage : la victoire) |
| F9 | donne l'expérience qui manque pour le niveau suivant : exactement un niveau |

## Écrans

| Concept | Texte |
|---|---|
| Menu | Start Game, Options, Quit |
| Options | Mode: Fullscreen / Mode: Windowed, Resolution: < 1280 × 720 >, Back |
| Choix de l'escouade | Choose 4 classes (le nombre suit `s_tailleDeLEscouade`), Start, Back |
| Partie | Depth N / 5 (profondeur, `interface/AffichageProfondeur.cpp`), Level N (niveau de l'escouade, `interface/AffichageNiveau.cpp`, aussi dans l'inventaire) et sa barre d'expérience, Monsters: N (compteur), Seed: N (graine de l'étage, dans la console) |
| Inventaire | Inventory, Bag, Armor: N, (dead), Points: N, Apply, Reset (voir « Caractéristiques ») |
| Fin de partie | Victory, Defeat, Press R to return to squad selection |
