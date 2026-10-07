// ./src/Monstre.cpp
#include "Monstre.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

Monstre::Monstre(sf::Vector2f centre, int id)
    : m_mobile(centre, {30.0f, 30.0f}, 120.0f),
      m_pointsDeVie(100),
      m_pointsDeVieMax(100),
      m_id(id),
      m_attaque(80.0f, 10, 1.0f)
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

void Monstre::mettreAJour(float dt) {
    m_attaque.mettreAJour(dt);
}

int Monstre::frapper(sf::Vector2f cible) {
    return m_attaque.frapper(m_mobile.getCentre(), cible);
}
