// ./src/Deplacement.cpp
#include "Deplacement.hpp"
#include "TileMap.hpp"

sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement) {
    sf::Vector2f position = rect.position;

    const sf::Vector2f essaiX = position + sf::Vector2f(deplacement.x, 0.0f);
    if (!carte.chevaucheUnMur(sf::FloatRect(essaiX, rect.size))) {
        position = essaiX;
    }

    const sf::Vector2f essaiY = position + sf::Vector2f(0.0f, deplacement.y);
    if (!carte.chevaucheUnMur(sf::FloatRect(essaiY, rect.size))) {
        position = essaiY;
    }

    return position;
}
