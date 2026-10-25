// ./src/interface/AffichageStatistiques.cpp
#include "interface/AffichageStatistiques.hpp"
#include <algorithm>
#include "combat/Esquive.hpp"

namespace {
// Un nombre signé : « +5 », « -3 ».
std::string signe(int valeur) {
    return (valeur > 0 ? "+" : "") + std::to_string(valeur);
}
}  // namespace

std::vector<std::string> lignesDeStatistiques(const StatistiquesDeCombat& statistiques) {
    const int chance = statistiques.montantBloque > 0 ? std::clamp(statistiques.chanceDeBlocage, 0, 100) : 0;
    const std::string blocage = chance > 0 ? "Block " + std::to_string(chance) + " % (" + std::to_string(statistiques.montantBloque) + ")"
                                           : "Block 0 %";
    return {"Offensive Ability " + std::to_string(statistiques.capaciteOffensive),
            "Defensive Ability " + std::to_string(statistiques.capaciteDefensive),
            blocage,
            "Melee Dodge " + std::to_string(esquiveEffective(statistiques.esquiveContact)) + " %",
            "Projectile Dodge " + std::to_string(esquiveEffective(statistiques.esquiveProjectile)) + " %"};
}

std::vector<std::string> lignesDeBonusDeCombat(const StatistiquesDeCombat& bonus) {
    std::vector<std::string> lignes;
    if (bonus.capaciteOffensive != 0) {
        lignes.push_back("Offensive Ability: " + signe(bonus.capaciteOffensive));
    }
    if (bonus.capaciteDefensive != 0) {
        lignes.push_back("Defensive Ability: " + signe(bonus.capaciteDefensive));
    }
    if (bonus.esquiveContact != 0) {
        lignes.push_back("Dodge vs Melee: " + signe(bonus.esquiveContact) + " %");
    }
    if (bonus.esquiveProjectile != 0) {
        lignes.push_back("Dodge vs Projectiles: " + signe(bonus.esquiveProjectile) + " %");
    }
    return lignes;
}
