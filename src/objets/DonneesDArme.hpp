// ./src/objets/DonneesDArme.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"
#include "combat/TypeDArme.hpp"

// Le gabarit d'une arme, porté par une `DefinitionObjet` (pas de sous-classe `Arme` : les armes sont des données). Il fixe
// l'identité de l'arme : sa vitesse, sa portée, sa nature, ses mains, sa pénétration et l'écart de ses coups, qui ne dépendent
// JAMAIS du niveau ni de la qualité de l'objet. Ses dégâts, eux, se calculent depuis l'objet (`descriptionDeLArme`) : le gabarit
// n'en donne que la référence. Une arme se porte en main droite ; à deux mains, elle exige la main gauche vide.
struct DonneesDArme {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    TypeDArme type;
    int mains;                // 1 ou 2
    int degatsDeReference;    // les dégâts moyens, physiques, d'un objet de niveau 1 et de qualité moyenne
    int ecartPourCent;        // la demi-largeur de la plage de chaque coup autour de la moyenne (15 : de 85 % à 115 %)
    float rechargeMin;        // en secondes, tirée au hasard dans [rechargeMin, rechargeMax] à chaque coup
    float rechargeMax;
    float tempsExecution;     // en secondes, immobile entre l'engagement du coup et son départ
    float portee;             // en pixels, bord à bord
    NatureDAttaque nature;    // contact ou projectile ; la zone de contrôle en découle
    int penetration;          // en pour cent de l'armure du défenseur ignorée (0 : aucune)

    bool operator==(const DonneesDArme&) const = default;
};
