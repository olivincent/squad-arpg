// ./src/Personnage.cpp
#include "Personnage.hpp"
#include <SFML/Graphics/RectangleShape.hpp>

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur)
    : m_centre(centre),
      m_taille(20.0f, 20.0f),
      m_couleur(couleur),
      m_vitesse(240.0f)
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_centre - m_taille / 2.0f);
    forme.setFillColor(m_couleur);
    rendu.draw(forme);
}

void Personnage::mettreAJour(float dt, sf::Vector2f cible) {
    const sf::Vector2f direction = cible - m_centre;
    const float distance = direction.length();
    const float pasDuTic = m_vitesse * dt;

    if (distance <= pasDuTic) {
        m_centre = cible;
    } else {
        m_centre += direction.normalized() * pasDuTic;
    }
}
