// ./src/escouade/Escouade.cpp
#include "escouade/Escouade.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <utility>
#include <variant>
#include "combat/EntiteVue.hpp"
#include "escouade/Agresseurs.hpp"
#include "escouade/EcartEntrePersonnages.hpp"
#include "escouade/Personnage.hpp"
#include "escouade/Provocations.hpp"
#include "escouade/Separation.hpp"
#include "combat/Skill.hpp"
#include "combat/Autodefense.hpp"
#include "monde/ChampDeVision.hpp"
#include "monde/Encerclement.hpp"
#include "monde/TileMap.hpp"
#include "rendu/TraceDeChemin.hpp"

namespace {
constexpr float s_coteAncre = 20.0f;         // côté du carré de l'ancre, en pixels
constexpr float s_vitesseAncre = 180.0f;     // en pixels par seconde
constexpr float s_vitesseRotation = 360.0f;  // de la formation, en degrés par seconde
constexpr float s_laisseAgresseur = 100.0f;      // en pixels, centre à centre, mesurée depuis la place de formation
constexpr float s_fractionPorteeAnneau = 0.9f;    // anneau d'attaque : part de la portée, pour que la place soit à portée
constexpr float s_vitesseSeparation = 120.0f;    // poussée maximale entre deux personnages trop proches, en pixels par seconde (plus lente que la marche, 240 : voir docs/formation.md)
constexpr float s_toleranceArrivee = 0.01f;       // en pixels : comme le seuil de Mobile, un personnage à cette distance de son point y est arrivé

// Seule conversion d'une place (de 1 à N) vers l'indice (de 0 à N-1).
std::size_t indiceDeLaPlace(std::size_t place) {
    return place - 1;
}
}  // namespace

namespace {
std::vector<Rang> rangsDe(const std::vector<DefinitionPersonnage>& composition) {
    std::vector<Rang> rangs;
    for (const DefinitionPersonnage& definition : composition) {
        rangs.push_back(definition.rang);
    }
    return rangs;
}
}  // namespace

Escouade::Escouade(sf::Vector2f positionDepart, std::vector<DefinitionPersonnage> composition)
    : m_mobile(positionDepart + sf::Vector2f(s_coteAncre / 2.0f, s_coteAncre / 2.0f), {s_coteAncre, s_coteAncre}, s_vitesseAncre),
      m_formation(decalagesDeFormation(rangsDe(composition)), m_mobile.getCentre(), s_vitesseRotation),
      m_personnages(),
      m_cibleVerrouillee(),
      m_skillEnAttente(),
      m_cibleCombat(),
      m_monstreAPortee(composition.size(), false),
      m_placesDAttaque(composition.size()),
      m_pointsVises(),
      m_degagements(composition.size()),
      m_agresseurs(),
      m_provocations()
{
    assert(!composition.empty());
    for (std::size_t i = 0; i < composition.size(); ++i) {
        DefinitionPersonnage& definition = composition[i];
        m_personnages.emplace_back(m_formation.getPlaces()[i], definition.couleur, std::move(definition.attaqueDeBase),
                                   std::move(definition.skills), definition.armure, definition.rayonVision, definition.zoneDeControle);
    }
    m_pointsVises = m_formation.getPlaces();
}

bool Escouade::placeValide(std::size_t place) const {
    return place >= 1 && place <= m_personnages.size();
}

Personnage& Escouade::personnageDeLaPlace(std::size_t place) {
    return m_personnages.at(indiceDeLaPlace(place));
}

const Personnage& Escouade::personnageDeLaPlace(std::size_t place) const {
    return m_personnages.at(indiceDeLaPlace(place));
}

bool Escouade::skillEnAttenteDeDegats() const {
    return m_skillEnAttente.has_value() && std::holds_alternative<CibleMonstre>(m_skillEnAttente->cible);
}

void Escouade::annulerSkillDeDegatsEnAttente() {
    if (skillEnAttenteDeDegats()) {
        m_skillEnAttente = std::nullopt;
    }
}

void Escouade::dessiner(sf::RenderTarget& rendu, bool afficherAncre) const {
    if (afficherAncre) {
        const sf::Vector2f taille = m_mobile.getTaille();
        sf::RectangleShape forme(taille);
        forme.setPosition(getCentre() - taille / 2.0f);
        forme.setFillColor(sf::Color::Green);
        rendu.draw(forme);
    }

    for (const Personnage& personnage : m_personnages) {
        personnage.dessiner(rendu);
    }
}

// Ordre du joueur : les coups en exécution sont annulés, sans recharge consommée.
void Escouade::allerVers(sf::Vector2f destination, const TileMap& carte) {
    annulerAttaquesEngagees();
    m_formation.oublierPointARegarder();
    m_mobile.allerVers(destination, carte);
}

void Escouade::regarderVers(sf::Vector2f point) {
    m_formation.regarderVers(point);
}

void Escouade::annulerAttaquesEngagees() {
    for (Personnage& personnage : m_personnages) {
        personnage.annulerAttaque();
    }
}

std::vector<Coup> Escouade::attaquer(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    actualiserMemoires(monstres);
    engagerSkillDeZoneEnAttente(carte);
    approcherVerrou(monstres, carte);
    engagerSkillDeDegatsEnAttente(monstres, carte);

    std::vector<Coup> coups = avancerSkills(dt, monstres, carte, generateur);
    const std::vector<Coup> coupsDAutodefense = autodefense(dt, monstres, carte, generateur);
    coups.insert(coups.end(), coupsDAutodefense.begin(), coupsDAutodefense.end());
    return coups;
}

// Un agresseur ou un monstre provoqué absent de la liste est mort : on
// l'oublie ; le cercle des agresseurs restants est mis à jour.
void Escouade::actualiserMemoires(const std::vector<EntiteVue>& monstres) {
    m_agresseurs.actualiser(monstres);
    m_provocations.retirerMonstresAbsents(monstres);
}

// Skill de zone en attente : son activation commence dès que son lanceur est
// libre (aucun skill en activation), à portée et en vue du point fixé au clic.
// Il ne dépend pas du verrou. La mort du lanceur l'annule (avancerSkillEnAttente).
void Escouade::engagerSkillDeZoneEnAttente(const TileMap& carte) {
    if (!m_skillEnAttente.has_value() || !std::holds_alternative<CiblePoint>(m_skillEnAttente->cible)) {
        return;
    }
    const SkillEnAttente enAttente = *m_skillEnAttente;
    const CiblePoint point = std::get<CiblePoint>(enAttente.cible);
    Personnage& lanceur = personnageDeLaPlace(enAttente.adresse.place);
    if (!lanceur.peutEngagerSkill(enAttente.adresse.id) || !peutLancerAuPoint(enAttente.adresse, point.point, carte)) {
        return;
    }
    [[maybe_unused]] const bool engage = lanceur.engagerSkill(enAttente.adresse.id, enAttente.cible);
    assert(engage);
    m_skillEnAttente = std::nullopt;
}

bool Escouade::tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const {
    return std::all_of(m_personnages.begin(), m_personnages.end(), [&](const Personnage& personnage) {
        return !personnage.estVivant() || peutAtteindre(personnage, std::nullopt, cible, carte);
    });
}

// Approche du monstre verrouillé, s'il y en a un dans la liste : mémorise son
// cercle, et l'ancre avance tant qu'un vivant ne l'atteint pas.
void Escouade::approcherVerrou(const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    if (!m_cibleVerrouillee.has_value()) {
        return;
    }
    const EntiteVue* verrou = trouverEntite(monstres, *m_cibleVerrouillee);
    if (verrou == nullptr) {
        return;
    }
    const Cercle cible = verrou->cercle;
    m_cibleCombat = cible;

    if (tousLesVivantsAtteignent(cible, carte)) {
        m_mobile.arreter();
    } else {
        m_formation.oublierPointARegarder();
        m_mobile.allerVers(cible.centre, carte);  // pas allerVers : ce n'est pas un ordre du joueur
    }
}

// Skill de dégâts en attente : son activation commence dès que son lanceur est
// libre (aucun skill en activation) et atteint le cercle à jour de sa cible.
// Avec un verrou, la cible est le monstre verrouillé et le lanceur marche vers
// lui. Sans verrou (skill demandé pendant l'activation d'un autre skill de son
// lanceur), personne ne marche : une fois libre, le lanceur qui n'atteint plus
// sa cible, ou dont la cible a disparu, abandonne le skill, sans recharge.
void Escouade::engagerSkillDeDegatsEnAttente(const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    if (!m_skillEnAttente.has_value() || !std::holds_alternative<CibleMonstre>(m_skillEnAttente->cible)) {
        return;
    }
    const SkillEnAttente enAttente = *m_skillEnAttente;
    const AdresseSkill adresse = enAttente.adresse;
    Personnage& lanceur = personnageDeLaPlace(adresse.place);
    const EntiteVue* cible = trouverEntite(monstres, std::get<CibleMonstre>(enAttente.cible).id);
    const bool atteinte = cible != nullptr && peutAtteindre(lanceur, adresse.id, cible->cercle, carte);
    if (atteinte && lanceur.peutEngagerSkill(adresse.id)) {
        [[maybe_unused]] const bool engage = lanceur.engagerSkill(adresse.id, enAttente.cible);
        assert(engage);
        m_skillEnAttente = std::nullopt;
    } else if (!m_cibleVerrouillee.has_value() && lanceur.peutEngagerSkill(adresse.id)) {
        m_skillEnAttente = std::nullopt;
    }
}

// Fait avancer l'activation de chaque skill engagé ; les coups de ceux qui
// partent à cet appel sont renvoyés.
std::vector<Coup> Escouade::avancerSkills(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    std::vector<Coup> coups;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        const std::vector<Coup> coupsDuSkill = avancerSkillDe(i, dt, monstres, carte, generateur);
        coups.insert(coups.end(), coupsDuSkill.begin(), coupsDuSkill.end());
    }
    return coups;
}

// Activation d'un skill : annulée, sans recharge consommée, par la mort du
// lanceur ou la disparition de sa cible (monstre ou allié mort). Au départ,
// l'effet est évalué à ce moment (effetDuSkill). Les ordres du joueur ne
// l'annulent pas.
std::vector<Coup> Escouade::avancerSkillDe(std::size_t indice, float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    Personnage& lanceur = m_personnages[indice];
    const std::optional<ActionEngagee> action = lanceur.skillEngage();
    if (!action.has_value()) {
        return {};
    }

    const bool cibleDisparue = action->cible.has_value() && std::visit(
        Surcharges{
            [&](const CibleMonstre& monstre) { return trouverEntite(monstres, monstre.id) == nullptr; },
            [&](const CibleAllie& allie) { return !placeEstVivante(allie.place); },
            [](const CiblePoint&) { return false; },
        },
        *action->cible);
    if (!lanceur.estVivant() || cibleDisparue) {
        lanceur.annulerSkill();
        return {};
    }

    const int puissance = lanceur.avancerSkill(dt, generateur);
    if (puissance <= 0) {
        return {};
    }
    assert(action->skill.has_value());  // une action de skill porte toujours son identifiant (engagerSkill)
    return effetDuSkill(indice + 1, *action->skill, action->cible, puissance, monstres, carte);
}

// Effet d'un skill qui part, évalué au moment du départ. La provocation et le
// soin sont appliqués ici ; les dégâts et la zone sortent en `Coup`.
std::vector<Coup> Escouade::effetDuSkill(std::size_t place, IdSkill id, const std::optional<Cible>& cible, int puissance, const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    const Personnage& lanceur = personnageDeLaPlace(place);

    // Provocation (sans cible) : zone centrée sur le lanceur ; touche les
    // monstres de la zone et en vue du lanceur (`ligneDeVue`). La puissance
    // est la durée.
    if (!cible.has_value()) {
        const Cercle zone = lanceur.zoneDuSkill(id);
        for (const EntiteVue& monstre : monstres) {
            if (distanceBordABord(zone, monstre.cercle) == 0.0f && ligneDeVue(carte, lanceur.getCentre(), monstre.cercle.centre)) {
                provoquer(monstre.id, place, static_cast<float>(puissance));
            }
        }
        return {};
    }

    return std::visit(
        Surcharges{
            [&](const CibleMonstre& monstre) -> std::vector<Coup> { return {{monstre.id, puissance}}; },
            [&](const CibleAllie& allie) -> std::vector<Coup> {
                Personnage& soigne = personnageDeLaPlace(allie.place);
                if (const int rendu = soigne.recevoirSoin(puissance); rendu > 0) {
                    m_soinsAppliques.push_back({allie.place, rendu, soigne.getCentre()});
                }
                return {};
            },
            // Zone : tous les monstres dont le bord est dans la zone autour du
            // point, et en vue du point d'impact (`ligneDeVue`) : le sort ne traverse pas les murs.
            [&](const CiblePoint& point) -> std::vector<Coup> {
                const Cercle zone{point.point, lanceur.zoneDuSkill(id).rayon};
                std::vector<Coup> coups;
                for (const EntiteVue& monstre : monstres) {
                    if (distanceBordABord(zone, monstre.cercle) == 0.0f && ligneDeVue(carte, point.point, monstre.cercle.centre)) {
                        coups.push_back({monstre.id, puissance});
                    }
                }
                return coups;
            },
        },
        *cible);
}

// Chaque personnage choisit pour lui-même le monstre qu'il frappe, parmi
// ceux qu'il atteint (à portée et en vue). Frapper se fait en deux temps : un
// personnage immobile engage son coup, qui part à la fin de son temps
// d'exécution, même si la cible a bougé entre-temps. Le coup engagé n'est
// annulé que par la mort de son auteur ou la disparition de sa cible (et par
// un ordre du joueur ou un skill, voir allerVers et Personnage).
std::vector<Coup> Escouade::autodefense(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    std::vector<Coup> coups;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        Personnage& personnage = m_personnages[i];
        const std::optional<int> id = choisirCibleAFrapper(
            personnage.getCercle(), [&](Cercle c) { return peutAtteindre(personnage, std::nullopt, c, carte); }, monstres, m_cibleVerrouillee);
        m_monstreAPortee[i] = id.has_value();
        if (const std::optional<int> idEngage = personnage.cibleEngagee()) {
            if (!personnage.estVivant() || trouverEntite(monstres, *idEngage) == nullptr) {
                personnage.annulerAttaque();
            } else if (const int degats = personnage.avancerAttaque(dt, generateur); degats > 0) {
                coups.push_back({*idEngage, degats});
            }
            continue;
        }
        // Un personnage qui se dégage ne frappe pas : il va jusqu'à sa place d'attaque.
        if (id.has_value() && !m_degagements[i].has_value()) {
            // Tentative à chaque image : l'échec est normal (en marche, en recharge, skill en activation…).
            static_cast<void>(personnage.engagerAttaque(*id));
        }
    }
    return coups;
}

std::vector<Coup> Escouade::declencherSkill(AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    if (!placeValide(adresse.place)) {
        return {};
    }

    const std::size_t place = adresse.place;
    const IdSkill id = adresse.id;
    Personnage& lanceur = personnageDeLaPlace(place);
    const std::optional<TypeSkill> type = lanceur.typeSkill(id);
    if (!type.has_value() || m_skillEnAttente.has_value() || !lanceur.skillEstPret(id)) {
        return {};
    }

    // Provocation : zone centrée sur le lanceur, sans cible. Son activation
    // commence tout de suite ; elle touche les monstres au départ, même s'il
    // n'y en a aucun (la recharge est alors consommée).
    if (*type == TypeSkill::Provocation) {
        if (!lanceur.engagerSkill(id, std::nullopt)) {
            return {};
        }
        return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
    }

    // Zone : elle se lance à un point choisi (declencherSkillDeZone), pas ici.
    if (*type == TypeSkill::Zone) {
        return {};
    }

    const std::optional<Cible> cible = choisirCible(*type, adresse, monstres, carte);
    if (!cible.has_value()) {
        return {};
    }

    return std::visit(
        Surcharges{
            // Dégâts : l'activation commence si la cible est atteinte et le
            // lanceur libre (les coups sont renvoyés au départ, ici ou dans
            // attaquer()) ; sinon le skill est mis en attente : le lanceur
            // marche vers la cible, ou finit d'abord l'activation en cours.
            [&](const CibleMonstre& monstre) -> std::vector<Coup> {
                if (peutAtteindre(lanceur, id, monstre.cercle, carte) && lanceur.peutEngagerSkill(id)) {
                    [[maybe_unused]] const bool engage = lanceur.engagerSkill(id, *cible);
                    assert(engage);
                    return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
                }
                m_skillEnAttente = SkillEnAttente{adresse, *cible};
                return {};
            },
            // Soin : appliqué au départ, il ne sort pas d'Escouade. Même règle d'attente.
            [&](const CibleAllie& allie) -> std::vector<Coup> {
                if (peutAtteindre(lanceur, id, personnageDeLaPlace(allie.place).getCercle(), carte) && lanceur.peutEngagerSkill(id)) {
                    [[maybe_unused]] const bool engage = lanceur.engagerSkill(id, *cible);
                    assert(engage);
                    return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
                }
                m_skillEnAttente = SkillEnAttente{adresse, *cible};
                return {};
            },
            // choisirCible ne renvoie jamais de CiblePoint (réservée au skill de zone).
            [](const CiblePoint&) -> std::vector<Coup> { return {}; },
        },
        *cible);
}

bool Escouade::skillEstPret(AdresseSkill adresse) const {
    return placeValide(adresse.place) && personnageDeLaPlace(adresse.place).skillEstPret(adresse.id);
}

std::optional<EtatSkill> Escouade::etatDuSkill(AdresseSkill adresse) const {
    if (!placeValide(adresse.place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(adresse.place).etatDuSkill(adresse.id);
}

std::optional<TypeSkill> Escouade::typeDuSkill(AdresseSkill adresse) const {
    if (!placeValide(adresse.place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(adresse.place).typeSkill(adresse.id);
}

std::vector<AdresseSkill> Escouade::adressesDesSkills() const {
    std::vector<AdresseSkill> adresses;
    for (std::size_t place = 1; place <= m_personnages.size(); ++place) {
        for (const IdSkill id : personnageDeLaPlace(place).idsDesSkills()) {
            adresses.push_back({place, id});
        }
    }
    return adresses;
}

std::optional<sf::Color> Escouade::couleurDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getCouleur();
}

std::optional<int> Escouade::pointsDeVieDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getPointsDeVie();
}

float Escouade::rayonZoneDuSkill(AdresseSkill adresse) const {
    if (!placeValide(adresse.place)) {
        return 0.0f;
    }
    return personnageDeLaPlace(adresse.place).zoneDuSkill(adresse.id).rayon;
}

Cercle Escouade::cercleDePorteeDuSkill(AdresseSkill adresse) const {
    if (!placeValide(adresse.place)) {
        return {{0.0f, 0.0f}, 0.0f};
    }
    return personnageDeLaPlace(adresse.place).cercleDePorteeDuSkill(adresse.id);
}

bool Escouade::peutAtteindre(const Personnage& personnage, std::optional<IdSkill> skill, Cercle cible, const TileMap& carte) const {
    // À portée (bord à bord) ET en vue (`ligneDeVue`) : la seule règle d'atteinte d'Escouade.
    const bool aPortee = skill.has_value() ? personnage.skillEstAPortee(*skill, cible) : personnage.estAPortee(cible);
    return aPortee && ligneDeVue(carte, personnage.getCentre(), cible.centre);
}

bool Escouade::peutLancerAuPoint(AdresseSkill adresse, sf::Vector2f point, const TileMap& carte) const {
    // Le point est un cercle de rayon 0 pour la portée (bord à bord).
    return peutAtteindre(personnageDeLaPlace(adresse.place), adresse.id, Cercle{point, 0.0f}, carte);
}

std::vector<Coup> Escouade::declencherSkillDeZone(AdresseSkill adresse, sf::Vector2f point, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    if (!skillEstPret(adresse) || m_skillEnAttente.has_value()) {
        return {};
    }

    const std::size_t place = adresse.place;
    const IdSkill id = adresse.id;
    Personnage& lanceur = personnageDeLaPlace(place);
    if (lanceur.typeSkill(id) != TypeSkill::Zone) {
        return {};
    }

    // Un point dans un mur est refusé : ni départ, ni attente, ni recharge.
    const sf::Vector2i coordCase = carte.caseContenant(point);
    if (estBloquante(carte.getTile(coordCase.x, coordCase.y))) {
        return {};
    }

    // À portée du point, en vue et lanceur libre : l'activation commence tout de suite.
    // Sinon il est mis en attente : le lanceur marche vers le point, ou finit d'abord
    // l'activation en cours.
    if (peutLancerAuPoint(adresse, point, carte) && lanceur.peutEngagerSkill(id)) {
        [[maybe_unused]] const bool engage = lanceur.engagerSkill(id, Cible(CiblePoint{point}));
        assert(engage);
        return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
    }
    m_skillEnAttente = SkillEnAttente{adresse, Cible(CiblePoint{point})};
    return {};
}

std::optional<sf::Vector2f> Escouade::avancerSkillEnAttente(const TileMap& carte) {
    if (!m_skillEnAttente.has_value()) {
        return std::nullopt;
    }

    // Copie : la suite (et les lambdas ci-dessous) peuvent vider m_skillEnAttente.
    const SkillEnAttente enAttente = *m_skillEnAttente;
    const std::size_t placeLanceur = enAttente.adresse.place;
    const IdSkill idSkill = enAttente.adresse.id;
    const Cible& cible = enAttente.cible;
    const Personnage& lanceur = personnageDeLaPlace(placeLanceur);

    if (!lanceur.estVivant()) {
        m_skillEnAttente = std::nullopt;
        return std::nullopt;
    }

    return std::visit(
        Surcharges{
            // Dégâts : le skill part dans attaquer(), qui reçoit le cercle à jour du
            // monstre. Ici, le lanceur marche vers ce cercle, seulement avec un verrou
            // (sans verrou, m_cibleCombat peut dater d'un ancien combat, et personne ne
            // marche). L'annulation (combat rompu) se fait dans verrouiller() et deverrouiller().
            [&](const CibleMonstre&) -> std::optional<sf::Vector2f> {
                if (!m_cibleVerrouillee.has_value() || !m_cibleCombat.has_value()) {
                    return std::nullopt;
                }
                return m_cibleCombat->centre;
            },
            // Point (skill de zone) : le lanceur marche vers le point fixé au clic.
            [](const CiblePoint& point) -> std::optional<sf::Vector2f> { return point.point; },
            // Soin : annulation si la cible est morte ou remontée à pleine vie.
            [&](const CibleAllie& cibleAllie) -> std::optional<sf::Vector2f> {
                const Personnage& allie = personnageDeLaPlace(cibleAllie.place);
                if (!allie.estVivant() || allie.getPointsDeVie() >= allie.getPointsDeVieMax()) {
                    m_skillEnAttente = std::nullopt;
                    return std::nullopt;
                }

                if (peutAtteindre(lanceur, idSkill, allie.getCercle(), carte) && lanceur.peutEngagerSkill(idSkill)) {
                    [[maybe_unused]] const bool engage = personnageDeLaPlace(placeLanceur).engagerSkill(idSkill, cible);
                    assert(engage);
                    m_skillEnAttente = std::nullopt;
                    return std::nullopt;
                }

                return allie.getCentre();
            },
        },
        cible);
}

float Escouade::porteeDe(const Personnage& personnage, std::optional<IdSkill> skill) const {
    if (!skill.has_value()) {
        return personnage.getPorteeAttaqueDeBase();
    }
    return personnage.cercleDePorteeDuSkill(*skill).rayon - personnage.getCercle().rayon;
}

// Point que vise un personnage pour approcher ce monstre : sa place d'attaque,
// sur un anneau autour du monstre (rayon du monstre + rayon du personnage + 90 %
// de la portée de l'action), pour que les personnages arrivent chacun de leur
// côté. La place est un point où l'on marche : le personnage s'arrête dès que
// le monstre est à portée et en vue. L'angle est gardé (`ancienne`) tant que le
// personnage approche ce même monstre et que la place reste valide ; seule la
// position suit le monstre. Sinon, nouvelle attribution : la place valide (hors
// mur, en vue du centre du monstre) et libre (à plus de 34 px de celle des
// autres pour ce monstre, et à plus de 34 px d'un personnage immobile) la plus proche de l'angle actuel du personnage, les
// candidates partant de la direction du monstre vers l'ancre. Aucune place :
// le centre du monstre.
sf::Vector2f Escouade::pointVersMonstre(std::size_t indice, int idMonstre, Cercle monstre, std::optional<IdSkill> skill,
                                        const std::optional<PlaceDAttaque>& ancienne, const TileMap& carte) {
    const Personnage& personnage = m_personnages[indice];
    const float rayonPersonnage = personnage.getCercle().rayon;
    const float rayonAnneau = monstre.rayon + rayonPersonnage + s_fractionPorteeAnneau * porteeDe(personnage, skill);

    const auto angleVers = [&](sf::Vector2f point) {
        const sf::Vector2f direction = point - monstre.centre;
        return direction.length() > 0.0f ? direction.angle() : sf::degrees(0.0f);
    };
    const auto estValide = [&](sf::Angle angle) {
        const sf::Vector2f place = positionDePlace(monstre.centre, rayonAnneau, angle);
        const sf::Vector2f demiCote(rayonPersonnage, rayonPersonnage);
        return !carte.chevaucheUnMur(sf::FloatRect(place - demiCote, 2.0f * demiCote)) &&
               ligneDeVue(carte, place, monstre.centre);
    };

    // Occupée : un autre personnage vivant et immobile se tient déjà à cet endroit.
    const auto estOccupee = [&](sf::Vector2f place) {
        for (std::size_t autre = 0; autre < m_personnages.size(); ++autre) {
            if (autre != indice && m_personnages[autre].estVivant() && !m_personnages[autre].estEnMouvement() &&
                (m_personnages[autre].getCentre() - place).length() <= s_ecartEntrePersonnages) {
                return true;
            }
        }
        return false;
    };

    std::optional<sf::Angle> angle;
    if (ancienne.has_value() && ancienne->idMonstre == idMonstre && estValide(ancienne->angle) &&
        !estOccupee(positionDePlace(monstre.centre, rayonAnneau, ancienne->angle))) {
        angle = ancienne->angle;
    } else {
        const auto estLibreEtValide = [&](sf::Angle candidat) {
            if (!estValide(candidat)) {
                return false;
            }
            const sf::Vector2f place = positionDePlace(monstre.centre, rayonAnneau, candidat);
            if (estOccupee(place)) {
                return false;
            }
            for (std::size_t autre = 0; autre < m_placesDAttaque.size(); ++autre) {
                if (autre == indice || !m_placesDAttaque[autre].has_value() || m_placesDAttaque[autre]->idMonstre != idMonstre) {
                    continue;
                }
                const sf::Vector2f placeDeLAutre =
                    positionDePlace(monstre.centre, m_placesDAttaque[autre]->rayon, m_placesDAttaque[autre]->angle);
                if ((place - placeDeLAutre).length() <= s_ecartEntrePersonnages) {
                    return false;
                }
            }
            return true;
        };
        angle = choisirAngleLibre(anglesCandidats(angleVers(m_mobile.getCentre()), rayonAnneau, s_ecartEntrePersonnages),
                                  angleVers(personnage.getCentre()), estLibreEtValide);
    }

    if (!angle.has_value()) {
        return monstre.centre;
    }
    m_placesDAttaque[indice] = PlaceDAttaque{idMonstre, rayonAnneau, *angle};
    return positionDePlace(monstre.centre, rayonAnneau, *angle);
}

// Point à viser pour un personnage vivant qui a un agresseur dans sa laisse :
// son propre centre s'il a déjà un monstre à portée (il reste sur place),
// sinon la place d'attaque autour de l'agresseur le plus proche. Vide : il vise sa place.
std::optional<sf::Vector2f> Escouade::pointVersAgresseur(std::size_t indice, const std::vector<EntiteVue>& agresseurs,
                                                         const std::optional<PlaceDAttaque>& ancienne, const TileMap& carte) {
    const std::optional<EntiteVue> cible = choisirAgresseurAViser(
        m_personnages[indice].getCercle(), m_formation.getPlaces()[indice], s_laisseAgresseur, agresseurs);
    if (!cible.has_value()) {
        return std::nullopt;
    }
    if (m_monstreAPortee[indice]) {
        if (ancienne.has_value() && ancienne->idMonstre == cible->id) {
            m_placesDAttaque[indice] = ancienne;  // il reste sur place : sa place d'attaque est gardée
        }
        return m_personnages[indice].getCentre();
    }
    return pointVersMonstre(indice, cible->id, cible->cercle, std::nullopt, ancienne, carte);
}

// Point que vise un personnage : la première règle qui s'applique gagne.
// 0. un personnage qui exécute un coup ou active un skill reste sur place ;
// 1. le lanceur d'un skill en attente vise sa cible (pour un skill de dégâts,
//    sa place d'attaque autour du monstre) ;
// 2. un personnage qui se dégage (degagementVoulu) marche jusqu'à sa place d'attaque autour
//    du monstre dont il se dégage, sans s'arrêter à portée ;
// 3. en combat, un vivant déjà à portée de la cible reste sur place (les
//    tireurs s'arrêtent à distance pendant que les mêlées vont jusqu'au contact) ;
//    un vivant pas encore à portée marche vers sa place d'attaque ;
// 4. ancre à l'arrêt, un vivant qui a un agresseur dans sa laisse va vers lui,
//    à sa place d'attaque (l'ordre du joueur gagne : ancre en mouvement, il garde sa place) ;
// 5. sinon sa place en formation.
// La place d'attaque retenue est oubliée dès qu'aucune règle ne la renouvelle.
sf::Vector2f Escouade::pointVoulu(std::size_t indice, const std::optional<sf::Vector2f>& pointSkill, bool ancreArretee,
                                  const std::vector<EntiteVue>& agresseurs, const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    const Personnage& personnage = m_personnages[indice];
    const std::optional<PlaceDAttaque> ancienne = m_placesDAttaque[indice];
    m_placesDAttaque[indice].reset();

    if (personnage.estEnExecution()) {
        m_placesDAttaque[indice] = ancienne;  // le coup en cours ne fait pas oublier la place
        return personnage.getCentre();        // un coup ou un skill en exécution : il reste sur place
    }
    if (pointSkill.has_value()) {
        // pointSkill vient d'avancerSkillEnAttente : il n'existe que si un skill attend.
        assert(m_skillEnAttente.has_value());
        const SkillEnAttente enAttente = *m_skillEnAttente;
        if (indice == indiceDeLaPlace(enAttente.adresse.place)) {
            if (const CibleMonstre* monstre = std::get_if<CibleMonstre>(&enAttente.cible)) {
                // Le point d'un skill de dégâts n'existe qu'avec un verrou et son cercle (avancerSkillEnAttente).
                assert(m_cibleCombat.has_value());
                return pointVersMonstre(indice, monstre->id, *m_cibleCombat, enAttente.adresse.id, ancienne, carte);
            }
            return *pointSkill;
        }
    }
    if (const std::optional<int> degagement = m_degagements[indice]; degagement.has_value()) {
        // Le monstre est dans `monstres` (physiques : tous les monstres) tant qu'il est vivant.
        if (const EntiteVue* monstre = trouverEntite(monstres, *degagement)) {
            const sf::Vector2f place = pointVersMonstre(indice, monstre->id, monstre->cercle, std::nullopt, ancienne, carte);
            if (m_placesDAttaque[indice].has_value()) {
                return place;
            }
        }
        // Pas de monstre ou aucune place libre : se dégager n'a plus de sens, les règles habituelles reprennent.
        m_degagements[indice].reset();
    }
    const bool enCombat = m_cibleVerrouillee.has_value() && m_cibleCombat.has_value();
    if (enCombat && personnage.estVivant()) {
        if (peutAtteindre(personnage, std::nullopt, *m_cibleCombat, carte)) {
            if (ancienne.has_value() && ancienne->idMonstre == *m_cibleVerrouillee) {
                m_placesDAttaque[indice] = ancienne;
            }
            return personnage.getCentre();
        }
        return pointVersMonstre(indice, *m_cibleVerrouillee, *m_cibleCombat, std::nullopt, ancienne, carte);
    }
    if (ancreArretee && personnage.estVivant()) {
        if (const std::optional<sf::Vector2f> pointAgresseur = pointVersAgresseur(indice, agresseurs, ancienne, carte)) {
            return *pointAgresseur;
        }
    }
    return m_formation.getPlaces()[indice];
}

// Un agresseur qui n'a pas frappé depuis assez longtemps est oublié ; une
// provocation prend fin quand sa durée est écoulée ou que son provocateur
// est mort.
void Escouade::ecoulerMemoires(float dt) {
    m_agresseurs.mettreAJour(dt);
    m_provocations.mettreAJour(dt);
    for (std::size_t place = 1; place <= m_personnages.size(); ++place) {
        if (!personnageDeLaPlace(place).estVivant()) {
            m_provocations.retirerProvocateur(place);
        }
    }
}

std::vector<Cercle> Escouade::monstresAContourner(std::size_t indice, const std::vector<EntiteVue>& monstres) const {
    std::optional<int> approche;
    if (const std::optional<PlaceDAttaque>& place = m_placesDAttaque[indice]; place.has_value()) {
        approche = place->idMonstre;
    } else if (m_cibleVerrouillee.has_value() && m_cibleCombat.has_value()) {
        approche = m_cibleVerrouillee;
    }
    std::vector<Cercle> aContourner;
    for (const EntiteVue& monstre : monstres) {
        if (monstre.id != approche) {
            aContourner.push_back(monstre.cercle);
        }
    }
    return aContourner;
}

void Escouade::mettreAJour(float dt, const TileMap& carte, const std::vector<EntiteVue>& monstres,
                           const std::vector<EntiteDeControle>& monstresDeControle) {
    std::vector<Cercle> obstacles;
    for (const EntiteVue& monstre : monstres) {
        obstacles.push_back(monstre.cercle);
    }

    // Direction voulue lue avant l'essai de collision : une ancre qui glisse contre un mur oriente
    // quand même la formation vers la direction qu'elle essayait de suivre.
    m_formation.choisirOrientation(m_mobile.directionVoulue(), getCentre());

    ecoulerMemoires(dt);
    const std::vector<EntiteVue> agresseurs = m_agresseurs.monstresVus();  // une fois par image, pour tous les personnages

    // L'ancre n'est pas bloquée par les obstacles : seuls les personnages le sont.
    m_mobile.mettreAJour(dt, carte, {});
    const bool ancreArretee = !m_mobile.directionVoulue().has_value();

    std::vector<float> rayons;
    rayons.reserve(m_personnages.size());
    for (const Personnage& personnage : m_personnages) {
        rayons.push_back(personnage.getCercle().rayon);
    }
    m_formation.mettreAJour(dt, getCentre(), rayons, carte, obstacles);

    mettreAJourDegagements(ancreArretee, monstresDeControle);
    const std::optional<sf::Vector2f> pointSkill = avancerSkillEnAttente(carte);
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        m_pointsVises[i] = pointVoulu(i, pointSkill, ancreArretee, agresseurs, monstres, carte);
        m_personnages[i].mettreAJour(dt, m_pointsVises[i], carte, obstacles, monstresAContourner(i, monstres));
    }
    separerPersonnages(dt, carte, obstacles);
}

// Tous les personnages sont jugés d'après les positions du début de l'image (aucun n'a encore bougé
// dans celle-ci), avant que l'un d'eux ne se déplace.
void Escouade::mettreAJourDegagements(bool ancreArretee, const std::vector<EntiteDeControle>& monstresDeControle) {
    const std::vector<EntiteDeControle> personnages = entitesDeControle();
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        m_degagements[i] = degagementVoulu(i, ancreArretee, personnages, monstresDeControle);
    }
}

// LA règle « tenir ou se dégager » : seul endroit qui sait quand un personnage quitte le contact d'un
// monstre pour sa place d'attaque. Renvoie le monstre dont il se dégage (vide : il ne se dégage pas),
// d'après l'état actuel (m_degagements[indice]) ; un futur gambit remplacera cette fonction.
// Départ (voir monstreDontSeDegager) : sans zone de contrôle, engagé, chaque monstre qui l'engage tient
// un autre personnage. Il faut en plus l'ancre à l'arrêt (l'ordre du joueur gagne), aucun skill en attente
// pour lui et aucune action en exécution (un coup en cours part d'abord). Fin : arrivé à sa place,
// monstre disparu (mort, retiré ou qui rentre à son poste : absent de `monstresDeControle`), ou encore
// engagé alors que la condition n'est plus vraie (le porteur est parti : rester évite le coup gratuit).
// Une action en exécution, l'ancre en marche ou un skill en attente y mettent fin sans condition.
std::optional<int> Escouade::degagementVoulu(std::size_t indice, bool ancreArretee, const std::vector<EntiteDeControle>& personnages,
                                             const std::vector<EntiteDeControle>& monstresDeControle) const {
    const Personnage& personnage = m_personnages[indice];
    const bool skillEnAttentePourLui = m_skillEnAttente.has_value() && m_skillEnAttente->adresse.place == indice + 1;
    if (!personnage.estVivant() || !ancreArretee || skillEnAttentePourLui || personnage.estEnExecution()) {
        return std::nullopt;
    }
    const auto lui = std::ranges::find(personnages, static_cast<int>(indice + 1), &EntiteDeControle::id);
    if (lui == personnages.end()) {
        return std::nullopt;
    }
    const std::optional<int> candidat = monstreDontSeDegager(*lui, personnages, monstresDeControle);

    const std::optional<int>& enCours = m_degagements[indice];
    if (!enCours.has_value()) {
        return candidat;
    }
    const bool monstreDisparu = std::ranges::find(monstresDeControle, *enCours, &EntiteDeControle::id) == monstresDeControle.end();
    const bool arrive = (personnage.getCentre() - m_pointsVises[indice]).length() <= s_toleranceArrivee;
    const bool engageEncore = std::ranges::any_of(monstresDeControle, [&](const EntiteDeControle& monstre) { return engage(monstre, *lui); });
    if (monstreDisparu || arrive || (engageEncore && !candidat.has_value())) {
        return std::nullopt;
    }
    return enCours;
}

// Séparation douce (pousseesDeSeparation) : un mort est absent, un personnage en exécution
// est fixe. Chaque poussée passe par la collision habituelle (murs, monstres).
void Escouade::separerPersonnages(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    std::vector<MembreASeparer> membres;
    membres.reserve(m_personnages.size());
    for (const Personnage& personnage : m_personnages) {
        EtatDeSeparation etat = EtatDeSeparation::Libre;
        if (!personnage.estVivant()) {
            etat = EtatDeSeparation::Absent;
        } else if (personnage.estEnExecution()) {
            etat = EtatDeSeparation::Fixe;
        }
        membres.push_back({personnage.getCentre(), etat});
    }

    const std::vector<sf::Vector2f> poussees =
        pousseesDeSeparation(membres, s_ecartEntrePersonnages, s_vitesseSeparation, dt);
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        if (poussees[i] != sf::Vector2f(0.0f, 0.0f)) {
            m_personnages[i].decaler(poussees[i], carte, obstacles);
        }
    }
}

std::optional<sf::Vector2f> Escouade::centreDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getCentre();
}

std::vector<Escouade::SoinApplique> Escouade::prendreSoins() {
    return std::exchange(m_soinsAppliques, {});
}

std::optional<sf::Vector2f> Escouade::pointVise(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return m_pointsVises[indiceDeLaPlace(place)];
}

std::optional<int> Escouade::degagementDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return m_degagements[indiceDeLaPlace(place)];
}

void Escouade::dessinerPointsVises(sf::RenderTarget& rendu) const {
    constexpr float s_rayonDuRepere = 4.0f;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        if (!m_personnages[i].estVivant()) {
            continue;
        }
        const sf::Vector2f centre = m_personnages[i].getCentre();
        const std::array<sf::Vertex, 2> trait = {sf::Vertex{centre, sf::Color::Yellow}, sf::Vertex{m_pointsVises[i], sf::Color::Yellow}};
        rendu.draw(trait.data(), trait.size(), sf::PrimitiveType::Lines);
        dessinerTraceDeChemin(rendu, centre, m_personnages[i].cheminRestant(), sf::Color::Cyan);

        sf::CircleShape repere(s_rayonDuRepere);
        repere.setOrigin({s_rayonDuRepere, s_rayonDuRepere});
        repere.setPosition(m_pointsVises[i]);
        repere.setFillColor(sf::Color::Transparent);
        repere.setOutlineColor(sf::Color::Yellow);
        repere.setOutlineThickness(1.0f);
        rendu.draw(repere);
    }
}

const std::vector<sf::Vector2f>& Escouade::getPlacesDeFormation() const {
    return m_formation.getPlaces();
}

sf::Vector2f Escouade::getCentre() const {
    return m_mobile.getCentre();
}

std::optional<std::size_t> Escouade::indicePersonnageLePlusProche(sf::Vector2f point) const {
    std::optional<std::size_t> indicePlusProche;
    float distancePlusProche = 0.0f;

    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        if (!m_personnages[i].estVivant()) {
            continue;
        }
        const float distance = (m_personnages[i].getCentre() - point).length();
        if (!indicePlusProche.has_value() || distance < distancePlusProche) {
            distancePlusProche = distance;
            indicePlusProche = i;
        }
    }

    return indicePlusProche;
}

// Seule règle de « quel personnage ce monstre vise » : le provocateur s'il
// est provoqué et vivant, sinon le vivant le plus proche du monstre.
std::optional<std::size_t> Escouade::indiceProieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const {
    const std::optional<std::size_t> provocateur = m_provocations.provocateurDe(idMonstre);
    if (provocateur.has_value() && personnageDeLaPlace(*provocateur).estVivant()) {
        return indiceDeLaPlace(*provocateur);
    }
    return indicePersonnageLePlusProche(positionMonstre);
}

std::optional<EntiteVue> Escouade::proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const {
    const std::optional<std::size_t> indice = indiceProieDuMonstre(idMonstre, positionMonstre);
    if (!indice.has_value()) {
        return std::nullopt;
    }
    return EntiteVue{static_cast<int>(*indice + 1), m_personnages[*indice].getCercle()};
}

std::vector<EntiteVue> Escouade::candidatsDeFrappe(int idMonstre) const {
    const std::optional<std::size_t> provocateur = m_provocations.provocateurDe(idMonstre);
    if (provocateur.has_value() && personnageDeLaPlace(*provocateur).estVivant()) {
        return {EntiteVue{static_cast<int>(*provocateur), personnageDeLaPlace(*provocateur).getCercle()}};
    }
    std::vector<EntiteVue> candidats;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        if (m_personnages[i].estVivant()) {
            candidats.push_back(EntiteVue{static_cast<int>(i + 1), m_personnages[i].getCercle()});
        }
    }
    return candidats;
}

bool Escouade::placeEstVivante(std::size_t place) const {
    return placeValide(place) && personnageDeLaPlace(place).estVivant();
}

int Escouade::recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite) {
    const std::optional<EntiteVue> proie = proieDuMonstre(idMonstre, origine);
    if (!proie.has_value()) {
        return 0;
    }
    return recevoirCoup(idMonstre, static_cast<std::size_t>(proie->id), origine, quantite);
}

int Escouade::recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite) {
    if (!placeEstVivante(place)) {
        return 0;
    }
    const int subis = personnageDeLaPlace(place).recevoirDegats(quantite);

    // Un coup qui fait des dégâts désigne son auteur comme agresseur.
    if (quantite > 0) {
        m_agresseurs.enregistrer(idMonstre, origine);
    }
    return subis;
}

const std::vector<Agresseurs::Agresseur>& Escouade::getAgresseurs() const {
    return m_agresseurs.liste();
}

void Escouade::provoquer(int idMonstre, std::size_t place, float duree) {
    if (!placeValide(place)) {
        return;
    }
    m_provocations.provoquer(idMonstre, place, duree);
}

const std::vector<Provocations::Provocation>& Escouade::getProvocations() const {
    return m_provocations.liste();
}

bool Escouade::estVivante() const {
    for (const Personnage& personnage : m_personnages) {
        if (personnage.estVivant()) {
            return true;
        }
    }
    return false;
}

std::vector<Cercle> Escouade::cerclesDesVivants() const {
    std::vector<Cercle> cercles;
    for (const Personnage& personnage : m_personnages) {
        if (personnage.estVivant()) {
            cercles.push_back(personnage.getCercle());
        }
    }
    return cercles;
}

std::vector<EntiteDeControle> Escouade::entitesDeControle() const {
    std::vector<EntiteDeControle> entites;
    for (std::size_t indice = 0; indice < m_personnages.size(); ++indice) {
        const Personnage& personnage = m_personnages[indice];
        if (personnage.estVivant()) {
            entites.push_back({static_cast<int>(indice + 1), personnage.getCercle(), personnage.getZoneDeControle()});
        }
    }
    return entites;
}

std::optional<int> Escouade::puissanceDeLAttaqueDeBase(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getPuissanceAttaqueDeBase();
}

std::vector<Cercle> Escouade::cerclesDeVision() const {
    std::vector<Cercle> cercles;
    for (const Personnage& personnage : m_personnages) {
        if (personnage.estVivant()) {
            cercles.push_back({personnage.getCentre(), personnage.getRayonVision()});
        }
    }
    return cercles;
}

bool Escouade::unVivantPresDe(sf::FloatRect zone, float ecart) const {
    for (const Cercle& cercle : cerclesDesVivants()) {
        if (distanceCercleRectangle(cercle, zone) <= ecart) {
            return true;
        }
    }
    return false;
}

std::optional<Cible> Escouade::choisirCible(TypeSkill type, AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte) const {
    // Skills de zone, sans cible.
    if (type == TypeSkill::Provocation || type == TypeSkill::Zone) {
        return std::nullopt;
    }
    if (type == TypeSkill::Degats) {
        // Avec un verrou : le monstre verrouillé, à portée ou non.
        if (m_cibleVerrouillee.has_value()) {
            const EntiteVue* verrou = trouverEntite(monstres, *m_cibleVerrouillee);
            if (verrou == nullptr) {
                return std::nullopt;
            }
            return Cible(CibleMonstre{verrou->id, verrou->cercle});
        }

        // Sans verrou : le monstre que le lanceur frapperait en autodéfense,
        // mais avec la portée de son skill.
        if (!placeValide(adresse.place)) {
            return std::nullopt;
        }
        const Personnage& lanceur = personnageDeLaPlace(adresse.place);
        const IdSkill idSkill = adresse.id;
        const std::optional<int> id = choisirCibleAFrapper(
            lanceur.getCercle(), [&](Cercle c) { return peutAtteindre(lanceur, idSkill, c, carte); }, monstres, std::nullopt);
        if (!id.has_value()) {
            return std::nullopt;
        }
        const EntiteVue* cible = trouverEntite(monstres, *id);
        if (cible == nullptr) {
            return std::nullopt;
        }
        return Cible(CibleMonstre{cible->id, cible->cercle});
    }

    // Le plus blessé parmi les vivants qui ne sont pas à pleine vie ;
    // l'inégalité stricte garde la plus petite place en cas d'égalité.
    std::optional<std::size_t> indiceChoisi;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        const Personnage& personnage = m_personnages[i];
        if (!personnage.estVivant() || personnage.getPointsDeVie() >= personnage.getPointsDeVieMax()) {
            continue;
        }
        if (!indiceChoisi.has_value() ||
            personnage.getPointsDeVie() < m_personnages[*indiceChoisi].getPointsDeVie()) {
            indiceChoisi = i;
        }
    }

    if (!indiceChoisi.has_value()) {
        return std::nullopt;
    }
    return Cible(CibleAllie{*indiceChoisi + 1});  // place de 1 à N, comme AdresseSkill
}

void Escouade::verrouiller(int idCible) {
    // Un skill de dégâts en attente et les coups en exécution ne survivent
    // pas à un changement de cible.
    if (m_cibleVerrouillee != idCible) {
        annulerSkillDeDegatsEnAttente();
        annulerAttaquesEngagees();
    }
    m_cibleVerrouillee = idCible;
    m_formation.oublierPointARegarder();
}

void Escouade::deverrouiller() {
    annulerSkillDeDegatsEnAttente();
    m_cibleVerrouillee = std::nullopt;
}

std::optional<int> Escouade::getCibleVerrouillee() const {
    return m_cibleVerrouillee;
}
