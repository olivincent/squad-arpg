// ./src/combat/IdSkill.hpp
#pragma once

// Identifiant STABLE d'un skill : il ne dépend ni de l'ordre des skills dans la
// liste d'un personnage ni de la touche qui les lance (la barre de raccourcis le stocke).
enum class IdSkill {
    FrappePuissante,
    Provocation,
    Soin,
    Explosion,
    FrappeSacree,
    ImpositionDesMains,
    TirPercant,
    PluieDeFleches,
    Benediction,
    Renouvellement,
    Contresort,  // le sort du mage qui coupe les skills d'un monstre (Disruption, étape 113)
    Aegis        // le sort du paladin qui pose un Aegis sur l'allié le plus blessé (étape 114)
};
