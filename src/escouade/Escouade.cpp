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
#include "escouade/Intention.hpp"
#include "escouade/Personnage.hpp"
#include "escouade/Provocations.hpp"
#include "escouade/Separation.hpp"
#include "combat/Skill.hpp"
#include "combat/Autodefense.hpp"
#include "monde/ChampDeVision.hpp"
#include "monde/TileMap.hpp"
#include "objets/PositionDObjet.hpp"
#include "rendu/TraceDeChemin.hpp"

namespace {
constexpr float s_coteAncre = 20.0f;         // côté du carré de l'ancre, en pixels
constexpr float s_vitesseAncre = 180.0f;     // en pixels par seconde
constexpr float s_vitesseRotation = 360.0f;  // de la formation, en degrés par seconde
constexpr float s_laisseAgresseur = 100.0f;      // en pixels, centre à centre, mesurée depuis la place de formation
constexpr float s_vitesseSeparation = 120.0f;    // poussée maximale entre deux personnages trop proches, en pixels par seconde (plus lente que la marche, 240 : voir docs/formation.md)
constexpr std::size_t s_capaciteDuSac = 40;      // cases du sac de l'escouade
constexpr float s_toleranceArrivee = 0.01f;      // en pixels : comme le seuil de Mobile, un personnage à cette distance de son point y est arrivé

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

// Règle d'oubli des places d'attaque, appliquée une fois par personnage, après la résolution : la place
// retenue n'est gardée qu'autour du monstre de l'intention (aller à un point la fait oublier), et une
// approche qui n'a pas trouvé de place libre l'oublie aussi.
std::optional<int> monstreDontLaPlaceEstGardee(const Intention& intention, bool placeTrouvee) {
    if (std::holds_alternative<ApprocherMonstre>(intention) && !placeTrouvee) {
        return std::nullopt;
    }
    return monstreDeLIntention(intention);
}
}  // namespace

Escouade::Escouade(sf::Vector2f positionDepart, std::vector<DefinitionPersonnage> composition)
    : m_mobile(positionDepart + sf::Vector2f(s_coteAncre / 2.0f, s_coteAncre / 2.0f), {s_coteAncre, s_coteAncre}, s_vitesseAncre),
      m_formation(decalagesDeFormation(rangsDe(composition)), m_mobile.getCentre(), s_vitesseRotation),
      m_personnages(),
      m_cibleVerrouillee(),
      m_skillEnAttente(),
      m_placesDAttaque(composition.size(), s_ecartEntrePersonnages),
      m_pointsVises(),
      m_degagements(composition.size()),
      m_agresseurs(),
      m_provocations(),
      m_sac(s_capaciteDuSac)
{
    assert(!composition.empty());
    for (std::size_t i = 0; i < composition.size(); ++i) {
        m_personnages.emplace_back(m_formation.getPlaces()[i], std::move(composition[i]));
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
    for (Personnage& personnage : m_personnages) {
        personnage.relancerSiLArmeAChange(generateur);  // une arme changée dans l'inventaire : recharge complète
    }
    actualiserMemoires(monstres);
    engagerSkillDeZoneEnAttente(carte);
    approcherVerrou(monstres, carte);
    engagerSkillDeDegatsEnAttente(monstres, carte);

    std::vector<Coup> coups = avancerSkills(dt, monstres, carte, generateur);
    const std::vector<Coup> coupsDAutodefense = autodefense(dt, monstres, carte, generateur);
    coups.insert(coups.end(), coupsDAutodefense.begin(), coupsDAutodefense.end());
    return coups;
}

// Un agresseur ou un monstre provoqué absent de la liste est mort : on l'oublie.
void Escouade::actualiserMemoires(const std::vector<EntiteVue>& monstres) {
    m_agresseurs.retirerMonstresAbsents(monstres);
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

// Approche du monstre verrouillé, s'il y en a un dans la liste : l'ancre avance
// tant qu'un vivant ne l'atteint pas.
void Escouade::approcherVerrou(const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    if (!m_cibleVerrouillee.has_value()) {
        return;
    }
    const EntiteVue* verrou = trouverEntite(monstres, *m_cibleVerrouillee);
    if (verrou == nullptr) {
        return;
    }
    const Cercle cible = verrou->cercle;
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

    const std::optional<Attaque> attaque = lanceur.avancerSkill(dt, generateur);
    if (!attaque.has_value()) {
        return {};
    }
    assert(action->skill.has_value());  // une action de skill porte toujours son identifiant (engagerSkill)
    return effetDuSkill(indice + 1, *action->skill, action->cible, *attaque, monstres, carte, generateur);  // NOLINT(bugprone-unchecked-optional-access): vérifié par l'assert au-dessus (_assert de MinGW n'est pas noreturn)
}

// Effet d'un skill qui part, évalué au moment du départ. La provocation et le
// soin (critique éventuel compris) sont appliqués ici ; les dégâts et la zone sortent en `Coup`.
std::vector<Coup> Escouade::effetDuSkill(std::size_t place, IdSkill id, const std::optional<Cible>& cible, const Attaque& attaque, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    const Personnage& lanceur = personnageDeLaPlace(place);
    m_faits.push_back(SkillPart{place, id, lanceur.getCentre()});

    // Provocation (sans cible) : zone centrée sur le lanceur ; touche les
    // monstres de la zone et en vue du lanceur (`ligneDeVue`). La puissance
    // est la durée.
    if (!cible.has_value()) {
        const Cercle zone = lanceur.zoneDuSkill(id);
        for (const EntiteVue& monstre : monstres) {
            if (distanceBordABord(zone, monstre.cercle) == 0.0f && ligneDeVue(carte, lanceur.getCentre(), monstre.cercle.centre)) {
                provoquer(monstre.id, place, static_cast<float>(attaque.montant));
                m_faits.push_back(MonstreProvoque{monstre.id});
            }
        }
        return {};
    }

    return std::visit(
        Surcharges{
            [&](const CibleMonstre& monstre) -> std::vector<Coup> { return {{monstre.id, attaque}}; },
            [&](const CibleAllie& allie) -> std::vector<Coup> {
                Personnage& soigne = personnageDeLaPlace(allie.place);
                const ResultatDuCoup soin = resoudreSoin(attaque, generateur);
                if (const int rendu = soigne.recevoirSoin(soin.montant); rendu > 0) {
                    m_faits.push_back(SoinApplique{allie.place, rendu, soigne.getCentre(), soin.critique});
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
                        coups.push_back({monstre.id, attaque});
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
        if (const std::optional<int> idEngage = personnage.cibleEngagee()) {
            if (!personnage.estVivant() || trouverEntite(monstres, *idEngage) == nullptr) {
                personnage.annulerAttaque();
            } else if (const std::optional<Attaque> attaque = personnage.avancerAttaque(dt, generateur)) {
                coups.push_back({*idEngage, *attaque});
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

std::optional<int> Escouade::pointsDeVieMaxDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getPointsDeVieMax();
}

std::optional<Caracteristiques> Escouade::caracteristiquesDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getCaracteristiques();
}

std::optional<StatistiquesDeCombat> Escouade::statistiquesDeCombatDuPersonnage(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getStatistiques();
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

std::optional<Escouade::IntentionDuSkill> Escouade::avancerSkillEnAttente(const TileMap& carte,
                                                                          const std::optional<EntiteVue>& verrou) {
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

    const std::optional<Intention> intention = std::visit(
        Surcharges{
            // Dégâts : le skill part dans attaquer(), qui reçoit le cercle à jour du
            // monstre. Ici, le lanceur approche le verrou, seulement s'il est vu. L'exigence
            // d'un verrou est la règle de l'étape 37 : sans verrou, personne ne marche vers la
            // cible d'un skill de dégâts. Elle ne sert plus à lire un état périmé (R2 a supprimé
            // ce risque, quel que soit l'ordre d'attaquer et de mettreAJour) : elle garde la
            // règle de jeu. L'annulation (combat rompu) se fait dans verrouiller() et
            // deverrouiller().
            [&](const CibleMonstre& monstre) -> std::optional<Intention> {
                if (!verrou.has_value()) {
                    return std::nullopt;
                }
                return ApprocherMonstre{monstre.id, verrou->cercle, porteeDe(lanceur, idSkill)};
            },
            // Point (skill de zone) : le lanceur marche vers le point fixé au clic.
            [](const CiblePoint& point) -> std::optional<Intention> { return AllerAuPoint{point.point}; },
            // Soin : annulation si la cible est morte ou remontée à pleine vie.
            [&](const CibleAllie& cibleAllie) -> std::optional<Intention> {
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

                return AllerAuPoint{allie.getCentre()};
            },
        },
        cible);
    if (!intention.has_value()) {
        return std::nullopt;
    }
    return IntentionDuSkill{indiceDeLaPlace(placeLanceur), *intention};
}

float Escouade::porteeDe(const Personnage& personnage, std::optional<IdSkill> skill) const {
    if (!skill.has_value()) {
        return personnage.getPorteeAttaqueDeBase();
    }
    return personnage.cercleDePorteeDuSkill(*skill).rayon - personnage.getCercle().rayon;
}

// Place d'attaque (PlacesDAttaque::placer) : la référence de la première candidate est l'ancre, la
// portée celle de l'action avec laquelle le personnage approche.
PlacesDAttaque::Approche Escouade::approcheDe(std::size_t indice, EntiteVue monstre, float portee) const {
    return {
        .cible = monstre,
        .assaillant = indice,
        .cercleAssaillant = m_personnages[indice].getCercle(),
        .portee = portee,
        .reference = m_mobile.getCentre(),
    };
}

// Lus à chaque appel de placer : les personnages déjà déplacés dans cette image le sont à leur
// nouvelle position.
std::vector<std::optional<sf::Vector2f>> Escouade::occupants() const {
    std::vector<std::optional<sf::Vector2f>> centres;
    centres.reserve(m_personnages.size());
    for (const Personnage& personnage : m_personnages) {
        if (personnage.estVivant() && !personnage.estEnMouvement()) {
            centres.emplace_back(personnage.getCentre());
        } else {
            centres.emplace_back(std::nullopt);
        }
    }
    return centres;
}

// Dégagement : gardé seulement si le monstre est encore là (dans `monstres`, les monstres physiques) et
// qu'une place libre l'attend autour de lui, question posée avec les entrées du placement qui suivra ;
// sinon il est effacé et les règles habituelles reprennent. Renvoie le monstre dont il se dégage.
std::optional<EntiteVue> Escouade::confirmerDegagement(std::size_t indice,
                                                       const std::optional<PlacesDAttaque::Place>& ancienne,
                                                       const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    std::optional<int>& degagement = m_degagements[indice];
    if (!degagement.has_value()) {
        return std::nullopt;
    }
    if (const EntiteVue* monstre = trouverEntite(monstres, *degagement)) {
        const float portee = porteeDe(m_personnages[indice], std::nullopt);
        if (m_placesDAttaque.peutPlacer(approcheDe(indice, *monstre, portee), ancienne, occupants(), carte)) {
            return *monstre;
        }
    }
    degagement.reset();
    return std::nullopt;
}

// La situation du personnage de cet indice, au moment de décider : lui n'a pas encore bougé dans cette
// image, ceux d'indice inférieur si. Chaque monstre nommé porte le cercle de sa source : le verrou celui
// des monstres ciblables, l'agresseur celui qu'il a dans les ciblables (un agresseur qui n'y est pas est
// ignoré pour l'image), le dégagement celui des monstres physiques. « Un monstre à portée » : un ciblable que son attaque de base atteint, la règle
// même de l'autodéfense (choisirCibleAFrapper ne trouve une cible que s'il y en a une).
SituationDuPersonnage Escouade::situationDe(std::size_t indice, const std::optional<PlacesDAttaque::Place>& ancienne,
                                            const std::optional<EntiteVue>& degagement,
                                            const ContexteDesIntentions& contexte,
                                            const std::vector<EntiteVue>& ciblables, const TileMap& carte) const {
    const Personnage& personnage = m_personnages[indice];
    const sf::Vector2f placeDeFormation = m_formation.getPlaces()[indice];
    std::optional<Intention> skill;
    if (contexte.skill.has_value() && contexte.skill->lanceur == indice) {
        skill = contexte.skill->intention;
    }
    std::optional<int> monstreDeLaPlace;
    if (ancienne.has_value()) {
        monstreDeLaPlace = ancienne->idCible;
    }
    const auto atteint = [&](Cercle cercle) { return peutAtteindre(personnage, std::nullopt, cercle, carte); };
    const bool verrouAtteint = contexte.verrou.has_value() && atteint(contexte.verrou->cercle);
    const bool monstreAPortee =
        std::ranges::any_of(ciblables, [&](const EntiteVue& monstre) { return atteint(monstre.cercle); });
    return {
        .vivant = personnage.estVivant(),
        .enExecution = personnage.estEnExecution(),
        .monstreDeLaPlace = monstreDeLaPlace,
        .skillEnAttente = skill,
        .degagement = degagement,
        .verrou = contexte.verrou,
        .verrouAtteint = verrouAtteint,
        .ancreArretee = contexte.ancreArretee,
        .agresseur =
            choisirAgresseurAViser(personnage.getCercle(), placeDeFormation, s_laisseAgresseur, contexte.agresseurs),
        .monstreAPortee = monstreAPortee,
        .placeDeFormation = placeDeFormation,
        .porteeAttaqueDeBase = porteeDe(personnage, std::nullopt),
    };
}

// Résolution : rester, c'est viser son propre centre ; aller à un point, ce point ; approcher un monstre,
// sa place d'attaque (placer la retient), ou son centre si aucune place n'est libre.
Escouade::PointVise Escouade::pointDeLIntention(std::size_t indice, const Intention& intention,
                                                const std::optional<PlacesDAttaque::Place>& ancienne,
                                                const TileMap& carte) {
    return std::visit(
        Surcharges{
            [&](const ResterSurPlace&) { return PointVise{m_personnages[indice].getCentre(), false}; },
            [](const AllerAuPoint& aller) { return PointVise{aller.point, false}; },
            [&](const ApprocherMonstre& approche) {
                const EntiteVue monstre{approche.idMonstre, approche.cercle};
                const std::optional<sf::Vector2f> place =
                    m_placesDAttaque.placer(approcheDe(indice, monstre, approche.portee), ancienne, occupants(), carte);
                return PointVise{place.value_or(approche.cercle.centre), place.has_value()};
            },
        },
        intention);
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

void Escouade::mettreAJour(float dt, const TileMap& carte, const VueDesMonstres& monstres) {
    std::vector<Cercle> obstacles;
    for (const EntiteVue& monstre : monstres.physiques) {
        obstacles.push_back(monstre.cercle);
    }

    // Direction voulue lue avant l'essai de collision : une ancre qui glisse contre un mur oriente
    // quand même la formation vers la direction qu'elle essayait de suivre.
    const std::optional<sf::Vector2f> directionDeMarche = m_mobile.directionVoulue();

    ecoulerMemoires(dt);

    // L'ancre n'est pas bloquée par les obstacles : seuls les personnages le sont.
    m_mobile.mettreAJour(dt, carte, {});
    const bool ancreArretee = !m_mobile.directionVoulue().has_value();

    std::vector<float> rayons;
    rayons.reserve(m_personnages.size());
    for (const Personnage& personnage : m_personnages) {
        rayons.push_back(personnage.getCercle().rayon);
    }
    m_formation.mettreAJour(dt, getCentre(), directionDeMarche, rayons, carte, obstacles);

    mettreAJourDegagements(ancreArretee, monstres.deControle);
    // Le monstre verrouillé, lu ici dans les ciblables comme attaquer le lit dans les siens (approcherVerrou).
    std::optional<EntiteVue> verrou;
    if (m_cibleVerrouillee.has_value()) {
        if (const EntiteVue* vu = trouverEntite(monstres.ciblables, *m_cibleVerrouillee)) {
            verrou = *vu;
        }
    }
    const ContexteDesIntentions contexte{
        .ancreArretee = ancreArretee,
        .verrou = verrou,
        .agresseurs = m_agresseurs.vusParmi(monstres.ciblables),
        .skill = avancerSkillEnAttente(carte, verrou),
    };

    // Par indice croissant : situation, intention, point, oubli, déplacement. Les places d'attaque et les
    // occupants qu'un personnage voit dépendent de ceux qui l'ont précédé dans la boucle.
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        const std::optional<PlacesDAttaque::Place> ancienne = m_placesDAttaque.placeRetenue(i);
        const std::optional<EntiteVue> degagement = confirmerDegagement(i, ancienne, monstres.physiques, carte);
        const Intention intention =
            choisirIntention(situationDe(i, ancienne, degagement, contexte, monstres.ciblables, carte));
        const PointVise vise = pointDeLIntention(i, intention, ancienne, carte);
        m_pointsVises[i] = vise.point;
        m_placesDAttaque.oublierSaufAutourDe(i, monstreDontLaPlaceEstGardee(intention, vise.placeTrouvee));
        const std::vector<Cercle> aContourner = cerclesAContourner(monstres.physiques, intention);
        m_personnages[i].mettreAJour(dt, m_pointsVises[i], carte, obstacles, aContourner);
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
// d'après l'état actuel (m_degagements[indice]) ; choisirIntention en fait sa règle 2, où se brancheront
// les futurs gambits.
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

std::vector<Escouade::FaitDEscouade> Escouade::prendreFaits() {
    return std::exchange(m_faits, {});
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

ResultatDuCoup Escouade::recevoirCoup(int idMonstre, std::size_t place, const Attaque& attaque, Generateur& generateur) {
    if (!placeEstVivante(place)) {
        return {.montant = 0, .critique = false};
    }
    const ResultatDuCoup resultat = personnageDeLaPlace(place).recevoirDegats(attaque, generateur);

    // Une attaque qui a un montant désigne son auteur comme agresseur.
    if (attaque.montant > 0) {
        m_agresseurs.enregistrer(idMonstre);
    }
    return resultat;
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

std::optional<Attaque> Escouade::attaqueDeBase(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).getAttaqueDeBase();
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

const Inventaire& Escouade::sac() const {
    return m_sac;
}

const Equipement& Escouade::equipementDuPersonnage(std::size_t place) const {
    assert(placeValide(place));
    return personnageDeLaPlace(place).getEquipement();
}

const Equipement& Escouade::equipementActifDuPersonnage(std::size_t place) const {
    assert(placeValide(place));
    return personnageDeLaPlace(place).getEquipementActif();
}

bool Escouade::ramasser(Objet objet) {
    return m_sac.ajouter(objet);
}

std::optional<Objet> Escouade::objetA(const PositionDObjet& position) const {
    return std::visit(Surcharges{[&](const PositionDansLeSac& dansLeSac) { return m_sac.a(dansLeSac.caseDuSac); },
                                 [&](const PositionPortee& portee) {
                                     assert(placeValide(portee.place));
                                     return personnageDeLaPlace(portee.place).getEquipement().a(portee.emplacement);
                                 }},
                      position);
}

EtatDesObjets Escouade::etatDesObjets() const {
    EtatDesObjets etat{m_sac, {}, {}};
    for (const Personnage& personnage : m_personnages) {
        etat.equipements.push_back(personnage.getEquipement());
        etat.bases.push_back(personnage.getCaracteristiquesDeBase());
    }
    return etat;
}

std::vector<SeuilDeCaracteristique> Escouade::seuilsManques(const PositionDObjet& de, const PositionDObjet& vers) const {
    return ::seuilsManques(etatDesObjets(), de, vers);
}

bool Escouade::objetPorteEstInactif(std::size_t place, Emplacement emplacement) const {
    assert(placeValide(place));
    const Personnage& personnage = personnageDeLaPlace(place);
    return personnage.getEquipement().a(emplacement).has_value() && !personnage.getEquipementActif().a(emplacement).has_value();
}

bool Escouade::peutDeplacerObjet(const PositionDObjet& de, const PositionDObjet& vers) const {
    return deplacementPossible(etatDesObjets(), de, vers);
}

bool Escouade::deplacerObjet(const PositionDObjet& de, const PositionDObjet& vers) {
    EtatDesObjets etat = etatDesObjets();
    if (!deplacerLesObjets(etat, de, vers)) {
        return false;
    }
    m_sac = std::move(etat.sac);
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        // Les différences, par emplacement : d'abord tout ce qui est retiré, puis tout ce qui est posé (chaque appel
        // recalcule l'armure, les caractéristiques et les défenses du personnage).
        Personnage& personnage = m_personnages[i];
        const Equipement& voulu = etat.equipements[i];
        for (std::size_t e = 0; e < s_nombreEmplacements; ++e) {
            const Emplacement emplacement = static_cast<Emplacement>(e);
            if (personnage.getEquipement().a(emplacement).has_value() && personnage.getEquipement().a(emplacement) != voulu.a(emplacement)) {
                static_cast<void>(personnage.retirerEquipement(emplacement));  // l'objet est ailleurs dans `etat` : rien à rendre
            }
        }
        for (std::size_t e = 0; e < s_nombreEmplacements; ++e) {
            const Emplacement emplacement = static_cast<Emplacement>(e);
            const std::optional<Objet>& objetVoulu = voulu.a(emplacement);
            if (objetVoulu.has_value() && personnage.getEquipement().a(emplacement) != objetVoulu) {
                static_cast<void>(personnage.poserEquipement(emplacement, *objetVoulu));  // l'emplacement vient d'être vidé
            }
        }
    }
    // Un skill en attente dont le lanceur n'a plus l'arme qu'il exige est abandonné (sans recharge consommée).
    if (m_skillEnAttente.has_value()) {
        const AdresseSkill adresse = m_skillEnAttente->adresse;
        if (!personnageDeLaPlace(adresse.place).armeConvientAuSkill(adresse.id)) {
            m_skillEnAttente = std::nullopt;
        }
    }
    return true;
}
