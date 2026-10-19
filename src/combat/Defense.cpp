// ./src/combat/Defense.cpp
#include "combat/Defense.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {
constexpr float s_constanteArmure = 100.0f;
}  // namespace

int appliquerArmure(int montant, int armure) {
    assert(montant >= 0);
    assert(armure >= 0);
    if (montant == 0) {
        return 0;
    }
    // montant * (1 - armure / (armure + K)) = montant * K / (armure + K) :
    // une seule division, donc exacte quand le résultat tombe juste (12,5).
    const float subis = static_cast<float>(montant) * s_constanteArmure
        / (static_cast<float>(armure) + s_constanteArmure);
    return std::max(1, static_cast<int>(std::lround(subis)));
}
