// ./src/combat/StatistiquesDeCombat.cpp
#include "combat/StatistiquesDeCombat.hpp"
#include <algorithm>

StatistiquesDeCombat statistiquesDuPersonnage(const Caracteristiques& caracteristiques,
                                              const StatistiquesDeCombat& bonusDEquipement) {
    const int capacite = s_capaciteDeBase + s_capaciteParPointDAdresse * caracteristiques.valeur(Caracteristique::Adresse);
    StatistiquesDeCombat statistiques = bonusDEquipement;
    statistiques.capaciteOffensive = std::max(0, capacite + bonusDEquipement.capaciteOffensive);
    statistiques.capaciteDefensive = std::max(1, capacite + bonusDEquipement.capaciteDefensive);
    return statistiques;
}
