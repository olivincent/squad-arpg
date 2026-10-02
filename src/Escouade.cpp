// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include "TileMap.hpp"
#include <SFML/Graphics/Rect.hpp>

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_position(positionDepart),
      m_taille(20.0f, 20.0f),
      m_chemin({positionDepart + m_taille / 2.0f}),
      m_indexChemin(0),
      m_vitesse(180.0f)
{
}

void Escouade::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_position);
    forme.setFillColor(sf::Color::Green);
    rendu.draw(forme);
}

void Escouade::allerVers(sf::Vector2f destination) {
    m_chemin = {destination};
    m_indexChemin = 0;
}

void Escouade::mettreAJour(float dt, const TileMap& carte) {
    sf::Vector2f deplacement(0.0f, 0.0f);

    if (m_indexChemin < m_chemin.size()) {
        const sf::Vector2f pointCourant = m_chemin[m_indexChemin];
        const sf::Vector2f direction = pointCourant - getCentre();
        const float distance = direction.length();
        const float pasDuTic = m_vitesse * dt;

        if (distance <= pasDuTic) {
            deplacement = direction;
            ++m_indexChemin;
        } else {
            deplacement = direction.normalized() * pasDuTic;
        }
    }

    const sf::Vector2f essaiX = m_position + sf::Vector2f(deplacement.x, 0.0f);
    if (!carte.chevaucheUnMur(sf::FloatRect(essaiX, m_taille))) {
        m_position = essaiX;
    }

    const sf::Vector2f essaiY = m_position + sf::Vector2f(0.0f, deplacement.y);
    if (!carte.chevaucheUnMur(sf::FloatRect(essaiY, m_taille))) {
        m_position = essaiY;
    }
}

sf::Vector2f Escouade::getCentre() const {
    return m_position + m_taille / 2.0f;
}
