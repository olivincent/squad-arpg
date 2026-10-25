// ./src/combat/DescriptionDAttaque.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"

// Ce qui décrit une attaque de base : l'arme que porte un personnage (ou ses mains nues) la fournit. C'est aussi le
// point de départ d'un futur skill de frappe (ses dégâts plus un bonus, sa nature, sa pénétration) : un type valeur,
// sans état ; `Skill` en est la machine à états (recharge, exécution).
struct DescriptionDAttaque {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int degats;               // avant l'armure de la cible et les caractéristiques de l'attaquant
    float rechargeMin;        // en secondes, tirée au hasard dans [rechargeMin, rechargeMax] à chaque coup
    float rechargeMax;
    float tempsExecution;     // en secondes, immobile entre l'engagement du coup et son départ
    float portee;             // en pixels, bord à bord
    NatureDAttaque nature;    // contact ou projectile ; la zone de contrôle en découle (un coup de contact engage)

    bool operator==(const DescriptionDAttaque&) const = default;
};
