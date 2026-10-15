// ./src/Autodefense.hpp
#pragma once
#include <functional>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "Cercle.hpp"
#include "EntiteVue.hpp"

// Entité que frappe celui qui est en `origine` : l'entité préférée
// (verrouillée, ou proie) si elle est à sa portée, sinon la plus proche
// (bord à bord) parmi celles à sa portée (égalité : la première de la
// liste), sinon aucune. `estAPortee` dit si un cercle est à portée (celle de
// l'attaque ou celle d'un skill). Même règle pour les deux camps. Renvoie
// l'identifiant de l'entité.
[[nodiscard]] std::optional<int> choisirCibleAFrapper(Cercle origine,
                                          const std::function<bool(Cercle)>& estAPortee,
                                          const std::vector<EntiteVue>& monstres,
                                          std::optional<int> idPrefere);

// Agresseur vers lequel va un combattant (en `personnage`) dont la place en
// formation est `place` : le plus proche de lui (bord à bord ; égalité : le
// premier de la liste) parmi ceux dont le centre est à `laisse` ou moins de
// `place`, sinon aucun. Renvoie l'agresseur (identifiant et cercle).
[[nodiscard]] std::optional<EntiteVue> choisirAgresseurAViser(Cercle personnage,
                                             sf::Vector2f place,
                                             float laisse,
                                             const std::vector<EntiteVue>& agresseurs);
