// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace {
constexpr float s_seuilRecalcul = 40.0f;  // une case, en pixels
}  // namespace

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::optional<Skill> skill)
    : m_mobile(centre, {20.0f, 20.0f}, 240.0f),
      m_taille(20.0f, 20.0f),
      m_couleur(couleur),
      m_attaqueDeBase(attaqueDeBase),
      m_skill(skill),
      m_cibleChemin(),
      m_pointsDeVie(100),
      m_pointsDeVieMax(100)
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme(m_taille);
    forme.setPosition(getCentre() - m_taille / 2.0f);
    forme.setFillColor(estVivant() ? m_couleur : sf::Color(100, 100, 100));
    rendu.draw(forme);

    if (!estVivant()) {
        return;
    }

    const float hauteurBarre = 5.0f;
    const float ecart = 6.0f;
    const sf::Vector2f centre = getCentre();
    const sf::Vector2f positionBarre(centre.x - m_taille.x / 2.0f,
                                      centre.y - m_taille.y / 2.0f - ecart - hauteurBarre);

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

    // Règle hybride : en ligne droite quand le trajet est libre (tout
    // chemin en cours est alors abandonné), par un chemin A* sinon.
    const sf::Vector2f centre = getCentre();
    if (carte.trajetLibre(sf::FloatRect(centre - m_taille / 2.0f, m_taille), cible - centre)) {
        m_mobile.allerToutDroit(cible);
        m_cibleChemin.reset();
    } else if (!m_cibleChemin.has_value() || !m_mobile.directionVoulue().has_value() ||
               (cible - *m_cibleChemin).length() > s_seuilRecalcul) {
        // Chemin à (re)calculer : aucun en cours, ou fini, ou la cible
        // s'est éloignée de sa fin de plus d'une case. allerToutDroit passe
        // d'abord, pour que le personnage garde la ligne droite si le A*
        // ne trouve aucun chemin (allerVers laisse alors le chemin
        // intact). Même sans chemin trouvé, la cible est mémorisée : on
        // ne relance pas un A* voué à l'échec à chaque image.
        m_mobile.allerToutDroit(cible);
        m_mobile.allerVers(cible, carte);
        m_cibleChemin = cible;
    }
    m_mobile.mettreAJour(dt, carte, obstacles);

    m_attaqueDeBase.mettreAJour(dt);
    if (m_skill.has_value()) {
        m_skill->mettreAJour(dt);
    }
}

bool Personnage::estAPortee(Cercle cible) const {
    return m_attaqueDeBase.estAPortee(getCercle(), cible);
}

int Personnage::frapper(Cercle cible) {
    if (!estVivant()) {
        return 0;
    }

    return m_attaqueDeBase.declencher(getCercle(), cible);
}

std::optional<TypeSkill> Personnage::typeSkill() const {
    if (!m_skill.has_value()) {
        return std::nullopt;
    }
    return m_skill->getType();
}

bool Personnage::skillEstPret() const {
    return estVivant() && m_skill.has_value() && m_skill->getProgressionRecharge() >= 1.0f;
}

bool Personnage::skillEstAPortee(Cercle cible) const {
    return m_skill.has_value() && m_skill->estAPortee(getCercle(), cible);
}

Cercle Personnage::cercleDePorteeDuSkill() const {
    // La portée se mesure depuis le bord du personnage : rayon du personnage + portée.
    return {getCentre(), getCercle().rayon + (m_skill.has_value() ? m_skill->getPortee() : 0.0f)};
}

Cercle Personnage::zoneDuSkill() const {
    return {getCentre(), m_skill.has_value() ? m_skill->getRayonZone() : 0.0f};
}

int Personnage::declencherSkill(Cercle cible) {
    if (!m_skill.has_value() || !estVivant()) {
        return 0;
    }

    const int degats = m_skill->declencher(getCercle(), cible);
    if (degats > 0) {
        // Le skill remplace l'attaque de base à ce moment : elle repart
        // pour une recharge complète.
        m_attaqueDeBase.relancerRecharge();
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
    return m_mobile.getCentre();
}

Cercle Personnage::getCercle() const {
    return {getCentre(), m_taille.x / 2.0f};
}
