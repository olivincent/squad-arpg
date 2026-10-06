// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>
#include <SFML/Graphics/Color.hpp>
#include "TileMap.hpp"
#include "Chemin.hpp"
#include "Personnage.hpp"
#include "Deplacement.hpp"
#include <SFML/Graphics/Rect.hpp>

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_position(positionDepart),
      m_taille(20.0f, 20.0f),
      m_chemin({positionDepart + m_taille / 2.0f}),
      m_indexChemin(0),
      m_vitesse(180.0f),
      m_decalages({{25.0f, -25.0f}, {25.0f, 25.0f}, {-25.0f, -25.0f}, {-25.0f, 25.0f}}),
      m_orientation(),
      m_orientationVoulue(),
      m_vitesseRotation(360.0f),
      m_personnages({
          Personnage(placePersonnage(0), sf::Color::Red),
          Personnage(placePersonnage(1), sf::Color::Yellow),
          Personnage(placePersonnage(2), sf::Color::Blue),
          Personnage(placePersonnage(3), sf::Color::Magenta),
      })
{
}

sf::Vector2f Escouade::placePersonnage(std::size_t indice) const {
    return getCentre() + m_decalages[indice].rotatedBy(m_orientation);
}

void Escouade::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_position);
    forme.setFillColor(sf::Color::Green);
    rendu.draw(forme);

    for (const Personnage& personnage : m_personnages) {
        personnage.dessiner(rendu);
    }
}

void Escouade::allerVers(sf::Vector2f destination, const TileMap& carte) {
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

void Escouade::mettreAJour(float dt, const TileMap& carte) {
    sf::Vector2f deplacement(0.0f, 0.0f);

    if (m_indexChemin < m_chemin.size()) {
        const sf::Vector2f pointCourant = m_chemin[m_indexChemin];
        const sf::Vector2f direction = pointCourant - getCentre();
        const float distance = direction.length();
        const float pasDuTic = m_vitesse * dt;

        if (distance > 0.0f) {
            m_orientationVoulue = direction.angle();
        }

        if (distance <= pasDuTic) {
            deplacement = direction;
            ++m_indexChemin;
        } else {
            deplacement = direction.normalized() * pasDuTic;
        }
    }

    m_position = deplacerAvecGlissement(carte, sf::FloatRect(m_position, m_taille), deplacement);

    // Rotation progressive vers l'orientation voulue, par le plus court
    // côté (wrapSigned ramène l'écart entre -180° et 180°), à vitesse
    // angulaire bornée, que l'ancre bouge ou non.
    const sf::Angle ecart = (m_orientationVoulue - m_orientation).wrapSigned();
    const float pasRotation = m_vitesseRotation * dt;
    if (std::abs(ecart.asDegrees()) <= pasRotation) {
        m_orientation = m_orientationVoulue;
    } else {
        const float signe = (ecart.asDegrees() > 0.0f) ? 1.0f : -1.0f;
        m_orientation += sf::degrees(pasRotation * signe);
    }

    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        m_personnages[i].mettreAJour(dt, placePersonnage(i), carte);
    }
}

sf::Vector2f Escouade::getCentre() const {
    return m_position + m_taille / 2.0f;
}
