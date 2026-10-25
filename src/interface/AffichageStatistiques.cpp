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
    const std::string blocage = chance > 0 ? "Blocage " + std::to_string(chance) + " % (" + std::to_string(statistiques.montantBloque) + ")"
                                           : "Blocage 0 %";
    return {"Capacité offensive " + std::to_string(statistiques.capaciteOffensive),
            "Capacité défensive " + std::to_string(statistiques.capaciteDefensive),
            blocage,
            "Esquive mêlée " + std::to_string(esquiveEffective(statistiques.esquiveContact)) + " %",
            "Esquive projectile " + std::to_string(esquiveEffective(statistiques.esquiveProjectile)) + " %"};
}

std::vector<std::string> lignesDeBonusDeCombat(const StatistiquesDeCombat& bonus) {
    std::vector<std::string> lignes;
    if (bonus.capaciteOffensive != 0) {
        lignes.push_back("Capacité offensive : " + signe(bonus.capaciteOffensive));
    }
    if (bonus.capaciteDefensive != 0) {
        lignes.push_back("Capacité défensive : " + signe(bonus.capaciteDefensive));
    }
    if (bonus.esquiveContact != 0) {
        lignes.push_back("Esquive contre la mêlée : " + signe(bonus.esquiveContact) + " %");
    }
    if (bonus.esquiveProjectile != 0) {
        lignes.push_back("Esquive contre les projectiles : " + signe(bonus.esquiveProjectile) + " %");
    }
    return lignes;
}
