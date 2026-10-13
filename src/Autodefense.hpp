// ./src/Autodefense.hpp
#pragma once
#include <functional>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "Cercle.hpp"
#include "MonstreVu.hpp"

// Monstre que frappe celui qui est en `origine` : le monstre verrouillé
// s'il est à sa portée, sinon le plus proche (bord à bord) parmi ceux à sa
// portée (égalité : le premier de la liste), sinon aucun. `estAPortee`
// dit si un cercle est à portée (celle de l'attaque ou celle d'un skill).
// Renvoie l'identifiant du monstre.
[[nodiscard]] std::optional<int> choisirMonstreAFrapper(Cercle origine,
                                          const std::function<bool(Cercle)>& estAPortee,
                                          const std::vector<MonstreVu>& monstres,
                                          std::optional<int> idVerrouille);

// Agresseur vers lequel va un combattant (en `personnage`) dont la place en
// formation est `place` : le plus proche de lui (bord à bord ; égalité : le
// premier de la liste) parmi ceux dont le centre est à `laisse` ou moins de
// `place`, sinon aucun. Renvoie le cercle de l'agresseur.
[[nodiscard]] std::optional<Cercle> choisirAgresseurAViser(Cercle personnage,
                                             sf::Vector2f place,
                                             float laisse,
                                             const std::vector<MonstreVu>& agresseurs);
