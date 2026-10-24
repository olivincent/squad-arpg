// ./src/combat/Esquive.cpp
#include "combat/Esquive.hpp"

namespace {
constexpr float s_cent = 100.0f;
}

float chanceDEsquive(NatureDAttaque nature, const StatistiquesDeCombat& defenseur) {
    // Le `switch` n'a pas de `default` : une nature ajoutée sans règle d'esquive fait avertir le compilateur.
    switch (nature) {
        case NatureDAttaque::Contact: return static_cast<float>(esquiveEffective(defenseur.esquiveContact)) / s_cent;
        case NatureDAttaque::Projectile: return static_cast<float>(esquiveEffective(defenseur.esquiveProjectile)) / s_cent;
        case NatureDAttaque::Zone: return 0.0f;
    }
    return 0.0f;
}

bool coupEsquive(float chance, float tirage) {
    return tirage < chance;
}
