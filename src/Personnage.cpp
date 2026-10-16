// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "BarreDeVie.hpp"
#include "Defense.hpp"
#include "Chemin.hpp"
#include "Eclaircir.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace {
constexpr float s_seuilRecalcul = 40.0f;  // une case, en pixels
constexpr float s_cote = 20.0f;           // côté du carré, en pixels
constexpr float s_vitesse = 240.0f;       // en pixels par seconde
constexpr int s_pointsDeVieMax = 100;
constexpr float s_hauteurBarreSkill = 3.0f;
constexpr float s_eclaircissementExecution = 0.5f;  // part de blanc pendant l'exécution d'un coup
const sf::Color s_couleurContour(40, 40, 40);       // contour de 1 px vers l'intérieur, pour distinguer deux personnages collés
constexpr float s_epaisseurContour = -1.0f;         // négative : vers l'intérieur
}  // namespace

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::optional<Skill> skill, int armure, float rayonVision)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_couleur(couleur),
      m_attaqueDeBase(attaqueDeBase),
      m_skill(skill),
      m_cibleChemin(),
      m_pointsDeVie(s_pointsDeVieMax),
      m_armure(armure),
      m_rayonVision(rayonVision),
      m_actionEngagee()
{
}

void Personnage::dessiner(sf::RenderTarget& rendu) const {
    const sf::Vector2f taille = m_mobile.getTaille();
    sf::RectangleShape forme(taille);
    forme.setPosition(getCentre() - taille / 2.0f);
    // Un coup en préparation éclaircit le carré.
    const sf::Color couleur = estEnExecution() ? eclaircir(m_couleur, s_eclaircissementExecution) : m_couleur;
    forme.setFillColor(estVivant() ? couleur : sf::Color(100, 100, 100));
    forme.setOutlineColor(s_couleurContour);
    forme.setOutlineThickness(s_epaisseurContour);
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

void Personnage::mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles,
                             const std::vector<Cercle>& aContourner) {
    if (!estVivant()) {
        return;
    }

    // Règle hybride : en ligne droite quand le trajet est libre de murs et loin des obstacles à
    // contourner (tout chemin en cours est alors abandonné), par un chemin A* sinon.
    const sf::Vector2f centre = getCentre();
    const sf::Vector2f taille = m_mobile.getTaille();
    if (carte.trajetLibre(sf::FloatRect(centre - taille / 2.0f, taille), cible - centre) &&
        !traverseUnObstacle(centre, cible, aContourner, taille.x / 2.0f)) {
        m_mobile.allerToutDroit(cible);
        m_cibleChemin.reset();
    } else if (!m_cibleChemin.has_value() || !m_mobile.directionVoulue().has_value() ||
               (cible - *m_cibleChemin).length() > s_seuilRecalcul || m_mobile.estBloque()) {
        // Chemin à (re)calculer : aucun en cours, ou fini, ou la cible
        // s'est éloignée de sa fin de plus d'une case, ou le personnage est bloqué (allerVers
        // remet le compteur à zéro : au plus un recalcul toutes les 0,3 s). allerToutDroit passe
        // d'abord, pour que le personnage garde la ligne droite si le A*
        // ne trouve aucun chemin (allerVers laisse alors le chemin
        // intact). Même sans chemin trouvé, la cible est mémorisée : on
        // ne relance pas un A* voué à l'échec à chaque image.
        m_mobile.allerToutDroit(cible);
        m_mobile.allerVers(cible, carte, aContourner);
        m_cibleChemin = cible;
    }
    m_mobile.mettreAJour(dt, carte, obstacles);

    m_attaqueDeBase.mettreAJour(dt);
    if (m_skill.has_value()) {
        m_skill->mettreAJour(dt);
    }
}

void Personnage::decaler(sf::Vector2f decalage, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    if (estVivant()) {
        m_mobile.decaler(decalage, carte, obstacles);
    }
}

bool Personnage::estEnMouvement() const {
    return m_mobile.estEnMouvement();
}

std::vector<sf::Vector2f> Personnage::cheminRestant() const {
    return m_mobile.cheminRestant();
}

float Personnage::getPorteeAttaqueDeBase() const {
    return m_attaqueDeBase.getPortee();
}

bool Personnage::estAPortee(Cercle cible) const {
    return m_attaqueDeBase.estAPortee(getCercle(), cible);
}

bool Personnage::engagerAttaque(int idCible) {
    if (!estVivant() || m_mobile.estEnMouvement() || m_actionEngagee.has_value() || !m_attaqueDeBase.commencer()) {
        return false;
    }
    // Seul l'identifiant sert à l'attaque de base : le cercle n'est pas renseigné.
    m_actionEngagee = ActionEngagee{TypeAction::AttaqueDeBase, CibleMonstre{idCible, Cercle{}}};
    return true;
}

std::optional<int> Personnage::cibleEngagee() const {
    if (!m_actionEngagee.has_value() || m_actionEngagee->action != TypeAction::AttaqueDeBase ||
        m_attaqueDeBase.getPhase() != PhaseSkill::EnExecution) {
        return std::nullopt;
    }
    return std::get<CibleMonstre>(*m_actionEngagee->cible).id;
}

int Personnage::avancerAttaque(float dt, Generateur& generateur) {
    if (!cibleEngagee().has_value() || !m_attaqueDeBase.avancer(dt, generateur)) {
        return 0;
    }
    m_actionEngagee.reset();
    return m_attaqueDeBase.getPuissance();
}

void Personnage::annulerAttaque() {
    m_attaqueDeBase.annuler();
    if (m_actionEngagee.has_value() && m_actionEngagee->action == TypeAction::AttaqueDeBase) {
        m_actionEngagee.reset();
    }
}

bool Personnage::estEnExecution() const {
    return m_actionEngagee.has_value();
}

std::optional<TypeSkill> Personnage::typeSkill() const {
    if (!m_skill.has_value()) {
        return std::nullopt;
    }
    return m_skill->getType();
}

bool Personnage::skillEstPret() const {
    return estVivant() && m_skill.has_value() && m_skill->getPhase() == PhaseSkill::Pret;
}

std::optional<EtatSkill> Personnage::etatDuSkill() const {
    if (!m_skill.has_value()) {
        return std::nullopt;
    }
    return EtatSkill{m_skill->getType(), m_skill->getProgressionRecharge(), m_skill->getRechargeRestante(), estVivant(), skillEngage().has_value()};
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

bool Personnage::engagerSkill(std::optional<Cible> cible) {
    if (!estVivant() || !m_skill.has_value() || !m_skill->commencer()) {
        return false;
    }
    // Le skill remplace l'attaque de base à ce moment : un coup en exécution est annulé.
    annulerAttaque();
    m_actionEngagee = ActionEngagee{TypeAction::Skill, cible};
    return true;
}

std::optional<ActionEngagee> Personnage::skillEngage() const {
    if (!m_actionEngagee.has_value() || m_actionEngagee->action != TypeAction::Skill) {
        return std::nullopt;
    }
    return m_actionEngagee;
}

int Personnage::avancerSkill(float dt, Generateur& generateur) {
    if (!skillEngage().has_value() || !m_skill->avancer(dt, generateur)) {
        return 0;
    }
    m_actionEngagee.reset();
    // Le skill est parti : l'attaque de base repart pour une recharge complète.
    m_attaqueDeBase.relancerRecharge(generateur);
    return m_skill->getPuissance();
}

void Personnage::annulerSkill() {
    if (m_skill.has_value()) {
        m_skill->annuler();
    }
    if (skillEngage().has_value()) {
        m_actionEngagee.reset();
    }
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

float Personnage::getRayonVision() const {
    return m_rayonVision;
}
