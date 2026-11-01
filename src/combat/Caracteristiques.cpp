// ./src/combat/Caracteristiques.cpp
#include "combat/Caracteristiques.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include "combat/Contributions.hpp"

int pointsDeVieMax(const Caracteristiques& caracteristiques, int niveau) {
    assert(niveau >= 1);
    const float apport = apportDesCaracteristiques(StatistiqueAlimentee::PointsDeVieMax, caracteristiques);
    const int maximum = s_pointsDeVieDeReference + static_cast<int>(std::lround(apport)) + s_pvParNiveau * (niveau - 1);
    assert(maximum > 0);
    return maximum;
}

int montantEffectif(int montant, const Coefficients& coefficients, const Caracteristiques& caracteristiques) {
    assert(montant >= 0);
    float bonus = 0.0f;
    for (const Caracteristique caracteristique : s_caracteristiques) {
        bonus += coefficients.valeur(caracteristique) * s_bonusParPoint *
                 static_cast<float>(caracteristiques.valeur(caracteristique) - s_valeurDeReference);
    }
    const float effectif = static_cast<float>(montant) * (1.0f + bonus);
    return std::max(0, static_cast<int>(std::lround(effectif)));
}
