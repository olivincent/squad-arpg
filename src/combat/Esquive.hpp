// ./src/combat/Esquive.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// L'esquive (`docs/classes-et-combat.md`, sections 6 et 7) : première étape du coup après « peut-il agir ».
// Que des fonctions pures ; le tirage est fait ailleurs (`resoudreCoup`) et reçu en paramètre.

// Plafond de l'esquive, en pour cent (question 3 de `docs/classes-et-combat.md`, tranchée pour l'esquive).
inline constexpr int s_esquiveMaximale = 50;

// Une esquive en pour cent telle qu'elle joue : ramenée entre 0 et `s_esquiveMaximale`. Lue par le coup et par
// l'affichage, pour qu'un bonus au-delà du plafond ne s'affiche pas.
[[nodiscard]] constexpr int esquiveEffective(int pourCent) {
    return pourCent < 0 ? 0 : (pourCent > s_esquiveMaximale ? s_esquiveMaximale : pourCent);
}

// La chance (de 0 à 0,5) que ce défenseur esquive une attaque de cette nature : son esquive contre le contact
// ou contre les projectiles, plafonnée. Une zone ne s'esquive jamais (elle s'évite en se déplaçant).
[[nodiscard]] float chanceDEsquive(NatureDAttaque nature, const StatistiquesDeCombat& defenseur);

// Le coup est esquivé : `tirage` (de 0 inclus à 1 exclu) sous la chance.
[[nodiscard]] bool coupEsquive(float chance, float tirage);
