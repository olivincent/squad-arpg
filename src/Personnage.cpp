// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "Deplacement.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur, Attaque attaque, std::optional<Skill> skill)
    : m_centre(centre),
      m_taille(20.0f, 20.0f),
      m_couleur(couleur),
      m_vitesse(240.0f),
      m_attaque(attaque),
      m_skill(skill),
      m_pointsDeVie(100),
      m_pointsDeVieMax(100)
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(m_centre - m_taille / 2.0f);
    forme.setFillColor(estVivant() ? m_couleur : sf::Color(100, 100, 100));
    rendu.draw(forme);

    if (!estVivant()) {
        return;
    }

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

    if (!m_skill.has_value()) {
        return;
    }

    // Barre de recharge du skill, juste sous la barre de vie : pleine = prêt.
    const float hauteurBarreSkill = 3.0f;
    const sf::Vector2f positionBarreSkill(positionBarre.x, positionBarre.y + hauteurBarre + 1.0f);

    sf::RectangleShape fondSkill({m_taille.x, hauteurBarreSkill});
    fondSkill.setPosition(positionBarreSkill);
    fondSkill.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fondSkill);

    sf::RectangleShape recharge({m_taille.x * m_skill->getProgressionRecharge(), hauteurBarreSkill});
    recharge.setPosition(positionBarreSkill);
    recharge.setFillColor(sf::Color(255, 165, 0));
    rendu.draw(recharge);
}

void Personnage::mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    if (!estVivant()) {
        return;
    }

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
    m_centre = deplacerAvecGlissement(carte, rect, deplacement, obstacles) +m_taille / 2.0f;

    m_attaque.mettreAJour(dt);
    if (m_skill.has_value()) {
        m_skill->mettreAJour(dt);
    }
}

bool Personnage::estAPortee(Cercle cible) const {
    return m_attaque.estAPortee(getCercle(), cible);
}

int Personnage::frapper(Cercle cible) {
    if (!estVivant()) {
        return 0;
    }

    return m_attaque.frapper(getCercle(), cible);
}

std::optional<TypeSkill> Personnage::typeSkill() const {
    if (!m_skill.has_value()) {
        return std::nullopt;
    }
    return m_skill->getType();
}

int Personnage::declencherSkill(Cercle cible) {
    if (!m_skill.has_value() || !estVivant()) {
        return 0;
    }

    const int degats = m_skill->declencher(getCercle(), cible);
    if (degats > 0) {
        // Le skill remplace l'attaque de base à ce moment : elle repart
        // pour une recharge complète.
        m_attaque.relancerRecharge();
    }
    return degats;
}

void Personnage::recevoirDegats(int quantite) {
    m_pointsDeVie -= quantite;
    if (m_pointsDeVie < 0) {
        m_pointsDeVie = 0;
    }
}

void Personnage::recevoirSoin(int quantite) {
    if (!estVivant()) {
        return;
    }
    m_pointsDeVie += quantite;
    if (m_pointsDeVie > m_pointsDeVieMax) {
        m_pointsDeVie = m_pointsDeVieMax;
    }
}

bool Personnage::estVivant() const {
    return m_pointsDeVie > 0;
}

int Personnage::getPointsDeVie() const {
    return m_pointsDeVie;
}

int Personnage::getPointsDeVieMax() const {
    return m_pointsDeVieMax;
}

sf::Vector2f Personnage::getCentre() const {
    return m_centre;
}

Cercle Personnage::getCercle() const {
    return {m_centre, m_taille.x / 2.0f};
}
