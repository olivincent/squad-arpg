// ./src/IdSkill.hpp
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
    PluieDeFleches
};
