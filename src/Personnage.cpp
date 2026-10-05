// ./src/Personnage.cpp
#include "Personnage.hpp"
#include <SFML/Graphics/RectangleShape.hpp>

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur)
    : m_centre(centre),
      m_taille(20.0f, 20.0f),
      m_couleur(couleur)
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_centre - m_taille / 2.0f);
    forme.setFillColor(m_couleur);
    rendu.draw(forme);
}
