// ./src/monstres/RoleDeMonstre.cpp
#include "monstres/RoleDeMonstre.hpp"
#include "combat/Cible.hpp"

EtatDuRole etatInitialDuRole(const RoleDeMonstre& role) {
    return std::visit(Surcharges{[](const RoleMelee&) -> EtatDuRole { return EtatMelee{}; },
                                 [](const RoleTireur&) -> EtatDuRole { return EtatTireur{}; },
                                 [](const RoleSoigneur&) -> EtatDuRole { return EtatSoigneur{}; }},
                      role);
}
