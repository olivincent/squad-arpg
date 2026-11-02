// ./src/combat/Energie.cpp
#include "combat/Energie.hpp"
#include <cmath>
#include "combat/Contributions.hpp"

int energieMax(const Caracteristiques& caracteristiques, int niveauDeClasse, int energieParNiveauDeClasse) {
    assert(niveauDeClasse >= 1);
    const float apport = apportDesCaracteristiques(StatistiqueAlimentee::EnergieMax, caracteristiques);
    const int maximum = s_energieDeBase + static_cast<int>(std::lround(apport)) + energieParNiveauDeClasse * (niveauDeClasse - 1);
    assert(maximum > 0);
    return maximum;
}
