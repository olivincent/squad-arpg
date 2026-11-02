// ./src/escouade/Personnage.cpp
#include "escouade/Personnage.hpp"
#include "combat/Cible.hpp"
#include "combat/Controle.hpp"
#include "combat/Energie.hpp"
#include "combat/ValeursEffectives.hpp"
#include "rendu/BarreDeVie.hpp"
#include "rendu/BarreDEnergie.hpp"
#include "rendu/MarquesDEffets.hpp"
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
// La machine à états d'une attaque de base décrite par une arme (ou les mains nues). Elle ne porte que le temps (portée,
// recharge, exécution) : ses dégâts, typés, sont ceux de la description (`attaqueDeLArme`), d'où la puissance à 0.
Skill skillDeLAttaqueDeBase(const DescriptionDAttaque& description) {
    return Skill({.mode = ModeDeCible::Monstre,
                  .portee = description.portee,
                  .rechargeMin = description.rechargeMin,
                  .rechargeMax = description.rechargeMax,
                  .effets = {EffetDegats{.montant = 0, .nature = description.nature, .coefficients = description.coefficients}},
                  .tempsExecution = description.tempsExecution});
}
}  // namespace

Personnage::Personnage(sf::Vector2f centre, DefinitionPersonnage definition, Equipement equipement, const Caracteristiques& pointsDepenses,
                       int niveau)
    : m_mobile(centre, {s_cote, s_cote}, s_vitesse),
      m_couleur(definition.couleur),
      m_caracteristiquesDeClasse(definition.caracteristiques),
      m_croissance(definition.croissance),
      m_pvParNiveauDeClasse(definition.pvParNiveauDeClasse),
      m_energieParNiveauDeClasse(definition.energieParNiveauDeClasse),
      m_niveau(niveau),
      m_pointsDepenses(pointsDepenses),
      m_caracteristiquesDeBase(baseAuNiveau(niveau)),
      m_equipement(equipement),
      m_equipementActif(m_equipement.actifPour(m_caracteristiquesDeBase)),
      m_descriptionDeBase(m_equipementActif.attaqueDeBase()),
      m_attaqueDeBase(skillDeLAttaqueDeBase(m_descriptionDeBase)),
      m_skills(std::move(definition.skills)),
      m_cibleChemin(),
      m_caracteristiques(m_caracteristiquesDeBase + m_equipementActif.bonusTotal()),
      m_statistiques(statistiquesDuPersonnage(m_caracteristiques, m_equipementActif.bonusDeCombatTotal())),
      m_vitalite(pointsDeVieMax(m_caracteristiques, niveauDeClasse(m_niveau), m_pvParNiveauDeClasse), defenses()),
      m_energie(energieMax(m_caracteristiques, niveauDeClasse(m_niveau), m_energieParNiveauDeClasse), regenerationDEnergie(m_equipementActif.bonusDeRegenerationDEnergie())),
      m_rayonVision(definition.rayonVision),
      m_actionEngagee()
{
    assert(niveau >= 1);
    assert(depensePermise(niveau, Caracteristiques(), pointsDepenses));
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
    dessinerAbsorption(rendu, positionBarre, taille.x, m_vitalite.getRatio(), m_vitalite.getRatioDAbsorption(FamilleDAbsorption::Aegis),
                       m_vitalite.getRatioDAbsorption(FamilleDAbsorption::Ward));
    dessinerBarreDEnergie(rendu, positionBarreDEnergie(getCentre(), taille), taille.x, m_energie.getRatio());
    dessinerMarquesDEffets(rendu, positionMarquesDEffets(getCentre(), taille), m_vitalite.getEffets().pilesDesBonus(),
                           m_vitalite.getEffets().pilesDesMalus(), m_vitalite.getEffets().nombreDeControles());
}

void Personnage::mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles,
                             const std::vector<Cercle>& aContourner) {
    if (!estVivant()) {
        return;
    }

    // Étourdi, il reste sur place (son chemin est abandonné : il en retrouvera un à la fin de l'étourdissement). Sinon, ralenti, il
    // avance moins vite : c'est le temps qui s'écoule pour son déplacement qui est multiplié.
    if (!peutBouger(m_vitalite.getEffets())) {
        m_mobile.arreter();
    } else {
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
        m_mobile.mettreAJour(dt * facteurDeDeplacement(m_vitalite.getEffets()), carte, obstacles);
    }

    m_vitalite.mettreAJour(dt);
    m_energie.regenerer(dt);  // le mort est sorti plus haut : il ne régénère pas
    // Les recharges continuent sous le contrôle ; seule l'entrave ralentit celle de l'attaque de base.
    m_attaqueDeBase.mettreAJour(dt * facteurDAttaque(m_vitalite.getEffets()));
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

Attaque Personnage::getAttaqueDeBase(Generateur& generateur) const {
    return attaqueDeLArme(generateur);
}

float Personnage::getZoneDeControle() const {
    return zoneDeControleDe(m_descriptionDeBase.nature);
}

bool Personnage::estAPortee(Cercle cible) const {
    return m_attaqueDeBase.estAPortee(getCercle(), cible);
}

bool Personnage::engagerAttaque(IdMonstre idCible) {
    if (!estVivant() || !peutAttaquer() || m_mobile.estEnMouvement() || m_actionEngagee.has_value() || m_rechargeARelancer ||
        !m_attaqueDeBase.commencer()) {
        return false;
    }
    // Seul l'identifiant sert à l'attaque de base : le cercle n'est pas renseigné.
    m_actionEngagee = ActionEngagee{TypeAction::AttaqueDeBase, CibleMonstre{idCible, Cercle{}}, std::nullopt};
    return true;
}

std::optional<IdMonstre> Personnage::cibleEngagee() const {
    if (!m_actionEngagee.has_value() || m_actionEngagee->action != TypeAction::AttaqueDeBase ||
        m_attaqueDeBase.getPhase() != PhaseSkill::EnExecution) {
        return std::nullopt;
    }
    // engagerAttaque pose toujours une CibleMonstre.
    assert(m_actionEngagee->cible.has_value() && std::holds_alternative<CibleMonstre>(*m_actionEngagee->cible));
    return std::get<CibleMonstre>(*m_actionEngagee->cible).id;  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
}

std::optional<Attaque> Personnage::avancerAttaque(float dt, Generateur& generateur) {
    // L'entrave allonge l'exécution et la recharge de l'attaque de base : le temps qui s'écoule pour elle est multiplié.
    if (!cibleEngagee().has_value() || !m_attaqueDeBase.avancer(dt * facteurDAttaque(m_vitalite.getEffets()), generateur)) {
        return std::nullopt;
    }
    m_actionEngagee.reset();
    return attaqueDeLArme(generateur);
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

std::optional<ModeDeCible> Personnage::modeSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    if (skill == nullptr) {
        return std::nullopt;
    }
    return skill->getMode();
}

const Skill* Personnage::skillConnu(IdSkill id) const {
    return trouverSkill(id);
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
    return estVivant() && peutUtiliserUnSkill() && skill != nullptr && skill->getPhase() == PhaseSkill::Pret && armeConvientA(*skill);
}

std::optional<EtatSkill> Personnage::etatDuSkill(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    if (skill == nullptr) {
        return std::nullopt;
    }
    const std::optional<ActionEngagee> action = skillEngage();
    const bool enActivation = action.has_value() && action->skill == id;
    return EtatSkill{id, skill->getProgressionRecharge(), skill->getRechargeRestante(), estVivant(),
                     !armeConvientA(*skill), !m_energie.suffit(skill->getCout()), enActivation};
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
    return {getCentre(), skill != nullptr ? rayonEffectifDuSkill(*skill).value_or(0.0f) : 0.0f};
}

std::optional<float> Personnage::rayonEffectifDuSkill(const Skill& skill) const {
    const std::optional<float> rayon = skill.getRayonZone();
    if (!rayon.has_value()) {
        return std::nullopt;
    }
    return rayonEffectif(*rayon, skill.getCoefficientDAire(), m_caracteristiques);
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
    return estVivant() && peutUtiliserUnSkill() && skill != nullptr && !skillEngage().has_value() &&
           skill->getPhase() == PhaseSkill::Pret && armeConvientA(*skill) && m_energie.suffit(skill->getCout());
}

bool Personnage::energieSuffit(IdSkill id) const {
    const Skill* skill = trouverSkill(id);
    return skill != nullptr && m_energie.suffit(skill->getCout());
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
    // Le skill est parti : son coût est payé (l'énergie a été vérifiée à l'engagement, et seul un skill à la fois s'active, donc rien
    // n'a pu la dépenser depuis) et l'attaque de base repart pour une recharge complète.
    m_energie.payer(skill.getCout());
    m_attaqueDeBase.relancerRecharge(generateur);
    return attaqueDuSkill(skill, *action->skill, generateur);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
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
    const ResultatDuCoup resultat = m_vitalite.recevoirDegats(attaque, generateur);
    appliquerLeControle();
    return resultat;
}

int Personnage::recevoirSoin(int quantite) {
    return m_vitalite.soigner(quantite);
}

bool Personnage::estVivant() const {
    return m_vitalite.estVivant();
}

void Personnage::poserEffet(const EffetActif& effet) {
    m_vitalite.poserEffet(effet);
    appliquerLeControle();
}

bool Personnage::peutAttaquer() const {
    return ::peutAttaquer(m_vitalite.getEffets());
}

bool Personnage::peutUtiliserUnSkill() const {
    return ::peutUtiliserUnSkill(m_vitalite.getEffets());
}

std::vector<ControleApplique> Personnage::prendreLesControles() {
    return m_vitalite.prendreLesControles();
}

void Personnage::appliquerLeControle() {
    if (!peutAttaquer()) {
        annulerAttaque();
    }
    if (!peutUtiliserUnSkill()) {
        annulerSkill();
    }
}

const EffetsActifs& Personnage::getEffets() const {
    return m_vitalite.getEffets();
}

std::vector<Tick> Personnage::prendreLesTicks() {
    return m_vitalite.prendreLesTicks();
}

int Personnage::getArmure() const {
    return m_vitalite.getArmure();
}

int Personnage::getPointsDeVie() const {
    return m_vitalite.getValeur();
}

int Personnage::getPointsDeVieMax() const {
    return m_vitalite.getMaximum();
}

int Personnage::getEnergie() const {
    return m_energie.getValeur();
}

int Personnage::getEnergieMax() const {
    return m_energie.getMaximum();
}

float Personnage::getRegenerationDEnergie() const {
    return m_energie.getRegeneration();
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

Caracteristiques Personnage::baseAuNiveau(int niveau) const {
    return m_caracteristiquesDeClasse + croissanceAuNiveau(niveauDeClasse(niveau), m_croissance) + m_pointsDepenses;
}

const Caracteristiques& Personnage::getPointsDepenses() const {
    return m_pointsDepenses;
}

int Personnage::getPointsLibres() const {
    return pointsLibresDisponibles(m_niveau, m_pointsDepenses);
}

bool Personnage::depenserDesPoints(const Caracteristiques& depense) {
    if (!depensePermise(m_niveau, m_pointsDepenses, depense)) {
        return false;
    }
    m_pointsDepenses += depense;
    m_caracteristiquesDeBase = baseAuNiveau(m_niveau);
    recalculerDepuisLEquipement();
    return true;
}

int Personnage::getNiveau() const {
    return m_niveau;
}

void Personnage::monterAuNiveau(int niveau) {
    assert(niveau >= m_niveau);
    m_niveau = niveau;
    m_caracteristiquesDeBase = baseAuNiveau(niveau);
    recalculerDepuisLEquipement();
}

void Personnage::recalculerDepuisLEquipement() {
    m_equipementActif = m_equipement.actifPour(m_caracteristiquesDeBase);
    m_caracteristiques = m_caracteristiquesDeBase + m_equipementActif.bonusTotal();
    m_statistiques = statistiquesDuPersonnage(m_caracteristiques, m_equipementActif.bonusDeCombatTotal());
    m_vitalite.definirMaximum(pointsDeVieMax(m_caracteristiques, niveauDeClasse(m_niveau), m_pvParNiveauDeClasse));
    m_vitalite.definirDefenses(defenses());
    const int energieMaximum = energieMax(m_caracteristiques, niveauDeClasse(m_niveau), m_energieParNiveauDeClasse);
    if (estVivant()) {
        m_energie.changerMaximum(energieMaximum);
    } else {
        m_energie.definirMaximum(energieMaximum);
    }
    m_energie.definirRegeneration(regenerationDEnergie(m_equipementActif.bonusDeRegenerationDEnergie()));
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

Attaque Personnage::attaqueDeLArme(Generateur& generateur) const {
    Attaque attaque = {.degats = {},
                       .capaciteOffensive = m_statistiques.capaciteOffensive,
                       .nature = m_descriptionDeBase.nature,
                       .penetration = m_descriptionDeBase.penetration};
    // Le coup tire ses dégâts dans la plage de l'arme, puis les caractéristiques que l'arme fait jouer (la Puissance, ou le Savoir
    // pour une arme de lanceur) agissent sur chaque portion, arrondie une fois par portion.
    const Degats tires = tirerLesDegats(generateur, m_descriptionDeBase.degats);
    for (const Element element : s_elements) {
        attaque.degats[element] = montantEffectif(tires[element], m_descriptionDeBase.coefficients, m_caracteristiques);
    }
    return attaque;
}

Attaque Personnage::attaqueDuSkill(const Skill& skill, IdSkill id, Generateur& generateur) const {
    const auto attaqueSansDegats = [&] {
        return Attaque{.degats = {}, .capaciteOffensive = m_statistiques.capaciteOffensive, .nature = NatureDAttaque::Contact};
    };
    Attaque duSkill = std::visit(
        Surcharges{
            [&](const EffetDegats& degats) {
                Attaque attaque = {.degats = {},
                                   .capaciteOffensive = m_statistiques.capaciteOffensive,
                                   .nature = degats.nature,
                                   .penetration = 0};  // un skill a sa propre puissance : aucune pénétration
                attaque.degats = degatsDe(degats.element, montantEffectif(degats.montant, degats.coefficients, m_caracteristiques));
                return attaque;
            },
            [&](const EffetFrappe& frappe) { return attaqueDeFrappe(frappe, generateur); },
            [&](const EffetSoin& soin) {
                // La nature d'une attaque ne sert pas à un soin : le contact, comme avant l'effet en données.
                Attaque attaque = {.degats = {}, .capaciteOffensive = m_statistiques.capaciteOffensive, .nature = NatureDAttaque::Contact};
                attaque.soin = montantEffectif(soin.montant, soin.coefficients, m_caracteristiques);
                return attaque;
            },
            [&](const EffetProvocation& provocation) {
                Attaque attaque = {.degats = {}, .capaciteOffensive = m_statistiques.capaciteOffensive, .nature = NatureDAttaque::Contact};
                // Une durée n'est pas un montant : seul le bonus de durée de l'Intellect y joue, selon le coefficient de l'effet.
                attaque.dureeDeProvocation = dureeEffective(static_cast<float>(provocation.duree), provocation.coefficientDeDuree, m_caracteristiques);
                return attaque;
            },
            // Un modificateur ou un effet dans le temps n'a ni dégâts ni soin : tout ce qu'il produit est dans les effets actifs, ajoutés
            // plus bas (des dégâts dans le temps n'est jamais l'effet principal : le constructeur de `Skill` l'exige).
            [&](const EffetModificateur&) { return attaqueSansDegats(); },
            [&](const EffetDegatsDansLeTemps&) { return attaqueSansDegats(); },
            [&](const EffetSoinDansLeTemps&) { return attaqueSansDegats(); },
            [&](const EffetControle&) { return attaqueSansDegats(); },
            [&](const EffetAbsorption&) { return attaqueSansDegats(); },
        },
        skill.getEffet());
    duSkill.effets = effetsActifsDuSkill(skill, id);
    return duSkill;
}

std::vector<EffetActif> Personnage::effetsActifsDuSkill(const Skill& skill, IdSkill id) const {
    std::vector<EffetActif> effets;
    for (std::size_t rang = 0; rang < skill.getEffets().size(); ++rang) {
        // Les sortes d'effet actif de plus (étape 114) s'ajouteront ici. Un effet de skill qui n'en pose pas (dégâts, frappe, soin,
        // provocation) ne produit rien.
        std::visit(
            Surcharges{
                [&](const EffetModificateur& modificateur) {
                    float duree = dureeEffective(static_cast<float>(modificateur.duree), modificateur.coefficientDeDuree, m_caracteristiques);
                    if (modificateur.pourcentage >= 0) {
                        duree = std::min(duree, skill.getRechargeMin());  // un buff ne dure jamais plus longtemps que sa recharge
                    }
                    effets.push_back({.source = id,
                                      .rang = static_cast<int>(rang),
                                      .dureeRestante = duree,
                                      .sorte = ModificateurDeStatistique{.statistique = modificateur.statistique, .pourcentage = modificateur.pourcentage}});
                },
                // Le montant par tick est figé ici, à la pose : les caractéristiques de ce moment-là, pas celles du tick.
                [&](const EffetDegatsDansLeTemps& degats) {
                    effets.push_back({.source = id,
                                      .rang = static_cast<int>(rang),
                                      .dureeRestante = dureeEffective(static_cast<float>(degats.duree), degats.coefficientDeDuree, m_caracteristiques),
                                      .sorte = DegatsDansLeTemps{.element = degats.element,
                                                                 .montant = montantEffectif(degats.montant, degats.coefficients, m_caracteristiques)},
                                      .pilesMax = degats.pilesMax});
                },
                [&](const EffetSoinDansLeTemps& soin) {
                    // Un soin dans le temps est un bonus pour ses alliés : comme un buff, il ne dure jamais plus longtemps que la recharge.
                    const float duree = std::min(dureeEffective(static_cast<float>(soin.duree), soin.coefficientDeDuree, m_caracteristiques),
                                                 skill.getRechargeMin());
                    effets.push_back({.source = id,
                                      .rang = static_cast<int>(rang),
                                      .dureeRestante = duree,
                                      .sorte = SoinDansLeTemps{.montant = montantEffectif(soin.montant, soin.coefficients, m_caracteristiques)},
                                      .pilesMax = soin.pilesMax});
                },
                // Un contrôle : la durée du skill, majorée par l'Intellect selon le coefficient du skill ; la résistance de la cible et le
                // rendement décroissant jouent à la pose, chez elle (`Vitalite::poserEffet`).
                [&](const EffetControle& controle) {
                    effets.push_back({.source = id,
                                      .rang = static_cast<int>(rang),
                                      .dureeRestante = dureeEffective(controle.duree, controle.coefficientDeDuree, m_caracteristiques),
                                      .sorte = Controle{.etat = controle.etat, .pourcentage = controle.pourcentage}});
                },
                // Un bouclier est un buff : sa durée profite de l'Intellect selon le coefficient du skill, mais ne dépasse jamais la recharge ;
                // ses points sont figés ici, à la pose, avec les caractéristiques du lanceur (comme un soin).
                [&](const EffetAbsorption& absorption) {
                    const float duree = std::min(dureeEffective(static_cast<float>(absorption.duree), absorption.coefficientDeDuree, m_caracteristiques),
                                                 skill.getRechargeMin());
                    effets.push_back({.source = id,
                                      .rang = static_cast<int>(rang),
                                      .dureeRestante = duree,
                                      .sorte = Absorption{.famille = absorption.famille,
                                                          .points = montantEffectif(absorption.montant, absorption.coefficients, m_caracteristiques)}});
                },
                [](const EffetDegats&) {},
                [](const EffetFrappe&) {},
                [](const EffetSoin&) {},
                [](const EffetProvocation&) {},
            },
            skill.getEffets()[rang]);
    }
    return effets;
}

Attaque Personnage::attaqueDeFrappe(const EffetFrappe& frappe, Generateur& generateur) const {
    // Les dégâts viennent de l'arme (toutes ses portions, affixes compris, tirées dans sa plage), comme pour l'attaque de
    // base ; le skill n'ajoute que son bonus.
    Attaque attaque = {.degats = {},
                       .capaciteOffensive = m_statistiques.capaciteOffensive,
                       .nature = m_descriptionDeBase.nature,
                       .penetration = m_descriptionDeBase.penetration};
    const Degats tires = tirerLesDegats(generateur, m_descriptionDeBase.degats);
    for (const Element element : s_elements) {
        attaque.degats[element] = montantEffectif(tires[element], frappe.coefficients, m_caracteristiques);
        attaque.degats[element] += montantEffectif(frappe.bonus[element], frappe.coefficients, m_caracteristiques);
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
