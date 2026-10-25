# Squad-ARPG — Objets, équipement et sac

Le modèle des objets (étape 83) : le catalogue, ce que porte chaque personnage, le sac de
l'escouade et les opérations qui déplacent un objet. L'écran d'inventaire (étape 84) les affiche, et
le glisser-déposer (étape 85) les déplace. Les armes (étape 91) donnent l'attaque de base et la zone de
contrôle, et le bouclier son blocage. L'armure et son effet sur les dégâts sont dans `docs/combat.md`,
la construction des personnages dans `docs/formation.md`.

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
- **`Objet` est l'instance, `DefinitionObjet` le modèle** : `Objet` ne porte que l'`IdObjet`, c'est
  une petite valeur copiable, sans pointeur vers le catalogue. Pas d'affixe aléatoire ni de rareté
  pour l'instant : deux objets du même modèle sont égaux (`operator==`).
- **Un objet existe à un seul endroit** (un emplacement ou une case du sac). Les déplacements se
  font par valeur, et les fonctions qui rendent l'objet remplacé sont `[[nodiscard]]` : oublier
  l'ancien objet le ferait disparaître. Un test compte les exemplaires de chaque modèle dans le sac
  et tous les équipements avant et après chaque opération.
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
- **La capacité (40) appartient à `Escouade`** (`s_capaciteDuSac`, dans `Escouade.cpp`), pas à
  `Inventaire`, qui reçoit sa capacité à la construction. `Partie` remplit le sac avec
  `sacDeDepart()` à sa construction.

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

## Affichage (étape 84)

L'écran d'inventaire (`EcranInventaire`, `docs/ecrans.md`) montre l'équipement des trois personnages
et le sac, en lecture seule ; aucune fonction de modification n'est appelée.

- **Les mots sont dans l'interface, pas dans `objets/`** : `interface/AffichageObjets` donne le nom
  d'un emplacement (« Tête », « Main droite »…), le nom d'une catégorie (« Casque », « Jambières »…) et le
  sigle d'un objet. Chaque table est un `switch` sans `default` sur l'enum, comme les sigles des
  skills : un emplacement, une catégorie ou un objet ajouté sans texte fait avertir le compilateur.
  Les noms des objets, eux, viennent du catalogue (`DefinitionObjet::nom`). Textes en UTF-8,
  convertis par `sf::String::fromUtf8`.
- **Emplacement vide** : rectangle grisé avec le nom de l'emplacement en petit ; **occupé** : le nom
  de l'objet. **Case du sac vide** : carré vide ; **occupée** : le sigle de deux lettres (CF casque de
  fer, CM cuirasse de mailles, JF jambières de fer, BR bouclier rond, GC gants de cuir, BC bottes de
  cuir, CC casque de cuir, TM tunique matelassée, CP collier de protection, AP anneau de protection, AU anneau de puissance, CE collier d'endurance).
  Un test vérifie que les sigles ont deux lettres et sont tous différents.
- **Infobulle** au survol d'un objet (`lignesDeLInfobulle`) : nom, catégorie, « Armure : +N » (absente si
  l'armure est nulle, étape 88), une ligne par bonus de caractéristique (« Puissance : +3 »), depuis
  l'étape 90 une ligne par bonus de combat (« Esquive contre la mêlée : +5 % », « Esquive contre les
  projectiles : +5 % » : `lignesDeBonusDeCombat`) et, depuis l'étape 91, pour un bouclier son blocage (« Blocage :
  30 %, 12 points ») et pour une arme son attaque : « Dégâts : 15 », « Recharge : 0,3 à 0,7 s » (« 0,5 s » si les
  bornes sont égales), « Portée : 20 px », « Contact » ou « Projectile » et « Deux mains » à deux mains. Rien au
  survol d'une case vide.
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

- L'affichage est un sigle de deux lettres par objet : pas d'icônes, pas de couleurs par rareté ou par
  catégorie.
- Pas de restriction d'équipement par classe, pas de butin au sol : une nouvelle partie repart toujours du
  départ (aucune sauvegarde de l'équipement). Pas de deux armes à la fois (la main gauche ne tient qu'un
  bouclier), ni de lancer, ni de munitions.
- Une arme à deux mains n'est pas encore liée à un seuil de caractéristiques (étape 92).
- `Monstre` n'a pas d'équipement : son armure reste fixe (`Niveau::armureMonstres`).
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
Arc }` ; `struct Objet { IdObjet id; }` avec `operator==` par défaut.

### `CatalogueObjets` (`objets/`)

`CatalogueObjets.hpp` / `.cpp` : `struct DefinitionObjet { IdObjet id; std::string nom;
CategorieObjet categorie; int armure; Caracteristiques bonus = {}; StatistiquesDeCombat bonusDeCombat = {};
std::optional<DonneesDArme> arme = {}; std::optional<DonneesDeBouclier> bouclier = {}; }` et cinq
fonctions libres, sans état.

| Fonction | Rôle |
|---|---|
| `const std::vector<DefinitionObjet>& catalogueObjets()` | Tous les modèles, dans l'ordre de `IdObjet` ; l'unique source de leurs valeurs ; `[[nodiscard]]` |
| `const DefinitionObjet& definitionObjet(IdObjet id)` | Le modèle de cet identifiant ; `assert` : le catalogue suit l'ordre de l'enum ; `[[nodiscard]]` |
| `bool estADeuxMains(IdObjet)` | Une arme à deux mains (épée à deux mains, bâton, arc) ; faux pour tout autre objet ; `[[nodiscard]]` |
| `const DescriptionDAttaque& mainsNues()` | L'attaque de base sans arme ; `[[nodiscard]]` |
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
même type que les statistiques d'un personnage, qui s'y additionnent (`docs/combat.md`). Deux objets en ont
un (valeurs « à tester », constantes nommées de `CatalogueObjets.cpp`) : les **bottes de cuir** +5 %
d'esquive contre le contact, le **casque de cuir** +5 % d'esquive contre les projectiles. Le **bouclier rond**
(30 % de chance de blocage, 12 points absorbés) est depuis l'étape 91.6 une propriété du bouclier
(`DonneesDeBouclier`), plus un bonus.

### `Equipement` (`objets/`)

`Equipement.hpp` / `.cpp` : `struct ObjetDeDepart { Emplacement emplacement; IdObjet objet; }` (avec
`operator==`) et la classe `Equipement`, possédée par valeur par chaque `Personnage`.

Données membres : `m_objets` (`std::array<std::optional<Objet>, s_nombreEmplacements>`, indexé par
`Emplacement`).

| Fonction | Rôle |
|---|---|
| `explicit Equipement(const std::vector<ObjetDeDepart>& objetsDeDepart = {})` | Équipé des objets de départ (vide par défaut) ; `assert` : chaque objet peut aller à son emplacement et deux entrées n'ont pas le même emplacement |
| `const std::optional<Objet>& a(Emplacement) const` | L'objet porté à cet emplacement, s'il y en a un ; `[[nodiscard]]` |
| `std::optional<Objet> poser(Emplacement, Objet)` | Porte l'objet et rend celui qui y était ; `assert(peutAller(...))` : c'est à l'appelant de vérifier ; `[[nodiscard]]` |
| `std::optional<Objet> retirer(Emplacement)` | Enlève l'objet et le rend (vide si l'emplacement l'était) ; `[[nodiscard]]` |
| `int armureTotale() const` | Somme de l'armure des objets portés, recalculée à chaque appel ; `[[nodiscard]]` |
| `StatistiquesDeCombat bonusDeCombatTotal() const` | Somme des bonus de combat des objets portés et du blocage de leurs boucliers (chance, montant), recalculée à chaque appel ; `[[nodiscard]]` |
| `Caracteristiques bonusTotal() const` | Somme des bonus de caractéristiques des objets portés, recalculée à chaque appel (étape 88) ; `[[nodiscard]]` |
| `DescriptionDAttaque attaqueDeBase() const` | L'attaque de base que donne l'équipement : celle de l'arme de la main droite, sinon `mainsNues()` (étape 91.4) ; `[[nodiscard]]` |
| `std::optional<TypeDArme> typeDArme() const` | Le type de l'arme de la main droite, vide sans arme (étape 91.5) ; `[[nodiscard]]` |
| `bool operator==(const Equipement&) const` | Défaut : l'`Escouade` compare l'état voulu à l'état courant (étape 91.3) |

### `Inventaire` (`objets/`)

`Inventaire.hpp` / `.cpp` : le sac, des cases à position fixe. Les indices partent de 0 ; `assert` :
un indice est inférieur à `capacite()`.

Données membres : `m_cases` (`std::vector<std::optional<Objet>>`, dimensionné à la construction).

| Fonction | Rôle |
|---|---|
| `explicit Inventaire(std::size_t capacite)` | Un sac de cette taille, toutes les cases vides |
| `std::size_t capacite() const` | Nombre de cases ; `[[nodiscard]]` |
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
Dague, EpeeADeuxMains, Sceptre, Baton, Arc }` ; `struct DescriptionDAttaque { int degats; float rechargeMin;
float rechargeMax; float tempsExecution; float portee; NatureDAttaque nature; }` (toutes avec `operator==`, sans
valeur par défaut : un oubli est signalé par le compilateur), accompagnée de `s_zoneDeControleDeContact` (20 px) et
de `zoneDeControleDe(NatureDAttaque)` (contact : 20, projectile et zone : 0).

### `DeplacementDObjets` (`objets/`) — la règle des déplacements (étape 91.3)

`DeplacementDObjets.hpp` / `.cpp` : `struct EtatDesObjets { Inventaire sac; std::vector<Equipement> equipements; }`
(un équipement par place, indice = place − 1) et deux fonctions libres (voir « Armes (étape 91) »).

| Fonction | Rôle |
|---|---|
| `bool deplacerLesObjets(EtatDesObjets&, const PositionDObjet& de, const PositionDObjet& vers)` | Déplace l'objet de `de` vers `vers` (échange si occupé, deux mains comprises), ou rend faux sans rien changer ; `[[nodiscard]]` |
| `bool deplacementPossible(const EtatDesObjets&, de, vers)` | Le même calcul sur une copie ; `[[nodiscard]]` |

### `AffichageObjets` (`interface/`)

`AffichageObjets.hpp` / `.cpp` : trois tables de texte et les lignes de l'infobulle, fonctions
libres sans état, qui rendent des `std::string` en UTF-8.

| Fonction | Rôle |
|---|---|
| `std::string nomDeLEmplacement(Emplacement)` | « Tête », « Torse », « Jambes », « Gants », « Bottes », « Main droite », « Main gauche », « Collier », « Anneau 1 », « Anneau 2 » ; `switch` sans `default` ; `[[nodiscard]]` |
| `std::string nomDeLaCategorie(CategorieObjet)` | « Casque », « Armure », « Jambières », « Gants », « Bottes », « Bouclier », « Collier », « Anneau » ; `[[nodiscard]]` |
| `std::string sigleDeLObjet(IdObjet)` | Deux lettres par objet du catalogue (EL épée longue, MA masse, DA dague, E2 épée à deux mains, SC sceptre, BT bâton, AR arc depuis l'étape 91) ; `[[nodiscard]]` |
| `std::vector<std::string> lignesDeLInfobulle(IdObjet)` | Nom, catégorie, « Armure : +N » si non nulle, une ligne « Nom : +N » par bonus de caractéristique, les bonus de combat, le blocage d'un bouclier, l'attaque d'une arme ; `[[nodiscard]]` |

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
| `deplacerObjet(de, vers)` | `deplacerLesObjets` (règle de base et deux mains) | l'objet de `de` va à `vers` ; si `vers` est occupé, échange ; une arme à deux mains (ou un bouclier sur elle) range l'autre main dans le sac ; `de == vers` : rien ne bouge |
