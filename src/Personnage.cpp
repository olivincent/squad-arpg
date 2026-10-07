// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "Deplacement.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur)
    : m_centre(centre),
      m_taille(20.0f, 20.0f),
      m_couleur(couleur),
      m_vitesse(240.0f),
      m_portee(80.0f),
      m_degats(5),
      m_tempsRecharge(0.5f),
      m_recharge(0.0f),
      m_pointsDeVie(100),
      m_pointsDeVieMax(100)
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_centre - m_taille / 2.0f);
    forme.setFillColor(m_couleur);
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

    m_recharge -= dt;
    if (m_recharge < 0.0f) {
        m_recharge = 0.0f;
    }
}

bool Personnage::estAPortee(sf::Vector2f cible) const {
    return (cible - m_centre).length() <= m_portee;
}

int Personnage::frapper(sf::Vector2f cible) {
    if (!estAPortee(cible) || m_recharge > 0.0f) {
        return 0;
    }

    m_recharge = m_tempsRecharge;
    return m_degats;
}

void Personnage::recevoirDegats(int quantite) {
    m_pointsDeVie -= quantite;
    if (m_pointsDeVie < 0) {
        m_pointsDeVie = 0;
    }
}

bool Personnage::estVivant() const {
    return m_pointsDeVie > 0;
}

sf::Vector2f Personnage::getCentre() const {
    return m_centre;
}
