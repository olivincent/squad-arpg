// ./src/combat/Energie.hpp
#pragma once
#include <cassert>

// L'énergie des personnages (étape 108) : une seule ressource pour toutes les classes, que dépensent les skills de classe et qui se
// régénère au même rythme partout (la `Reserve` de chaque personnage). Aucune caractéristique n'y joue encore (étape 109) ; l'attaque
// de base et les skills des monstres n'en coûtent pas, les monstres n'en ont pas. Toutes les valeurs sont provisoires, à tester.

// Énergie maximum au niveau 1, la même pour toutes les classes.
inline constexpr int s_energieDeBase = 100;

// Énergie maximum que donne chaque niveau au-delà du premier.
inline constexpr int s_energieParNiveau = 5;

// Régénération de base, en points par seconde : la même en combat et hors combat.
inline constexpr float s_regenerationDEnergieDeBase = 2.0f;

// Énergie maximum d'un personnage de ce niveau : `s_energieDeBase` plus `s_energieParNiveau` par niveau au-delà du premier.
// Précondition : niveau >= 1.
[[nodiscard]] constexpr int energieMax(int niveau) {
    assert(niveau >= 1);
    return s_energieDeBase + s_energieParNiveau * (niveau - 1);
}
