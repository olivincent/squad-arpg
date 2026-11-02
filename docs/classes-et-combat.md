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
- **Retenu.** Un jeu sauvegardé, à la *Titan Quest* : pas un roguelite ; le butin et la progression s'accumulent d'une session à l'autre.
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

- **Proposé.** Ce qu'une couleur ne sait **pas** faire compte autant que ce qu'elle fait : c'est le manque qui force la synergie entre les personnages de l'escouade (4 depuis l'étape 106).
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

- **Retenu.** Une seule classe par personnage. **Quatre personnages depuis l'étape 106** (trois auparavant, étape 74) : le feel d'un groupe de jeu de rôle à la *D&D*, plus de place pour les gambits et l'organisation de l'escouade, et moins de skills actifs par personnage (trois par personnage sur la barre de 12 emplacements). Une même classe peut être prise jusqu'à 4 fois (3 à l'étape 76), en quatre teintes (0, 20, 40, 60 % de noir). Les valeurs d'équilibrage (monstres, courbe d'expérience, butin) n'ont pas changé avec ce passage : à ajuster en jouant.
- **Retenu (dans le principe).** Une **classe de base qu'on spécialise ensuite**, par exemple un mage qui devient mage du feu ou mage du givre.
- **Retenu.** Progression à la *Titan Quest* : en montant le niveau de sa classe, on débloque des skills et on choisit lesquels prendre. On ne peut pas tout prendre, c'est ce qui rend les personnages uniques.
- **Retenu.** Pas d'arbre géant à la *Path of Exile* (intimidant, on s'y perd). La classe garde le joueur dans une lignée cohérente.
- **Proposé.** Combinaison : une barre de maîtrise à la *Titan Quest* (paliers de skills) et une fourche de spécialisation unique et définitive à un certain niveau (à la WoW / BG3). 5 classes × 2 spécialisations donnent 10 profils et 220 escouades possibles avec doublons.

### Les armes : le modèle C (étape 91)

- **Retenu.** **Pas de restriction par classe** (modèle de *Titan Quest*) : n'importe quel personnage peut porter n'importe quelle arme, sous réserve de ses caractéristiques (seuils, étape 92) et de la règle des deux mains.
- **Retenu.** **L'arme décide de l'attaque de base** (dégâts, intervalle de recharge, temps d'exécution, portée, nature contact ou projectile) **et de la zone de contrôle** : une attaque de contact engage (20 px), un projectile non. Sans arme, les mains nues (6 dégâts, contact).
- **Retenu.** **La classe garde** ses skills, son rang dans la formation, ses caractéristiques de base et son équipement de départ.
- **Retenu.** **Les skills peuvent exiger un type d'arme** (liste de types ; vide : aucune exigence). Sans l'arme exigée, le skill est refusé comme celui d'un lanceur mort et son icône est grisée. Un guerrier qui reçoit un arc tire à distance, n'engage plus, et perd sa frappe puissante.
- **Retenu.** Sept types d'arme : épée longue, masse, dague, épée à deux mains, sceptre, bâton, arc. Une arme à deux mains exige la main gauche vide ; la poser range ce qui gêne dans la première case libre du sac (et refuse si le sac est plein), et un bouclier range de même une arme à deux mains.
- **Retenu.** Le **bouclier** a des données propres (chance de blocage, montant absorbé) ; la **récupération** du blocage (3 s) est **commune à tous**, plus une donnée d'objet. D'autres objets pourront plus tard ajouter de la chance de blocage ou réduire la récupération.
- **À tester.** Valeurs des armes : épée longue et masse 15 dégâts (0,3 – 0,7 s, 0,15 s d'exécution), dague 14 (0,4 – 0,8 s), épée à deux mains 60 (1,2 – 2,8 s, 0,3 s), sceptre, bâton et arc 15 à 120 px (0,4 – 0,6 s) ; mains nues 6. Équipement de départ : guerrier épée longue et bouclier rond, paladin masse et bouclier rond, prêtre sceptre, mage bâton, rôdeur arc.
- **Remplacé à l'étape 102.** Les sept armes prototypes et leurs valeurs sont remplacées par **treize gabarits** (dague, épée, hache et masse à une main, lance, épée, hache et masse à deux mains, baguette, sceptre, bâton, arc, arbalète) : un gabarit fixe l'identité (vitesse, portée, nature, mains, pénétration, écart par coup), les dégâts se calculent depuis le niveau et la qualité de l'objet et chaque coup les tire dans une plage. Tableau et règles : `docs/objets.md`, « Les gabarits d'armes, le niveau et la qualité ». Équipement de départ : guerrier épée à une main, paladin masse à une main (tous deux avec bouclier rond et pièces de fer), prêtre sceptre, mage bâton, rôdeur arc.

### L'expérience et les niveaux (étape 99)

- **Retenu.** **Un seul niveau pour toute l'escouade** : tous les personnages ont toujours le même niveau (un niveau par personnage
  viendrait avec un recrutement). L'`Expedition` ne stocke que l'**expérience totale** ; le niveau en est déduit par la fonction
  pure `niveauPour` (`combat/Progression`). Une valeur dérivée n'est jamais stockée, pour qu'elle ne puisse pas diverger de sa
  source.
- **Retenu.** **Toute mort de monstre rapporte son expérience à l'escouade**, quel que soit le tueur (une explosion, un coup
  d'opportunité, l'autodéfense), même si des personnages sont morts. Elle survit à la descente ; une nouvelle expédition (F5,
  « Start ») repart de 0 (niveau 1).
- **Retenu.** **La courbe** : l'expérience cumulée pour atteindre le niveau n est `k × n × (n − 1)` (niveau 1 à 0), sans niveau
  maximum. **À tester** : `k = 21` (`s_experienceDeLaCourbe`).
- **À tester.** **L'expérience des monstres** est une donnée de `DefinitionDeMonstre` : nuée 3, infanterie 10, infanterie lourde 25,
  créature monstrueuse 100 ; `renforcer` la majore de 25 % par étage au-delà du premier, comme les points de vie.
- **Calibrage de `k`** (étape 99.3). Cible : en tuant tous les monstres, environ le niveau 3 à la fin du 1er étage et 10 à 11 à la
  fin du 5e. L'expérience moyenne d'un étage a été mesurée sur 5 000 graines (cartes générées, avec les compositions, leurs poids
  et la créature) : **262** au 1er étage, 332 au 2e, 398 au 3e, 458 au 4e, 523 au 5e, soit un cumul de **262, 593, 991, 1 449 et
  1 972**. Les deux bouts de la cible demandent `k` entre 21,8 et 43,6 (exactement le niveau 3 au 1er étage : `6 k ≤ 262 < 12 k`) et
  entre 14,9 et 21,9 (niveau 10 à 11 au 5e : `90 k ≤ 1 972 < 132 k`) : la fenêtre commune, de 21,8 à 21,9, est trop étroite pour un
  entier. **Retenu après les tests visuels de l'étape 99** (des montées rapides au début, le niveau 10 au 5e étage). Avec **`k = 21`**, la moyenne
  donne les niveaux 4, 5, 7, 8 et 10 ; la répartition sur les graines est 43 % de niveau 3 et 57 % de niveau 4 au 1er étage, 29 %
  de niveau 9 et 70 % de niveau 10 au 5e. Avec `k = 22`, le 1er étage donne 54 % de niveau 3, mais le 5e seulement 53 % de niveau 9
  et 46 % de niveau 10 (jamais 11). `k = 21` garde le 5e étage dans la cible ; le 1er étage est à 3 ou 4 (« environ »). Seul `k`
  s'ajuste, pas l'expérience des monstres. `test_CalibrageDeLExperience.cpp` épingle la cible (niveau 3 à 4 après le 1er étage,
  10 à 11 après le 5e, chaque étage fait monter).
- **Retenu.** **Croissance automatique** (levier 4 de la section 5 : jamais une seule caractéristique) : chaque niveau gagné donne
  **+1 point**, alternativement dans la première puis la seconde caractéristique de la **paire de la classe** (le niveau 2 donne la
  première, le niveau 3 la seconde, le niveau 4 la première…). La paire est une donnée de la classe (`Specialite`) :

| Classe | Première (niveaux pairs) | Seconde (niveaux impairs) |
|---|---|---|
| Guerrier (Warrior) | Puissance (Might) | Endurance (Constitution) |
| Paladin | Puissance (Might) | Volonté (Resolve) *(étape 110 ; Endurance avant)* |
| Prêtre (Priest) | Volonté (Resolve) *(étape 110 ; Puissance avant)* | Savoir (Intellect) *(Volonté avant)* |
| Mage | Savoir (Intellect) *(Puissance avant)* | Volonté (Resolve) *(Savoir avant)* |
| Rôdeur (Ranger) | Adresse (Dexterity) | Puissance (Might) |

- **Retenu.** Les caractéristiques de base d'un personnage sont **calculées** (la base de sa classe plus la croissance du niveau,
  `croissanceAuNiveau`), plus, depuis l'étape 100, les points libres que le joueur a dépensés (section suivante), jamais copiées
  dans l'`Expedition` : une seule source, la classe, l'expérience et les points dépensés. Le Savoir et la Volonté ont des effets depuis
  l'étape 110, et les paires du mage, du prêtre et du paladin en tiennent compte (section 5, « Les effets des caractéristiques »).
- **À tester.** **Points de vie par niveau** : `pointsDeVieMax = 300 + 6 × (Endurance − 10) + 8 × (niveau de classe − 1)` (le gain par niveau de classe, 8, est une donnée de la classe depuis l'étape 110 ; `s_pvParNiveau` en est la valeur par défaut). C'était `240 + 6 × Endurance + 8 × (niveau − 1)` : le même résultat.
- **Retenu.** **Montée de niveau au milieu d'un étage** : les caractéristiques, les statistiques, le maximum de points de vie et
  les objets actifs sont recalculés tout de suite, par le chemin de l'équipement (`Personnage::recalculerDepuisLEquipement`) ; les
  points de vie actuels d'un vivant montent du même montant que son maximum (`PointsDeVie::changerMaximum`) ; un mort reste mort
  (il revient à l'étage suivant, avec son nouveau niveau). Plusieurs niveaux d'un coup (la créature au fond) s'appliquent tous. Un
  seuil d'équipement atteint grâce à la croissance remet l'objet en service.
- **Pistes** (étapes à part) : un niveau par personnage si un recrutement arrive ; un niveau maximum ; l'expérience refusée aux
  morts.

### Les points libres (étape 100)

- **À tester.** **Trois points libres par niveau et par personnage**, à partir du niveau 2 (`s_pointsLibresParNiveau`) : au niveau n,
  un personnage a reçu `3 × (n − 1)` points. Chacun les dépense à la main dans ses cinq caractéristiques, en plus de la croissance
  automatique (qui reste, +1 par niveau dans la paire de la classe).
- **Retenu.** **Une seule source** : l'`Expedition` garde, pour chaque personnage, les points **dépensés** dans chaque
  caractéristique (`MembreDExpedition::pointsDepenses`, un `Caracteristiques`). Les points **disponibles** ne sont jamais stockés :
  `pointsLibresDisponibles(niveau, dépensés)` = reçus au niveau moins la somme des dépensés, une fonction pure (`combat/Progression`).
  La base d'un personnage est toujours sa classe, plus la croissance de son niveau, plus ses points dépensés
  (`Personnage::baseAuNiveau`, l'unique calcul).
- **Retenu.** **Dépenser est définitif** : pas de remboursement. La redistribution viendra avec les refuges, payante.
- **Retenu.** **Une dépense est transactionnelle** : `Personnage::depenserDesPoints` (et `Escouade::depenserDesPoints(place, dépense)`)
  refuse tout, sans rien changer, si une valeur est négative ou si la somme dépasse les points disponibles (`depensePermise`).
  Sinon elle passe par le même chemin qu'une montée de niveau (`recalculerDepuisLEquipement`) : caractéristiques, statistiques,
  maximum de points de vie et objets actifs sont recalculés ; les points de vie actuels d'un vivant montent du même montant que son
  maximum ; un mort peut dépenser ses points (il les aura en revenant) ; un seuil d'équipement atteint grâce à la dépense remet
  l'objet en service. On dépense à tout moment, même en combat.
- **Retenu.** **Le brouillon est un état de l'écran** (`EcranInventaire`, `docs/ecrans.md`), pas de l'expédition : « + » met un
  point en attente, « − » ne retire qu'un point en attente (jamais un point déjà dépensé), « Apply » dépense, « Reset » et la fermeture
  de l'inventaire effacent. Les valeurs dérivées (points de vie, capacités, armure) ne montrent pas le brouillon : elles changent à « Apply ».
- **Retenu.** **L'infobulle d'une caractéristique** dit ce qu'un point fait, avec les nombres lus dans les constantes du jeu
  (`s_bonusParPoint`, `s_pvParPointDEndurance`, `s_capaciteParPointDAdresse`…), jamais recopiés (textes de l'étape 110, `docs/glossaire.md`) : « Might: +1% weapon damage per
  point », « Constitution: +6 health and +0.3% Stun and Slow resistance per point », « Dexterity: +5 offensive and defensive capability per point »,
  « Intellect: +1% spell damage, +0.3% area and +0.3% duration per point », « Resolve: +3 energy, +1% healing and +0.3% Charm, Fear and Disruption resistance per point ». Le signal « Points to spend » apparaît en haut de l'écran tant qu'un personnage (mort compris) a
  des points libres.
- **Pistes** (étapes à part) : la **redistribution payante aux refuges** ; **l'aperçu des valeurs dérivées** pendant le brouillon (points de vie,
  capacités avant « Apply ») ; l'effet de l'Intellect et de la Resolve ; les rangs de skills et les points de skills.

## 5. Les caractéristiques

- **Retenu.** Des caractéristiques **par effet** (à la *Pillars of Eternity*), pas une stat de dégâts par classe : le joueur doit vraiment hésiter où mettre ses points.
- **Retenu.** Des noms **immersifs**, avec une infobulle qui donne les chiffres exacts. Les noms ci-dessous sont des noms de travail, faciles à changer. **Depuis l'étape 95, le jeu affiche des noms anglais** (noms de travail eux aussi) : Puissance → Might, Endurance → Constitution, Adresse → Dexterity, Savoir → Intellect, Volonté → Resolve (`docs/glossaire.md`). Ce document garde les noms français, qui sont aussi ceux du code.
- **Retenu.** **Cinq** caractéristiques. Trois, c'était trop peu : la Vigueur donnait dégâts **et** points de vie (une caractéristique que tout le monde monte), et l'Esprit mélangeait un besoin de lanceur de sorts (zones) et un besoin de tank (résistance au contrôle).
- **Retenu.** La **vitesse** n'est pas une caractéristique (difficile à contrôler, casse le jeu). Elle vient de l'arme (intervalle de recharge propre à chaque type d'arme), de l'équipement (réduction de recharge, vitesse) et des skills (*Hâte*).
- **Retenu.** Deux caractéristiques offensives, deux défensives et un pivot :

| Caractéristique | Effet principal | Effet secondaire | Seuil d'équipement |
|---|---|---|---|
| **Puissance** | Dégâts et soins (+ %, modéré) | — | Armes lourdes |
| **Endurance** | Points de vie | Résistance au contrôle physique (étourdissement, recul, enracinement) | Armures lourdes |
| **Adresse** | Capacité offensive (critiques) | Capacité défensive | Armes de finesse, arcs |
| **Savoir** | Taille des zones | Durée des effets infligés | Bâtons, grimoires |
| **Volonté** | Résistance au contrôle magique (peur, silence, charme) | Résistance à l'interruption | Symboles sacrés, focaliseurs |

- **Retenu.** Le contrôle est séparé en **physique** (Endurance) et **magique** (Volonté), en miroir d'Aegis et de Ward (section 9).
- **Retenu** (étape 109). Les états de la liste cible sont Stun et Slow (contrôle physique), Charm, Fear et Disruption (contrôle magique), chacun avec une résistance qui en réduit la durée (section 13). Les exemples d'états du tableau ci-dessus (recul, enracinement, silence) sont des noms de travail antérieurs : la section 13 prévaut.
- **Retenu.** Garde-fous pour le Savoir (taille des zones et durée des effets sont les deux effets qui cassent le plus souvent un jeu) :
  1. **Coefficients par skill** pour la zone et la durée, comme pour les dégâts (levier 2) : chaque skill déclare combien il profite du Savoir, et certains n'en profitent pas (par exemple la provocation).
  2. **Le bonus de zone porte sur l'aire, pas sur le rayon** (+40 % d'aire ≈ +18 % de rayon), avec un plafond. La surface croît avec le carré du rayon : un bonus sur le rayon grandirait trop vite.
  3. **Contrôle : coefficient faible et rendement décroissant sur la cible.** Un monstre contrôlé plusieurs fois de suite le reste de moins en moins longtemps (section 12).
  4. **Un buff ne dure jamais plus longtemps que sa recharge**, même au plafond de Savoir. Sinon, un petit gain de caractéristique rend le buff permanent (effet de palier). Règle vérifiée par un test qui parcourt le catalogue de skills.
- Note de structure : **un seul endroit calcule les valeurs effectives d'un skill** (zone, durée, dégâts) à partir de sa définition et des caractéristiques du lanceur. L'infobulle, le cercle de prévisualisation et le combat lisent tous ce résultat, pour que l'infobulle ne mente jamais.
- **Retenu.** Une caractéristique peut ne servir à rien pour une classe donnée. En revanche, une classe ne doit **jamais se définir par une seule caractéristique plus Endurance** (schéma de WoW et de Diablo).
- **Retenu.** Leviers contre ce schéma :
  1. Adresse aussi rentable que Puissance pour frapper (critique fort, à la *Titan Quest*).
  2. Les skills puisent dans plusieurs caractéristiques (un saignement profite du Savoir par sa durée) : c'est le choix des skills qui oriente les caractéristiques, pas la classe.
  3. Des seuils d'équipement sur deux caractéristiques (épée bâtarde : Puissance et Adresse).
  4. Une croissance automatique répartie sur deux caractéristiques, jamais une seule.
- **Retenu** (fait à l'étape 99 pour la classe, pas encore pour la spécialisation). Trois sources : une **petite croissance automatique orientée par la spécialisation** (un mage du feu gagne plutôt de la Puissance, un mage du givre plutôt du Savoir), l'**essentiel en points libres**, et l'**équipement**.
- **Retenu.** L'équipement peut augmenter une caractéristique **et/ou** directement un élément de la mécanique de combat (par exemple + critique, + résistance au feu, + esquive contre les projectiles).
- **Retenu.** L'**énergie** (étape 108, la ressource des skills) : son maximum vient du niveau de classe et, depuis l'étape 110, de la Volonté (3 par point au-dessus de 10).
- **Retenu.** Redistribution des points **payante, seulement dans les refuges** (village, campement), jamais en pleine quête.
- **Retenu.** Seuils d'équipement à la *Titan Quest* (étape 92) : un objet peut exiger des valeurs minimales de caractéristiques ; **sous le seuil, impossible de l'équiper** (la vérification compte les caractéristiques effectives du destinataire sans l'objet posé, en tenant compte de ce que le déplacement retire).
- **Retenu** (étape 92). **Objet inactif** : un objet porté dont le seuil n'est plus atteint (les caractéristiques ont baissé) **reste porté mais n'a plus aucun effet** (ni armure, ni bonus, ni arme, ni bouclier) ; il se remet en service quand le seuil revient. Calcul par point fixe (un objet retiré peut en désactiver un autre). Affichage : seuils dans l'infobulle, en rouge ceux qu'un personnage n'atteint pas pendant un glissement, objet inactif encadré en rouge.
- **À tester** (étape 92). Valeurs : épée longue et masse Puissance 11 ; épée à deux mains Puissance 13 et Endurance 12 ; dague Adresse 10 ; arc Adresse 12 ; bâton Savoir 12 ; sceptre Volonté 11 ; bouclier rond Endurance 11 ; casque, cuirasse et jambières de fer Endurance 12 ; cuir et bijoux aucun. Chaque classe porte son équipement de départ avec ses caractéristiques de base ; personne ne porte l'épée à deux mains au départ.
- **Remplacé à l'étape 102.** Les seuils fixes par modèle deviennent une règle unique : `6 + pente × (niveau de l'objet − 1)` par caractéristique exigée (pente 2 pour la principale, 1 pour une secondaire). Au niveau 1 tout vaut 6, donc toute classe porte tout objet du catalogue.
- **Retenu** (étape 91). **Échelle × 3** : 1000 points de vie au départ, c'est trop haut pour un jeu dont les nombres grandiront avec les niveaux (*Titan Quest* commence à 300). Toutes les valeurs en points de vie (points de vie, dégâts, soins, armures, montant bloqué) passent à 30 % de leur valeur d'avant ; les pourcentages et les rapports ne changent pas (3 % par point de Puissance, capacités `50 + 5 × Adresse`, esquives, plancher de 20 %).
- **Retenu** (étape 91). **Points de vie de départ d'environ 300** : une **base commune à toutes les classes, 240**, plus **6 par point d'Endurance** (guerrier 324, paladin 318, prêtre et rôdeur 300, mage 288) ; plus de base de points de vie par classe, l'Endurance fait seule la différence.
- **À tester** (étape 88 ; bases des trois classes remplacées et pourcentage de la Puissance passé à 1 % à l'étape 110, voir « Les effets des caractéristiques »). Valeurs de référence : référence 10 pour tous les effets ; 3 % de bonus par point de Puissance au-dessus (ou en dessous) de 10 ; 6 points de vie par point d'Endurance (20 sur une base par classe avant l'étape 91) ; bases de classe de somme 50 (guerrier 12/14/10/6/8, paladin 11/13/8/7/11, prêtre 10/10/8/10/12, mage 10/8/8/14/10, rôdeur 10/10/14/8/8 pour Puissance/Endurance/Adresse/Savoir/Volonté). Puissance et Endurance ont un effet depuis l'étape 88, l'Adresse depuis l'étape 89 (capacités offensive et défensive, section 7) ; Savoir et Volonté n'en ont pas encore.

### Les effets des caractéristiques (étape 110)

Principe : une caractéristique donne un **bonus d'appoint** ; l'essentiel vient de l'équipement et des skills, et une caractéristique seule n'atteint jamais un plafond. Niveau maximum visé : 60. Deux familles d'effets. Toutes les valeurs sont « à tester ».

**Exception à `ARCHITECTURE.md`** (décision d'Olivier, pour cette étape seulement) : les statistiques que les caractéristiques alimentent (résistances au contrôle, bonus d'aire et de durée) sont **calculées, affichées et testées maintenant**, même si aucune mécanique ne les lit encore ; Olivier pourra ainsi concevoir ses skills. Les mécaniques qui les liront (étourdissement, ralentissement, charme, peur, Disruption, Aegis, Ward, buffs, dégâts dans le temps) viennent plus tard.

- **Retenu.** **Effets intrinsèques**, toujours actifs : la **table de contributions** (`combat/Contributions`), des lignes (caractéristique, statistique, poids) et **une seule forme de calcul**, `valeur = base + poids × (caractéristique − 10)` (`apportDesCaracteristiques` fait la somme des lignes d'une statistique ; une statistique pourra en avoir plusieurs). La base reste dans la règle de chaque statistique (`pointsDeVieMax`, `energieMax`, `statistiquesDuPersonnage`), qui arrondit.

| Caractéristique | Statistique | Base | Poids par point |
|---|---|---|---|
| Constitution | Max Life | 300 plus le gain par niveau de classe | 6 |
| Constitution | Stun Resistance, Slow Resistance | 0 % | 0,3 % |
| Dexterity | Offensive Ability, Defensive Ability | 100 | 5 |
| Resolve | Max Energy | 100 plus le gain par niveau de classe | 3 |
| Resolve | Charm, Fear et Disruption Resistance | 0 % | 0,3 % |
| Intellect | Area Size (bonus d'**aire**) | 0 % | 0,3 %, plafond +100 % |
| Intellect | Effect Duration | 0 % | 0,3 %, plafond +100 % |

Max Life et les capacités donnent exactement les résultats d'avant (300 + 6 × (C − 10) = 240 + 6 × C ; 100 + 5 × (D − 10) = 50 + 5 × D). Sous 10, une caractéristique donne un léger malus. Les résistances au contrôle (un `ParEtat<float>` dans `StatistiquesDeCombat`) réduisent la durée de l'état subi et sont bornées de −100 % à 80 % à l'usage (`resistanceAuControleEffective`) ; les monstres les ont dans leurs statistiques, à 0 par défaut.

- **Retenu.** **Effets par coefficient** : le skill déclare combien il profite de chaque caractéristique (de 0 à 1) pour son **montant**, son **aire** et sa **durée**.
  - **Montant** (dégâts, soins, plus tard Aegis et Ward donnés) : **1 % par point au-dessus de 10** (3 % avant l'étape 110), sans plafond, la même règle pour Might, Intellect et Resolve (`s_bonusParPoint`, `montantEffectif`).
  - **Aire** : 0,3 % **d'aire** par point d'Intellect, `rayon = rayon de base × √(1 + coefficient × bonus d'aire)`, bonus total plafonné à +100 % d'aire (rayon ×√2). **Durée** : 0,3 % par point d'Intellect, `durée × (1 + coefficient × bonus)`, plafonnée à +100 %. Le plafond porte sur le **total des sources** (aujourd'hui la seule est la caractéristique ; l'équipement et les skills s'y ajouteront) et précède le coefficient du skill (`combat/ValeursEffectives`).
  - **Le seul endroit** qui calcule les valeurs effectives d'un skill est `Personnage` (`attaqueDuSkill` pour le montant et la durée, `rayonEffectifDuSkill` pour le rayon) ; le cercle de visée et la zone touchée lisent le même rayon.
- **Retenu.** **Convention des coefficients** (skills existants) : arme, frappes et attaques de base, **Might** ; sorts de dégâts, **Intellect** (montant et aire) ; soins, plus tard Aegis et Ward donnés, **Resolve** ; une zone qui n'est pas un sort (la pluie de flèches) a un **coefficient d'aire 0** ; la provocation n'a aucun coefficient (ni montant, ni aire, ni durée). Catalogue : Frappe puissante Might ; Provocation aucun ; Soin Resolve ; Explosion Intellect, aire 1 ; Frappe sacrée Might ; Imposition des mains Resolve ; Tir perçant et Pluie de flèches Might (aire 0). **L'attaque de base** vient du gabarit d'arme (`DonneesDArme::coefficients`, copié dans `DescriptionDAttaque`) : Intellect pour la baguette, le sceptre et le bâton (armes de lanceur), Might pour toutes les autres et pour les mains nues.
- **Retenu.** **Dégâts dans le temps** (quand ils existeront) : **coefficient de durée 0**, pour éviter le double gain montant × durée. À respecter à la création de ces skills.
- **Retenu, documenté seulement.** **Un buff ne dure jamais plus longtemps que sa recharge**, même au plafond d'Intellect (ci-dessus, garde-fou 4). Elle sera codée et testée avec les buffs.
- **Retenu.** **Niveau de classe** (préparation du système à la *Titan Quest*, codé plus tard) : plus tard, les points de skill iront soit dans des skills, soit dans le niveau de la classe, qui augmente les statistiques de base ; le niveau du joueur ne donnera plus de points de vie ni d'énergie. Dès l'étape 110, les gains par niveau sont des **données de la classe** (`DefinitionPersonnage` : `pvParNiveauDeClasse` 8, `energieParNiveauDeClasse` 5, la paire de croissance) et les calculs reçoivent un niveau de classe (`niveauDeClasse`, égal au niveau du joueur tant que les points de skill n'existent pas : le jeu se joue comme avant). Les points libres de caractéristiques (3 par niveau du joueur) ne changent pas.
- **À tester.** **Bases des classes** (somme toujours 50, ordre Might / Constitution / Dexterity / Intellect / Resolve) : guerrier 12 / 14 / 10 / 6 / 8, paladin 11 / 13 / 8 / 7 / 11, prêtre 8 / 10 / 8 / 12 / 12, mage 6 / 8 / 8 / 16 / 12, rôdeur 10 / 10 / 14 / 8 / 8. Paires de croissance : mage Intellect puis Resolve, prêtre Resolve puis Intellect, paladin Might puis Resolve ; guerrier et rôdeur inchangés. L'énergie maximum au niveau 1 devient donc propre à chaque classe (100 + 3 × (Resolve − 10) : guerrier 94, paladin 103, prêtre 106, mage 106, rôdeur 94).
- **Hors portée de l'étape 110.** Les mécaniques qui lisent les nouvelles statistiques ; le système de points de skill ; des affixes d'équipement pour elles ; les contributions croisées (deux caractéristiques vers une même statistique) ; le rendement décroissant du contrôle sur une même cible ; les poids des capacités ; le rééquilibrage des monstres.

## 6. Le déroulement d'un coup

**Retenu.** L'ordre compte autant que les nombres. Des jets **successifs et indépendants**, plutôt que la table de jet unique de WoW (plus opaque, elle crée des seuils que les joueurs optimisent).

1. **Peut-il agir ?** Recharge écoulée, pas sous contrôle, activation terminée.
2. **Le coup est-il esquivé ?** Esquive contre la mêlée ou contre les projectiles. Si oui : « Esquivé », le coup s'arrête là. *(Fait à l'étape 90, selon la nature de l'attaque, section 7.)*
3. **Est-ce un critique ?** Capacité offensive contre capacité défensive. *(Fait à l'étape 89 : fonction pure d'un tirage, voir section 7.)*
4. **Dégâts de départ.** Arme ou skill, plus les coefficients par caractéristique du skill, multipliés par le critique. Le critique s'applique **avant** toute réduction : contre une armure par soustraction, c'est ce qui en fait l'arme naturelle contre l'infanterie lourde. *(Depuis l'étape 94, le critique et la Puissance s'appliquent à **toutes** les portions du coup, bonus élémentaires compris.)*
5. **Blocage au bouclier** (section 7). Un montant fixe est retiré. Si tout est absorbé : « Bloqué ». *(Fait à l'étape 90 : le montant retiré l'est avant l'armure ; un blocage total saute l'armure et son plancher. Depuis l'étape 94, il ne retire que de la portion **physique**.)*
6. **Armure et résistances.** L'armure réduit la part physique, chaque résistance réduit la part de son élément. *(Fait à l'étape 94.)*
7. **Aegis** (le physique) **et Ward** (le reste) (section 9) absorbent ce qui reste, puis les points de vie encaissent le reste.
8. **Effets secondaires.** Vol de vie, effets dans le temps, contrôle.

- **Retenu** (étape 94). Un coup est une petite collection de **portions typées** (`Degats`, une quantité par `Element` : par exemple 30 physique + 10 feu), chacune réduite par sa propre défense, plutôt qu'un seul nombre. Chaque étape est une fonction pure testable seule, dans l'esprit d'`appliquerArmure` (étape 55) ; `resoudreCoup` les enchaîne.
- **Retenu** (étape 94). **Un seul arrondi par portion, en fin de calcul**, au plus proche, sans arrondi intermédiaire (ni après le critique, ni après le blocage, ni après l'armure : comme la pénétration). Le total est la somme des portions arrondies ; **un coup ni esquivé ni entièrement bloqué inflige au moins 1**, dans la plus grosse portion (à égalité, la première de l'énumération). Cela remplace le « au moins 1 » par étape de l'armure.
- **Retenu** (étape 94). Le soin et la provocation ne sont pas des portions : un soin a son propre montant (il critique, sans élément) et une provocation sa durée.

## 7. Toucher, esquive et critique

- **Proposé.** Presque tout touche : pas de jet de toucher qui rate. La variété vient des critiques et de l'esquive.
- **Retenu.** Esquive en pourcentages, à la *Titan Quest* : une contre la mêlée, une contre les projectiles.
- **Retenu** (étape 90). **Plafond de l'esquive : 50 %** (`s_esquiveMaximale`). Plus tard, une distribution pseudo-aléatoire (à la *Dota 2*) contre les séries d'esquives.
- **Retenu** (étape 90). **La nature de l'attaque** décide : chaque attaque est de nature `contact`, `projectile` ou `zone`, une donnée de l'attaque (à l'étape 91, l'arme la fournira pour l'attaque de base). Aujourd'hui : mêlées, monstres, frappe puissante, frappe sacrée, coups d'opportunité = contact ; tireurs, tir perçant = projectile ; explosion, pluie de flèches = zone. Les projectiles magiques (boule de feu) s'esquivent comme une flèche. **Les zones ne s'esquivent jamais** (ni ne se bloquent) : elles s'évitent en se déplaçant (section 10). Les soins et la provocation ne sont pas des attaques.
- **À tester** (étape 90). Valeurs de l'esquive : monstres 5 % contre le contact et 5 % contre les projectiles ; personnages 0, plus l'équipement (bottes de cuir +5 % contre le contact, casque de cuir +5 % contre les projectiles). Un coup esquivé ou entièrement bloqué **compte comme une attaque reçue** : le monstre visé s'alerte, et un monstre dont le coup sur un personnage est esquivé ou bloqué devient agresseur de l'escouade (c'est l'intention d'attaquer qui compte).
- **Retenu (dans le principe).** Le critique dépend de la **relation** entre la capacité offensive de l'attaquant et la capacité défensive du défenseur (à la *Titan Quest* / *Grim Dawn*), pas d'un pourcentage fixe de l'attaquant. Plus l'écart est grand, plus le critique est fréquent et fort.
- **Retenu** (étape 89). La relation est le **rapport** `r = capacité offensive / capacité défensive`. C'est lui, et pas une différence ni un niveau, qui règle à la fois la chance et la force : même capacité des deux côtés (`r = 1`) donne un petit critique rare, le double donne un critique fréquent et fort. Le critique s'applique avant le blocage et l'armure, au montant effectif (Puissance comprise).
- **À tester** (étape 89). Chance `min(0,05 × r², 0,50)` (1,25 % à `r = 0,5`, 5 % à `r = 1`, 20 % à `r = 2`, 45 % à `r = 3`, plafond de 50 %) ; multiplicateur `clamp(1,5 + 0,25 × (r − 1), 1,5 ; 2,0)` (×1,5 à `r = 1`, ×1,75 à `r = 2`, ×2 dès `r = 3`). Constantes nommées dans `Critique.hpp`. La capacité d'un personnage est `50 + 5 × Adresse` pour les deux côtés (100 à Adresse 10, 120 pour le rôdeur) ; celle d'un monstre ordinaire, 100 et 100, est une donnée de sa définition.
- **Retenu** (étape 89). **La magie critique**, comme le reste : les attaques de base, les skills de dégâts et de zone (un tirage par monstre touché) et les coups d'opportunité, dans les deux camps. **Les soins critiquent aussi**, par la même courbe, contre une **capacité défensive de référence de 100** (`s_capaciteDefensiveDeReference`, qui suivra le niveau de la zone quand les niveaux existeront) ; le soin critique reste plafonné aux points de vie maximum. Les effets dans le temps ne critiqueront pas.
- **Retenu** (étape 89). Le calcul est séparé du tirage : chaque étape qui dépend du hasard est une fonction pure qui reçoit un nombre de 0 inclus à 1 exclu ; le tirage a lieu à un seul endroit, dans `resoudreCoup`, et seulement si la chance est non nulle. Aucune distribution pseudo-aléatoire contre les séries de critiques (comme pour l'esquive, voir plus haut).
- **Retenu.** Blocage au bouclier à la *Titan Quest* : une **chance** de bloquer, un **montant fixe** absorbé (pas tout le coup), puis une **récupération** pendant laquelle le bouclier ne bloque plus. Effet voulu : une nuée « consomme » le bouclier avec ses petits coups, et le gros coup d'une créature monstrueuse passe pendant la récupération. Le tank a une vraie faiblesse face aux mélanges d'ennemis.
- **À tester.** Les valeurs du blocage (chance, montant, durée de récupération). *(Étape 90 : bouclier rond, 30 % de chance, 40 points absorbés, 3 s de récupération ; étape 103 : le Shield, 20 % de chance plus 1 point par niveau, 8 points absorbés au niveau 1, 3 s.)*
- **Retenu** (étape 90). Règles du blocage : il bloque le contact et les projectiles, jamais les zones ; il vient après le critique et avant l'armure ; un blocage réussi lance la récupération, pendant laquelle aucun blocage n'est tenté ; d'autres bonus de chance ou de montant s'additionneraient, la récupération est celle du bouclier ; un blocage qui absorbe tout le coup l'arrête, sans armure ni plancher. La récupération est un état du défenseur (`Vitalite`), qui s'écoule avec le temps. Les monstres n'ont pas de bouclier, mais rien n'empêche d'en avoir un. 
- **Retenu** (étape 94, question 12 tranchée). **Le bouclier ne réduit que la portion physique.** Les portions élémentaires passent toujours : la magie se distingue par son élément, sans drapeau « magique ». Le blocage n'est tenté que si l'attaque est de contact ou un projectile **et** a une portion physique non nulle : une boule de feu (projectile purement élémentaire) ne déclenche ni tirage ni récupération. Une portion physique entièrement absorbée ne subit ni l'armure ni son plancher ; les autres portions continuent. Le texte « Bloqué » (sans montant) n'apparaît que si le coup entier est absorbé, donc s'il n'y avait que du physique ; sinon « Bloqué −N », puis le total des dégâts qui passent. *Piste : un bouclier « anti-magie » qui bloquerait aussi des portions élémentaires (section 15).*

## 8. Armure, pénétration et résistances

- **Retenu.** Un modèle **mixte** soustraction / pourcentage, avec de la **pénétration d'armure**.
- **Retenu.** Une réduction ne ramène **jamais** un coup à zéro. Seuls deux événements l'annulent, et ils sont affichés : l'**esquive** (« Esquivé ») et un **blocage** qui absorbe tout le coup (« Bloqué »).
- **Retenu** (étape 87). Modèle M1 : `dégâts = max(D − armure, D × plancher)`. L'armure retire un montant fixe par coup, le coup garde toujours au moins un pourcentage. Lisible (« retire 15 par coup, jamais plus de 80 % ») et à effets émergents : la dague s'effondre contre l'infanterie lourde, l'épée à deux mains tient ; une nuée ne fait presque rien au tank, donc elle cherche à le contourner. Il remplace le modèle de l'étape 55 (pourcentage à rendement décroissant, K = 100), avec le même plancher à 1. Les points de vie, dégâts et soins sont à l'échelle × 10 pour que les entiers aient assez de finesse, puis ramenés à 30 % à l'étape 91 (échelle × 3 par rapport à l'étape 86 ; armure de départ 9, monstres 3).
  - Alternative écartée pour l'instant : M2 à la *Path of Exile*, `réduction = A / (A + 5 × D)`, plus douce mais moins lisible.
- **À tester.** Plancher de départ : 20 % (`s_plancherArmure`, `Defense.cpp`). C'est lui qui décide si une nuée est « inoffensive » ou « gênante » contre un tank.
- **Retenu** (étape 92). Pénétration d'armure **en %** (ignore X % de l'armure) : identité des armes de spécialiste (estoc, dague de miséricorde, arbalète, pic de guerre). Elle réduit l'armure du défenseur **avant** la soustraction M1 : `armure effective = armure × (1 − pénétration)`, sans arrondi intermédiaire. Propriété de l'arme, portée par l'attaque de base (coups d'opportunité compris) ; un skill n'en a pas (il a sa propre puissance) ; pas de bonus de pénétration par l'équipement à cette étape. La dague : 30 %.
- **À tester** (étape 92). Plafond de la pénétration : 75 % (`s_penetrationMaximale`, `Defense.hpp`).
- **Retenu.** Les effets dans le temps (saignement, poison…) **ignorent l'armure** ; seules les résistances s'appliquent. Le saignement (du physique dans le temps, voir plus bas) devient une réponse naturelle à l'infanterie lourde. *Étape 109 : tout élément peut avoir des dégâts dans le temps, résistés par la résistance de cet élément ; ils ne critiquent pas (section 13).*
- **Retenu** (étape 94). **L'armure ne protège que contre le physique.** Chaque élément a sa **résistance en %** : une seule règle pour tous, physique compris (`portion × (1 − résistance)`, sans arrondi), appliquée après l'armure. La résistance physique vaut 0 partout pour l'instant. Valeurs : `docs/combat.md`.
- **Retenu** (étape 109). **La résistance physique s'applique à tout le physique**, direct ou dans le temps, mais elle est **rare** et son **plafond sera plus bas** que celui des autres résistances. *Le code applique aujourd'hui à la résistance physique le même plafond de 80 % que les autres (`resistanceEffective`) ; la valeur du plafond physique reste à déterminer (section 13, « Pistes non décidées »).*
- **À tester** (étape 94, question 3 tranchée pour les résistances). **Plafond de 80 %** (`s_resistanceMaximale`, `Defense.hpp`, à côté de la pénétration d'armure) ; les résistances **négatives** (vulnérabilités) sont permises jusqu'à −100 % (`s_resistanceMinimale`, le coup est doublé). Hors de [−100, 80], la valeur est ramenée dans l'intervalle à l'application. Les personnages ont toutes leurs résistances à 0 (l'équipement en donnera à l'étape 95) ; les monstres les ont dans leur définition (`docs/monstre.md`) : toutes à 0 sauf la créature monstrueuse, 40 % contre le feu. Les attaques des monstres restent 100 % physiques.
- **Proposé.** Une **pénétration de résistance** pourra exister pour les sorts, gagnée par l'équipement ou les skills, pas propre à toute la magie. *Pas à l'étape 94 : la pénétration d'armure ne touche que l'armure.*
- ~~**Retenu** (étape 94, question 5 tranchée). **Les éléments** : physique, feu, froid, foudre, poison, sacré, nécrotique, saignement. Le **saignement est un type de dégâts** avec sa propre résistance.~~ **Remplacé à l'étape 109** : les éléments sont **huit** — Physical, Fire, Cold, Lightning, Arcane, Necrotic, Holy, Poison — et le saignement n'est plus un type de dégâts, c'est du physique dans le temps (section 13, « Décisions de l'étape 109 »). Arcane remplace la piste « Psychique / Arcane ».
- **Retenu** (étape 94, inchangé). **Dans le code, seulement les éléments qui ont une source** : physique, feu (l'explosion du mage), sacré (la frappe sacrée du paladin) ; les autres s'ajouteront avec leur premier skill ou monstre.
- **Principe.** On ne peut pas se protéger contre tout, et c'est voulu :
  - l'équipement a des profils (plate : beaucoup d'armure, peu de résistances ; robe enchantée : l'inverse ; cuir : entre les deux) ;
  - les couleurs ont un rôle défensif (blanc : armure ; bleu : protection magique) ;
  - les ennemis ont un profil (nid de chamans du feu, chevaliers en plate) ;
  - les points de vie restent la défense universelle.

## 9. Aegis et Ward (à la *Divinity: Original Sin 2*)

*Aegis et Ward sont les noms de la liste cible de l'étape 109 ; ils remplacent les anciens noms de travail « Égide » (physique, blanc) et « Barrière » (magique, bleu).*

- **Retenu.** Garder le principe de *Divinity 2* : des réserves de protection physique et magique en plus des points de vie.
- **Retenu.** Les intégrer comme **effets temporaires** donnés par des skills et un peu d'équipement, pas comme système de base pour tout le monde (qui doublerait nos défenses et pousse, dans *Divinity 2*, aux groupes tout physiques ou tout magiques). Exemple d'équipement : une amulette rare donnant un petit Ward qui se recharge hors combat.
- **Retenu** (étape 109, c'était « proposé »). Ce sont des **points d'absorption**. **Aegis absorbe le physique** ; tant qu'il en reste, la cible est **immunisée à Stun et Slow**. **Ward absorbe tout ce qui n'est pas physique, poison compris** ; tant qu'il en reste, la cible est **immunisée à Charm, Fear et Disruption**. **Tant qu'une réserve tient, le contrôle de son type ne passe pas** (la règle forte de *Divinity 2*) : le combat devient « percer la défense, puis contrôler ».
- **Retenu.** Aegis et Ward absorbent **en dernier**, après l'armure et les résistances, comme une bulle par-dessus les points de vie (à la *Titan Quest*). L'armure garde ainsi sa valeur même sous une réserve. **Les deux absorbent aussi les dégâts dans le temps** (étape 109).
- Note : « armure » désigne déjà la réduction par soustraction ; ces réserves ont besoin d'un autre nom.
- Aegis protège du contrôle physique et Ward du contrôle magique, en miroir de la Constitution et de la Resolve (section 5).
- **Existe** : non (étape 108). Ni Aegis ni Ward, ni aucun état de contrôle, ne sont dans le code.

## 10. Les zones et l'escouade

**Le problème (Olivier).** Le joueur ne contrôle pas les personnages individuellement. Si un sort de zone vise le mage à l'arrière, il faudrait bouger toute l'escouade pour l'éviter.

**Retenu.** Trois réponses combinées comme principes de conception des monstres :

- **a. Les grosses zones visent le groupe** et s'évitent en déplaçant l'escouade entière, comme les grenades de *Dawn of War 2* : larges, bien annoncées (cercle au sol pendant l'activation du monstre).
- **b. Les petites zones sont évitées par réflexe** : un personnage dans une zone annoncée fait un pas pour en sortir, dans la limite de sa laisse autour de sa place en formation (étape 40). Le réflexe pourrait dépendre d'un trait de classe (un voleur réagit vite, un chevalier en plate pas du tout).
- **c. Les sorts dangereux ciblés sur un personnage se gèrent sans bouger** : interrompre le lanceur, protéger la cible (Ward, Aegis), ou encaisser et soigner.
- **d. Plus tard, si le besoin s'en fait sentir :** un ordre de formation resserrée ou dispersée. Dispersée, elle encaisse moins les zones, mais les auras touchent moins de monde et la ligne de front tient moins bien.

## 11. Catégories d'ennemis (à la *Warhammer 40k*)

- **Retenu (étape 93).** Quatre catégories d'ennemis, qui existent dans le jeu : **nuée**, **infanterie**, **infanterie lourde**, **créature monstrueuse** (peut-être **commandant** pour les boss, plus tard). Par exemple, des attaques au couteau rapides et peu puissantes, très bonnes contre une nuée, moins contre l'infanterie lourde.
- **À tester (étape 93).** Leurs valeurs (`docs/monstre.md`, « Catégories de monstres »), à l'échelle × 3 :

  | Catégorie | PV | Armure | Attaque | Recharge | Vitesse | Rayon |
  |---|---|---|---|---|---|---|
  | Nuée (par monstre) | 60 | 0 | 10 | 0,6 – 1,0 s | 190 | 9 |
  | Infanterie | 300 | 3 | 30 | 0,7 – 1,3 s | 160 | 15 |
  | Infanterie lourde | 450 | 10 | 45 | 1,2 – 1,8 s | 110 | 17 |
  | Créature monstrueuse | 1500 | 6 | 70 | 1,5 – 2,5 s | 140 | 16 (30 voulus, voir `docs/monstre.md`) |

  (plus les capacités offensive et défensive et l'esquive, dans `docs/monstre.md`.) Une **nuée** est un groupe de petits monstres (5 à 7, placés en grappe) qui s'alertent ensemble, pas une vraie escouade de monstres : celle-ci (ancre, formation, places d'attaque autour de leur cible, cible commune) est une étape à part, plus tard.
- **Retenu, pas encore fait (étape 93).** La **créature monstrueuse** n'a que ses valeurs ; ses règles propres, ci-dessous (« immunisée au recul et à l'étourdissement », « ne peut pas être bloquée par un seul personnage »), viendront plus tard, avec le recul, l'étourdissement et le blocage entre entités.
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
4. **Effet** : dégâts (typés), soin, absorption (Ward, Aegis), modification de statistique, contrôle, déplacement (charge, recul, téléportation), invocation, menace.
5. **Coût** : recharge, mana, ressource de classe, ou points de vie (identité du noir).

Note de structure : un skill deviendra probablement une **composition** (un ciblage, un déclenchement, une liste d'effets) plutôt qu'un `enum` et un `switch` qui grossissent à chaque nouveau skill. Extension connue, à accueillir le moment venu.

Points de conception associés (**proposés**) :

- **Contrôle de foule, du plus doux au plus fort** : ralentir le déplacement ; ralentir attaques et recharges ; enraciner ; silence ou désarmement ; interrompre ; étourdir ; peur (attention aux attaques d'opportunité de l'étape 77) ; recul et attraction. Une règle contre l'enchaînement infini : rendement décroissant, immunité temporaire ou boss résistants.
- **L'interruption** est peut-être le skill le plus important pour le rôle de « chef d'orchestre » : manuel, lisible, à fort impact, à condition que les monstres lancent des sorts avec une activation visible.
- **Les auras** récompensent une escouade serrée en formation, et créent une tension avec la distance des personnages arrière. Elles **réservent** une partie du mana (à la *Diablo 2* / *Path of Exile*) au lieu de le dépenser.
- **Empilement des buffs** : un même effet ne s'empile pas, il est rafraîchi ; des effets différents s'additionnent (règle de WoW). Important avec les doublons de classe.
- **Les formes de soin ont chacune un rôle** : direct (réagit à l'urgence), dans le temps (anticipe, s'automatise bien), absorption (choix de timing), vol de vie (oblige à attaquer).
- **Instantané contre activation** : l'instantané compense par des nombres plus faibles ou une recharge plus longue ; l'activation peut être interrompue.
- **Retenu.** Chaque skill déclare ses **coefficients par caractéristique** (levier 2 de la section 5). Ce sont des données du skill, pas du code propre à chaque skill.
- **Retenu** (étape 91, documentation seulement). **Deux familles de skills de dégâts** : les **skills de frappe** utilisent l'arme (ses dégâts plus un bonus, sa nature et sa pénétration ; candidats : frappe puissante, frappe sacrée, tir perçant) et les **skills à dégâts propres** (explosion, pluie de flèches). La description de l'attaque construite à partir de l'arme (`DescriptionDAttaque`) peut servir de point de départ à un skill de frappe ; aucun skill ne change à l'étape 91.
- **Retenu** (étape 94). **La frappe sacrée est le premier skill de frappe** (`DefinitionDeSkill::bonusDeFrappe`). De l'arme : les dégâts physiques (Puissance comprise), la nature et la pénétration. Du skill : un **bonus fixe, typé** (un `Degats` : ici du sacré), la recharge, le temps d'activation et la portée. Les exigences d'arme ne changent pas (épée longue, masse, épée à deux mains). **À tester** : bonus sacré de 45, calibré sur la masse de départ (15 + 45 = 60, le total d'avant) ; avec l'épée à deux mains (60) : 105. Contre l'infanterie lourde, l'armure écrase le physique et le nombre est doré. La frappe puissante et le tir perçant pourront devenir des skills de frappe avec un bonus physique, sans changer la structure (section 15).

## 13. Liste cible des statistiques (étape 109)

Liste dressée par Olivier à l'étape 109 (documentation seulement : le code n'a pas changé). Elle remplace l'ancien « inventaire des statistiques de combat » et sert de référence aux étapes futures. « Existe » dit ce que le code fait **à la fin de l'étape 110** (l'étape 109 l'avait dressé à la fin de l'étape 108). Les noms en anglais sont les noms affichés (noms de travail, `docs/glossaire.md`) ; quand le code a un équivalent, il est cité. Toutes les valeurs d'équilibrage citées sont « à tester ».

### Trois niveaux, à ne pas mélanger

- **Retenu.** **Les caractéristiques** (cinq, réparties par le joueur) : Might, Constitution, Dexterity, Intellect, Resolve. Elles *donnent* des effets aux autres statistiques (section 5) ; ce ne sont pas des statistiques de combat.
- **Retenu.** **Les ressources** : les points de vie et l'énergie sont des **jauges**, pas des caractéristiques. Les statistiques d'une ressource sont son maximum, sa régénération, et ce qui la remplit autrement (Life Steal, Energy Steal, Life per Kill, Healing Received).
- **Retenu.** **Les statistiques de combat** : ce que le moteur du coup lit d'une entité (`StatistiquesDeCombat`, `combat/StatistiquesDeCombat.hpp`).
- **Retenu (confirmé par écrit).** **Un monstre a des statistiques de combat mais pas de caractéristiques.** Ses valeurs viennent de sa `DefinitionDeMonstre`, ajustées par la profondeur de l'étage (`renforcer`, `docs/monstre.md`). C'est l'architecture actuelle : rien à changer.

### Caractéristiques

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Might (Puissance) | Montant des skills et attaques d'arme (+ %, modéré) | Classe, croissance, points libres, équipement | Oui (étape 88 : 3 % par point au-dessus de 10 ; **1 % depuis l'étape 110**, avec des coefficients par skill : Might pour l'arme, Intellect pour les sorts, Resolve pour les soins) |
| Constitution (Endurance) | Points de vie ; résistance au contrôle physique (section 5) | idem | Oui : points de vie (étape 88/91 : 6 par point) et, depuis l'étape 110, Stun et Slow Resistance (0,3 % par point) |
| Dexterity (Adresse) | Capacités offensive et défensive | idem | Oui (étape 89 : 5 par point) |
| Intellect (Savoir) | Taille des zones et durée des effets infligés (section 5) ; montant des sorts | idem | Oui (étape 110) : montant des sorts (1 %), aire (0,3 %) et durée (0,3 %), les deux derniers plafonnés à +100 % ; calculés, aucun skill n'utilise encore la durée |
| Resolve (Volonté) | Résistance au contrôle magique (section 5) ; énergie maximum ; montant des soins | idem | Oui (étape 110) : Max Energy (3 par point), Charm, Fear et Disruption Resistance (0,3 % par point), montant des soins (1 %) |

### Ressources

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Max Life | Survie | Constitution, niveau de classe, équipement | Oui (`pointsDeVieMax` : 300 plus 6 par point de Constitution au-dessus de 10 plus le gain par niveau de classe, 8, au-delà du premier ; échelle × 3) ; monstres : leur définition |
| Life Regen | Points de vie regagnés dans le temps | Équipement, skills | Non (seul le monstre qui rentre à son poste se régénère, `regenerationRetour`) |
| Life Steal | % des dégâts infligés rendus en points de vie | Équipement, skills | Non |
| Life per Kill | Points de vie rendus par monstre tué | Équipement, skills | Non |
| Healing Received | Majore (ou réduit) les soins reçus | Équipement, skills | Non |
| Max Energy | Budget de skills | Niveau de classe (100 plus 5 par niveau au-delà du premier) et Resolve (3 par point au-dessus de 10) | Oui (étape 108, `energieMax` ; Resolve depuis l'étape 110) |
| Energy Regen | Énergie regagnée dans le temps | Base de 2 par seconde, majorée en % par l'affixe « of Replenishment » | Oui (étape 108, `regenerationDEnergie`) |
| Energy Steal | Énergie rendue par les dégâts infligés | Équipement, skills | Non |
| Coût d'un skill | Énergie (points de vie pour le noir, un jour) | Skill | Oui (étape 108) : entier, 0 pour l'attaque de base et pour les monstres |

### Offense

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Dégâts de base | Montant de départ du coup, par élément | Arme, skill ; modifiés par Might | Oui (l'attaque de base vient de l'arme depuis l'étape 91 ; plage tirée à chaque coup depuis l'étape 102) |
| Dégâts dans le temps | Un élément infligé sur la durée | Skills, monstres | Non (règles décidées plus bas : ignorent l'armure, ne critiquent pas) |
| Offensive Ability | Chance et force du critique, contre la Defensive Ability du défenseur | Dexterity, équipement | Oui (`capaciteOffensive`, étape 89 : `50 + 5 × Dexterity` ; 100 pour un monstre ordinaire ; bonus d'équipement : le +10 des reliques) |
| Crit Damage | Multiplicateur d'un critique (une statistique) | Équipement, skills | **Partiel** : le multiplicateur existe (`multiplicateurDeCritique`, de ×1,5 à ×2) mais n'est que fonction du rapport des capacités, sans bonus d'équipement ni de skill |
| Armor Penetration (%) | Ignore X % de l'armure du défenseur | Arme, équipement | Oui (étape 92 : propriété de l'arme, la dague 30 %, plafond de 75 %, préfixe « Keen ») |
| Puissance de soin | Montant soigné | Skill, Might, équipement | Oui (par skill ; Might, comme les dégâts) |

Il n'y a **pas de Crit Chance** : la chance de critique vient seulement du rapport entre Offensive Ability et Defensive Ability (section 7).

### Défense

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Armor | Réduit le physique direct (soustraction avec plancher de 20 %) | Équipement | Oui (étape 87 ; seulement la portion physique depuis l'étape 94) |
| Résistances (huit, voir « Les huit éléments ») | Réduit la part de son élément, en % plafonné | Équipement, skills | Oui pour le mécanisme (étape 94 : une valeur par `Element`, plafond de 80 %, vulnérabilité jusqu'à −100 %) ; trois éléments seulement dans le code ; aucun objet n'en donne encore ; monstres dans leur définition (la créature monstrueuse 40 % de feu) |
| Dodge | % d'éviter un coup de **contact** | Équipement, skills (plafond) | Oui (`esquiveContact`, étape 90 : plafond de 50 %, monstres 5 %, bottes de cuir +5 %, malus des pièces lourdes) ; affiché aujourd'hui « Melee Dodge » |
| Deflect | % d'éviter un **projectile** | Équipement, skills (plafond) | Oui (`esquiveProjectile`, étape 90 : plafond de 50 %, monstres 5 %, casque de cuir +5 %) ; affiché aujourd'hui « Projectile Dodge » |
| Block Chance | Chance de bloquer un coup de contact ou un projectile | Bouclier, skills | Oui (`chanceDeBlocage`, étape 103 : Shield 20 % plus 1 par niveau) |
| Block Amount | Points absorbés par un blocage (seulement la portion physique) | Bouclier, skills | Oui (`montantBloque`, étape 103 : 8 points au niveau 1) |
| Block Recovery | Temps pendant lequel un bouclier qui a bloqué ne bloque plus | Bouclier, skills | **Partiel** : une constante commune de 3 s (`s_recuperationDuBlocage`), pas une statistique |
| Defensive Ability | Réduit le critique subi | Dexterity, équipement | Oui (`capaciteDefensive`, étape 89 ; une référence de 100 pour les soins) |
| Aegis | Points d'absorption du **physique**, appliqués en dernier ; immunité à Stun et Slow tant qu'il en reste | Skills, équipement | Non |
| Ward | Points d'absorption de **tout ce qui n'est pas physique** (poison compris), appliqués en dernier ; immunité à Charm, Fear et Disruption tant qu'il en reste | Skills, équipement | Non |

Il n'y a **pas de statistique défensive contre la pénétration d'armure** : elle annulerait l'identité des armes qui percent l'armure (dague, lance, arc, arbalète). Les autres couches défensives (Dodge et Deflect, le blocage, Aegis, la résistance physique) en sont la parade.

#### Les huit éléments

| Élément | Résistance | Dans `Element` aujourd'hui |
|---|---|---|
| Physical | Physical Resistance (voir les décisions) | Oui (`Physique`), résistance 0 partout |
| Fire | Fire Resistance | Oui (`Feu`) |
| Holy | Holy Resistance | Oui (`Sacre`) |
| Cold | Cold Resistance | Non |
| Lightning | Lightning Resistance | Non |
| Arcane | Arcane Resistance | Non |
| Necrotic | Necrotic Resistance | Non |
| Poison | Poison Resistance | Non |

### Contrôle (résistances aux états)

Une résistance à un état **réduit sa durée**.

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Stun Resistance | Contrôle physique : durée de l'étourdissement | Constitution, équipement | **Calculée** depuis l'étape 110 (`resistancesAuControle`, 0,3 % par point de Constitution, affichée dans l'inventaire) ; aucun état de contrôle ne la lit encore |
| Slow Resistance | Contrôle physique : durée du ralentissement | Constitution, équipement | Calculée (idem) |
| Charm Resistance | Contrôle magique : durée du charme | Resolve, équipement | Calculée depuis l'étape 110 (0,3 % par point de Resolve) ; aucun état ne la lit encore |
| Fear Resistance | Contrôle magique : durée de la peur | Resolve, équipement | Calculée depuis l'étape 110 (0,3 % par point de Resolve) ; aucun état ne la lit encore |
| Disruption Resistance | Contrôle magique : durée de la perturbation (Disruption empêche d'utiliser les skills) | Resolve, équipement | Calculée depuis l'étape 110 (0,3 % par point de Resolve) ; aucun état ne la lit encore |

### Utilité

| Statistique | Rôle | Source principale | Existe |
|---|---|---|---|
| Movement Speed | Poursuite, fuite, placement | Classe, équipement, skills | **Partiel** : une vitesse de base fixe (`Mobile`), sans modificateur d'équipement ni de skill |
| Attack Speed | **Modificateur** (100 % de base) de la cadence des attaques ; la vitesse de base vient de l'arme | Équipement, skills | **Partiel** : la vitesse de base vient de l'arme depuis l'étape 91 ; le modificateur n'existe pas |
| Cast Speed | Modificateur du temps d'incantation des skills (n'a d'effet que si les skills ont un temps d'incantation) | Équipement, skills | Non (les skills ont déjà un temps d'activation, mais aucun modificateur) |
| Skill Recharge (%) | Raccourcit les recharges des skills | Équipement, skills | Non (la réduction de recharge reste ouverte : question 3) |
| Effect Duration (%) | Durée des effets infligés | Intellect, équipement | **Calculée** depuis l'étape 110 (`bonusDeDuree`, 0,3 % par point d'Intellect, plafond +100 %) ; la provocation l'accepte (coefficient 0), aucun skill du catalogue ne l'utilise |
| Area Size (%) | Aire des skills de zone (section 5 : le bonus porte sur l'aire) | Intellect, équipement | Oui depuis l'étape 110 (`bonusDAire`, 0,3 % d'aire par point, plafond +100 %, rayon par racine carrée) pour les skills dont le coefficient d'aire n'est pas nul (l'explosion) |

### Propriétés d'une attaque (une arme ou un skill, pas des statistiques d'entité)

| Propriété | Rôle | Source | Existe |
|---|---|---|---|
| Type de dégâts | Élément de chaque portion | Arme, skill | Oui (étape 94 : trois éléments sur huit ; un coup est une collection de portions) |
| Intervalle de recharge | Temps entre deux coups (min-max) | Arme, skill | Oui (par arme depuis l'étape 91 ; changer d'arme relance une recharge complète) |
| Temps d'exécution / d'activation | Durée d'immobilité de l'attaquant | Arme, skill | Oui |
| Portée | Distance bord à bord | Arme, skill | Oui |
| Zone de contrôle | Déclenche l'attaque d'opportunité | Arme (contact) depuis l'étape 91 | Oui |
| Pénétration de résistance | Ignore une part de la résistance | Équipement, skills | Non, et hors de la liste cible : reste une piste (section 15) |

### Propriétés de la cible

| Propriété | Rôle | Existe |
|---|---|---|
| Catégorie | Nuée, infanterie, infanterie lourde, créature monstrueuse (plus tireur et soigneur, étape 107) | Oui (étape 93) |
| Taille (cercle) | Portée bord à bord, blocage, encerclement | Oui |
| Immunités | Ex. pas de recul pour une créature monstrueuse | Non |

### Décisions de l'étape 109

- **Retenu.** **Huit types de dégâts et de résistances** : Physical, Fire, Cold, Lightning, Arcane, Necrotic, Holy, Poison. **Cette décision remplace celle de l'étape 94** (section 8) : le **saignement n'est plus un type de dégâts** ; c'est du **physique dans le temps**. Arcane remplace la piste « Psychique / Arcane ».
- **Retenu.** **Pas de Piercing comme type de dégâts** : la pénétration d'armure (un attribut d'attaque, en %) joue ce rôle.
- **Retenu.** **Tout élément peut avoir des dégâts dans le temps**, résistés par la résistance de cet élément. Règles inchangées : les dégâts dans le temps **ignorent l'armure** et **ne critiquent pas**. Un saignement est donc du physique dans le temps : l'armure ne le réduit pas, la résistance physique oui.
- **Retenu.** **La résistance physique (%) s'applique à tout le physique**, direct ou dans le temps. Elle est **rare**, avec un **plafond plus bas** que celui des autres résistances. *Valeur du plafond à déterminer : aucune n'est proposée ici.* L'armure reste la défense principale contre les coups physiques directs.
- **Retenu.** **Aegis et Ward** (anciens noms de travail, section 9) sont des **points d'absorption appliqués en dernier** dans le déroulement du coup (étape 7, section 6), comme déjà décidé. Aegis absorbe le physique ; tant qu'il en reste, la cible est immunisée à Stun et Slow. Ward absorbe tout ce qui n'est pas physique, poison compris ; tant qu'il en reste, la cible est immunisée à Charm, Fear et Disruption. **Les deux absorbent aussi les dégâts dans le temps.**
- **Retenu.** **Offensive Ability et Defensive Ability** gardent leurs noms actuels (pas « Attack Power » ni « Defense Power »).
- **Retenu.** **Pas de Crit Chance** : la chance de critique vient seulement du rapport des deux capacités. **Crit Damage** est une statistique (un multiplicateur).
- **Retenu.** **Pas de statistique défensive contre la pénétration d'armure** (voir « Défense »).
- **Retenu.** **Dodge** = l'esquive contre le contact ; **Deflect** = l'esquive contre les projectiles. Le blocage garde ses **trois valeurs** (Block Chance, Block Amount, Block Recovery), comme à l'étape 90.
- **Retenu.** **Attack Speed** est un modificateur (100 % de base) : la vitesse de base d'une attaque vient de l'arme. **Cast Speed** n'a d'effet que si les skills ont un temps d'incantation.
- **Retenu.** Les **résistances aux états réduisent la durée** : Stun et Slow (contrôle physique), Charm, Fear et Disruption (contrôle magique ; Disruption empêche d'utiliser les skills).

### Pistes non décidées

Rien de ceci n'est une décision.

- **Bonus de dégâts par type** (+ % Fire, etc.) : ajouterait une statistique par élément côté attaquant.
- **Menace (Threat)** pour les tanks : aujourd'hui les monstres frappent toujours le plus proche (section 15, hystérésis).
- **Résistance des monstres à la provocation.**
- **Portée de Slow** : le déplacement, l'attaque, ou les deux.
- **Le plafond de la résistance physique** (voir plus haut).
- **La résistance à l'interruption** de l'ancienne liste (chance de garder son activation, section 5) n'est pas dans la liste cible : à trancher si Disruption la remplace, ou si les deux existent.
- **Block Recovery** comme vraie statistique (aujourd'hui une constante commune) : à décider avec les skills de blocage.

## 14. Questions ouvertes

1. *Tranchée* : blocage à la *Titan Quest* (section 7).
2. **Le niveau** entre-t-il dans les formules (pénalité selon l'écart, à la *Diablo 2* / WoW), ou un monstre plus fort l'est-il seulement par ses nombres ? *(En partie éclairée à l'étape 89 : le critique traite le niveau par les nombres, car il ne lit que le rapport des capacités ; un monstre de zone plus profonde aura de plus grandes capacités, sans formule d'écart de niveau. Les autres étapes du coup restent à décider. **Étape 98** : les monstres sont renforcés par la profondeur de l'étage, toujours par leurs nombres : points de vie +25 %, dégâts +15 %, capacités +10 par étage au-delà du premier (`renforcer`, `docs/monstre.md`) ; aucune formule d'écart de niveau, ni pénalité ni bonus. Reste à décider quand les personnages auront un niveau (étape 99). **Étape 101** : le niveau d'un *objet* est la profondeur où il est généré ; il agit par les nombres aussi, jamais par une formule d'écart : les plages des affixes grandissent de 25 % du catalogue par niveau et les raretés Magic et Rare deviennent plus fréquentes (`docs/objets.md`, « Le niveau d'objet »).)*
3. **Les plafonds** : esquive, résistances, réduction de recharge ; lesquels, et à combien ? *(En partie tranchée : l'esquive est plafonnée à 50 % (étape 90), les résistances à 80 % avec des vulnérabilités jusqu'à −100 % (étape 94, à tester). Reste la réduction de recharge, et, depuis l'étape 109, le plafond de la résistance physique, qui sera plus bas que celui des autres résistances : sa valeur est à déterminer.)*
4. *Tranchée* : Aegis et Ward absorbent en dernier (section 9).
5. *Tranchée* (étape 94, **modifiée à l'étape 109**) : **la liste des éléments** : huit — Physical, Fire, Cold, Lightning, Arcane, Necrotic, Holy, Poison ; le saignement n'est plus un type, c'est du physique dans le temps (sections 8 et 13).
6. **La ressource** : *en partie tranchée à l'étape 108* : une **énergie** par personnage, la même pour toutes les classes, à la *Titan Quest* et *Grim Dawn* (section 14 bis). Reste ouvert : l'**Élan**, ressource partagée de l'escouade, en couche au-dessus (section 15), et quelles classes ont leur propre ressource (rage, foi…), qui ne sont pas faites. La structure qui accueillera l'Élan existe (`Reserve`). Le lien entre l'énergie et une caractéristique est fait à l'étape 110 : la Volonté donne 3 points d'énergie maximum par point au-dessus de 10 (section 5).
7. *Tranchée* (étape 89) : **la magie critique**, comme le reste (section 7). Dans *Titan Quest*, non.
8. **Les classes de départ** : quelles classes de base, quelles spécialisations, quels paliers ?
9. **L'automatisation** : quels skills partent seuls (gambits, `docs/brainstorm.md`), lesquels restent au clavier ?
10. **Un bonus pour une escouade d'une seule couleur** ? (Moins pertinent depuis que les couleurs ne sont qu'un guide de conception.)
11. **Les noms définitifs** des cinq caractéristiques. *(Depuis l'étape 95, les noms de travail sont en anglais à l'écran : Might, Constitution, Dexterity, Intellect, Resolve. Les noms français restent ceux du code et de ce document ; le choix définitif reste à faire.)*
12. *Tranchée* (étape 94) : **le blocage et les portions typées** : le bouclier ne réduit que la portion physique (section 7).

## 14 bis. L'énergie (étape 108)

Statuts : *retenu* = décidé et fait, *à tester* = valeur provisoire.

- **Retenu.** Une seule ressource, l'**énergie** (« Energy » à l'écran, `Energie` dans le code), pour toutes les classes : une `Reserve`
  par personnage (une `Jauge` entière, un débit de régénération et un accumulateur `float` qui ne laisse passer que la partie entière).
  Ni ressource de classe ni Élan à cette étape.
- **À tester.** **Maximum** : 100 plus 5 par niveau au-delà du premier (`s_energieDeBase`, `s_energieParNiveau`) ; commun à toutes les
  classes ; à la montée de niveau un vivant gagne autant d'énergie que son maximum augmente (comme les points de vie), un mort ne gagne
  rien. **Régénération** : 2 points par seconde (`s_regenerationDEnergieDeBase`), la même en combat et hors combat, majorée en pour
  cent par l'affixe « of Replenishment » ; un mort ne régénère pas. **Pleine au début de chaque étage** : l'`Expedition` ne garde pas
  l'énergie.
- **Retenu.** **Le coût d'un skill** est un entier de `DefinitionDeSkill` (`cout`, 0 par défaut). L'attaque de base et les skills des
  monstres coûtent 0 (les monstres n'ont pas d'énergie ; un test de cohérence le vérifie, comme celui qui exige un coût de plus de 0
  pour chaque skill de classe). On **vérifie** l'énergie à l'engagement (début de l'activation) et on la **paie** au départ de l'effet :
  un skill annulé (cible morte, soin sans blessé, arme retirée) ne coûte rien, un skill de zone paie même si la zone est vide, comme sa
  recharge.
- **Retenu.** **Énergie insuffisante** : à la demande du joueur le skill est refusé, jamais mis en attente, et « Not enough energy »
  apparaît sur le personnage ; un skill déjà en attente dont le lanceur est libre mais n'a plus de quoi payer (il a payé un autre skill
  entre-temps) est abandonné sans coût, avec le même texte.
- **À tester.** **Calibrage par une règle** et non au cas par cas : à partir d'une réserve pleine de niveau 1, un personnage qui lance un
  skill à chaque fin de recharge, avec la régénération de base, tombe à sec en 30 s pour un sort et en 120 s pour une technique. Avec une
  recharge `r`, une réserve de 100 et 2 de régénération : `coût / r − 2 = 100 / T`, donc `coût = r × (2 + 100 / T)`, arrondi au plus
  proche. La distinction sort / technique ne sert qu'au calibrage : ce n'est pas un concept du jeu, aucun champ ni énumération.

  | Skill | Classe | Recharge | Calibrage | Coût |
  |---|---|---|---|---|
  | Explosion | mage | 8 s | sort (30 s) | 43 |
  | Soin | prêtre | 8 s | sort (30 s) | 43 |
  | Frappe puissante | guerrier | 5 s | technique (120 s) | 14 |
  | Provocation | guerrier | 10 s | technique (120 s) | 28 |
  | Frappe sacrée | paladin | 5 s | technique (120 s) | 14 |
  | Imposition des mains | paladin | 10 s | technique (120 s) | 28 |
  | Tir perçant | rôdeur | 6 s | technique (120 s) | 17 |
  | Pluie de flèches | rôdeur | 8 s | technique (120 s) | 23 |

  L'imposition des mains (un soin) est rangée parmi les techniques : le prompt de l'étape ne nommait comme sorts que l'explosion et le
  soin du prêtre. Son coût passerait à 53 si on la voulait sort.
- **À tester.** Le **débit** d'énergie d'un skill lancé en boucle (43 points en 8 s, soit 5,4 par seconde, contre 2 de régénération)
  vide la réserve en 30 s ; les techniques, bien plus lentement.

## 15. Pistes

Idées retenues pour plus tard, sans étape prévue.

- **Éléments sans source** (étape 94, modifiée à l'étape 109) : froid, foudre, arcane, nécrotique et poison s'ajouteront à l'énumération `Element` avec leur premier skill ou monstre ; le saignement n'est plus un élément (du physique dans le temps).
- **Les statistiques de la liste cible** (étape 109, section 13) qui n'existent pas encore : une étape à part chacune ou par groupe (ressources, états de contrôle, Aegis et Ward, modificateurs de vitesse), à discuter dans Claude.ai.
- **Pénétration de résistance** : par l'équipement ou des skills, comme la pénétration d'armure (à décider).
- **Bouclier anti-magie** : un bouclier qui bloquerait aussi une part des portions élémentaires (le Shield ne bloque que le physique).
- **Frappe puissante et tir perçant en skills de frappe** : la même structure que la frappe sacrée, avec un bonus physique.
- **Hystérésis du changement de cible** (inspirée de la menace de WoW) : un monstre ne change de cible que si la nouvelle est **nettement** plus proche, pas dès qu'elle l'est un peu. Évite les oscillations (« ping-pong »), utile dès les escouades de monstres. Pas de table de menace : les monstres frappent toujours le plus proche.
- **Projectiles physiques** (inspirés de *Titan Quest*) : une flèche ou un sort touche le **premier** corps sur sa trajectoire. Avec le blocage entre entités, la ligne de front protège les tireurs arrière des tirs ennemis. Plus coûteux en code : les projectiles deviennent des objets du monde.
- **Postures d'escouade** (inspirées de *Dawn of War 2* et *Dragon Age*) : une bascule globale du comportement automatique. **Offensive** : on poursuit et on engage. **Défensive** : on tient la formation, on ne poursuit pas. **Prudente** : les tireurs restent à leur portée maximale. Une seule décision change les trois personnages. C'est une première marche, plus simple, vers les gambits, et ça reprend l'idée d'un état de l'escouade.
- **Personnage à terre** (inspiré de *Baldur's Gate 3*) : à 0 PV, un personnage tombe **à terre** au lieu de mourir. Un allié peut le relever pendant quelques secondes, sinon il meurt. Décision forte en plein combat : envoyer quelqu'un sous le feu, ou tenir la ligne. À trancher : avec combien de PV il se relève, si le relèvement est une activation interruptible, et si la défaite arrive quand les trois sont à terre.
- **L'Élan, ressource partagée de l'escouade** (inspiré des points de commandement d'*Aliens: Dark Descent*) : le moral du groupe au combat, plutôt qu'une réserve de mana commune. Il se **construit en combattant** (coups portés, blocages, soins, monstres tués) et se vide lentement hors combat ; une petite réserve de départ et un léger rechargement de base évitent de commencer chaque combat à vide. Il se dépense en skills. Affiché par quelques segments près de la barre de skills, ou un étendard qui s'illumine. C'est une **couche au-dessus** : les recharges par skill restent, et certaines classes gardent leur propre ressource au besoin (par exemple les PV pour le noir). Liens possibles : relever un personnage à terre coûte de l'Élan ; la posture offensive en génère plus mais expose davantage.

## Ordre de discussion proposé

1. Finir les maths du combat (questions 2, 3, 6 et 7 ; 5 et 12 sont tranchées depuis l'étape 94).
2. Les catégories d'ennemis, qui découlent du modèle d'armure (faites à l'étape 93, valeurs à tester ; restent leurs règles propres et leurs comportements).
3. La progression des classes : barre de maîtrise, paliers, fourche de spécialisation.
4. Les skills de chaque classe, conçus avec les couleurs comme boussole.