// ./src/interface/AffichageStatistiques.cpp
#include "interface/AffichageStatistiques.hpp"

std::vector<std::string> lignesDeStatistiques(const StatistiquesDeCombat& statistiques) {
    return {"Capacité offensive " + std::to_string(statistiques.capaciteOffensive),
            "Capacité défensive " + std::to_string(statistiques.capaciteDefensive)};
}
