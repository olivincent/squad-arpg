// ./src/Chemin.cpp
#include "Chemin.hpp"

std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee) {
    if (depart == arrivee) {
        return {depart};
    }
    return {};
}
