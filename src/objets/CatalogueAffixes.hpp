// ./src/objets/CatalogueAffixes.hpp
#pragma once
#include <string>
#include <vector>
#include "combat/Caracteristiques.hpp"
#include "combat/Degats.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "objets/Affixe.hpp"
#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Où un affixe se place dans le nom d'un objet, et dans l'objet : un préfixe avant le modèle (« Blazing Mace »), un
// suffixe après (« Bow of the Fox »).
enum class PositionDAffixe { Prefixe, Suffixe };

// Ce qu'un affixe ajoute, avec sa valeur. Une valeur de l'énumération par effet, pas un champ de plus pour dire quelle
// caractéristique : le `switch` qui applique les effets (`apportDesAffixes`) n'a pas de `default`, donc un effet
// ajouté sans règle fait avertir le compilateur.
enum class EffetDAffixe {
    DegatsDeFeu,         // points de feu ajoutés à l'attaque de base de l'arme
    DegatsSacres,        // points de sacré ajoutés à l'attaque de base de l'arme
    PenetrationDArmure,  // pour cent de pénétration, ajoutés à celle de l'arme
    Armure,              // points d'armure
    Puissance,           // un point de caractéristique, par valeur
    Endurance,
    Adresse,
    Savoir,
    Volonte,
    EsquiveContact,      // pour cent d'esquive contre le contact
    EsquiveProjectile,   // pour cent d'esquive contre les projectiles
    ChanceDeBlocage      // pour cent de chance de blocage, ajoutés à ceux du bouclier (le montant reste le sien)
};

// Les valeurs qu'un affixe peut prendre sur un objet tiré : un entier dans [min, max], bornes incluses. Elle ne sert qu'au
// tirage : la valeur d'un affixe est celle de l'objet qui le porte (`Affixe::valeur`), que rééquilibrer la plage ne change pas.
struct PlageDeValeurs {
    int min;
    int max;
};

// Le modèle d'un affixe : ce qui est commun à toutes ses instances.
struct DefinitionDAffixe {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    IdAffixe id;
    std::string nom;  // le texte affiché, en anglais (« Blazing », « of the Fox »)
    PositionDAffixe position;
    EffetDAffixe effet;
    PlageDeValeurs valeurs;                  // les valeurs tirées (même plage sur un objet magique et sur un objet rare)
    std::vector<CategorieObjet> categories;  // les catégories d'objet qui peuvent le porter
};

// Tous les affixes, dans l'ordre de `IdAffixe` : l'unique source de leurs données.
[[nodiscard]] const std::vector<DefinitionDAffixe>& catalogueAffixes();

// L'affixe de cet identifiant.
[[nodiscard]] const DefinitionDAffixe& definitionAffixe(IdAffixe id);

// Un objet de cette catégorie peut-il porter cet affixe ?
[[nodiscard]] bool affixePermis(IdAffixe id, CategorieObjet categorie);

// Les affixes de cette position qu'un objet de cette catégorie peut porter, dans l'ordre du catalogue.
[[nodiscard]] std::vector<IdAffixe> affixesPermisPour(CategorieObjet categorie, PositionDAffixe position);

// Ce que les affixes d'un objet ajoutent, rangé par total que l'équipement tient déjà (`Equipement` y additionne ces
// apports à ceux du modèle : `armureTotale`, `bonusTotal`, `bonusDeCombatTotal`, `attaqueDeBase`, qui restent les seuls
// endroits qui calculent). Rien n'est stocké : recalculé à chaque appel, depuis les valeurs de l'objet.
struct ApportDesAffixes {
    int armure = 0;
    Caracteristiques bonus = {};
    StatistiquesDeCombat bonusDeCombat = {};  // les esquives, et la chance de blocage (le montant reste celui du bouclier)
    Degats degatsAjoutes = {};                // portions ajoutées à l'attaque de base de l'arme (feu, sacré)
    int penetration = 0;                      // pour cent ajoutés à celle de l'arme (le plafond s'applique plus loin)
};

// L'apport des affixes de cet objet (celui du préfixe plus celui du suffixe) ; nul pour un objet commun.
[[nodiscard]] ApportDesAffixes apportDesAffixes(const Objet& objet);
