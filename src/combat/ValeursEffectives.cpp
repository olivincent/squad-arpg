// ./src/combat/ValeursEffectives.cpp
#include "combat/ValeursEffectives.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "combat/Contributions.hpp"

float bonusDAire(const Caracteristiques& caracteristiques) {
    return std::min(apportDesCaracteristiques(StatistiqueAlimentee::BonusDAire, caracteristiques), s_plafondDuBonusDAire);
}

float bonusDeDuree(const Caracteristiques& caracteristiques) {
    return std::min(apportDesCaracteristiques(StatistiqueAlimentee::BonusDeDuree, caracteristiques), s_plafondDuBonusDeDuree);
}

float rayonEffectif(float rayon, float coefficientDAire, const Caracteristiques& caracteristiques) {
    assert(rayon > 0.0f && coefficientDAire >= 0.0f && coefficientDAire <= 1.0f);
    return rayon * std::sqrt(1.0f + coefficientDAire * bonusDAire(caracteristiques));
}

float dureeEffective(float duree, float coefficientDeDuree, const Caracteristiques& caracteristiques) {
    assert(duree >= 0.0f && coefficientDeDuree >= 0.0f && coefficientDeDuree <= 1.0f);
    return duree * (1.0f + coefficientDeDuree * bonusDeDuree(caracteristiques));
}
