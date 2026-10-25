// ./src/objets/DonneesDArme.hpp
#pragma once
#include "combat/DescriptionDAttaque.hpp"
#include "combat/TypeDArme.hpp"

// Les données d'une arme, portées par une `DefinitionObjet` (pas de sous-classe `Arme` : les armes sont des
// données). Une arme se porte en main droite ; à deux mains, elle exige la main gauche vide.
struct DonneesDArme {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    TypeDArme type;
    int mains;                    // 1 ou 2
    DescriptionDAttaque attaque;  // l'attaque de base que l'arme donne à qui la porte

    bool operator==(const DonneesDArme&) const = default;
};
