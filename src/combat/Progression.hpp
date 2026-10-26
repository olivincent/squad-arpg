// ./src/combat/Progression.hpp
#pragma once
#include <array>
#include "combat/Caracteristiques.hpp"

// La courbe de l'expérience de l'escouade (`docs/classes-et-combat.md`, progression) : l'expérience cumulée pour atteindre
// le niveau n est `s_experienceDeLaCourbe` × n × (n − 1), donc 0 pour le niveau 1. Pas de niveau maximum. Des fonctions
// pures : le niveau se déduit toujours de l'expérience totale, il n'est jamais stocké.

// Le coefficient k de la courbe (« à tester »). Calibré sur l'expérience moyenne d'un étage (tous les monstres tués) : environ
// niveau 3 à 4 à la fin du 1er étage, niveau 10 à 11 à la fin du 5e (`tests/jeu/test_CalibrageDeLExperience.cpp`).
inline constexpr int s_experienceDeLaCourbe = 21;

// Où en est cette expérience entre son niveau et le suivant, de 0 (juste arrivé) à moins de 1 (le niveau suivant est atteint à
// 1) : pour la barre d'expérience. `assert` : expérience >= 0.
[[nodiscard]] float progressionVersLeNiveauSuivant(int experience);

// L'expérience qui manque pour atteindre le niveau suivant (toujours au moins 1). `assert` : expérience >= 0.
[[nodiscard]] int experienceManquante(int experience);

// L'expérience cumulée qu'il faut pour atteindre ce niveau. `assert` : niveau >= 1.
[[nodiscard]] int experiencePourAtteindre(int niveau);

// La paire de caractéristiques qui grandissent avec le niveau d'une classe, dans l'ordre : la première gagne au niveau 2, la
// seconde au niveau 3, et ainsi de suite en alternance (une donnée de chaque classe, `Specialite`).
using PaireDeCroissance = std::array<Caracteristique, 2>;

// Ce que les niveaux ajoutent à la base d'une classe : +1 point par niveau gagné au-delà du premier, alternativement dans la
// première puis la seconde caractéristique de la paire (le niveau 2 donne la première, le niveau 3 la seconde…). Rien au niveau
// 1. Une fonction pure : la base d'un personnage est toujours sa classe plus cette croissance, jamais une valeur copiée.
// `assert` : niveau >= 1.
[[nodiscard]] Caracteristiques croissanceAuNiveau(int niveau, const PaireDeCroissance& paire);

// Les points libres que reçoit chaque personnage à chaque niveau gagné au-delà du premier (« à tester »). Ils se dépensent à la
// main dans les caractéristiques, pour de bon : la redistribution viendra avec les refuges.
inline constexpr int s_pointsLibresParNiveau = 3;

// Les points libres qu'un personnage a reçus en tout au niveau n : `s_pointsLibresParNiveau` × (n − 1), rien au niveau 1.
// `assert` : niveau >= 1.
[[nodiscard]] int pointsLibresRecus(int niveau);

// Ceux qui lui restent à dépenser : les reçus moins la somme de ce qu'il a déjà dépensé. Jamais stockés : toujours calculés
// depuis le niveau et les points dépensés. `assert` : niveau >= 1.
[[nodiscard]] int pointsLibresDisponibles(int niveau, const Caracteristiques& depenses);

// Cette dépense est-elle permise ? Aucune valeur négative (on ne reprend rien) et pas plus que les points disponibles.
// `depenses` : ce qui est déjà dépensé. `assert` : niveau >= 1.
[[nodiscard]] bool depensePermise(int niveau, const Caracteristiques& depenses, const Caracteristiques& depense);

// Le niveau de qui a cette expérience totale : le plus grand dont `experiencePourAtteindre` est atteinte. `assert` :
// expérience >= 0.
[[nodiscard]] int niveauPour(int experience);
