// ./src/Autodefense.hpp
#pragma once
#include <optional>
#include <vector>
#include "Coup.hpp"

class Personnage;

// Monstre que ce personnage frappe : le monstre verrouillé s'il est à sa
// portée, sinon le plus proche (bord à bord) parmi ceux à sa portée
// (égalité : le premier de la liste), sinon aucun. Renvoie son identifiant.
std::optional<int> choisirMonstreAFrapper(const Personnage& personnage,
                                          const std::vector<MonstreVu>& monstres,
                                          std::optional<int> idVerrouille);
