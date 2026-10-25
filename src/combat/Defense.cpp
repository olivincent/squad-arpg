// ./src/combat/Defense.cpp
#include "combat/Defense.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {
constexpr float s_plancherArmure = 0.2f;  // part minimale du coup qui passe toujours (provisoire, à tester)
}  // namespace

float armureAppliquee(float montant, int armure, int penetration) {
    assert(montant >= 0.0f);
    assert(armure >= 0);
    // La pénétration réduit l'armure, sans arrondi, avant la soustraction.
    const float partIgnoree = static_cast<float>(std::clamp(penetration, 0, s_penetrationMaximale)) / 100.0f;
    const float armureEffective = static_cast<float>(armure) * (1.0f - partIgnoree);
    // Modèle M1 : l'armure retire un montant fixe, mais au moins le plancher du coup passe.
    return std::max(montant - armureEffective, montant * s_plancherArmure);
}

int appliquerArmure(int montant, int armure, int penetration) {
    assert(montant >= 0);
    assert(armure >= 0);
    if (montant == 0) {
        return 0;
    }
    return std::max(1, static_cast<int>(std::lround(armureAppliquee(static_cast<float>(montant), armure, penetration))));
}

float appliquerResistance(float montant, int resistance) {
    assert(montant >= 0.0f);
    return montant * (1.0f - static_cast<float>(resistanceEffective(resistance)) / 100.0f);
}
