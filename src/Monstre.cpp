// ./src/Monstre.cpp
#include "Monstre.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

Monstre::Monstre(sf::Vector2f centre)
    : m_centre(centre),
      m_taille(30.0f, 30.0f),
      m_pointsDeVie(100)
{
}

void Monstre::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_centre - m_taille / 2.0f);
    forme.setFillColor(sf::Color::Red);
    rendu.draw(forme);
}

bool Monstre::contient(sf::Vector2f point) const {
    return sf::FloatRect(m_centre - m_taille / 2.0f, m_taille).contains(point);
}
