// ./src/TraceDeChemin.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

// Débogage (F3) : ligne brisée fine de `depart` à travers `points`, dans l'ordre.
void dessinerTraceDeChemin(sf::RenderTarget& rendu, sf::Vector2f depart, const std::vector<sf::Vector2f>& points,
                           sf::Color couleur);
