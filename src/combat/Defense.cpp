// ./src/combat/Defense.cpp
#include "combat/Defense.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {
constexpr float s_plancherArmure = 0.2f;  // part minimale du coup qui passe toujours (provisoire, à tester)
}  // namespace

int appliquerArmure(int montant, int armure) {
    assert(montant >= 0);
    assert(armure >= 0);
    if (montant == 0) {
        return 0;
    }
    // Modèle M1 : l'armure retire un montant fixe, mais au moins le plancher du coup passe.
    const float subis = std::max(static_cast<float>(montant - armure), static_cast<float>(montant) * s_plancherArmure);
    return std::max(1, static_cast<int>(std::lround(subis)));
}
