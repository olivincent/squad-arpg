// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_position(positionDepart),
      m_taille(20.0f, 20.0f),
      m_cible(positionDepart),
      m_vitesse(180.0f)
{
}

void Escouade::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_position);
    forme.setFillColor(sf::Color::Green);
    rendu.draw(forme);
}