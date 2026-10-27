// ./src/objets/CatalogueObjets.hpp
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/DescriptionDAttaque.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/CatalogueAffixes.hpp"
#include "objets/DonneesDArme.hpp"
#include "objets/DonneesDeBouclier.hpp"
#include "objets/ObjetsActifs.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"
#include "objets/TypeDArmure.hpp"

// Une caractéristique qu'un objet exige de qui le porte, et de combien le seuil monte à chaque niveau de l'objet. Le seuil
// lui-même se calcule (`seuilsDeLObjet`) : il dépend du niveau de l'objet, que le modèle ne connaît pas.
struct ExigenceDeCaracteristique {
    Caracteristique caracteristique;
    int pente;

    bool operator==(const ExigenceDeCaracteristique&) const = default;
};

// Le seuil de toute exigence au niveau 1 (valeur provisoire, « à tester ») : la même pour tous les objets, qui ne se
// distinguent que par leur pente.
inline constexpr int s_seuilAuNiveau1 = 6;

// Le modèle d'un objet : ce qui est commun à toutes ses instances. Les valeurs de base (armure, bonus, capacités, bouclier) sont
// celles d'un objet de niveau 1 et de qualité moyenne ; ce que porte VRAIMENT un objet se calcule (`effetsDeBase`).
struct DefinitionObjet {
    IdObjet id;
    std::string nom;
    CategorieObjet categorie;
    int armure;
    Caracteristiques bonus = {};  // ajouté aux caractéristiques de qui le porte (vide : aucun bonus)
    // Ajouté aux statistiques de combat de qui le porte : les capacités grandissent avec le niveau et la qualité, les esquives et
    // les résistances sont fixes. Le blocage n'y est pas : c'est `bouclier`.
    StatistiquesDeCombat bonusDeCombat = {};
    std::optional<DonneesDArme> arme = {};    // seulement pour une arme
    std::optional<DonneesDeBouclier> bouclier = {};  // seulement pour un bouclier : son blocage
    std::vector<ExigenceDeCaracteristique> exigences = {};  // vide : aucun seuil (tissu, bijoux)
    int niveauDApparition = 1;  // `genererObjet` ne tire ce modèle que pour un objet d'au moins ce niveau (la profondeur)
    std::optional<TypeDArmure> typeDArmure = {};  // seulement pour une pièce d'armure (torse, jambes, tête, gants, bottes)
    // Les affixes que ce gabarit rend plus probables : une courte liste, tout affixe absent vaut 1 (`affiniteDe`). Les cinq pièces
    // d'un type d'armure ont la liste de leur type.
    std::vector<AffiniteDAffixe> affinites = {};
};

// Ce qu'un objet apporte par lui-même (son modèle, son niveau et sa qualité), sans ses affixes : en entiers, le tout rangé comme
// l'équipement le tient déjà.
struct EffetsDeBase {
    int armure = 0;
    Caracteristiques bonus = {};
    StatistiquesDeCombat bonusDeCombat = {};  // esquives (malus compris), capacités, et le blocage d'un bouclier
};

// Tous les modèles, dans l'ordre de `IdObjet` : l'unique source de leurs valeurs.
[[nodiscard]] const std::vector<DefinitionObjet>& catalogueObjets();

// Le modèle de cet identifiant.
[[nodiscard]] const DefinitionObjet& definitionObjet(IdObjet id);

// Le poids d'un affixe dans le tirage sur ce modèle : son multiplicateur d'affinité, ou 1 sans affinité. L'unique lecture des
// affinités ; le tirage ne l'applique qu'aux affixes que la catégorie permet.
[[nodiscard]] int affiniteDe(IdObjet modele, IdAffixe affixe);

// L'objet respecte-t-il ses invariants ? Une fonction de validation plutôt qu'une fonction de construction avec
// `assert` : `Objet` est un agrégat (on l'écrit en toutes lettres, comme le fait le tirage de `genererObjet`), donc aucune
// construction n'est obligée de passer par une fonction ; la validation, elle, se vérifie à chaque frontière
// (`Equipement::poser`, le sac, les tests de données) et se teste sans `assert`. Un objet est valide quand :
// - son niveau est d'au moins 1 et sa qualité va de 0 à `s_qualiteMaximale` ;
// - chaque affixe est à sa position (un préfixe dans `prefixe`, un suffixe dans `suffixe`), permis pour la catégorie de
//   son modèle (`affixePermis`), et de valeur strictement positive (le calcul des objets actifs suppose des bonus
//   positifs ou nuls) ;
// - le nombre d'affixes correspond à la rareté : commun, aucun ; magique, exactement un (préfixe ou suffixe) ; rare, un
//   préfixe et un suffixe.
[[nodiscard]] bool objetValide(const Objet& objet);

// Le nom affiché de cet objet : « [préfixe] modèle [suffixe] » (« Blazing Mace », « Keen Bow of the Fox »). L'anglais
// n'accorde pas : aucun genre ni nombre à gérer sur les modèles.
[[nodiscard]] std::string nomDeLObjet(const Objet& objet);

// Les seuils de cet objet : pour chaque exigence de son modèle, `s_seuilAuNiveau1 + pente × (niveau − 1)`. UNE règle pour
// tous les objets, l'unique endroit qui la calcule : tout ce qui a besoin d'un seuil (équiper, objets actifs, surbrillance,
// infobulle) passe par cette fonction. Vide pour un objet sans exigence.
[[nodiscard]] std::vector<SeuilDeCaracteristique> seuilsDeLObjet(const Objet& objet);

// Ces caractéristiques atteignent-elles tous les seuils de cet objet ? Toujours vrai sans seuil.
[[nodiscard]] bool seuilsAtteints(const Objet& objet, const Caracteristiques& caracteristiques);

// Cet objet prend-il les deux mains (une arme à deux mains) ?
[[nodiscard]] bool estADeuxMains(IdObjet id);

// L'attaque de base de qui ne porte aucune arme : les mains nues, qui ne sont pas un objet.
[[nodiscard]] const DescriptionDAttaque& mainsNues();

// Les effets de base de cet objet. L'UNIQUE endroit qui les calcule : `Equipement` (armure, bonus, objets actifs), l'infobulle et
// les tests y passent. Rien n'est stocké dans l'objet. Chaque valeur vient du modèle :
// - l'armure, les bonus de caractéristiques, les capacités et le montant absorbé par un bouclier sont `base × facteurDeNiveau ×
//   facteurDeQualite`, arrondis au plus proche (un arrondi par valeur) ;
// - la chance de blocage d'un bouclier est celle du niveau 1 plus `s_gainDeBlocageParNiveau` par niveau au-delà du premier (le
//   niveau seul, pas la qualité) ;
// - les esquives et les résistances du modèle sont fixes : ni le niveau ni la qualité n'y changent rien.
// Les affixes s'ajoutent à part (`apportDesAffixes`).
[[nodiscard]] EffetsDeBase effetsDeBase(const Objet& objet);

// Combien de points de pourcentage de blocage un bouclier gagne à chaque niveau au-delà du premier (« à tester »).
inline constexpr int s_gainDeBlocageParNiveau = 1;

// L'attaque que donne cette arme, sans ses affixes. L'UNIQUE endroit qui la calcule : `Equipement::attaqueDeBase`, l'infobulle
// et les tests y passent. Rien n'est stocké dans l'objet ; tout vient du gabarit de son modèle (`DonneesDArme`) et de son
// niveau et de sa qualité :
// - les dégâts moyens (physiques) sont `degatsDeReference × facteurDeNiveau(niveau) × (0,85 + 0,30 × qualité / 100)` ;
// - la plage de chaque coup va de `moyens × (1 − écart)` à `moyens × (1 + écart)`, chaque borne arrondie au plus proche (un seul
//   arrondi) et d'au moins 1 ;
// - la recharge, l'exécution, la portée, la nature et la pénétration sont celles du gabarit, JAMAIS influencées par le niveau.
// `assert` : l'objet est une arme.
[[nodiscard]] DescriptionDAttaque descriptionDeLArme(const Objet& arme);
