// ./src/combat/StatistiquesDeCombat.cpp
#include "combat/StatistiquesDeCombat.hpp"
#include <algorithm>
#include <cmath>
#include "combat/Contributions.hpp"

namespace {
int capaciteDe(StatistiqueAlimentee statistique, const Caracteristiques& caracteristiques) {
    return s_capaciteDeReference + static_cast<int>(std::lround(apportDesCaracteristiques(statistique, caracteristiques)));
}
}  // namespace

StatistiquesDeCombat statistiquesDuPersonnage(const Caracteristiques& caracteristiques,
                                              const StatistiquesDeCombat& bonusDEquipement) {
    StatistiquesDeCombat statistiques = bonusDEquipement;
    statistiques.capaciteOffensive =
        std::max(0, capaciteDe(StatistiqueAlimentee::CapaciteOffensive, caracteristiques) + bonusDEquipement.capaciteOffensive);
    statistiques.capaciteDefensive =
        std::max(1, capaciteDe(StatistiqueAlimentee::CapaciteDefensive, caracteristiques) + bonusDEquipement.capaciteDefensive);
    for (const EtatDeControle etat : s_etatsDeControle) {
        statistiques.resistancesAuControle[etat] += apportDesCaracteristiques(resistanceAuControle(etat), caracteristiques);
    }
    return statistiques;
}
