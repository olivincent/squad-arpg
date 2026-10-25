// ./src/objets/CatalogueAffixes.hpp
#pragma once
#include <string>
#include <vector>
#include "objets/Affixe.hpp"
#include "objets/Emplacement.hpp"

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

// Le modèle d'un affixe : ce qui est commun à toutes ses instances. Pas de plage de valeurs ici : elle arrive avec le
// tirage des objets (étape 96) ; la valeur d'un affixe est celle de l'objet qui le porte (`Affixe::valeur`).
struct DefinitionDAffixe {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    IdAffixe id;
    std::string nom;  // le texte affiché, en anglais (« Blazing », « of the Fox »)
    PositionDAffixe position;
    EffetDAffixe effet;
    std::vector<CategorieObjet> categories;  // les catégories d'objet qui peuvent le porter
};

// Tous les affixes, dans l'ordre de `IdAffixe` : l'unique source de leurs données.
[[nodiscard]] const std::vector<DefinitionDAffixe>& catalogueAffixes();

// L'affixe de cet identifiant.
[[nodiscard]] const DefinitionDAffixe& definitionAffixe(IdAffixe id);

// Un objet de cette catégorie peut-il porter cet affixe ?
[[nodiscard]] bool affixePermis(IdAffixe id, CategorieObjet categorie);
