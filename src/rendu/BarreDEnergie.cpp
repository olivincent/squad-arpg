// ./src/rendu/BarreDEnergie.cpp
#include "rendu/BarreDEnergie.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "rendu/BarreDeVie.hpp"

namespace {
constexpr float s_ecartSousLaBarreDeVie = 1.0f;  // entre le bas de la barre de vie et le haut de la barre d'énergie
}  // namespace

sf::Vector2f positionBarreDEnergie(sf::Vector2f centre, sf::Vector2f taille) {
    const sf::Vector2f barreDeVie = positionBarreDeVie(centre, taille);
    return {barreDeVie.x, barreDeVie.y + s_hauteurBarreDeVie + s_ecartSousLaBarreDeVie};
}

void dessinerBarreDEnergie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio) {
    sf::RectangleShape fond({largeur, s_hauteurBarreDEnergie});
    fond.setPosition(position);
    fond.setFillColor(sf::Color(30, 30, 50));
    rendu.draw(fond);

    sf::RectangleShape energie({largeur * ratio, s_hauteurBarreDEnergie});
    energie.setPosition(position);
    energie.setFillColor(sf::Color(60, 120, 255));
    rendu.draw(energie);
}
