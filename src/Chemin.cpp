// ./src/Chemin.cpp
#include "Chemin.hpp"
#include "TileMap.hpp"

std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee) {
    if (carte.getTile(depart.x, depart.y) == Tile::Wall || carte.getTile(arrivee.x, arrivee.y) == Tile::Wall) {
        return {};
    }
    if (depart == arrivee) {
        return {depart};
    }
    return {};
}
