// ./src/rendu/BarreDeVie.cpp
#include "rendu/BarreDeVie.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace {
constexpr float s_ecartBarreDeVie = 6.0f;  // entre le haut du carré et le bas de la barre
}  // namespace

sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille) {
    return {centre.x - taille.x / 2.0f, centre.y - taille.y / 2.0f - s_ecartBarreDeVie - s_hauteurBarreDeVie};
}

void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio) {
    sf::RectangleShape fond({largeur, s_hauteurBarreDeVie});
    fond.setPosition(position);
    fond.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fond);

    sf::RectangleShape vie({largeur * ratio, s_hauteurBarreDeVie});
    vie.setPosition(position);
    vie.setFillColor(sf::Color::Green);
    rendu.draw(vie);
}
