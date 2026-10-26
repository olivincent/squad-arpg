// ./src/combat/Progression.hpp
#pragma once

// La courbe de l'expérience de l'escouade (`docs/classes-et-combat.md`, progression) : l'expérience cumulée pour atteindre
// le niveau n est `s_experienceDeLaCourbe` × n × (n − 1), donc 0 pour le niveau 1. Pas de niveau maximum. Des fonctions
// pures : le niveau se déduit toujours de l'expérience totale, il n'est jamais stocké.

// Le coefficient k de la courbe (« à tester »). Calibré sur l'expérience moyenne d'un étage (tous les monstres tués) : environ
// niveau 3 à 4 à la fin du 1er étage, niveau 10 à 11 à la fin du 5e (`tests/jeu/test_CalibrageDeLExperience.cpp`).
inline constexpr int s_experienceDeLaCourbe = 21;

// L'expérience cumulée qu'il faut pour atteindre ce niveau. `assert` : niveau >= 1.
[[nodiscard]] int experiencePourAtteindre(int niveau);

// Le niveau de qui a cette expérience totale : le plus grand dont `experiencePourAtteindre` est atteinte. `assert` :
// expérience >= 0.
[[nodiscard]] int niveauPour(int experience);
