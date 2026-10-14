// ./src/Defense.hpp
#pragma once

// Dégâts réellement subis par une entité d'armure `armure` qui reçoit un coup
// de `montant` dégâts.
//
// Rendement décroissant : réduction = armure / (armure + K), avec K = 100,
// donc jamais 100 %. Dégâts subis = montant * (1 - réduction), arrondis au
// plus proche. Tout montant > 0 inflige au moins 1 ; un montant de 0 reste 0.
// Préconditions : montant >= 0, armure >= 0.
[[nodiscard]] int appliquerArmure(int montant, int armure);
