// ./src/Mobile.cpp
#include "Mobile.hpp"
#include "Chemin.hpp"
#include "Deplacement.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/Rect.hpp>

Mobile::Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)
    : m_position(centre - taille / 2.0f),
      m_taille(taille),
      m_chemin({centre}),
      m_indexChemin(0),
      m_vitesse(vitesse)
{
}

sf::Vector2f Mobile::getCentre() const {
    return m_position + m_taille / 2.0f;
}

sf::Vector2f Mobile::getTaille() const {
    return m_taille;
}

void Mobile::allerVers(sf::Vector2f destination, const TileMap& carte) {
    const sf::Vector2i caseDepart = carte.caseContenant(getCentre());
    const sf::Vector2i caseArrivee = carte.caseContenant(destination);
    const std::vector<sf::Vector2i> chemin = trouverChemin(carte, caseDepart, caseArrivee);

    if (chemin.empty()) {
        return;
    }

    // La case de départ est ignorée : l'escouade est déjà dessus, la
    // viser d'abord la ferait reculer vers le centre de sa propre case.
    // Le dernier point est le point exact cliqué, pas le centre de la
    // case d'arrivée, pour que l'escouade s'arrête précisément dessus.
    std::vector<sf::Vector2f> points;
    for (std::size_t i = 1; i < chemin.size() - 1; ++i) {
        points.push_back(carte.centreDeCase(chemin[i]));
    }
    points.push_back(destination);

    m_chemin = lisserChemin(carte, getCentre(), points, m_taille);
    m_indexChemin = 0;
}

void Mobile::arreter() {
    m_chemin = {getCentre()};
    m_indexChemin = 0;
}

void Mobile::mettreAJour(float dt, const TileMap& carte) {
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

    m_position = deplacerAvecGlissement(carte, sf::FloatRect(m_position, m_taille), deplacement);
}

std::optional<sf::Vector2f> Mobile::directionVoulue() const {
    if (m_indexChemin >= m_chemin.size()) {
        return std::nullopt;
    }

    const sf::Vector2f direction = m_chemin[m_indexChemin] - getCentre();
    if (direction.length() <= 0.0f) {
        return std::nullopt;
    }

    return direction;
}
