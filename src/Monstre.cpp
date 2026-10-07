// ./src/Monstre.cpp
#include "Monstre.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

Monstre::Monstre(sf::Vector2f centre)
    : m_centre(centre),
      m_taille(30.0f, 30.0f),
      m_pointsDeVie(100),
      m_pointsDeVieMax(100)
{
}

void Monstre::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_centre - m_taille / 2.0f);
    forme.setFillColor(sf::Color::Red);
    rendu.draw(forme);

    const float hauteurBarre = 5.0f;
    const float ecart = 6.0f;
    const sf::Vector2f positionBarre(m_centre.x - m_taille.x / 2.0f,
                                      m_centre.y - m_taille.y / 2.0f - ecart - hauteurBarre);

    sf::RectangleShape fond({m_taille.x, hauteurBarre});
    fond.setPosition(positionBarre);
    fond.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fond);

    const float ratio = static_cast<float>(m_pointsDeVie) / static_cast<float>(m_pointsDeVieMax);
    sf::RectangleShape vie({m_taille.x * ratio, hauteurBarre});
    vie.setPosition(positionBarre);
    vie.setFillColor(sf::Color::Green);
    rendu.draw(vie);
}

sf::Vector2f Monstre::getCentre() const {
    return m_centre;
}

bool Monstre::contient(sf::Vector2f point) const {
    return sf::FloatRect(m_centre - m_taille / 2.0f, m_taille).contains(point);
}

void Monstre::recevoirDegats(int quantite) {
    m_pointsDeVie -= quantite;
    if (m_pointsDeVie < 0) {
        m_pointsDeVie = 0;
    }
}

bool Monstre::estVivant() const {
    return m_pointsDeVie > 0;
}
