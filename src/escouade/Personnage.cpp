// ./src/escouade/Personnage.cpp
#include "escouade/Personnage.hpp"
#include "rendu/BarreDeVie.hpp"
#include "monde/Chemin.hpp"
#include "base/Eclaircir.hpp"
#include "monde/TileMap.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <cassert>
#include <utility>
#include <variant>

namespace {
constexpr float s_seuilRecalcul = 40.0f;  // une case, en pixels
constexpr float s_cote = 20.0f;           // côté du carré, en pixels
constexpr float s_vitesse = 240.0f;       // en pixels par seconde
const sf::Color s_couleurContour(40, 40, 40);       // contour de 1 px vers l'intérieur, pour distinguer deux personnages collés
constexpr float s_epaisseurContour = -1.0f;         // négative : vers l'intérieur
// L'attaque de base profite de la Puissance à plein, d'elle seule.
const Coefficients s_coefficientsDeLAttaqueDeBase = Coefficients::seul(Caracteristique::Puissance, 1.0f);

// La machine à états d'une attaque de base décrite par une arme (ou les mains nues). Elle ne porte que le temps (portée,
// recharge, exécution) : ses dégâts, typés, sont ceux de la description (`attaqueDeLArme`), d'où la puissance à 0.
Skill skillDeLAttaqueDeBase(const DescriptionDAttaque& description) {
    return Skill({.type = TypeSkill::DegatsSurCible, .portee = description.portee, .puissance = 0,
                  .rechargeMin = description.rechargeMin, .rechargeMax = description.rechargeMax,
                  .tempsExecution = description.tempsExecution, .coefficients = s_coefficientsDeLAttaqueDeBase,
                  .nature = description.nature});
}
}  // namespace

Personnage::Personnage(sf::Vector2f centre, DefinitionPersonnage definition, Equipement equipement)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_couleur(definition.couleur),
      m_caracteristiquesDeBase(definition.caracteristiques),
      m_equipement(std::move(equipement)),
      m_equipementActif(m_equipement.actifPour(m_caracteristiquesDeBase)),
      m_descriptionDeBase(m_equipementActif.attaqueDeBase()),
      m_attaqueDeBase(skillDeLAttaqueDeBase(m_descriptionDeBase)),
      m_skills(std::move(definition.skills)),
      m_cibleChemin(),
      m_caracteristiques(m_caracteristiquesDeBase + m_equipementActif.bonusTotal()),
      m_statistiques(statistiquesDuPersonnage(m_caracteristiques, m_equipementActif.bonusDeCombatTotal())),
      m_vitalite(pointsDeVieMax(m_caracteristiques), defenses()),
      m_rayonVision(definition.rayonVision),
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
    dessinerBarreDeVie(rendu, positionBarre, taille.x, m_vitalite.getRatio());
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

    m_vitalite.mettreAJour(dt);
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

Attaque Personnage::getAttaqueDeBase() const {
    return attaqueDeLArme();
}

float Personnage::getZoneDeControle() const {
    return zoneDeControleDe(m_descriptionDeBase.nature);
}

bool Personnage::estAPortee(Cercle cible) const {
    return m_attaqueDeBase.estAPortee(getCercle(), cible);
}

bool Personnage::engagerAttaque(int idCible) {
    if (!estVivant() || m_mobile.estEnMouvement() || m_actionEngagee.has_value() || m_rechargeARelancer ||
        !m_attaqueDeBase.commencer()) {
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
    // engagerAttaque pose toujours une CibleMonstre.
    assert(m_actionEngagee->cible.has_value() && std::holds_alternative<CibleMonstre>(*m_actionEngagee->cible));
    return std::get<CibleMonstre>(*m_actionEngagee->cible).id;  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
}

std::optional<Attaque> Personnage::avancerAttaque(float dt, Generateur& generateur) {
    if (!cibleEngagee().has_value() || !m_attaqueDeBase.avancer(dt, generateur)) {
        return std::nullopt;
    }
    m_actionEngagee.reset();
    return attaqueDeLArme();
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

bool Personnage::armeConvientA(const Skill& skill) const {
    return skill.convientA(m_equipementActif.typeDArme());
}

bool Personnage::armeConvientAuSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    return skill != nullptr && armeConvientA(*skill);
}

bool Personnage::skillEstPret(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    return estVivant() && skill != nullptr && skill->getPhase() == PhaseSkill::Pret && armeConvientA(*skill);
}

std::optional<EtatSkill> Personnage::etatDuSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    if (skill == nullptr) {
        return std::nullopt;
    }
    const std::optional<ActionEngagee> action = skillEngage();
    const bool enActivation = action.has_value() && action->skill == id;
    return EtatSkill{id, skill->getType(), skill->getProgressionRecharge(), skill->getRechargeRestante(), estVivant(),
                     !armeConvientA(*skill), enActivation};
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
    if (!peutEngagerSkill(id) || !trouverSkill(id)->commencer()) {
        return false;
    }
    // Le skill remplace l'attaque de base à ce moment : un coup en exécution est annulé.
    annulerAttaque();
    m_actionEngagee = ActionEngagee{TypeAction::Skill, cible, id};
    return true;
}

bool Personnage::peutEngagerSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    // Un seul skill en activation à la fois, même si les recharges sont indépendantes.
    return estVivant() && skill != nullptr && !skillEngage().has_value() && skill->getPhase() == PhaseSkill::Pret &&
           armeConvientA(*skill);
}

std::optional<ActionEngagee> Personnage::skillEngage() const {
    if (!m_actionEngagee.has_value() || m_actionEngagee->action != TypeAction::Skill) {
        return std::nullopt;
    }
    return m_actionEngagee;
}

std::optional<Attaque> Personnage::avancerSkill(float dt, Generateur& generateur) {
    const std::optional<ActionEngagee> action = skillEngage();
    if (!action.has_value()) {
        return std::nullopt;
    }
    // engagerSkill n'engage qu'un skill que le personnage connaît.
    assert(action->skill.has_value() && trouverSkill(*action->skill) != nullptr);
    Skill& skill = *trouverSkill(*action->skill);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
    if (!skill.avancer(dt, generateur)) {
        return std::nullopt;
    }
    m_actionEngagee.reset();
    // Le skill est parti : l'attaque de base repart pour une recharge complète.
    m_attaqueDeBase.relancerRecharge(generateur);
    if (skill.getBonusDeFrappe().has_value()) {
        return attaqueDeFrappe(skill);
    }
    return attaqueDe(skill);
}

void Personnage::annulerSkill() {
    if (const std::optional<ActionEngagee> action = skillEngage()) {
        // engagerSkill n'engage qu'un skill que le personnage connaît.
        assert(action->skill.has_value() && trouverSkill(*action->skill) != nullptr);
        trouverSkill(*action->skill)->annuler();  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
        m_actionEngagee.reset();
    }
}

ResultatDuCoup Personnage::recevoirDegats(const Attaque& attaque, Generateur& generateur) {
    return m_vitalite.recevoirDegats(attaque, generateur);
}

int Personnage::recevoirSoin(int quantite) {
    return m_vitalite.soigner(quantite);
}

bool Personnage::estVivant() const {
    return m_vitalite.estVivant();
}

int Personnage::getPointsDeVie() const {
    return m_vitalite.getValeur();
}

int Personnage::getPointsDeVieMax() const {
    return m_vitalite.getMaximum();
}

sf::Vector2f Personnage::getCentre() const {
    return m_mobile.getCentre();
}

Cercle Personnage::getCercle() const {
    return m_mobile.getCercle();
}

float Personnage::getRayonVision() const {
    return m_rayonVision;
}

sf::Color Personnage::getCouleur() const {
    return m_couleur;
}

const Equipement& Personnage::getEquipement() const {
    return m_equipement;
}

const Equipement& Personnage::getEquipementActif() const {
    return m_equipementActif;
}

std::optional<Objet> Personnage::poserEquipement(Emplacement emplacement, Objet objet) {
    std::optional<Objet> ancien = m_equipement.poser(emplacement, objet);
    recalculerDepuisLEquipement();
    return ancien;
}

std::optional<Objet> Personnage::retirerEquipement(Emplacement emplacement) {
    std::optional<Objet> retire = m_equipement.retirer(emplacement);
    recalculerDepuisLEquipement();
    return retire;
}

Defenses Personnage::defenses() const {
    return {.armure = m_equipementActif.armureTotale(), .statistiques = m_statistiques};
}

void Personnage::recalculerDepuisLEquipement() {
    m_equipementActif = m_equipement.actifPour(m_caracteristiquesDeBase);
    m_caracteristiques = m_caracteristiquesDeBase + m_equipementActif.bonusTotal();
    m_statistiques = statistiquesDuPersonnage(m_caracteristiques, m_equipementActif.bonusDeCombatTotal());
    m_vitalite.definirMaximum(pointsDeVieMax(m_caracteristiques));
    m_vitalite.definirDefenses(defenses());
    // Un skill en activation qui exige une arme qu'on ne porte plus est annulé, sans recharge consommée.
    if (const std::optional<ActionEngagee> action = skillEngage(); action.has_value() && action->skill.has_value()) {
        const Skill* skill = trouverSkill(*action->skill);
        if (skill != nullptr && !armeConvientA(*skill)) {
            annulerSkill();
        }
    }
    // L'attaque de base : refaite seulement si l'arme l'a changée (un anneau ne remet pas la recharge à zéro).
    if (const DescriptionDAttaque voulue = m_equipementActif.attaqueDeBase(); voulue != m_descriptionDeBase) {
        annulerAttaque();
        m_descriptionDeBase = voulue;
        m_attaqueDeBase = skillDeLAttaqueDeBase(voulue);
        m_rechargeARelancer = true;
    }
}

void Personnage::relancerSiLArmeAChange(Generateur& generateur) {
    if (m_rechargeARelancer) {
        m_attaqueDeBase.relancerRecharge(generateur);
        m_rechargeARelancer = false;
    }
}

Attaque Personnage::attaqueDeLArme() const {
    Attaque attaque = {.degats = {},
                       .capaciteOffensive = m_statistiques.capaciteOffensive,
                       .nature = m_descriptionDeBase.nature,
                       .penetration = m_descriptionDeBase.penetration};
    // La Puissance joue sur chaque portion, arrondie une fois par portion.
    for (const Element element : s_elements) {
        attaque.degats[element] = montantEffectif(m_descriptionDeBase.degats[element], s_coefficientsDeLAttaqueDeBase, m_caracteristiques);
    }
    return attaque;
}

Attaque Personnage::attaqueDe(const Skill& skill) const {
    const int montant = montantEffectif(skill.getPuissance(), skill.getCoefficients(), m_caracteristiques);
    Attaque attaque = {.degats = {},
                       .capaciteOffensive = m_statistiques.capaciteOffensive,
                       .nature = skill.getNature(),
                       .penetration = 0};  // un skill a sa propre puissance : aucune pénétration
    // Le type du skill dit ce que son montant est : des dégâts, un soin ou une durée.
    switch (skill.getType()) {
        case TypeSkill::DegatsSurCible:
        case TypeSkill::Zone:
            attaque.degats = degatsDe(skill.getElement(), montant);
            break;
        case TypeSkill::Soin:
            attaque.soin = montant;
            break;
        case TypeSkill::Provocation:
            attaque.dureeDeProvocation = montant;
            break;
    }
    return attaque;
}

Attaque Personnage::attaqueDeFrappe(const Skill& skill) const {
    assert(skill.getBonusDeFrappe().has_value());
    // Les dégâts viennent de l'arme (toutes ses portions, affixes compris), comme pour l'attaque de base ; le skill n'ajoute
    // que son bonus.
    Attaque attaque = {.degats = {},
                       .capaciteOffensive = m_statistiques.capaciteOffensive,
                       .nature = m_descriptionDeBase.nature,
                       .penetration = m_descriptionDeBase.penetration};
    for (const Element element : s_elements) {
        attaque.degats[element] = montantEffectif(m_descriptionDeBase.degats[element], skill.getCoefficients(), m_caracteristiques);
        attaque.degats[element] += montantEffectif((*skill.getBonusDeFrappe())[element], skill.getCoefficients(), m_caracteristiques);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
    }
    return attaque;
}

const Caracteristiques& Personnage::getCaracteristiques() const {
    return m_caracteristiques;
}

const Caracteristiques& Personnage::getCaracteristiquesDeBase() const {
    return m_caracteristiquesDeBase;
}

const StatistiquesDeCombat& Personnage::getStatistiques() const {
    return m_statistiques;
}
