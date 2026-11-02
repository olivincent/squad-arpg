// ./src/combat/ValeursEffectives.hpp
#pragma once
#include "combat/Caracteristiques.hpp"

// Ce que l'Intellect ajoute à l'aire et à la durée des effets d'un skill (étape 110). Le bonus de chaque statistique est la
// contribution des caractéristiques (`apportDesCaracteristiques`, la table de `Contributions.hpp`), plafonnée ; le skill dit
// ensuite, par un coefficient de 0 à 1, combien il en profite (0 : pas du tout, 1 : à plein). Des fonctions pures : le seul appelant
// est `Personnage`, qui calcule les valeurs effectives d'un skill de ses caractéristiques.

// Le bonus d'AIRE (et non de rayon) que donne chaque point d'Intellect au-dessus de la référence : 0,3 %. Provisoire, à tester.
inline constexpr float s_bonusDAireParPoint = 0.003f;

// Le bonus de durée que donne chaque point d'Intellect au-dessus de la référence : 0,3 %. Provisoire, à tester.
inline constexpr float s_bonusDeDureeParPoint = 0.003f;

// Le plafond du bonus TOTAL, toutes sources confondues (aujourd'hui la seule est la caractéristique ; l'équipement et les skills
// s'y additionneront), pour l'aire et pour la durée : +100 %. Provisoire, à tester.
inline constexpr float s_plafondDuBonusDAire = 1.0f;
inline constexpr float s_plafondDuBonusDeDuree = 1.0f;

// Le bonus d'aire que donnent ces caractéristiques, plafonné à `s_plafondDuBonusDAire` (0,25 : +25 % d'aire). Légèrement négatif sous
// la référence.
[[nodiscard]] float bonusDAire(const Caracteristiques& caracteristiques);

// Le bonus de durée que donnent ces caractéristiques, plafonné à `s_plafondDuBonusDeDuree`. Légèrement négatif sous la référence.
[[nodiscard]] float bonusDeDuree(const Caracteristiques& caracteristiques);

// Le rayon d'une zone une fois l'aire majorée : `rayon × √(1 + coefficient × bonus d'aire)`. C'est l'aire (le carré du rayon) qui
// grandit du bonus, pas le rayon : +100 % d'aire fait un rayon de √2 fois le rayon de base. Un coefficient nul rend le rayon de base.
// Précondition : rayon > 0 et coefficient entre 0 et 1.
[[nodiscard]] float rayonEffectif(float rayon, float coefficientDAire, const Caracteristiques& caracteristiques);

// La durée d'un effet une fois majorée : `durée × (1 + coefficient × bonus de durée)`. Un coefficient nul rend la durée de base (la
// convention des dégâts dans le temps, qui gagnent déjà par leur montant). Précondition : durée >= 0 et coefficient entre 0 et 1.
[[nodiscard]] float dureeEffective(float duree, float coefficientDeDuree, const Caracteristiques& caracteristiques);
