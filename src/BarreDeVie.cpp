// ./src/BarreDeVie.cpp
#include "BarreDeVie.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace {
constexpr float s_ecartBarreDeVie = 6.0f;  // entre le haut du carré et le bas de la barre
}  // namespace

sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille) {
    return {centre.x - taille.x / 2.0f, centre.y - taille.y / 2.0f - s_ecartBarreDeVie - s_hauteurBarreDeVie};
}

void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur,
                        int pointsDeVie, int pointsDeVieMax) {
    if (pointsDeVieMax <= 0) {
        return;
    }

    sf::RectangleShape fond({largeur, s_hauteurBarreDeVie});
    fond.setPosition(position);
    fond.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fond);

    const float ratio = static_cast<float>(pointsDeVie) / static_cast<float>(pointsDeVieMax);
    sf::RectangleShape vie({largeur * ratio, s_hauteurBarreDeVie});
    vie.setPosition(position);
    vie.setFillColor(sf::Color::Green);
    rendu.draw(vie);
}
