// ./src/objets/Objet.hpp
#pragma once
#include <optional>
#include "objets/Affixe.hpp"

// Les modèles d'objets du catalogue (`CatalogueObjets.hpp`).
enum class IdObjet {
    CasqueDeFer,
    CuirasseDeMailles,
    JambieresDeFer,
    BouclierRond,
    GantsDeCuir,
    BottesDeCuir,
    CasqueDeCuir,
    TuniqueMatelassee,
    CollierDeProtection,
    AnneauDeProtection,
    AnneauDePuissance,
    CollierDEndurance,
    EpeeLongue,
    Masse,
    Dague,
    EpeeADeuxMains,
    Sceptre,
    Baton,
    Arc
};

// Un objet : l'instance, distincte de son modèle. Petite valeur copiable, sans allocation ; un objet existe à un seul
// endroit à la fois (un emplacement ou une case du sac) et se déplace par valeur. Le type interdit un troisième
// affixe ; que les affixes soient aux bonnes positions, permis pour la catégorie du modèle et en nombre conforme à la
// rareté est un invariant que le type ne peut pas dire : `objetValide`. L'égalité est par valeur (modèle, rareté et
// affixes) : deux objets du même modèle sont égaux s'ils ont la même rareté et les mêmes affixes.
struct Objet {  // NOLINT(cppcoreguidelines-pro-type-member-init): le modèle n'a pas de valeur par défaut exprès, un oubli est signalé par le compilateur
    IdObjet modele;
    Rarete rarete = Rarete::Commun;
    std::optional<Affixe> prefixe = {};
    std::optional<Affixe> suffixe = {};

    bool operator==(const Objet&) const = default;
};
