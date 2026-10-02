// ./src/Chemin.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

class TileMap;

std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee);
