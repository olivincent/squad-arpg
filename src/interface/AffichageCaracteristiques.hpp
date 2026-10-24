// ./src/interface/AffichageCaracteristiques.hpp
#pragma once

#include <string>

#include "combat/Caracteristiques.hpp"

// Les textes de l'interface pour les caractéristiques, en UTF-8 (à passer à `sf::String::fromUtf8`).
// Le noyau (`combat/`) ne connaît que l'enum ; les mots à l'écran sont ici.

// « Puissance », « Endurance », « Adresse », « Savoir », « Volonté ».
[[nodiscard]] std::string nomDeLaCaracteristique(Caracteristique caracteristique);

// La ligne d'une caractéristique d'un personnage : son nom et sa valeur effective, suivie du bonus de
// l'équipement s'il y en a un (« Puissance 15 (+3) », « Endurance 14 »). Un bonus négatif s'écrit « (-2) ».
[[nodiscard]] std::string ligneDeCaracteristique(Caracteristique caracteristique, int valeurEffective, int bonus);
