// ./src/combat/Caracteristiques.cpp
#include "combat/Caracteristiques.hpp"
#include <cassert>

int pointsDeVieMax(int pointsDeVieDeBase, const Caracteristiques& caracteristiques) {
    const int maximum = pointsDeVieDeBase + s_pvParPointDEndurance * caracteristiques.valeur(Caracteristique::Endurance);
    assert(maximum > 0);
    return maximum;
}
