// ./src/Monstre.cpp
#include "Monstre.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

Monstre::Monstre(sf::Vector2f centre, int id)
    : m_mobile(centre, {30.0f, 30.0f}, 120.0f),
      m_pointsDeVie(100),
      m_pointsDeVieMax(100),
      m_id(id),
      m_attaque(TypeSkill::Degats, 15.0f, 10, 1.0f, 0.0f),  // bord à bord : ancienne portée (40) moins les deux rayons (10 + 15)
      m_rayonDetection(200.0f),
      m_rayonTerritoire(400.0f),
      m_etat(Etat::EnGarde),
      m_poste(centre)
{
}

void Monstre::dessiner(sf::RenderTarget& rendu, bool estCible) const {
    const sf::Vector2f centre = m_mobile.getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();

    sf::RectangleShape forme(taille);
    forme.setPosition(centre - taille / 2.0f);
    forme.setFillColor(sf::Color::Red);
    if (estCible) {
        forme.setOutlineColor(sf::Color::Yellow);
        forme.setOutlineThickness(-3.0f);
    }
    rendu.draw(forme);

    const float hauteurBarre = 5.0f;
    const float ecart = 6.0f;
    const sf::Vector2f positionBarre(centre.x - taille.x / 2.0f,
                                      centre.y - taille.y / 2.0f - ecart - hauteurBarre);

    sf::RectangleShape fond({taille.x, hauteurBarre});
    fond.setPosition(positionBarre);
    fond.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fond);

    const float ratio = static_cast<float>(m_pointsDeVie) / static_cast<float>(m_pointsDeVieMax);
    sf::RectangleShape vie({taille.x * ratio, hauteurBarre});
    vie.setPosition(positionBarre);
    vie.setFillColor(sf::Color::Green);
    rendu.draw(vie);
}

sf::Vector2f Monstre::getCentre() const {
    return m_mobile.getCentre();
}

Cercle Monstre::getCercle() const {
    return {m_mobile.getCentre(), m_mobile.getTaille().x / 2.0f};
}

int Monstre::getId() const {
    return m_id;
}

bool Monstre::contient(sf::Vector2f point) const {
    const sf::Vector2f centre = m_mobile.getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();
    return sf::FloatRect(centre - taille / 2.0f, taille).contains(point);
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

bool Monstre::proieDetectee(std::optional<Cercle> cible) const {
    if (!cible) {
        return false;
    }
    const float distanceMonstre = (cible->centre - m_mobile.getCentre()).length();
    const float distancePoste = (cible->centre - m_poste).length();
    return distanceMonstre <= m_rayonDetection && distancePoste <= m_rayonTerritoire;
}

void Monstre::mettreAJour(float dt, std::optional<Cercle> cible, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    m_attaque.mettreAJour(dt);

    const Etat etatPrecedent = m_etat;

    switch (m_etat) {
        case Etat::EnGarde:
            if (proieDetectee(cible)) {
                m_etat = Etat::Poursuite;
            }
            break;
        case Etat::Poursuite:
            if (!cible || (m_mobile.getCentre() - m_poste).length() > m_rayonTerritoire) {
                m_etat = Etat::Retour;
            }
            break;
        case Etat::Retour:
            if (proieDetectee(cible)) {
                m_etat = Etat::Poursuite;
            } else if (!m_mobile.directionVoulue()) {
                m_etat = Etat::EnGarde;
            }
            break;
    }

    if (m_etat == Etat::Retour && etatPrecedent != Etat::Retour) {
        m_mobile.arreter();
        m_mobile.allerVers(m_poste, carte);
    }

    if (m_etat == Etat::Poursuite) {
        if (m_attaque.estAPortee(getCercle(), *cible)) {
            m_mobile.arreter();
        } else {
            m_mobile.allerVers(cible->centre, carte);
        }
    }

    m_mobile.mettreAJour(dt, carte, obstacles);
}

int Monstre::frapper(Cercle cible) {
    return m_attaque.declencher(getCercle(), cible);
}
