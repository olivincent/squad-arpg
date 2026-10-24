// ./src/combat/Defense.hpp
#pragma once

// Dégâts réellement subis par une entité d'armure `armure` qui reçoit un coup
// de `montant` dégâts.
//
// Modèle M1 (`docs/classes-et-combat.md`, section 8) : l'armure retire un
// montant fixe, mais le coup garde toujours au moins 20 % de sa force :
// subis = max(montant - armure, montant × 20 %), arrondi au plus proche.
// Tout montant > 0 inflige au moins 1 ; un montant de 0 reste 0.
// Préconditions : montant >= 0, armure >= 0.
[[nodiscard]] int appliquerArmure(int montant, int armure);
