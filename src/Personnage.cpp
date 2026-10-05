// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "Deplacement.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>

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

void Personnage::mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte) {
    const sf::Vector2f direction = cible - m_centre;
    const float distance = direction.length();
    const float pasDuTic = m_vitesse * dt;
    sf::Vector2f deplacement(0.0f, 0.0f);

    if (distance <= pasDuTic) {
        deplacement = direction;
    } else {
        deplacement = direction.normalized() * pasDuTic;
    }

    const sf::FloatRect rect(m_centre - m_taille / 2.0f, m_taille);
    m_centre = deplacerAvecGlissement(carte, rect, deplacement) + m_taille / 2.0f;
}
