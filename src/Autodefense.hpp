// ./src/Autodefense.hpp
#pragma once
#include <functional>
#include <optional>
#include <vector>
#include "Cercle.hpp"
#include "Coup.hpp"

// Monstre que frappe celui qui est en `origine` : le monstre verrouillé
// s'il est à sa portée, sinon le plus proche (bord à bord) parmi ceux à sa
// portée (égalité : le premier de la liste), sinon aucun. `estAPortee`
// dit si un cercle est à portée (celle de l'attaque ou celle d'un skill).
// Renvoie l'identifiant du monstre.
std::optional<int> choisirMonstreAFrapper(Cercle origine,
                                          const std::function<bool(Cercle)>& estAPortee,
                                          const std::vector<MonstreVu>& monstres,
                                          std::optional<int> idVerrouille);
