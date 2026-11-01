// ./src/monstres/RoleDeMonstre.cpp
#include "monstres/RoleDeMonstre.hpp"
#include "combat/Cible.hpp"

EtatDuRole etatInitialDuRole(const RoleDeMonstre& role) {
    return std::visit(Surcharges{[](const RoleMelee&) -> EtatDuRole { return EtatMelee{}; },
                                 [](const RoleTireur& tireur) -> EtatDuRole { return EtatTireur{.role = tireur}; },
                                 [](const RoleSoigneur&) -> EtatDuRole { return EtatSoigneur{}; }},
                      role);
}
