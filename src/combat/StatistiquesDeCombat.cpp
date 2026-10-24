// ./src/combat/StatistiquesDeCombat.cpp
#include "combat/StatistiquesDeCombat.hpp"
#include <algorithm>

StatistiquesDeCombat statistiquesDuPersonnage(const Caracteristiques& caracteristiques) {
    const int capacite = s_capaciteDeBase + s_capaciteParPointDAdresse * caracteristiques.valeur(Caracteristique::Adresse);
    return {.capaciteOffensive = std::max(0, capacite), .capaciteDefensive = std::max(1, capacite)};
}
