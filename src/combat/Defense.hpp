// ./src/combat/Defense.hpp
#pragma once

// Plafond de la pénétration d'armure, en pour cent (étape 92). Provisoire, à tester.
inline constexpr int s_penetrationMaximale = 75;

// Bornes d'une résistance élémentaire, en pour cent (étape 94) : au plus `s_resistanceMaximale` (jamais
// d'immunité par ce seul nombre), et jusqu'à `s_resistanceMinimale` (une vulnérabilité : le coup est doublé à
// −100 %). Une valeur hors de l'intervalle est ramenée dedans à l'application. Provisoire, à tester.
inline constexpr int s_resistanceMaximale = 80;
inline constexpr int s_resistanceMinimale = -100;

// Dégâts réellement subis par une entité d'armure `armure` qui reçoit un coup
// de `montant` dégâts.
//
// Modèle M1 (`docs/classes-et-combat.md`, section 8) : l'armure retire un
// montant fixe, mais le coup garde toujours au moins 20 % de sa force :
// subis = max(montant - armure, montant × 20 %), arrondi au plus proche.
// Tout montant > 0 inflige au moins 1 ; un montant de 0 reste 0.
// `penetration` (en pour cent, ramenée entre 0 et `s_penetrationMaximale`) réduit l'armure **avant** la
// soustraction : armure effective = armure × (1 − pénétration), sans arrondi intermédiaire (l'arrondi reste celui
// de M1, en fin de calcul).
// Préconditions : montant >= 0, armure >= 0.
[[nodiscard]] int appliquerArmure(int montant, int armure, int penetration = 0);

// L'armure M1 sur un montant flottant, sans aucun arrondi : le moteur du coup l'applique à la portion physique, et
// n'arrondit qu'à la fin (`appliquerArmure` en est la version entière). subis = max(montant − armure effective,
// montant × 20 %) ; un montant de 0 reste 0 (le minimum de 1 du coup se règle sur le total). Mêmes préconditions.
[[nodiscard]] float armureAppliquee(float montant, int armure, int penetration = 0);

// Une résistance en pour cent telle qu'elle joue : ramenée entre `s_resistanceMinimale` et `s_resistanceMaximale`.
[[nodiscard]] constexpr int resistanceEffective(int pourCent) {
    return pourCent < s_resistanceMinimale ? s_resistanceMinimale
                                           : (pourCent > s_resistanceMaximale ? s_resistanceMaximale : pourCent);
}

// Ce qui reste d'une portion après la résistance de son élément : montant × (1 − résistance effective), sans
// arrondi. Une résistance négative (vulnérabilité) augmente la portion. Précondition : montant >= 0.
[[nodiscard]] float appliquerResistance(float montant, int resistance);
