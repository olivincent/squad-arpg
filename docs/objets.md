# Squad-ARPG — Objets, équipement et sac

Le modèle des objets (étape 83) : le catalogue, ce que porte chaque personnage, le sac de
l'escouade et les opérations qui déplacent un objet. L'écran d'inventaire (étape 84) les affiche, et
le glisser-déposer (étape 85) les déplace. L'armure et son effet sur les dégâts sont dans
`docs/combat.md`, la construction des personnages dans `docs/formation.md`.

## Décisions

- **Un dossier `objets/`, entre `combat` et `escouade`** : ordre `base → monde → combat → objets →
  escouade, monstres → jeu → …`. Le butin au sol (`jeu/`) et de futures tables de butin
  (`monstres/`) auront besoin des objets, et `monstres/` ne peut pas inclure `escouade/`. `objets/` ne
  dépend que de `base/`, de `combat/` (depuis l'étape 88 : les bonus de caractéristiques d'un objet, `combat/Caracteristiques`) et de la bibliothèque standard.
- **Emplacement ≠ catégorie** : dix emplacements (où l'on porte), huit catégories (ce qu'est
  l'objet). Deux emplacements (les anneaux) reçoivent la même catégorie, et plus tard une arme
  pourra aller dans deux mains. `peutAller(CategorieObjet, Emplacement)` est la seule source de la
  règle ; personne d'autre ne compare une catégorie à un emplacement.
- **Pas de catégorie d'arme** : la main droite reste vide (aucune catégorie n'y va). La classe
  `Arme` viendra plus tard ; une arme à deux mains sera une extension de `peutAller` avec une règle
  d'occupation de l'autre main, pas avant.
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
- **Équilibre inchangé** : le guerrier et le paladin partent équipés (casque de fer 10, cuirasse de
  mailles 20, jambières de fer 10, bouclier rond 10 : 50), les trois autres sans rien (0). Les
  valeurs sont provisoires.
- **Les tests ne lisent pas les valeurs du catalogue d'objets** (même règle que pour les classes) :
  les attentes sont calculées avec `definitionObjet(id).armure` ou `armureDe(equipement)`
  (`tests/aides.hpp`), jamais avec 50 ou 20 écrits en dur. `test_Specialite.cpp` reste le seul
  fichier qui lit l'équipement de départ du catalogue des classes.
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
  l'armure est nulle, étape 88) et une ligne par bonus de caractéristique (« Puissance : +3 »). Rien au
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
- Pas d'armes, pas de main droite, pas de restriction par classe, pas de butin au sol : une nouvelle
  partie repart toujours du départ (aucune sauvegarde de l'équipement).
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
CategorieObjet { Casque, Armure, Jambieres, Gants, Bottes, Bouclier, Collier, Anneau }`.

| Fonction | Rôle |
|---|---|
| `bool peutAller(CategorieObjet, Emplacement)` | Un objet de cette catégorie peut-il être porté à cet emplacement ? Chaque catégorie a un emplacement, sauf l'anneau (`Anneau1` et `Anneau2`) ; le bouclier va en `MainGauche` seulement ; `MainDroite` n'en accepte aucune ; `[[nodiscard]]` |

### `Objet` (`objets/`)

`Objet.hpp`, sans `.cpp`. `enum class IdObjet { CasqueDeFer, CuirasseDeMailles, JambieresDeFer,
BouclierRond, GantsDeCuir, BottesDeCuir, CasqueDeCuir, TuniqueMatelassee, CollierDeProtection,
AnneauDeProtection }` ; `struct Objet { IdObjet id; }` avec `operator==` par défaut.

### `CatalogueObjets` (`objets/`)

`CatalogueObjets.hpp` / `.cpp` : `struct DefinitionObjet { IdObjet id; std::string nom;
CategorieObjet categorie; int armure; }` et trois fonctions libres, sans état.

| Fonction | Rôle |
|---|---|
| `const std::vector<DefinitionObjet>& catalogueObjets()` | Tous les modèles, dans l'ordre de `IdObjet` ; l'unique source de leurs valeurs ; `[[nodiscard]]` |
| `const DefinitionObjet& definitionObjet(IdObjet id)` | Le modèle de cet identifiant ; `assert` : le catalogue suit l'ordre de l'enum ; `[[nodiscard]]` |
| `std::vector<Objet> sacDeDepart()` | Le contenu du sac au début d'une partie : gants de cuir, bottes de cuir, casque de cuir, tunique matelassée, collier de protection et deux anneaux de protection ; `[[nodiscard]]` |

Valeurs d'armure (provisoires, recalibrées à l'étape 87.3 pour le modèle M1) : casque de fer 6,
cuirasse de mailles 12, jambières de fer 6, bouclier rond 6, gants de cuir 2, bottes de cuir 2, casque
de cuir 3, tunique matelassée 5, collier de protection 4, anneau de protection 2.

Bonus de caractéristiques (étape 88) : `DefinitionObjet` reçoit `Caracteristiques bonus = {}` (vide pour les
objets de protection). Deux objets ajoutés au catalogue (`IdObjet::AnneauDePuissance`, `CollierDEndurance`,
en fin d'enum) et au sac de départ : « Anneau de puissance » (anneau, armure 0, Puissance +3) et « Collier
d'endurance » (collier, armure 0, Endurance +3). Un collier d'endurance donne +60 points de vie maximum
(20 par point d'Endurance).

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
| `Caracteristiques bonusTotal() const` | Somme des bonus de caractéristiques des objets portés, recalculée à chaque appel (étape 88) ; `[[nodiscard]]` |

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

### `AffichageObjets` (`interface/`)

`AffichageObjets.hpp` / `.cpp` : trois tables de texte et les lignes de l'infobulle, fonctions
libres sans état, qui rendent des `std::string` en UTF-8.

| Fonction | Rôle |
|---|---|
| `std::string nomDeLEmplacement(Emplacement)` | « Tête », « Torse », « Jambes », « Gants », « Bottes », « Main droite », « Main gauche », « Collier », « Anneau 1 », « Anneau 2 » ; `switch` sans `default` ; `[[nodiscard]]` |
| `std::string nomDeLaCategorie(CategorieObjet)` | « Casque », « Armure », « Jambières », « Gants », « Bottes », « Bouclier », « Collier », « Anneau » ; `[[nodiscard]]` |
| `std::string sigleDeLObjet(IdObjet)` | Deux lettres par objet du catalogue ; `[[nodiscard]]` |
| `std::vector<std::string> lignesDeLInfobulle(IdObjet)` | Nom, catégorie, « Armure : +N » si non nulle, une ligne « Nom : +N » par bonus de caractéristique ; `[[nodiscard]]` |

### Ce que `Personnage` et `Escouade` ajoutent

- `Personnage` : membre `m_equipement`, `getEquipement()`, `poserEquipement(Emplacement, Objet)` et
  `retirerEquipement(Emplacement)` (qui rendent l'objet remplacé ou retiré et recalculent, d'un seul appel à
  `recalculerDepuisLEquipement()` privée, les caractéristiques effectives, le maximum de points de vie, les
  statistiques de combat et les défenses de la `Vitalite`) ; constructeur : `const
  std::vector<ObjetDeDepart>&` à la place de l'armure. `Vitalite::definirDefenses(const Defenses&)`
  (préconditions `armure >= 0`, `capaciteDefensive > 0`).
- **Statistiques de combat (étape 89)** : l'équipement n'a pas encore de bonus de combat propre. Il agit sur
  les capacités offensive et défensive par l'**Adresse** : un objet qui donne de l'Adresse
  (`DefinitionObjet::bonus`) monte les deux capacités de 5 par point (`statistiquesDuPersonnage`,
  `docs/combat.md`). Le catalogue d'aujourd'hui n'a aucun objet d'Adresse ; la liste de bonus de statistiques
  de combat des objets (esquive, blocage) viendra à l'étape 90.
- `Escouade` : membre `m_sac` (`Inventaire`, 40 cases), `sac()`, `equipementDuPersonnage(place)`,
  `ramasser`, `objetA`, `peutDeplacerObjet`, `deplacerObjet` (détail dans `docs/formation.md`, tableau
  d'`Escouade`).
  Une `place` suit la convention des places (1 à N, `assert` sur sa validité).

| Opération d'`Escouade` | Réussit quand | Effet |
|---|---|---|
| `ramasser(Objet)` | le sac a une case libre | l'objet va dans la première case libre |
| `deplacerObjet(de, vers)` | `deplacementPermis` | l'objet de `de` va à `vers` ; si `vers` est occupé, échange ; `de == vers` : rien ne bouge |
