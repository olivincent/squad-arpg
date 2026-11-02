// ./src/rendu/MarquesDEffets.cpp
#include "rendu/MarquesDEffets.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace {
constexpr float s_ecartSousLeCarre = 3.0f;  // entre le bas du carré de l'entité et la rangée de marques

void dessinerUneRangee(sf::RenderTarget& rendu, sf::Vector2f debut, int nombre, sf::Color couleur) {
    for (int i = 0; i < nombre; ++i) {
        sf::RectangleShape marque({s_coteDUneMarque, s_coteDUneMarque});
        marque.setPosition({debut.x + static_cast<float>(i) * (s_coteDUneMarque + s_ecartEntreMarques), debut.y});
        marque.setFillColor(couleur);
        rendu.draw(marque);
    }
}
}  // namespace

sf::Vector2f positionMarquesDEffets(sf::Vector2f centre, sf::Vector2f taille) {
    return {centre.x - taille.x / 2.0f, centre.y + taille.y / 2.0f + s_ecartSousLeCarre};
}

void dessinerMarquesDEffets(sf::RenderTarget& rendu, sf::Vector2f position, int bonus, int malus) {
    dessinerUneRangee(rendu, position, bonus, sf::Color(60, 200, 60));
    dessinerUneRangee(rendu, {position.x + static_cast<float>(bonus) * (s_coteDUneMarque + s_ecartEntreMarques), position.y}, malus,
                      sf::Color(220, 50, 50));
}
