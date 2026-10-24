// ./src/combat/StatistiquesDeCombat.hpp
#pragma once
#include "combat/Caracteristiques.hpp"

// Ce que le moteur du coup lit d'une entité (`docs/classes-et-combat.md`, sections 6, 7 et 13). Le moteur
// ne connaît pas les caractéristiques : un personnage en tire ses statistiques (`statistiquesDuPersonnage`,
// seul endroit où elles se calculent), un monstre les reçoit de sa définition. Type valeur ; un champ
// n'existe que quand une étape du coup s'en sert.
struct StatistiquesDeCombat {
    int capaciteOffensive = 0;  // chance et force du critique, contre la capacité défensive du défenseur
    int capaciteDefensive = 0;  // réduit le critique subi

    constexpr bool operator==(const StatistiquesDeCombat&) const = default;
};

// Capacités d'un personnage sans Adresse (celle du point de référence 10 : 100). Provisoire, à tester.
inline constexpr int s_capaciteDeBase = 50;
// Capacité (offensive et défensive) que donne chaque point d'Adresse. Provisoire, à tester.
inline constexpr int s_capaciteParPointDAdresse = 5;

// Les statistiques d'un personnage : `s_capaciteDeBase` plus `s_capaciteParPointDAdresse` par point d'Adresse
// effective (base de classe plus équipement), pour les deux capacités. L'offensive est bornée à 0 par le
// bas ; la défensive à 1, parce que le rapport des capacités la divise.
[[nodiscard]] StatistiquesDeCombat statistiquesDuPersonnage(const Caracteristiques& caracteristiques);
