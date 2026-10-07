// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>
#include <SFML/Graphics/Color.hpp>
#include "Personnage.hpp"

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_mobile(positionDepart + sf::Vector2f(10.0f, 10.0f), {20.0f, 20.0f}, 180.0f),
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
    const sf::Vector2f taille = m_mobile.getTaille();
    sf::RectangleShape forme(taille);
    forme.setPosition(getCentre() - taille / 2.0f);
    forme.setFillColor(sf::Color::Green);
    rendu.draw(forme);

    for (const Personnage& personnage : m_personnages) {
        personnage.dessiner(rendu);
    }
}

void Escouade::allerVers(sf::Vector2f destination, const TileMap& carte) {
    m_mobile.allerVers(destination, carte);
}

int Escouade::attaquer(sf::Vector2f cible, const TileMap& carte) {
    bool tousAPortee = true;
    for (const Personnage& personnage : m_personnages) {
        if (personnage.estVivant() && !personnage.estAPortee(cible)) {
            tousAPortee = false;
            break;
        }
    }

    if (tousAPortee) {
        m_mobile.arreter();
    } else {
        allerVers(cible, carte);
    }

    int degats = 0;
    for (Personnage& personnage : m_personnages) {
        degats += personnage.frapper(cible);
    }
    return degats;
}

void Escouade::mettreAJour(float dt, const TileMap& carte) {
    if (const std::optional<sf::Vector2f> direction = m_mobile.directionVoulue()) {
        m_orientationVoulue = direction->angle();
    }

    m_mobile.mettreAJour(dt, carte);

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
    return m_mobile.getCentre();
}

std::optional<std::size_t> Escouade::indicePersonnageLePlusProche(sf::Vector2f point) const {
    std::optional<std::size_t> indicePlusProche;
    float distancePlusProche = 0.0f;

    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        if (!m_personnages[i].estVivant()) {
            continue;
        }
        const float distance = (m_personnages[i].getCentre() - point).length();
        if (!indicePlusProche.has_value() || distance < distancePlusProche) {
            distancePlusProche = distance;
            indicePlusProche = i;
        }
    }

    return indicePlusProche;
}

std::optional<sf::Vector2f> Escouade::personnageLePlusProche(sf::Vector2f point) const {
    const std::optional<std::size_t> indice = indicePersonnageLePlusProche(point);
    if (!indice.has_value()) {
        return std::nullopt;
    }
    return m_personnages[*indice].getCentre();
}

void Escouade::recevoirDegats(sf::Vector2f origine, int quantite) {
    const std::optional<std::size_t> indice = indicePersonnageLePlusProche(origine);
    if (!indice.has_value()) {
        return;
    }
    m_personnages[*indice].recevoirDegats(quantite);
}

bool Escouade::estVivante() const {
    for (const Personnage& personnage : m_personnages) {
        if (personnage.estVivant()) {
            return true;
        }
    }
    return false;
}
