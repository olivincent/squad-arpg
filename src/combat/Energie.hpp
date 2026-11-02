// ./src/combat/Energie.hpp
#pragma once
#include <cassert>
#include "combat/Caracteristiques.hpp"

// L'énergie des personnages (étape 108) : une seule ressource pour toutes les classes, que dépensent les skills de classe et qui se
// régénère au même rythme partout (la `Reserve` de chaque personnage). Son maximum dépend de la Volonté depuis l'étape 110 ; l'attaque
// de base et les skills des monstres n'en coûtent pas, les monstres n'en ont pas. Toutes les valeurs sont provisoires, à tester.

// Énergie maximum au niveau 1 d'un personnage dont la Volonté est à la référence, la même pour toutes les classes.
inline constexpr int s_energieDeBase = 100;

// Énergie maximum que donne chaque niveau de classe au-delà du premier : la valeur par défaut d'une classe
// (`DefinitionPersonnage::energieParNiveauDeClasse`).
inline constexpr int s_energieParNiveau = 5;

// Énergie maximum que donne chaque point de Volonté (une ligne de la table de contributions, `Contributions.hpp`).
inline constexpr int s_energieParPointDeVolonte = 3;

// Régénération de base, en points par seconde : la même en combat et hors combat.
inline constexpr float s_regenerationDEnergieDeBase = 2.0f;

// Régénération d'un personnage dont l'équipement actif donne `bonusEnPourcent` pour cent de régénération (les affixes « of
// Replenishment », additionnés entre objets) : la régénération de base majorée de ce pourcentage, en points par seconde.
// Précondition : bonusEnPourcent >= 0.
[[nodiscard]] constexpr float regenerationDEnergie(int bonusEnPourcent) {
    assert(bonusEnPourcent >= 0);
    return s_regenerationDEnergieDeBase * (1.0f + static_cast<float>(bonusEnPourcent) / 100.0f);
}

// Énergie maximum d'un personnage : `s_energieDeBase` plus l'apport de la Volonté (effective : base de classe plus croissance plus
// équipement ; `apportDesCaracteristiques`) plus `energieParNiveauDeClasse` par niveau de classe au-delà du premier
// (`niveauDeClasse`). Préconditions : niveauDeClasse >= 1, le résultat est > 0.
[[nodiscard]] int energieMax(const Caracteristiques& caracteristiques, int niveauDeClasse, int energieParNiveauDeClasse);
