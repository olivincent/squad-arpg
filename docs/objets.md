# Squad-ARPG — Objets, équipement et sac

Le modèle des objets (étape 83), sans interface : le catalogue, ce que porte chaque personnage, le
sac de l'escouade et les trois opérations qui déplacent un objet. L'écran d'inventaire est à
l'étape 84, le glisser-déposer à l'étape 85. L'armure et son effet sur les dégâts sont dans
`docs/combat.md`, la construction des personnages dans `docs/formation.md`.

## Décisions

- **Un dossier `objets/`, entre `combat` et `escouade`** : ordre `base → monde → combat → objets →
  escouade, monstres → jeu → …`. Le butin au sol (`jeu/`) et de futures tables de butin
  (`monstres/`) auront besoin des objets, et `monstres/` ne peut pas inclure `escouade/`. `objets/` ne
  dépend que de `base/` et de la bibliothèque standard (en fait, pour l'instant, que de lui-même).
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
  portés, recalculée en entier depuis son `Equipement` (`Personnage::recalculerArmure`, seul
  endroit qui appelle `Vitalite::definirArmure`) après chaque pose ou retrait. Aucune addition ni
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
- **Opérations transactionnelles** : `ramasser`, `equiper` et `desequiper` vérifient toutes leurs
  conditions avant de modifier quoi que ce soit ; vraies, tout est fait, fausses, rien n'a bougé.
  Une fois les conditions vérifiées, l'échange ne peut plus échouer : `equiper` ne dit jamais
  « sac plein » parce que l'objet porté prend la case que l'objet équipé vient de libérer.
- **Équiper un mort est permis** : aucune règle ne l'interdit pour l'instant.
- **Le sac est à positions fixes** : retirer un objet laisse sa case vide, les autres ne bougent
  pas ; `ajouter` prend la première case libre. Le rangement par le joueur est l'étape 85.
- **La capacité (40) appartient à `Escouade`** (`s_capaciteDuSac`, dans `Escouade.cpp`), pas à
  `Inventaire`, qui reçoit sa capacité à la construction. `Partie` remplit le sac avec
  `sacDeDepart()` à sa construction.

## Limites connues

- Rien ne s'affiche : l'équipement et le sac ne se voient que par l'armure des personnages.
- Pas d'armes, pas de main droite, pas de restriction par classe, pas de butin au sol : une nouvelle
  partie repart toujours du départ (aucune sauvegarde de l'équipement).
- `Monstre` n'a pas d'équipement : son armure reste fixe (`Niveau::armureMonstres`).
- Les modifications d'équipement ne sont pas annoncées (aucun événement) : l'écran lira l'état.
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

Valeurs d'armure (provisoires) : casque de fer 10, cuirasse de mailles 20, jambières de fer 10,
bouclier rond 10, gants de cuir 4, bottes de cuir 4, casque de cuir 5, tunique matelassée 8, collier
de protection 6, anneau de protection 3.

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

### Ce que `Personnage` et `Escouade` ajoutent

- `Personnage` : membre `m_equipement`, `getEquipement()`, `poserEquipement(Emplacement, Objet)` et
  `retirerEquipement(Emplacement)` (qui rendent l'objet remplacé ou retiré et recalculent l'armure),
  `recalculerArmure()` privée ; constructeur : `const std::vector<ObjetDeDepart>&` à la place de
  l'armure. `Vitalite::definirArmure(int)` (précondition `armure >= 0`).
- `Escouade` : membre `m_sac` (`Inventaire`, 40 cases), `sac()`, `equipementDuPersonnage(place)`,
  `ramasser`, `equiper`, `desequiper` (détail dans `docs/formation.md`, tableau d'`Escouade`).
  Une `place` suit la convention des places (1 à N, `assert` sur sa validité).

| Opération d'`Escouade` | Réussit quand | Effet |
|---|---|---|
| `ramasser(Objet)` | le sac a une case libre | l'objet va dans la première case libre |
| `equiper(place, caseDuSac, emplacement)` | la case est occupée et `peutAller` | l'objet passe à l'emplacement ; l'objet déjà porté prend la case libérée |
| `desequiper(place, emplacement, caseDuSac)` | l'emplacement est occupé, et la case est vide ou contient un objet qui peut aller à cet emplacement | case vide : l'objet y va ; case occupée : échange |
