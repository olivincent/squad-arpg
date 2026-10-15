// ./src/Encerclement.hpp
#pragma once
#include <functional>
#include <optional>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

// Angles candidats des places d'un anneau de rayon `rayon`. Le nombre de
// places est le plus grand n tel que deux places voisines (régulièrement
// réparties) soient à `ecartMin` ou plus l'une de l'autre en ligne droite
// (corde) ; au moins 1. Le premier angle est `depart`, puis on alterne
// gauche/droite : depart, +θ, −θ, +2θ, −2θ… (θ = un tour / n). Les n angles
// sont distincts.
[[nodiscard]] std::vector<sf::Angle> anglesCandidats(sf::Angle depart, float rayon, float ecartMin);

// Point de l'anneau de centre `centre` et de rayon `rayon` à cet angle (0 =
// vers +x ; le sens est celui de `sf::Angle`, donc horaire à l'écran).
[[nodiscard]] sf::Vector2f positionDePlace(sf::Vector2f centre, float rayon, sf::Angle angle);

// Parmi les candidats que `estLibreEtValide` accepte, celui dont l'angle est
// le plus proche de `angleActuel` (écart angulaire par le plus court côté ;
// égalité : le premier des candidats). Vide si aucun n'est accepté.
[[nodiscard]] std::optional<sf::Angle> choisirAngleLibre(const std::vector<sf::Angle>& candidats,
                                                         sf::Angle angleActuel,
                                                         const std::function<bool(sf::Angle)>& estLibreEtValide);
