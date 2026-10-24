// ./src/combat/Blocage.cpp
#include "combat/Blocage.hpp"
#include <algorithm>
#include <cassert>

namespace {
constexpr float s_cent = 100.0f;
}

float chanceDeBlocage(NatureDAttaque nature, const StatistiquesDeCombat& defenseur, bool blocagePret) {
    if (!blocagePret || defenseur.montantBloque <= 0) {
        return 0.0f;
    }
    // Le `switch` n'a pas de `default` : une nature ajoutée sans règle de blocage fait avertir le compilateur.
    switch (nature) {
        case NatureDAttaque::Contact:
        case NatureDAttaque::Projectile:
            return static_cast<float>(std::clamp(defenseur.chanceDeBlocage, 0, 100)) / s_cent;
        case NatureDAttaque::Zone:
            return 0.0f;
    }
    return 0.0f;
}

ResultatDuBlocage appliquerBlocage(int montant, int montantBloque, float chance, float tirage) {
    assert(montant >= 0);
    if (tirage >= chance) {
        return {.montant = montant, .bloque = 0};
    }
    const int bloque = std::min(montant, montantBloque);
    return {.montant = montant - bloque, .bloque = bloque};
}
