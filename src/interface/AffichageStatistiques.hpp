// ./src/interface/AffichageStatistiques.hpp
#pragma once

#include <string>
#include <vector>

#include "combat/StatistiquesDeCombat.hpp"

// Les textes de l'interface pour les statistiques de combat, en UTF-8 (à passer à `sf::String::fromUtf8`).
// Le noyau (`combat/`) ne connaît que les nombres ; les mots à l'écran sont ici.

// Les cinq lignes du bloc des statistiques de l'inventaire (valeurs effectives d'un personnage), dans l'ordre
// où la disposition les remplit, colonne par colonne : « Capacité offensive 100 », « Capacité défensive 100 »,
// « Blocage 30 % (40) » (« Blocage 0 % » sans bouclier), puis « Esquive mêlée 5 % » et « Esquive projectile
// 5 % ». Les esquives sont celles qui jouent, plafond compris (`esquiveEffective`).
[[nodiscard]] std::vector<std::string> lignesDeStatistiques(const StatistiquesDeCombat& statistiques);

// Les lignes d'infobulle du bonus de combat d'un objet, une par bonus non nul, dans cet ordre :
// « Capacité offensive : +10 », « Capacité défensive : +10 », « Esquive contre la mêlée : +5 % »,
// « Esquive contre les projectiles : +5 % » et « Blocage : 30 %, 40 points, 3 s ». Vide pour un objet sans bonus.
[[nodiscard]] std::vector<std::string> lignesDeBonusDeCombat(const StatistiquesDeCombat& bonus);
