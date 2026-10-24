// ./src/interface/AffichageStatistiques.hpp
#pragma once

#include <string>
#include <vector>

#include "combat/StatistiquesDeCombat.hpp"

// Les textes de l'interface pour les statistiques de combat d'un personnage, en UTF-8 (à passer à
// `sf::String::fromUtf8`). Le noyau (`combat/`) ne connaît que les nombres ; les mots à l'écran sont ici.

// Les lignes du bloc des statistiques de l'inventaire, dans l'ordre où la disposition les remplit :
// « Capacité offensive 100 », « Capacité défensive 100 ».
[[nodiscard]] std::vector<std::string> lignesDeStatistiques(const StatistiquesDeCombat& statistiques);
