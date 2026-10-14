// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "BarreDeVie.hpp"
#include "Defense.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace {
constexpr float s_seuilRecalcul = 40.0f;  // une case, en pixels
constexpr float s_cote = 20.0f;           // côté du carré, en pixels
constexpr float s_vitesse = 240.0f;       // en pixels par seconde
constexpr int s_pointsDeVieMax = 100;
constexpr float s_hauteurBarreSkill = 3.0f;
}  // namespace

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::optional<Skill> skill, int armure)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_couleur(couleur),
      m_attaqueDeBase(attaqueDeBase),
      m_skill(skill),
      m_cibleChemin(),
      m_pointsDeVie(s_pointsDeVieMax),
      m_armure(armure)
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    const sf::Vector2f taille = m_mobile.getTaille();
    sf::RectangleShape forme(taille);
    forme.setPosition(getCentre() - taille / 2.0f);
    forme.setFillColor(estVivant() ? m_couleur : sf::Color(100, 100, 100));
    rendu.draw(forme);

    if (!estVivant()) {
        return;
    }

    const sf::Vector2f positionBarre = positionBarreDeVie(getCentre(), taille);
    dessinerBarreDeVie(rendu, positionBarre, taille.x, m_pointsDeVie.getRatio());

    if (!m_skill.has_value()) {
        return;
    }

    // Barre de recharge du skill, juste sous la barre de vie : pleine = prêt.
    const sf::Vector2f positionBarreSkill(positionBarre.x, positionBarre.y + s_hauteurBarreDeVie + 1.0f);

    sf::RectangleShape fondSkill({taille.x, s_hauteurBarreSkill});
    fondSkill.setPosition(positionBarreSkill);
    fondSkill.setFillColor(sf::Color(60, 60, 60));
    rendu.draw(fondSkill);

    sf::RectangleShape recharge({taille.x * m_skill->getProgressionRecharge(), s_hauteurBarreSkill});
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
    const sf::Vector2f taille = m_mobile.getTaille();
    if (carte.trajetLibre(sf::FloatRect(centre - taille / 2.0f, taille), cible - centre)) {
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

int Personnage::frapper(Cercle cible, Generateur& generateur) {
    if (!estVivant()) {
        return 0;
    }

    return m_attaqueDeBase.declencher(getCercle(), cible, generateur);
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

std::optional<EtatSkill> Personnage::etatDuSkill() const {
    if (!m_skill.has_value()) {
        return std::nullopt;
    }
    return EtatSkill{m_skill->getType(), m_skill->getProgressionRecharge(), m_skill->getRechargeRestante(), estVivant()};
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

int Personnage::declencherSkill(Cercle cible, Generateur& generateur) {
    if (!m_skill.has_value() || !estVivant()) {
        return 0;
    }

    const int degats = m_skill->declencher(getCercle(), cible, generateur);
    if (degats > 0) {
        // Le skill remplace l'attaque de base à ce moment : elle repart
        // pour une recharge complète.
        m_attaqueDeBase.relancerRecharge(generateur);
    }
    return degats;
}

int Personnage::declencherSkillSansCible(Generateur& generateur) {
    if (!m_skill.has_value() || !estVivant()) {
        return 0;
    }

    const int puissance = m_skill->declencherSansCible(generateur);
    if (puissance > 0) {
        m_attaqueDeBase.relancerRecharge(generateur);
    }
    return puissance;
}

void Personnage::recevoirDegats(int quantite) {
    m_pointsDeVie.subirDegats(appliquerArmure(quantite, m_armure));
}

void Personnage::recevoirSoin(int quantite) {
    m_pointsDeVie.soigner(quantite);
}

bool Personnage::estVivant() const {
    return m_pointsDeVie.estVivant();
}

int Personnage::getPointsDeVie() const {
    return m_pointsDeVie.getValeur();
}

int Personnage::getPointsDeVieMax() const {
    return m_pointsDeVie.getMaximum();
}

sf::Vector2f Personnage::getCentre() const {
    return m_mobile.getCentre();
}

Cercle Personnage::getCercle() const {
    return {getCentre(), m_mobile.getTaille().x / 2.0f};
}
