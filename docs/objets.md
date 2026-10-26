# Squad-ARPG — Objets, équipement et sac

Le modèle des objets (étape 83) : le catalogue, ce que porte chaque personnage, le sac de
l'escouade et les opérations qui déplacent un objet. L'écran d'inventaire (étape 84) les affiche, et
le glisser-déposer (étape 85) les déplace. Les armes (étape 91) donnent l'attaque de base et la zone de
contrôle, et le bouclier son blocage. Depuis l'étape 95, un objet porte sa rareté et au plus un préfixe et un
suffixe (« Rareté et affixes »), et depuis l'étape 96 `genererObjet` en tire au hasard (« Génération aléatoire »). L'armure et son effet sur les dégâts sont dans `docs/combat.md`, la construction des
personnages dans `docs/formation.md`. Les textes affichés sont en anglais (`docs/glossaire.md`) : les noms ci-dessous
sont ceux du jeu, les identifiants du code restent en français.

## Décisions

- **Un dossier `objets/`, entre `combat` et `escouade`** : ordre `base → monde → combat → objets →
  escouade, monstres → jeu → …`. Le butin au sol (`jeu/`) et de futures tables de butin
  (`monstres/`) auront besoin des objets, et `monstres/` ne peut pas inclure `escouade/`. `objets/` ne
  dépend que de `base/`, de `combat/` (depuis l'étape 88 : les bonus de caractéristiques d'un objet, `combat/Caracteristiques`) et de la bibliothèque standard.
- **Emplacement ≠ catégorie** : dix emplacements (où l'on porte), huit catégories (ce qu'est
  l'objet). Deux emplacements (les anneaux) reçoivent la même catégorie, et plus tard une arme
  pourra aller dans deux mains. `peutAller(CategorieObjet, Emplacement)` est la seule source de la
  règle ; personne d'autre ne compare une catégorie à un emplacement.
- **Une catégorie d'arme, pas une classe `Arme`** (étape 91) : `CategorieObjet::Arme` va en main droite
  seulement ; qu'une arme prenne les deux mains est une donnée de l'arme (`DonneesDArme::mains`), et la règle
  d'occupation de l'autre main est dans `deplacerLesObjets` (voir « Armes (étape 91) »).
- **Données, pas de hiérarchie** : comme `Specialite`, un catalogue (`DefinitionObjet` : id, nom,
  catégorie, armure) dans l'ordre de l'enum `IdObjet`, lu par `definitionObjet(id)` ; l'ordre est
  contrôlé par un `assert`. Pas de sous-classes `Casque`, `Anneau`…
- **`Objet` est l'instance, `DefinitionObjet` le modèle** : `Objet` porte l'`IdObjet` du modèle, sa `Rarete` et
  au plus un préfixe et un suffixe (étape 95) ; c'est une petite valeur copiable, sans allocation ni pointeur vers
  le catalogue. L'égalité est par valeur (`operator==` : modèle, rareté et affixes) : deux masses sont égales
  seulement si elles ont la même rareté et les mêmes affixes aux mêmes valeurs.
- **Un objet existe à un seul endroit** (un emplacement ou une case du sac). Les déplacements se
  font par valeur, et les fonctions qui rendent l'objet remplacé sont `[[nodiscard]]` : oublier
  l'ancien objet le ferait disparaître. Un test compte les exemplaires de chaque objet (**par valeur complète**,
  depuis l'étape 95 : modèle, rareté et affixes) dans le sac et tous les équipements avant et après chaque
  opération.
- **Armure calculée, jamais ajustée** : l'armure d'un personnage est la somme de l'armure de ses objets
  portés, recalculée en entier depuis son `Equipement` (`Personnage::recalculerDepuisLEquipement`, seul
  endroit qui appelle `Vitalite::definirDefenses`, depuis l'étape 89 ; avant, `recalculerArmure` et
  `definirArmure`) après chaque pose ou retrait. Aucune addition ni
  soustraction incrémentale, donc aucun risque de dérive après une longue suite d'échanges. Plus
  aucune armure de classe : `DefinitionPersonnage::armure` est remplacé par
  `equipementDeDepart`. À la construction, la `Vitalite` reçoit `armureTotale()` de l'équipement
  de départ (`m_equipement` est déclaré avant `m_vitalite`).
- **Équilibre** : le guerrier et le paladin partent équipés (casque de fer 2, cuirasse de mailles 4,
  jambières de fer 2, bouclier rond 1 : 9 depuis l'échelle × 3 de l'étape 91.2 ; 50 avant l'étape 87), les trois
  autres n'ont que leur arme (armure 0). Les valeurs sont provisoires.
- **Les tests ne lisent pas les valeurs du catalogue d'objets** (même règle que pour les classes) :
  les attentes sont calculées avec `definitionObjet(id).armure` ou `armureDe(equipement)`
  (`tests/aides.hpp`), jamais avec 9 ou 4 écrits en dur. `test_Specialite.cpp` reste le seul
  fichier qui lit l'équipement de départ du catalogue des classes. Depuis l'étape 91, l'attaque de base des
  personnages de test vient d'une arme du catalogue : les constantes (`s_degatsBaseDeTest`…) sont **lues** dans la
  description de l'attaque de l'épée longue ou du sceptre (`docs/tests.md`).
- **Opérations transactionnelles** : `ramasser` et `deplacerObjet` vérifient toutes leurs
  conditions avant de modifier quoi que ce soit ; vraies, tout est fait, fausses, rien n'a bougé.
  Une fois les conditions vérifiées, l'échange ne peut plus échouer : il ne dit jamais « sac plein »
  parce que chaque objet prend la place que l'autre vient de libérer.
- **Équiper un mort est permis** : aucune règle ne l'interdit pour l'instant.
- **Le sac est à positions fixes** : retirer un objet laisse sa case vide, les autres ne bougent
  pas ; `ajouter` prend la première case libre. Le joueur range le sac par `deplacerObjet`.
- **La capacité (40) appartient à `Escouade`** (`Escouade::s_capaciteDuSac`, publique depuis l'étape 98.2), pas à
  `Inventaire`, qui reçoit sa capacité à la construction. Depuis l'étape 98.2, ce n'est plus `Partie` qui remplit le sac avec
  `sacDeDepart()` : c'est `nouvelleExpedition` (`jeu/Expedition`), et la `Partie` reçoit le sac de l'expédition, ses objets à
  leur case (`Escouade(positionDepart, composition, sac)`). Le sac et l'équipement des personnages survivent ainsi d'un étage à
  l'autre ; les objets au sol, non (`docs/boucle-et-entrees.md`, « Les étages et l'expédition »).
- **Un équipement déjà constitué** : `DefinitionPersonnage::equipementPorte` (`std::optional<Equipement>`, étape 98.2), s'il est
  présent, remplace `equipementDeDepart` à la construction du `Personnage`. Le kit de départ (`equipementDeDepart`) ne dit que des
  modèles (`IdObjet`) ; l'équipement qu'un personnage garde d'un étage à l'autre a des objets complets (rareté, affixes). Aucun
  seuil n'est revérifié à la reconstruction : un objet porté mais inactif (seuil non atteint) reste porté et inactif.

## Armes (étape 91)

- **Données, pas de hiérarchie** : une `DefinitionObjet` porte, si elle en est une, des `DonneesDArme`
  (`std::optional`) ; un bouclier porte des `DonneesDeBouclier`. Pas de sous-classe `Arme`.
- **Une arme** : un `TypeDArme` (`EpeeLongue`, `Masse`, `Dague`, `EpeeADeuxMains`, `Sceptre`, `Baton`, `Arc` ;
  `combat/TypeDArme.hpp`, parce que les skills l'exigent et que `combat/` ne connaît pas `objets/`), un nombre de
  mains (1 ou 2) et une `DescriptionDAttaque` (`combat/`) : dégâts, intervalle de recharge, temps d'exécution,
  portée bord à bord et nature (contact ou projectile). Les mains nues ne sont pas un objet : `mainsNues()`
  rend leur description (6 dégâts, 0,4 – 0,6 s, 0,15 s, 20 px, contact).
- **L'attaque de base est une valeur dérivée de l'équipement** : `Equipement::attaqueDeBase()` rend celle de
  l'arme de la main droite, sinon les mains nues ; le `Personnage` la recalcule à chaque changement d'équipement
  (`recalculerDepuisLEquipement`), comme l'armure et les caractéristiques, jamais stockée en double. Le skill de
  l'attaque de base n'est refait que si la description change (un anneau ne remet pas la recharge à zéro) ; un
  coup de base en cours est alors annulé sans recharge consommée, et l'attaque repart pour une **recharge
  complète** tirée dans l'intervalle de la nouvelle arme (`Personnage::relancerSiLArmeAChange`, appelé par
  `Escouade::attaquer` : l'équipement n'a pas de générateur). Sans cela, retirer et remettre une arme lente dans
  l'inventaire (partie figée) donnerait une attaque prête tout de suite.
- **La zone de contrôle vient de l'arme** : une attaque de contact engage (`s_zoneDeControleDeContact`, 20 px,
  `zoneDeControleDe(nature)`), un projectile non. `DefinitionPersonnage::zoneDeControle` a disparu, comme la
  portée et l'intervalle de l'attaque de base dans la définition de classe.
- **Deux mains** : une arme à deux mains occupe la main droite et exige la main gauche vide. Poser une arme à
  deux mains alors que la main gauche est occupée **range l'objet de la main gauche dans la première case libre du
  sac** ; poser un bouclier alors que la main droite tient une arme à deux mains range l'arme ; sac plein, le
  déplacement est refusé. Cela vaut aux deux extrémités d'un échange entre personnages.
- **Une seule fonction de décision, sur un état en valeurs** : `objets/DeplacementDObjets` définit
  `EtatDesObjets` (le sac et l'équipement de chaque place, copiés) et `deplacerLesObjets(etat, de, vers)`, qui
  applique la règle de base (`deplacementPermis` : catégorie et emplacement) puis les deux mains, **sur une
  copie** qu'elle ne garde que si tout réussit (transactionnel : un sac plein au moment de ranger refuse tout, état
  intact). `deplacementPossible` fait le même calcul sans rien changer : `Escouade::peutDeplacerObjet`
  (surbrillance) et `Escouade::deplacerObjet` (action) en dérivent, donc ne divergent pas. L'`Escouade` copie son
  état, déplace, puis recopie les différences dans les personnages (chaque pose recalcule armure, caractéristiques,
  défenses et attaque de base). Un skill en attente dont le lanceur perd l'arme qu'il exige est abandonné.
- **Le bouclier** : sa chance de blocage et son montant absorbé sont des propriétés du bouclier
  (`DonneesDeBouclier`), plus des bonus de statistiques ; `Equipement::bonusDeCombatTotal` les ajoute aux
  statistiques de combat du porteur. La **récupération** (3 s) est une constante commune
  (`s_recuperationDuBlocage`, `combat/Blocage.hpp`), plus une donnée d'objet : la règle « on garde la plus longue »
  a disparu. D'autres objets pourront plus tard ajouter de la chance de blocage ou réduire la récupération ; aucun
  champ n'existe pour cela, aucun objet ne s'en sert.
- **Équipement de départ** : guerrier épée longue + bouclier rond + pièces de fer, paladin masse + bouclier rond +
  pièces de fer, prêtre sceptre, mage bâton, rôdeur arc. Le sac de départ reçoit une dague et une épée à deux
  mains.
- **Les skills exigent un type d'arme** (`docs/combat.md`) : le refus est celui d'un lanceur mort.

## Seuils, objets inactifs et pénétration (étape 92)

- **Seuils** : une `DefinitionObjet` peut exiger des valeurs minimales de caractéristiques (`seuils`, liste de
  `SeuilDeCaracteristique { caracteristique, valeur }` ; vide : aucun seuil). Valeurs « à tester » :

  | Objet | Seuil |
  |---|---|
  | Épée longue, masse | Puissance 11 |
  | Épée à deux mains | Puissance 13 et Endurance 12 |
  | Dague | Adresse 10 |
  | Arc | Adresse 12 |
  | Bâton | Savoir 12 |
  | Sceptre | Volonté 11 |
  | Bouclier rond | Endurance 11 |
  | Casque de fer, cuirasse de mailles, jambières de fer | Endurance 12 |
  | Objets de cuir, bijoux | aucun |

  Chaque classe peut porter son équipement de départ avec ses caractéristiques de base (un test parcourt les
  classes) ; personne ne peut porter l'épée à deux mains au départ (le guerrier y arrive avec l'anneau de
  puissance).
- **Sous le seuil, impossible d'équiper** : `deplacerLesObjets` vérifie, pour chaque personnage qui reçoit un objet
  (la destination, et l'autre extrémité d'un échange entre personnages), qu'il atteint les seuils de l'objet posé
  avec ses **caractéristiques effectives sans compter cet objet** (un objet ne remplit pas son propre seuil) **et en
  tenant compte de ce que le déplacement retire** (l'objet échangé, l'objet rangé dans le sac par les deux mains).
  La règle reste dans une seule fonction de décision, utilisée par la surbrillance et par le déplacement ;
  `EtatDesObjets` porte pour cela les caractéristiques de base de chaque place (`bases`). Vers le sac, aucun seuil.
  L'équipement de départ d'un personnage n'est pas vérifié à la construction.
- **Objet inactif** : un objet **porté** dont le seuil n'est plus atteint (les caractéristiques ont baissé, par
  exemple en retirant l'anneau de puissance) **reste porté mais n'a plus aucun effet** : ni armure, ni bonus, ni
  données d'arme (l'attaque de base passe aux mains nues, et les skills qui exigent l'arme sont refusés) ou de
  bouclier (pas de blocage). Il redevient actif dès que le seuil revient.
- **Calcul par point fixe, à l'endroit unique des valeurs effectives** : `objetsActifs(base, portes)` (module
  `ObjetsActifs`, une règle pure sur des `ExigencesEtBonus`, donc testable sur des objets inventés) part de tous les
  objets portés, retire ceux dont le seuil n'est pas atteint par la base et les bonus des **autres** objets actifs,
  et recommence jusqu'à stabilité (la liste ne fait que diminuer, donc le calcul se termine ; l'ordre des retraits ne
  change pas le résultat : c'est le plus grand point fixe). `Equipement::actifPour(base)` en tire l'équipement actif,
  et `caracteristiquesEffectives(base, equipement)` la base plus les bonus des objets actifs. Le `Personnage`
  garde son équipement actif (`getEquipementActif`) et recalcule tout depuis lui (`recalculerDepuisLEquipement` :
  caractéristiques, points de vie maximum, statistiques de combat, défenses, attaque de base). Tests : la chaîne
  (désactiver un objet en désactive un autre) et la remise en service.
- **Limites du calcul** : il suppose des bonus positifs ou nuls (tous ceux du catalogue) ; deux objets qui se
  soutiennent l'un l'autre restent actifs (le plus grand point fixe) ; aucun objet du catalogue n'a à la fois un
  bonus et un seuil, donc la chaîne n'est testée que sur des objets inventés.
- **Affichage** : l'infobulle d'un objet liste ses seuils (« Requires Might 11 », en dernier depuis l'étape 95 :
  `ligneDeSeuil`) ;
  **pendant un glissement**, sur l'emplacement d'un personnage, l'infobulle de l'objet saisi apparaît avec ses seuils,
  **en rouge ceux que ce personnage n'atteint pas** (`Escouade::seuilsManques`, calculé comme la décision : sans compter
  l'objet posé, après ce que le déplacement retire) ; un objet **porté inactif est encadré en rouge** dans
  l'inventaire (`Escouade::objetPorteEstInactif`) et son infobulle dit « Inactive: requirement not met ». L'armure et les
  bonus affichés sont ceux de l'équipement actif (`Escouade::equipementActifDuPersonnage`).
- **Pénétration d'armure** : une propriété de l'arme (`DescriptionDAttaque::penetration`, en pour cent entier ; la
  dague : 30, les autres armes et les mains nues : 0). Elle réduit l'armure du défenseur **avant** la soustraction M1 :
  `armure effective = armure × (1 − pénétration)`, sans arrondi intermédiaire (l'arrondi reste celui de M1), plafonnée à
  75 % (`s_penetrationMaximale`, `Defense.hpp`). Elle circule dans l'`Attaque` (`penetration`, 0 par défaut) : l'attaque de
  base du `Personnage` en porte une (donc aussi les coups d'opportunité, qui en reprennent la puissance), les skills de
  dégâts n'en ont pas (la frappe sacrée prend celle de l'arme). Depuis l'étape 95, le préfixe « Keen » ajoute de la
  pénétration à celle de l'arme. L'infobulle de la dague affiche « Penetration: 30 % ».

## Rareté et affixes (étape 95)

- **Un objet** : `struct Objet { IdObjet modele; Rarete rarete = Commun; std::optional<Affixe> prefixe; std::optional<Affixe>
  suffixe; }`. Le type interdit un troisième affixe ; le reste est un invariant (ci-dessous). Un `Affixe` est
  `{ IdAffixe id; int valeur; }` : l'identifiant **et la valeur de cet objet**. La valeur est gardée dans l'objet, pas lue
  dans le catalogue : rééquilibrer un affixe plus tard ne change pas les objets déjà trouvés. Un agrégat à valeurs par
  défaut : `Objet{IdObjet::Masse}` est une masse commune, ce qui a laissé inchangés les sites d'appel qui n'avaient besoin que
  du modèle (équipement de départ, sac de départ, la plupart des tests).
- **Raretés** (à la *Titan Quest*, pour l'instant ; d'autres viendront) : **Common** n'a aucun affixe, **Magic** exactement
  un (préfixe ou suffixe), **Rare** un préfixe et un suffixe. L'équipement de départ et le sac de départ sont communs.
- **Invariants : une fonction de validation, pas une construction avec `assert`.** `objetValide(const Objet&)`
  (`CatalogueObjets`) vérifie que chaque affixe est à sa position (un préfixe dans `prefixe`), permis pour la catégorie du
  modèle (`affixePermis`) et de valeur strictement positive, et que le nombre d'affixes correspond à la rareté. Choix :
  `Objet` est un agrégat (`genererObjet`, les tests l'écrivent en toutes lettres), donc une fonction de
  construction n'obligerait personne à passer par elle, et un `assert` dans une construction ne protège pas les objets
  écrits à la main ; la validation se vérifie à chaque frontière (`assert(objetValide(...))` dans `Equipement::poser` et
  `Inventaire::poser` / `ajouter`, `assert` à la fin de `genererObjet`, test de données `test_CoherenceDesDonnees` pour le
  sac de départ, et les 10 000 tirages de `test_GenerationDObjets`) et se teste comme un prédicat, sans faire échouer le test par un `assert`. La valeur strictement positive vient
  d'une hypothèse de `objetsActifs` (bonus positifs ou nuls).
- **Le catalogue d'affixes** (`CatalogueAffixes`, en données comme les autres catalogues) : `DefinitionDAffixe { id, nom,
  position, effet, valeurs, categories }` dans l'ordre de `IdAffixe` (contrôlé par un `assert`). Douze affixes. Depuis
  l'étape 96, chacun a une **plage de valeurs** `{min, max}` (entiers, bornes incluses), qui ne sert qu'au tirage : la
  valeur d'un objet est dans l'objet, rééquilibrer une plage ne change pas les objets déjà tirés. Valeurs « à tester » :
  Blazing et Hallowed 3 – 8, Keen 10 – 20 (%), Reinforced 2 – 5, les cinq caractéristiques 1 – 3, Evasion et Deflection
  2 – 4 (%), Blocking 3 – 7 (%). Un objet Rare utilise les mêmes plages qu'un objet Magic.

  | Affixe | Position | Effet | Catégories permises |
  |---|---|---|---|
  | Blazing (`Flamboyant`) | préfixe | points de feu ajoutés à l'attaque de base | armes |
  | Hallowed (`Sanctifie`) | préfixe | points de sacré ajoutés à l'attaque de base | armes |
  | Keen (`Acere`) | préfixe | pénétration d'armure en pour cent | armes |
  | Reinforced (`Renforce`) | préfixe | armure | casque, armure, jambières, gants, bottes, bouclier |
  | of the Ox / Bear / Fox / Owl / Lion | suffixe | un point de Might / Constitution / Dexterity / Intellect / Resolve | toutes |
  | of Evasion (`DEsquive`) | suffixe | esquive contre le contact, en pour cent | bottes, armure |
  | of Deflection (`DeDeviation`) | suffixe | esquive contre les projectiles, en pour cent | casque, bouclier |
  | of Blocking (`DeBlocage`) | suffixe | chance de blocage, en pour cent | bouclier |

  Aucun affixe de résistance pour l'instant : les monstres ne frappent qu'en physique.
- **Nom affiché** : `nomDeLObjet(objet)` = `[préfixe] modèle [suffixe]` (« Blazing Mace », « Keen Bow of the Fox »,
  « Reinforced Chain Mail of Evasion »). L'anglais n'accorde pas : aucun genre ni nombre sur les modèles.
- **Les affixes entrent dans les totaux existants, qui restent les seuls endroits qui calculent**, recalculés à chaque
  appel, jamais ajustés. `apportDesAffixes(objet)` (`CatalogueAffixes`) rend ce que les affixes d'un objet ajoutent, rangé
  comme l'équipement le tient déjà (`ApportDesAffixes { armure, bonus, bonusDeCombat, degatsAjoutes, penetration }`), par
  un `switch` sans `default` sur l'effet. `Equipement` l'additionne à celui du modèle :
  `armureTotale` (armure), `bonusTotal` (caractéristiques), `bonusDeCombatTotal` (esquives, **chance de blocage ajoutée
  à celle du bouclier, le montant reste celui du bouclier**), `attaqueDeBase` (voir ci-dessous).
- **L'attaque de base devient typée** : `DescriptionDAttaque::degats` est un `Degats` (une portion par élément,
  `docs/combat.md`) au lieu d'un entier. Une arme est physique ; ses affixes y ajoutent du feu ou du sacré (« Blazing
  Mace » : physique + feu) et de la pénétration (celle de l'arme plus celle de « Keen » ; le plafond de 75 % ne change pas
  et s'applique avec la défense). `Personnage::attaqueDeLArme` applique la Puissance **à chaque portion** (un arrondi par
  portion, `montantEffectif`). Les coups d'opportunité utilisent l'attaque de base, donc en profitent ; un skill de
  frappe (la frappe sacrée) prend **toutes** les portions de l'arme, affixes compris, plus son bonus.
- **Les objets actifs** : le bonus de caractéristiques d'un affixe compte dans le point fixe de `objetsActifs`
  (`Equipement::actifPour` passe le bonus du modèle plus celui des affixes) : un suffixe « of the Ox » peut rendre actif un
  autre objet. Les **seuils ne dépendent que du modèle**. Un objet inactif perd tout : ses propriétés de modèle et ses
  affixes (on retire l'objet entier de l'équipement actif).
- **F7** (débogage) : `Partie::ramasserDesObjetsAleatoires(4)` tire quatre objets avec `genererObjet` et le `Generateur` de
  la partie, puis les met dans le sac par `Escouade::ramasserTout`, **tout ou rien** : sans quatre cases libres
  (`Inventaire::casesLibres`), rien n'est ajouté **et aucun tirage n'est fait** (la vérification précède le tirage, pour que
  le refus ne fasse pas avancer le générateur). Les anciens objets de démonstration fixes ont disparu.
- **Affichage** : le nom composé, de la couleur de la rareté dans l'infobulle ; le texte d'un objet magique ou rare (sigle
  dans le sac, nom sur un emplacement, objet saisi) prend la couleur de sa rareté ; le cadre rouge d'un objet inactif reste
  prioritaire. Voir « Affichage ».

## Génération aléatoire (étape 96)

`genererObjet(Generateur&)` (`GenerationDObjets`) tire un objet **toujours valide** (`objetValide`). Fonction libre de
`objets/`, sans état : le seul hasard est le `Generateur` reçu, que la `Partie` possède (`objets/` inclut `base/`). Le tirage
est le principal fabricant d'objets et le garant de leurs invariants ; on a gardé l'agrégat `Objet` (pas de classe à
constructeur).
1. **La rareté**, selon `poidsDeRarete` : Common 70, Magic 25, Rare 5 (des poids en données, comme les compositions de groupe ;
   « à tester »). Elle est tirée **en premier et jamais retirée** : les proportions observées sont celles des poids.
2. **Le modèle**, uniformément parmi ceux qui peuvent porter cette rareté. La règle est écrite une fois,
   `peutPorterLaRarete(modèle, rareté)` : Common, tous ; Magic, au moins un affixe permis pour la catégorie ; Rare, au moins un
   préfixe **et** un suffixe permis. Les anneaux et les colliers n'ont aucun préfixe : ils ne sont jamais Rare (voulu, voir les
   pistes). Les gants et les bottes peuvent être Rare depuis que Reinforced leur est permis (ce sont des pièces d'armure).
3. **Magic** : un affixe uniformément parmi **tous** ceux que la catégorie permet, préfixes et suffixes confondus
   (`affixesPermisPour`), placé selon sa position. Une catégorie qui a plus de suffixes que de préfixes (une arme : 3
   préfixes, 5 suffixes) donne donc plus souvent un suffixe. **Rare** : un préfixe uniforme parmi les préfixes permis et un
   suffixe uniforme parmi les suffixes permis. Pas de poids par affixe (ils seraient tous égaux).
4. **Chaque valeur** : un entier uniforme dans la plage de l'affixe.
L'ordre des tirages est fixe (rareté, modèle, affixe, valeur, puis le second affixe d'un Rare), donc une graine donne
toujours la même suite d'objets sur une même plateforme.

**Portabilité** : `std::mt19937` est défini exactement par la norme, mais `std::uniform_int_distribution` ne l'est pas. Une
même graine peut donc donner d'autres objets avec une autre bibliothèque standard (MSVC, libstdc++, libc++). C'est sans
conséquence sur une seule plateforme, sans sauvegarde de graine ni multijoueur : rien à corriger. Les tests n'attendent
donc aucun objet précis pour une graine : ils vérifient la validité, la couverture, les plages, les proportions et le
déterminisme.

**Proportions testées** : sur 10 000 tirages, l'écart type d'une proportion est d'au plus 0,5 point ; la tolérance de
2 points (plus de quatre écarts types) ne sert pas à absorber le hasard, la graine étant fixe, mais à détecter un
algorithme modifié (poids changés, rareté retirée après coup).

## Le butin au sol (étape 97)

Les objets existent maintenant hors du sac et de l'équipement : un monstre qui meurt peut en laisser, ils restent au sol, se
voient et se ramassent, et on peut en jeter. Un objet est toujours à **un seul endroit** : le sac, un emplacement ou le sol.
- **Les données de butin** sont dans `DefinitionDeMonstre` (`chanceDeButin` en pour cent, `nombreDObjets`) : nuée 5 % et
  1 objet, infanterie 20 % et 1, infanterie lourde 35 % et 1, créature monstrueuse 100 % et 3 (valeurs « à tester »). Pas de
  tables de butin (quels modèles pour quel monstre) ni de rareté minimale : chaque objet est un `genererObjet`.
- **Le tirage** (`genererButin(Generateur&, chance, nombre)`, `GenerationDObjets`) : un tirage pour la chance, puis `nombre`
  fois `genererObjet`. **Aucun tirage pour la chance quand elle vaut 0 ou 100**, comme le moteur du coup ne tire pas pour une
  chance nulle : un monstre sans butin ne fait pas avancer le `Generateur`, donc les suites d'une partie (critiques, esquives,
  objets) ne changent pas. Il a lieu à la mort, dans `Partie::retirerLesMorts`, avec le `Generateur` de la partie.
- **`ObjetAuSol { id, objet, position }`** (`jeu/ObjetAuSol`), rangé dans un `ObjetsAuSol` (`jeu/ObjetsAuSol`, étape 98.1)
  possédé par la `Partie`. L'identifiant est un entier **jamais réutilisé** dans l'ensemble (`ObjetsAuSol::m_prochainId`, comme
  l'identifiant d'un monstre) : tout ce qui désigne un objet au sol
  (survol, clic, visée de ramassage) passe par lui, jamais par un indice du conteneur, qui change quand on retire un élément.
  La position est le centre d'une case.
- **Le placement**, `ObjetsAuSol::positionPour(carte, origine)` (une fonction libre `positionPourUnObjet(carte, origine, auSol)` avant l'étape 98.1) : sans hasard, utilisée pour le butin (origine : le monstre
  mort) comme pour un objet jeté (origine : l'ancre). Le premier objet tombe au centre de la case de l'origine ; si un objet
  y est déjà, sur la case voisine la plus proche **praticable** (ni mur ni porte fermée) et **sans objet**, en cherchant par
  couronnes de cases jusqu'à trois cases (les quatre côtés avant les diagonales) ; si tout est pris ou bloqué, il s'empile
  sur la case de l'origine. Pas de hasard : les trois objets de la créature monstrueuse tombent toujours sur le même motif.
  **Depuis l'étape 98.4, la case de l'escalier compte comme prise** : un objet la quitte pour une voisine, sauf si tout est pris
  autour (alors il s'y pose faute de mieux : « quand c'est possible »).
- **Un objet au sol ne gêne pas** : ni collision ni chemin. Il appartient au niveau : une nouvelle `Partie` (F5, nouveau
  niveau, recommencement) les efface, rien de plus à faire.
- **Brouillard** : un objet se comporte comme le terrain. Il se dessine avant le voile de la vision : caché sous l'inconnu,
  assombri sous l'exploré, net dans ce qu'on voit. Il ne se désigne (survol, clic) que sur une case non inconnue
  (`Partie::objetAuSolSous`).
- **Le ramassage** : un clic gauche sur un objet (`cliquer`, après le monstre, avant l'escalier et la porte) est un ordre : l'escouade marche
  vers lui comme pour un clic sur le sol (le verrou est levé, les coups engagés annulés). La `Partie` retient l'identifiant
  visé (`m_objetAuSolVise`, `escouade/` ne connaît pas `jeu/`) ; `ramasserSiAPortee`, à chaque image après le déplacement,
  ramasse l'objet quand l'**ancre** est à moins de `s_distanceDeRamassage` (30 px, centre à centre, « à tester »). Tout nouvel
  ordre (clic sur le sol, un monstre, une porte, un autre objet ; `ordonnerDeplacement`) remplace la visée ; un objet visé qui
  n'existe plus l'abandonne.
- **Sac plein** : l'objet reste au sol, l'ordre est terminé (la visée est effacée, l'escouade ne retente pas) et l'événement
  `SacPlein` produit le texte « Bag full » au-dessus de l'escouade (`docs/combat.md`, `docs/glossaire.md`). L'escouade continue
  son déplacement jusqu'à l'objet (c'était son ordre), mais plus rien n'est tenté ni signalé.
- **Jeter** : glisser un objet (du sac ou d'un emplacement porté) et le lâcher hors du panneau de l'inventaire
  (`EcranInventaire`, `docs/ecrans.md`) appelle `jeterUnObjet(escouade, sol, carte, position)` (fonction libre de `jeu/ObjetsAuSol`, étape 98.1 ; c'était `Partie::jeterUnObjet`) : `Escouade::retirerObjet` l'ôte (retrait
  ordinaire : recalcul de l'armure, des caractéristiques et des défenses, skill en activation ou en attente abandonné si
  l'arme exigée disparaît, objets devenus inactifs restent portés) puis `ObjetsAuSol::poser` (donc `positionPour`) le pose près de l'ancre, avec un
  identifiant neuf. Les deux étapes sont dans le même appel : l'objet n'est jamais perdu ni dupliqué. Le test des exemplaires
  (`exemplaires(const Partie&)`) compte maintenant sac, équipements **et sol**.

## Affichage (étape 84)

L'écran d'inventaire (`EcranInventaire`, `docs/ecrans.md`) montre l'équipement des trois personnages
et le sac, en lecture seule ; aucune fonction de modification n'est appelée.

- **Les mots sont dans l'interface, pas dans `objets/`** : `interface/AffichageObjets` donne le nom
  d'un emplacement (« Head », « Main Hand »…), le nom d'une catégorie (« Helmet », « Leggings »…) et le
  sigle d'un objet. Chaque table est un `switch` sans `default` sur l'enum, comme les sigles des
  skills : un emplacement, une catégorie ou un objet ajouté sans texte fait avertir le compilateur.
  Les noms des objets, eux, viennent du catalogue (`DefinitionObjet::nom`). Textes en UTF-8,
  convertis par `sf::String::fromUtf8`.
- **Emplacement vide** : rectangle grisé avec le nom de l'emplacement en petit ; **occupé** : le nom
  de l'objet. **Case du sac vide** : carré vide ; **occupée** : le sigle de deux lettres (IH Iron Helm,
  CM Chain Mail, IG Iron Greaves, RS Round Shield, LG Leather Gloves, LB Leather Boots, LC Leather Cap, PT Padded Tunic,
  NP Necklace of Protection, RP Ring of Protection, RM Ring of Might, NC Necklace of Constitution, LS Longsword, MA Mace, DA Dagger,
  GS Greatsword, SC Scepter, ST Staff, BO Bow ; table complète dans `docs/glossaire.md`). Un test vérifie que les sigles
  ont deux lettres et sont tous différents.
- **Infobulle** au survol d'un objet (`lignesDeLInfobulle(const Objet&)`, depuis l'étape 95 : elle prend l'objet, pas
  son modèle). Dans cet ordre : le nom composé (de la couleur de la rareté : blanc, jaune, vert), la catégorie, les
  propriétés du modèle — « Armor: +N » (absente si l'armure est nulle), une ligne par bonus de caractéristique
  (« Might: +3 »), une par bonus de combat (« Dodge vs Melee: +5 % », « Dodge vs Projectiles: +5 % » :
  `lignesDeBonusDeCombat`), pour un bouclier son blocage (« Block: 30 %, 12 points »), pour une arme son attaque
  (« Damage: 15 », « Cooldown: 0.3 to 0.7 s » — « 0.5 s » si les bornes sont égales —, « Range: 20 px », « Melee » ou
  « Projectile », « Penetration: 30 % » et « Two-handed ») —, **une ligne par affixe** (`ligneDAffixe` : « Fire Damage: +6 »,
  « Penetration: +15 % », « Dexterity: +2 », « Block: +5 % »…), puis les seuils (« Requires Dexterity 12 »). La mention
  « Inactive: requirement not met » est ajoutée par l'écran pour un objet porté inactif. Les durées s'écrivent avec un point.
  Rien au survol d'une case vide.
- **« Armure : N » d'une colonne** est `Equipement::armureTotale()`, la valeur dont la `Vitalite`
  du personnage est calculée ; elle n'est pas mise en cache dans l'écran.
- **Disposition** : `interface/DispositionInventaire`, fonctions pures qui servent au dessin, au
  survol et aux tests ; l'étape 85 s'en servira pour le glisser-déposer (`docs/ecrans.md`).

## Déplacements (étape 85)

- **Une position, un seul type** : `PositionDObjet` est un `std::variant<PositionDansLeSac,
  PositionPortee>`. La place d'une `PositionPortee` reste un entier de 1 à N (convention des places) :
  `objets/` ne dépend pas d'`escouade/`. Un seul déplacement remplace les quatre cas (sac → emplacement,
  emplacement → sac, sac → sac, emplacement → emplacement) ; `equiper` et `desequiper` sont supprimées.
- **Une règle, une fonction pure** : `deplacementPermis(de, objetDe, vers, objetVers)` ne connaît ni
  `Escouade`, ni `Inventaire`, ni `Personnage`. Faux si la source est vide ; vrai si `de == vers` ;
  si `vers` est un emplacement, l'objet doit pouvoir y aller (`peutAller`) ; si `vers` est occupé et
  que `de` est un emplacement, l'objet de `vers` doit aussi pouvoir aller à celui de `de` (échange).
  Vers une case du sac, tout objet va.
- **Requête et commande** : `Escouade::peutDeplacerObjet` lit les deux objets et appelle
  `deplacementPermis` ; `deplacerObjet` commence par appeler la requête, donc la surbrillance de l'écran
  et l'action ne peuvent pas diverger. Le déplacement sort les deux objets (`retirerA`) avant de les
  poser en croix (`poserA`) : aucun n'existe à deux endroits, et l'armure des personnages est
  recalculée par `Personnage`.
- **Sens de l'échange** : avec le catalogue actuel (un emplacement par catégorie, sauf les deux
  anneaux), la compatibilité est symétrique, donc aucun cas « permis dans un sens seulement » n'existe.
  La condition d'échange est écrite quand même : les armes à deux mains la rendront utile.
- **Glissement** : le modèle ne bouge pas pendant le glissement ; tout se joue au relâchement
  (`docs/ecrans.md`).

## Limites connues

- L'affichage est un sigle de deux lettres par objet : pas d'icônes ; la seule couleur est celle de la rareté
  (étape 95).
- Pas de niveau d'objet (les plages sont les mêmes pour tous les objets, Magic comme Rare), pas d'affixe de résistance ni
  de modification des seuils ; les anneaux et les colliers n'ont aucun préfixe donc ne sont jamais Rare ; un seul préfixe et un seul suffixe ; pas d'objet unique ni de rareté supérieure à Rare.
- L'emplacement affiche le nom du modèle (en la couleur de la rareté), pas le nom composé : trop long pour la cellule.
- Pas de restriction d'équipement par classe. Une nouvelle partie repart toujours du départ (aucune sauvegarde de
  l'équipement ni des objets au sol). Pas de deux armes à la fois (la main gauche ne tient qu'un bouclier), ni de lancer, ni
  de munitions.
- Butin (étape 97) : pas de tables par monstre, pas de rareté minimale, pas d'or ; les objets au sol ne disparaissent pas avec
  le temps ; le nom d'un objet au sol ne s'affiche qu'au survol (pas de touche pour tous les montrer) ; pas de ramassage
  automatique ni de « tout ramasser » ; un objet jeté tombe toujours près de l'ancre (pas de choix de l'endroit) ; l'écran
  d'inventaire ne dit pas qu'un lâcher hors du panneau jette l'objet.
- Pas de réaction aux seuils au-delà de l'inactivité : un objet devenu inactif n'est ni déséquipé ni signalé par un
  événement (l'écran relit l'état).
- Un objet qui ne convient qu'un instant (le seuil franchi par un bonus qu'un échange retire dans la même
  opération) est jugé sur l'état après le déplacement, jamais sur un état intermédiaire.
- `Monstre` n'a pas d'équipement : son armure reste fixe (celle de sa définition, `docs/monstre.md`).
- Les modifications d'équipement ne sont pas annoncées (aucun événement) : l'écran relit l'état à
  chaque image.
- `armureDe` (tests) et `Equipement::armureTotale` répètent la même somme, volontairement : le test
  doit pouvoir détecter une erreur dans la fonction qu'il contrôle.

## Référence des modules

### `Emplacement` (`objets/`)

`Emplacement.hpp` / `.cpp` : deux énumérations et une fonction pure.

`enum class Emplacement { Tete, Torse, Jambes, Gants, Bottes, MainDroite, MainGauche, Collier,
Anneau1, Anneau2 }` ; `inline constexpr std::size_t s_nombreEmplacements = 10` ; `enum class
CategorieObjet { Casque, Armure, Jambieres, Gants, Bottes, Bouclier, Collier, Anneau, Arme }`.

| Fonction | Rôle |
|---|---|
| `bool peutAller(CategorieObjet, Emplacement)` | Un objet de cette catégorie peut-il être porté à cet emplacement ? Chaque catégorie a un emplacement, sauf l'anneau (`Anneau1` et `Anneau2`) ; le bouclier va en `MainGauche` seulement ; l'arme va en `MainDroite` seulement (la main droite n'accepte qu'elle) ; `[[nodiscard]]` |

### `Objet` (`objets/`)

`Objet.hpp`, sans `.cpp`. `enum class IdObjet { CasqueDeFer, CuirasseDeMailles, JambieresDeFer,
BouclierRond, GantsDeCuir, BottesDeCuir, CasqueDeCuir, TuniqueMatelassee, CollierDeProtection,
AnneauDeProtection, AnneauDePuissance, CollierDEndurance, EpeeLongue, Masse, Dague, EpeeADeuxMains, Sceptre, Baton,
Arc }` ; `struct Objet { IdObjet modele; Rarete rarete = Rarete::Commun; std::optional<Affixe> prefixe = {};
std::optional<Affixe> suffixe = {}; }` avec `operator==` par défaut (étape 95 ; avant : `struct Objet { IdObjet id; }`).

### `Affixe` (`objets/`)

`Affixe.hpp`, sans `.cpp`. `enum class Rarete { Commun, Magique, Rare }` ; `enum class IdAffixe { Flamboyant, Sanctifie,
Acere, Renforce, DuBoeuf, DeLOurs, DuRenard, DeLaChouette, DuLion, DEsquive, DeDeviation, DeBlocage }` (quatre préfixes puis
huit suffixes) ; `struct Affixe { IdAffixe id; int valeur; }` avec `operator==`.

### `CatalogueAffixes` (`objets/`)

`CatalogueAffixes.hpp` / `.cpp`. `enum class PositionDAffixe { Prefixe, Suffixe }` ; `enum class EffetDAffixe {
DegatsDeFeu, DegatsSacres, PenetrationDArmure, Armure, Puissance, Endurance, Adresse, Savoir, Volonte, EsquiveContact,
EsquiveProjectile, ChanceDeBlocage }` ; `struct DefinitionDAffixe { IdAffixe id; std::string nom; PositionDAffixe position;
EffetDAffixe effet; PlageDeValeurs valeurs; std::vector<CategorieObjet> categories; }` ; `struct PlageDeValeurs { int min; int max; }` ; `struct ApportDesAffixes { int armure = 0; Caracteristiques
bonus; StatistiquesDeCombat bonusDeCombat; Degats degatsAjoutes; int penetration = 0; }`.

| Fonction | Rôle |
|---|---|
| `const std::vector<DefinitionDAffixe>& catalogueAffixes()` | Tous les affixes, dans l'ordre de `IdAffixe` ; l'unique source de leurs données ; `[[nodiscard]]` |
| `const DefinitionDAffixe& definitionAffixe(IdAffixe)` | L'affixe de cet identifiant ; `assert` : le catalogue suit l'ordre de l'enum ; `[[nodiscard]]` |
| `bool affixePermis(IdAffixe, CategorieObjet)` | Un objet de cette catégorie peut-il le porter ? `[[nodiscard]]` |
| `std::vector<IdAffixe> affixesPermisPour(CategorieObjet, PositionDAffixe)` | Les affixes de cette position qu'une catégorie peut porter, dans l'ordre du catalogue (étape 96.2) ; `[[nodiscard]]` |
| `ApportDesAffixes apportDesAffixes(const Objet&)` | Ce que les affixes de l'objet ajoutent (préfixe plus suffixe) ; nul pour un objet commun ; `switch` sans `default` sur l'effet ; `[[nodiscard]]` |

### `CatalogueObjets` (`objets/`)

`CatalogueObjets.hpp` / `.cpp` : `struct DefinitionObjet { IdObjet id; std::string nom;
CategorieObjet categorie; int armure; Caracteristiques bonus = {}; StatistiquesDeCombat bonusDeCombat = {};
std::optional<DonneesDArme> arme = {}; std::optional<DonneesDeBouclier> bouclier = {};
std::vector<SeuilDeCaracteristique> seuils = {}; }` et des fonctions libres, sans état.

| Fonction | Rôle |
|---|---|
| `const std::vector<DefinitionObjet>& catalogueObjets()` | Tous les modèles, dans l'ordre de `IdObjet` ; l'unique source de leurs valeurs ; `[[nodiscard]]` |
| `const DefinitionObjet& definitionObjet(IdObjet id)` | Le modèle de cet identifiant ; `assert` : le catalogue suit l'ordre de l'enum ; `[[nodiscard]]` |
| `bool seuilsAtteints(IdObjet, const Caracteristiques&)` | Ces caractéristiques atteignent-elles tous les seuils de l'objet ? Toujours vrai sans seuil (étape 92.1) ; `[[nodiscard]]` |
| `bool estADeuxMains(IdObjet)` | Une arme à deux mains (épée à deux mains, bâton, arc) ; faux pour tout autre objet ; `[[nodiscard]]` |
| `const DescriptionDAttaque& mainsNues()` | L'attaque de base sans arme ; `[[nodiscard]]` |
| `bool objetValide(const Objet&)` | Les invariants de l'objet : affixes à leur position, permis pour la catégorie, de valeur > 0, nombre conforme à la rareté (étape 95.3) ; `[[nodiscard]]` |
| `std::string nomDeLObjet(const Objet&)` | « [préfixe] modèle [suffixe] » (« Blazing Mace ») ; `[[nodiscard]]` |
| `std::vector<Objet> sacDeDepart()` | Le contenu du sac au début d'une partie : gants de cuir, bottes de cuir, casque de cuir, tunique matelassée, collier de protection, deux anneaux de protection, l'anneau de puissance, le collier d'endurance, une dague et une épée à deux mains ; `[[nodiscard]]` |

Valeurs d'armure (provisoires ; recalibrées à l'étape 87.3 pour le modèle M1, puis à l'échelle × 3 à l'étape 91.2) :
casque de fer 2, cuirasse de mailles 4, jambières de fer 2, bouclier rond 1, gants de cuir 1, bottes de cuir 1,
casque de cuir 1, tunique matelassée 2, collier de protection 1, anneau de protection 1 ; armes 0.

Armes (étape 91, valeurs provisoires, « à tester ») :

| Arme | Dégâts | Recharge | Exécution | Portée | Nature | Mains |
|---|---|---|---|---|---|---|
| Épée longue, masse | 15 | 0,3 – 0,7 s | 0,15 s | 20 px | contact | 1 |
| Dague | 14 | 0,4 – 0,8 s | 0,15 s | 20 px | contact | 1 |
| Épée à deux mains | 60 | 1,2 – 2,8 s | 0,3 s | 20 px | contact | 2 |
| Sceptre | 15 | 0,4 – 0,6 s | 0,15 s | 120 px | projectile | 1 |
| Bâton, arc | 15 | 0,4 – 0,6 s | 0,15 s | 120 px | projectile | 2 |
| *Mains nues* (pas un objet) | 6 | 0,4 – 0,6 s | 0,15 s | 20 px | contact | — |

Le coefficient de Puissance de l'attaque de base est 1,0 (`Personnage`, comme avant l'étape 91).

Bonus de caractéristiques (étape 88) : `DefinitionObjet` reçoit `Caracteristiques bonus = {}` (vide pour les
objets de protection). Deux objets ajoutés au catalogue (`IdObjet::AnneauDePuissance`, `CollierDEndurance`,
en fin d'enum) et au sac de départ : « Anneau de puissance » (anneau, armure 0, Puissance +3) et « Collier
d'endurance » (collier, armure 0, Endurance +3). Un collier d'endurance donne +60 points de vie maximum
(20 par point d'Endurance).

Bonus de combat (étape 90.2) : `DefinitionObjet` reçoit `StatistiquesDeCombat bonusDeCombat = {}`, le
même type que les statistiques d'un personnage, qui s'y additionnent (`docs/combat.md`). Depuis l'étape 94, ce type
porte aussi les **résistances** par élément (`Resistances`), additionnées de la même façon ; aucun objet n'en donne
encore (étape 95), tous les personnages ont 0. Deux objets en ont
un (valeurs « à tester », constantes nommées de `CatalogueObjets.cpp`) : les **bottes de cuir** +5 %
d'esquive contre le contact, le **casque de cuir** +5 % d'esquive contre les projectiles. Le **bouclier rond**
(30 % de chance de blocage, 12 points absorbés) est depuis l'étape 91.6 une propriété du bouclier
(`DonneesDeBouclier`), plus un bonus.

### `GenerationDObjets` (`objets/`)

`GenerationDObjets.hpp` / `.cpp` : des fonctions libres, sans état. `Affixe.hpp` porte aussi `inline constexpr std::array
s_raretes` (les trois raretés, de la plus commune à la plus rare).

| Fonction | Rôle |
|---|---|
| `int poidsDeRarete(Rarete)` | Le poids de cette rareté dans le tirage : 70, 25, 5 (« à tester ») ; `[[nodiscard]]` |
| `bool peutPorterLaRarete(IdObjet, Rarete)` | L'unique endroit de la règle : Common toujours, Magic avec au moins un affixe permis, Rare avec un préfixe et un suffixe permis ; `[[nodiscard]]` |
| `Objet genererObjet(Generateur&)` | Un objet tiré au hasard, valide (voir « Génération aléatoire ») ; `assert(objetValide)` ; `[[nodiscard]]` |
| `std::vector<Objet> genererButin(Generateur&, int chancePourCent, int nombre)` | Le butin d'une mort (étape 97.3) : vide si la chance est nulle ou si le tirage de chance échoue, sinon `nombre` objets de `genererObjet` ; **un seul tirage pour la chance, aucun à 0 ou 100** ; `assert` : chance de 0 à 100 ; `[[nodiscard]]` |

### `Equipement` (`objets/`)

`Equipement.hpp` / `.cpp` : `struct ObjetDeDepart { Emplacement emplacement; IdObjet objet; }` (avec
`operator==`) et la classe `Equipement`, possédée par valeur par chaque `Personnage`.

Données membres : `m_objets` (`std::array<std::optional<Objet>, s_nombreEmplacements>`, indexé par
`Emplacement`).

| Fonction | Rôle |
|---|---|
| `explicit Equipement(const std::vector<ObjetDeDepart>& objetsDeDepart = {})` | Équipé des objets de départ, tous communs (vide par défaut) ; `assert` : chaque objet peut aller à son emplacement et deux entrées n'ont pas le même emplacement |
| `const std::optional<Objet>& a(Emplacement) const` | L'objet porté à cet emplacement, s'il y en a un ; `[[nodiscard]]` |
| `std::optional<Objet> poser(Emplacement, Objet)` | Porte l'objet et rend celui qui y était ; `assert(objetValide(...))` et `assert(peutAller(...))` : c'est à l'appelant de vérifier ; `[[nodiscard]]` |
| `std::optional<Objet> retirer(Emplacement)` | Enlève l'objet et le rend (vide si l'emplacement l'était) ; `[[nodiscard]]` |
| `int armureTotale() const` | Somme de l'armure des objets portés (modèle plus affixes), recalculée à chaque appel ; `[[nodiscard]]` |
| `StatistiquesDeCombat bonusDeCombatTotal() const` | Somme des bonus de combat des objets portés (modèle plus affixes) et du blocage de leurs boucliers (chance, montant ; la chance d'un affixe s'y ajoute), recalculée à chaque appel ; `[[nodiscard]]` |
| `Caracteristiques bonusTotal() const` | Somme des bonus de caractéristiques des objets portés (modèle plus affixes), recalculée à chaque appel (étape 88) ; `[[nodiscard]]` |
| `DescriptionDAttaque attaqueDeBase() const` | L'attaque de base que donne l'équipement : celle de l'arme de la main droite avec le feu, le sacré et la pénétration de ses affixes, sinon `mainsNues()` (étape 91.4, 95.4) ; `[[nodiscard]]` |
| `std::optional<TypeDArme> typeDArme() const` | Le type de l'arme de la main droite, vide sans arme (étape 91.5) ; `[[nodiscard]]` |
| `Equipement actifPour(const Caracteristiques& base) const` | L'équipement sans ses objets inactifs (seuil non atteint par `base` et les bonus des autres objets actifs : `objetsActifs`), étape 92.2 ; `[[nodiscard]]` |
| `bool operator==(const Equipement&) const` | Défaut : l'`Escouade` compare l'état voulu à l'état courant (étape 91.3) |

Fonction libre : `Caracteristiques caracteristiquesEffectives(const Caracteristiques& base, const Equipement&)` : la base plus
les bonus des objets actifs, seul endroit qui les calcule (`[[nodiscard]]`).

### `ObjetsActifs` (`objets/`) — quels objets portés ont un effet (étape 92.2)

`ObjetsActifs.hpp` / `.cpp` : `struct SeuilDeCaracteristique { Caracteristique caracteristique; int valeur; }`, `struct
ExigencesEtBonus { std::vector<SeuilDeCaracteristique> seuils; Caracteristiques bonus; }` et une fonction pure.

| Fonction | Rôle |
|---|---|
| `std::vector<bool> objetsActifs(const Caracteristiques& base, const std::vector<std::optional<ExigencesEtBonus>>& portes)` | Un booléen par emplacement de `portes` (faux s'il est vide) : le point fixe décrit plus haut ; `[[nodiscard]]` |

### `Inventaire` (`objets/`)

`Inventaire.hpp` / `.cpp` : le sac, des cases à position fixe. Les indices partent de 0 ; `assert` :
un indice est inférieur à `capacite()`.

Données membres : `m_cases` (`std::vector<std::optional<Objet>>`, dimensionné à la construction).

| Fonction | Rôle |
|---|---|
| `explicit Inventaire(std::size_t capacite)` | Un sac de cette taille, toutes les cases vides |
| `std::size_t capacite() const` | Nombre de cases ; `[[nodiscard]]` |
| `std::size_t casesLibres() const` | Nombre de cases vides (étape 96.4) ; `[[nodiscard]]` |
| `const std::optional<Objet>& a(std::size_t caseDuSac) const` | L'objet de cette case, s'il y en a un ; `[[nodiscard]]` |
| `std::optional<Objet> prendre(std::size_t caseDuSac)` | Vide la case et rend son objet ; les autres cases ne bougent pas ; `[[nodiscard]]` |
| `std::optional<Objet> poser(std::size_t caseDuSac, Objet)` | Met l'objet dans la case et rend l'ancien contenu ; `[[nodiscard]]` |
| `bool ajouter(Objet)` | Met l'objet dans la première case libre ; faux, sans rien changer, si le sac est plein ; `[[nodiscard]]` |

### `PositionDObjet` (`objets/`)

`PositionDObjet.hpp` / `.cpp`. `struct PositionDansLeSac { std::size_t caseDuSac; }`, `struct
PositionPortee { std::size_t place; Emplacement emplacement; }` (chacune avec `operator==`) et
`using PositionDObjet = std::variant<PositionDansLeSac, PositionPortee>`.

| Fonction | Rôle |
|---|---|
| `bool deplacementPermis(const PositionDObjet& de, const std::optional<Objet>& objetDe, const PositionDObjet& vers, const std::optional<Objet>& objetVers)` | Seule source de la règle des déplacements (voir « Déplacements ») ; `[[nodiscard]]` |

### `DonneesDArme`, `DonneesDeBouclier` (`objets/`) et `TypeDArme`, `DescriptionDAttaque` (`combat/`)

En-têtes seuls (étape 91). `struct DonneesDArme { TypeDArme type; int mains; DescriptionDAttaque attaque; }` ;
`struct DonneesDeBouclier { int chanceDeBlocage; int montantBloque; }` ; `enum class TypeDArme { EpeeLongue, Masse,
Dague, EpeeADeuxMains, Sceptre, Baton, Arc }` ; `struct DescriptionDAttaque { Degats degats; float rechargeMin;
float rechargeMax; float tempsExecution; float portee; NatureDAttaque nature; int penetration; }` (depuis l'étape 95,
`degats` est typé : une portion par élément ; avant, un entier) (toutes avec `operator==`, sans
valeur par défaut : un oubli est signalé par le compilateur), accompagnée de `s_zoneDeControleDeContact` (20 px) et
de `zoneDeControleDe(NatureDAttaque)` (contact : 20, projectile et zone : 0).

### `DeplacementDObjets` (`objets/`) — la règle des déplacements (étape 91.3)

`DeplacementDObjets.hpp` / `.cpp` : `struct EtatDesObjets { Inventaire sac; std::vector<Equipement> equipements; std::vector<Caracteristiques> bases; }`
(un équipement et des caractéristiques de base par place, indice = place − 1) et trois fonctions libres (voir « Armes
(étape 91) » et « Seuils… (étape 92) »).

| Fonction | Rôle |
|---|---|
| `bool deplacerLesObjets(EtatDesObjets&, const PositionDObjet& de, const PositionDObjet& vers)` | Déplace l'objet de `de` vers `vers` (échange si occupé, deux mains comprises), ou rend faux sans rien changer ; `[[nodiscard]]` |
| `bool deplacementPossible(const EtatDesObjets&, de, vers)` | Le même calcul sur une copie ; `[[nodiscard]]` |
| `std::vector<SeuilDeCaracteristique> seuilsManques(const EtatDesObjets&, de, vers)` | Les seuils de l'objet posé en `vers` que son porteur n'atteindrait pas (même calcul que la décision) ; vide vers le sac ou si le déplacement est refusé pour une autre raison ; `[[nodiscard]]` (étape 92.3) |

### `AffichageObjets` (`interface/`)

`AffichageObjets.hpp` / `.cpp` : trois tables de texte et les lignes de l'infobulle, fonctions
libres sans état, qui rendent des `std::string` en UTF-8.

| Fonction | Rôle |
|---|---|
| `std::string nomDeLEmplacement(Emplacement)` | « Head », « Chest », « Legs », « Hands », « Feet », « Main Hand », « Off Hand », « Necklace », « Ring 1 », « Ring 2 » ; `switch` sans `default` ; `[[nodiscard]]` |
| `std::string nomDeLaCategorie(CategorieObjet)` | « Helmet », « Body Armor », « Leggings », « Gloves », « Boots », « Shield », « Necklace », « Ring », « Weapon » ; `[[nodiscard]]` |
| `std::string ligneDeSeuil(const SeuilDeCaracteristique&)` / `std::string ligneDObjetInactif()` | « Requires Might 11 » ; « Inactive: requirement not met » (étape 92.3) ; `[[nodiscard]]` |
| `std::string sigleDeLObjet(IdObjet)` | Deux lettres par objet du catalogue (IH, CM, IG, RS, LG, LB, LC, PT, NP, RP, RM, NC, LS, MA, DA, GS, SC, ST, BO) ; `[[nodiscard]]` |
| `sf::Color couleurDeLaRarete(Rarete)` | Blanc (commun), jaune (magique), vert (rare) ; lisibles sur le fond le plus clair de l'inventaire (contraste ≥ 4,5, un test le vérifie) ; `[[nodiscard]]` (étape 95.5) |
| `std::string ligneDAffixe(const Affixe&)` | « Fire Damage: +6 », « Holy Damage: +4 », « Penetration: +15 % », « Armor: +3 », « Dexterity: +2 », « Dodge vs Melee: +3 % », « Dodge vs Projectiles: +4 % », « Block: +5 % » ; `switch` sans `default` ; `[[nodiscard]]` |
| `std::vector<std::string> lignesDeLInfobulle(const Objet&)` | Nom composé, catégorie, propriétés du modèle (armure, bonus, bonus de combat, blocage, attaque), une ligne par affixe, seuils ; `[[nodiscard]]` |

### Ce que `Personnage` et `Escouade` ajoutent

- `Personnage` : membre `m_equipement`, `getEquipement()`, `poserEquipement(Emplacement, Objet)` et
  `retirerEquipement(Emplacement)` (qui rendent l'objet remplacé ou retiré et recalculent, d'un seul appel à
  `recalculerDepuisLEquipement()` privée, les caractéristiques effectives, le maximum de points de vie, les
  statistiques de combat et les défenses de la `Vitalite`) ; constructeur : `const
  std::vector<ObjetDeDepart>&` à la place de l'armure. `Vitalite::definirDefenses(const Defenses&)`
  (préconditions `armure >= 0`, `statistiques.capaciteDefensive > 0`).
- **Statistiques de combat (étapes 89 et 90)** : l'équipement agit de deux façons. Par l'**Adresse**
  (`DefinitionObjet::bonus`) : un point monte les deux capacités de 5 (`statistiquesDuPersonnage`,
  `docs/combat.md`) ; le catalogue d'aujourd'hui n'a aucun objet d'Adresse. Et **directement**, par
  `DefinitionObjet::bonusDeCombat` : esquives et blocage (étape 90), mais aussi les capacités, que
  `Equipement::bonusDeCombatTotal` additionne et que `statistiquesDuPersonnage` ajoute aux statistiques du
  personnage. Le bouclier rond est donc maintenant porteur d'un blocage ; **les personnages de test ne le
  portent plus** (`s_equipementMeleeDeTest`), pour que leurs coups reçus ne soient pas bloqués au hasard :
  les tests de blocage l'ajoutent explicitement (`docs/tests.md`).
- `Escouade` : membre `m_sac` (`Inventaire`, 40 cases), `sac()`, `equipementDuPersonnage(place)`,
  `ramasser`, `objetA`, `peutDeplacerObjet`, `deplacerObjet` (détail dans `docs/formation.md`, tableau
  d'`Escouade`) ; depuis l'étape 91, les deux derniers copient l'état dans un `EtatDesObjets` et appellent
  `deplacementPossible` / `deplacerLesObjets`.
  Une `place` suit la convention des places (1 à N, `assert` sur sa validité).

| Opération d'`Escouade` | Réussit quand | Effet |
|---|---|---|
| `ramasser(Objet)` | le sac a une case libre | l'objet va dans la première case libre |
| `ramasserTout(const std::vector<Objet>&)` | le sac a autant de cases libres que d'objets | tous les objets vont dans les premières cases libres, dans l'ordre ; sinon rien ne bouge (F7, étape 95.5 ; depuis l'étape 96 par `Partie::ramasserDesObjetsAleatoires`) |
| `deplacerObjet(de, vers)` | `deplacerLesObjets` (règle de base et deux mains) | l'objet de `de` va à `vers` ; si `vers` est occupé, échange ; une arme à deux mains (ou un bouclier sur elle) range l'autre main dans le sac ; `de == vers` : rien ne bouge |
| `retirerObjet(position)` | la position est occupée (sinon vide, rien ne bouge) | l'objet est retiré et rendu ; sa case ou son emplacement reste vide, le personnage recalcule armure, caractéristiques et défenses, un skill (en activation ou en attente) qui exigeait l'arme retirée est abandonné ; l'objet n'est plus nulle part : `jeterUnObjet` (`jeu/ObjetsAuSol`) le pose au sol (étape 97.6) |

### `ObjetAuSol` et `ObjetsAuSol` (`jeu/`) — les objets posés au sol (étapes 97 et 98.1)

`ObjetAuSol.hpp` : `struct ObjetAuSol { int id; Objet objet; sf::Vector2f position; }` (comparable) ; constantes
`s_rayonDeSurvolDObjet` (14 px, la zone où le curseur le désigne : un peu plus que son dessin de 8 px de rayon) et
`s_distanceDeRamassage` (30 px).

`ObjetsAuSol.hpp` / `.cpp` : une classe, `ObjetsAuSol`, qui possède le `std::vector<ObjetAuSol>` et le compteur d'identifiants
(`m_prochainId`, jamais réutilisé). Elle ne garde ni la carte ni la vision (une `Partie` se déplace : une référence resterait
pendante) : elles lui sont passées à chaque appel qui en a besoin. Extraite de la `Partie` à l'étape 98.1 (principe de moindre
connaissance : l'inventaire n'a plus besoin de la `Partie` entière pour jeter un objet).

| Fonction | Rôle |
|---|---|
| `void poser(const Objet&, sf::Vector2f origine, const TileMap&)` | Pose l'objet à `positionPour(carte, origine)`, avec un identifiant neuf |
| `bool retirer(int id)` | Retire l'objet de cet identifiant ; faux, sans rien changer, s'il n'y en a pas ; `[[nodiscard]]` |
| `const ObjetAuSol* trouver(int id) const` | L'objet de cet identifiant, `nullptr` sinon ; le pointeur ne survit pas à un `poser` ni à un `retirer` ; `[[nodiscard]]` |
| `const ObjetAuSol* sous(sf::Vector2f point, const TileMap&, const Vision&) const` | L'objet le plus proche à moins de `s_rayonDeSurvolDObjet` du point, sur une case que la vision ne dit pas inconnue ; `nullptr` sinon ; `[[nodiscard]]` |
| `sf::Vector2f positionPour(const TileMap&, sf::Vector2f origine) const` | Où poser un objet de plus : le centre de la case de `origine` si aucun objet n'y est, sinon la case voisine la plus proche, praticable, sans objet et qui n'est pas l'escalier (couronnes jusqu'à trois cases, les quatre côtés avant les diagonales), sinon la case de `origine` ; sans hasard ; `[[nodiscard]]` |
| `const std::vector<ObjetAuSol>& tous() const` | Les objets, dans l'ordre où ils sont tombés ; dessin et tests ; `[[nodiscard]]` |
| `bool jeterUnObjet(Escouade&, ObjetsAuSol&, const TileMap&, const PositionDObjet&)` (fonction libre) | Retire l'objet de cette position de l'escouade (`Escouade::retirerObjet`) et le pose au sol près de l'ancre, dans le même appel : un objet n'est jamais perdu ni dupliqué ; faux, sans rien changer, si la position est vide ; appelée par `EcranInventaire` quand on lâche un objet hors du panneau ; `[[nodiscard]]` |

### `AffichageObjetsAuSol` (`interface/`) — les objets au sol à l'écran (étape 97.4)

`AffichageObjetsAuSol.hpp` / `.cpp`, fonctions libres à appeler avec la vue du monde. `dessinerObjetsAuSol(rendu, objets)` :
un losange (cercle à quatre côtés, 8 px de rayon, contour noir) dans la couleur de la rareté (`couleurDeLaRarete`) ;
`dessinerNomDObjetAuSol(rendu, police, objet)` : le nom composé (`nomDeLObjet`) au-dessus du losange, dans la couleur de la
rareté, 14 px, contour noir. `EcranJeu` les appelle : les losanges entre la carte et les entités (`Partie::dessinerLaCarte`
puis `Partie::dessinerLesEntites`), le nom au survol après les textes flottants.
