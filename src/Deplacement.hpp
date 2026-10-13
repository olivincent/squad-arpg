// ./src/Deplacement.hpp
#pragma once
#include "Cercle.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class TileMap;

[[nodiscard]] bool deplacementBloque(Cercle avant, Cercle apres, const std::vector<Cercle>& obstacles);

[[nodiscard]] sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement,
                                    const std::vector<Cercle>& obstacles);
