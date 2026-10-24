// ./src/combat/ResolutionDuCoup.cpp
#include "combat/ResolutionDuCoup.hpp"
#include "combat/Defense.hpp"

int resoudreCoup(int montant, const Defenses& defenses) {
    return appliquerArmure(montant, defenses.armure);
}
