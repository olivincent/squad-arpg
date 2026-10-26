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

// Les points libres. « Points: 3 » : ce qu'il reste à dépenser à un personnage, brouillon déduit.
[[nodiscard]] std::string ligneDePointsLibres(int points);

// Ce que le brouillon ajoute à une caractéristique, écrit après sa valeur : « (+2) » ; vide s'il n'y a rien en attente.
[[nodiscard]] std::string textePointsEnAttente(int points);

// L'infobulle d'une caractéristique : ce qu'un point fait, avec les nombres lus dans les constantes du jeu (« Might: +3% damage and
// healing per point », « Constitution: +6 health per point ») ; « no effect yet » pour celles qui n'ont encore aucun effet.
[[nodiscard]] std::string ligneDEffetDeLaCaracteristique(Caracteristique caracteristique);
