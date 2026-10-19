// ./src/Personnage.cpp
#include "Personnage.hpp"
#include "BarreDeVie.hpp"
#include "Defense.hpp"
#include "Chemin.hpp"
#include "Eclaircir.hpp"
#include "TileMap.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <cassert>
#include <utility>

namespace {
constexpr float s_seuilRecalcul = 40.0f;  // une case, en pixels
constexpr float s_cote = 20.0f;           // côté du carré, en pixels
constexpr float s_vitesse = 240.0f;       // en pixels par seconde
constexpr int s_pointsDeVieMax = 100;
constexpr float s_eclaircissementExecution = 0.5f;  // part de blanc pendant l'exécution d'un coup
const sf::Color s_couleurContour(40, 40, 40);       // contour de 1 px vers l'intérieur, pour distinguer deux personnages collés
constexpr float s_epaisseurContour = -1.0f;         // négative : vers l'intérieur
}  // namespace

Personnage::Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::vector<SkillConnu> skills, int armure, float rayonVision)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_couleur(couleur),
      m_attaqueDeBase(attaqueDeBase),
      m_skills(std::move(skills)),
      m_cibleChemin(),
      m_pointsDeVie(s_pointsDeVieMax),
      m_armure(armure),
      m_rayonVision(rayonVision),
      m_actionEngagee()
{
    for (std::size_t i = 0; i < m_skills.size(); ++i) {
        for (std::size_t j = i + 1; j < m_skills.size(); ++j) {
            assert(m_skills[i].id != m_skills[j].id);
        }
    }
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
    for (SkillConnu& connu : m_skills) {
        connu.skill.mettreAJour(dt);
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
    m_actionEngagee = ActionEngagee{TypeAction::AttaqueDeBase, CibleMonstre{idCible, Cercle{}}, std::nullopt};
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

const Skill* Personnage::trouverSkill(IdSkill id) const {
    const auto it = std::ranges::find(m_skills, id, &SkillConnu::id);
    return it == m_skills.end() ? nullptr : &it->skill;
}

Skill* Personnage::trouverSkill(IdSkill id) {
    return const_cast<Skill*>(std::as_const(*this).trouverSkill(id));
}

std::optional<TypeSkill> Personnage::typeSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    if (skill == nullptr) {
        return std::nullopt;
    }
    return skill->getType();
}

std::vector<IdSkill> Personnage::idsDesSkills() const {
    std::vector<IdSkill> ids;
    for (const SkillConnu& connu : m_skills) {
        ids.push_back(connu.id);
    }
    return ids;
}

bool Personnage::skillEstPret(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    return estVivant() && skill != nullptr && skill->getPhase() == PhaseSkill::Pret;
}

std::optional<EtatSkill> Personnage::etatDuSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    if (skill == nullptr) {
        return std::nullopt;
    }
    const std::optional<ActionEngagee> action = skillEngage();
    const bool enActivation = action.has_value() && action->skill == id;
    return EtatSkill{id, skill->getType(), skill->getProgressionRecharge(), skill->getRechargeRestante(), estVivant(), enActivation};
}

bool Personnage::skillEstAPortee(IdSkill id, Cercle cible) const {
    const Skill* skill = trouverSkill(id);
    return skill != nullptr && skill->estAPortee(getCercle(), cible);
}

Cercle Personnage::cercleDePorteeDuSkill(IdSkill id) const {
    // La portée se mesure depuis le bord du personnage : rayon du personnage + portée.
    const Skill* skill = trouverSkill(id);
    return {getCentre(), getCercle().rayon + (skill != nullptr ? skill->getPortee() : 0.0f)};
}

Cercle Personnage::zoneDuSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    return {getCentre(), skill != nullptr ? skill->getRayonZone() : 0.0f};
}

bool Personnage::engagerSkill(IdSkill id, std::optional<Cible> cible) {
    Skill* skill = trouverSkill(id);
    // Un seul skill en activation à la fois, même si les recharges sont indépendantes.
    if (!estVivant() || skill == nullptr || skillEngage().has_value() || !skill->commencer()) {
        return false;
    }
    // Le skill remplace l'attaque de base à ce moment : un coup en exécution est annulé.
    annulerAttaque();
    m_actionEngagee = ActionEngagee{TypeAction::Skill, cible, id};
    return true;
}

std::optional<ActionEngagee> Personnage::skillEngage() const {
    if (!m_actionEngagee.has_value() || m_actionEngagee->action != TypeAction::Skill) {
        return std::nullopt;
    }
    return m_actionEngagee;
}

int Personnage::avancerSkill(float dt, Generateur& generateur) {
    if (!skillEngage().has_value()) {
        return 0;
    }
    Skill& skill = *trouverSkill(*m_actionEngagee->skill);
    if (!skill.avancer(dt, generateur)) {
        return 0;
    }
    m_actionEngagee.reset();
    // Le skill est parti : l'attaque de base repart pour une recharge complète.
    m_attaqueDeBase.relancerRecharge(generateur);
    return skill.getPuissance();
}

void Personnage::annulerSkill() {
    if (skillEngage().has_value()) {
        trouverSkill(*m_actionEngagee->skill)->annuler();
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

sf::Color Personnage::getCouleur() const {
    return m_couleur;
}
