// ./src/rendu/TraceDeChemin.cpp
#include "rendu/TraceDeChemin.hpp"
#include <SFML/Graphics/Vertex.hpp>

void dessinerTraceDeChemin(sf::RenderTarget& rendu, sf::Vector2f depart, const std::vector<sf::Vector2f>& points,
                           sf::Color couleur) {
    std::vector<sf::Vertex> sommets;
    sommets.push_back(sf::Vertex{depart, couleur});
    for (const sf::Vector2f& point : points) {
        sommets.push_back(sf::Vertex{point, couleur});
    }
    rendu.draw(sommets.data(), sommets.size(), sf::PrimitiveType::LineStrip);
}
