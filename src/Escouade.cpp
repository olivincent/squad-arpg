// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <variant>
#include "EntiteVue.hpp"
#include "Agresseurs.hpp"
#include "Personnage.hpp"
#include "Provocations.hpp"
#include "Skill.hpp"
#include "Autodefense.hpp"
#include "Chemin.hpp"
#include "Encerclement.hpp"
#include "TileMap.hpp"
#include "TraceDeChemin.hpp"

namespace {
const sf::Color s_couleurMelee = sf::Color::Red;
const sf::Color s_couleurTireur = sf::Color::Cyan;
constexpr float s_coteAncre = 20.0f;         // côté du carré de l'ancre, en pixels
constexpr float s_vitesseAncre = 180.0f;     // en pixels par seconde
constexpr float s_vitesseRotation = 360.0f;  // de la formation, en degrés par seconde
constexpr float s_porteeMelee = 20.0f;    // bord à bord
constexpr float s_porteeTireur = 120.0f;  // bord à bord
constexpr int s_armureMelee = 50;
constexpr int s_armureTireur = 0;
constexpr float s_rayonVision = 320.0f;  // en pixels (8 cases), le même pour les quatre
constexpr int s_degatsAttaqueDeBase = 5;
constexpr float s_executionAttaqueDeBase = 0.15f;  // en secondes, immobile avant que le coup parte
constexpr float s_rechargeMeleeMin = 0.3f;   // attaque de base des mêlées, en secondes
constexpr float s_rechargeMeleeMax = 0.7f;
constexpr float s_rechargeTireurMin = 0.4f;  // attaque de base des tireurs, en secondes
constexpr float s_rechargeTireurMax = 0.6f;
constexpr int s_degatsSkill = 25;
constexpr float s_rechargeSkill = 5.0f;   // en secondes
constexpr float s_activationSkill = 0.3f; // en secondes, immobile avant que l'effet parte
constexpr float s_porteeSoin = 40.0f;     // bord à bord
constexpr int s_puissanceSoin = 30;
constexpr float s_rechargeSoin = 8.0f;    // en secondes
constexpr float s_activationSoin = 0.6f;  // en secondes
constexpr float s_rayonProvocation = 150.0f;  // rayon de zone, depuis le centre du provocateur
constexpr int s_dureeProvocation = 4;         // en secondes (la puissance de ce skill)
constexpr float s_rechargeProvocation = 10.0f;  // en secondes
constexpr float s_activationProvocation = 0.2f;  // en secondes
constexpr float s_porteeZone = 250.0f;        // portée de lancement du skill de zone, bord à bord
constexpr int s_degatsZone = 15;
constexpr float s_rechargeZone = 8.0f;        // en secondes
constexpr float s_activationZone = 0.8f;      // en secondes
constexpr float s_rayonZone = 60.0f;          // rayon de zone, depuis le point visé
constexpr float s_laisseAgresseur = 100.0f;      // en pixels, centre à centre, mesurée depuis la place de formation
constexpr float s_fractionPorteeAnneau = 0.9f;    // anneau d'attaque : part de la portée, pour que la place soit à portée
constexpr float s_vitesseSeparation = 120.0f;    // poussée maximale entre deux personnages trop proches, en pixels par seconde (plus lente que la marche, 240 : voir docs/formation.md)
constexpr float s_ecartEntrePersonnages = 34.0f;  // distance minimale voulue entre deux centres (≈ (20 + 4) × √2 : 4 px visibles même en diagonale) ; aussi entre deux places, ou une place et un personnage immobile
constexpr std::array<float, 3> s_rayonsDeSecoursPlaces = {34.0f, 68.0f, 102.0f};  // anneaux autour de l'ancre pour une place repliée sur une autre

// Attaque de base d'un personnage, mêlée ou tireur : la portée et l'intervalle de recharge changent.
Skill attaqueDeBase(float portee, float rechargeMin, float rechargeMax) {
    return Skill(TypeSkill::Degats, portee, s_degatsAttaqueDeBase, rechargeMin, rechargeMax, 0.0f,
                 s_executionAttaqueDeBase);
}

// Seule conversion d'une place (de 1 à 4, comme la touche du skill) vers l'indice (de 0 à 3).
std::size_t indiceDeLaPlace(std::size_t place) {
    return place - 1;
}
}  // namespace

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_mobile(positionDepart + sf::Vector2f(s_coteAncre / 2.0f, s_coteAncre / 2.0f), {s_coteAncre, s_coteAncre}, s_vitesseAncre),
      m_decalages({{25.0f, -25.0f}, {25.0f, 25.0f}, {-25.0f, -25.0f}, {-25.0f, 25.0f}}),
      m_orientation(),
      m_orientationVoulue(),
      m_vitesseRotation(s_vitesseRotation),
      m_personnages({
          Personnage(placePersonnage(0), s_couleurMelee, attaqueDeBase(s_porteeMelee, s_rechargeMeleeMin, s_rechargeMeleeMax),
                     Skill(TypeSkill::Degats, s_porteeMelee, s_degatsSkill, s_rechargeSkill, s_rechargeSkill, 0.0f, s_activationSkill), s_armureMelee, s_rayonVision),
          Personnage(placePersonnage(1), s_couleurMelee, attaqueDeBase(s_porteeMelee, s_rechargeMeleeMin, s_rechargeMeleeMax),
                     Skill(TypeSkill::Provocation, 0.0f, s_dureeProvocation, s_rechargeProvocation, s_rechargeProvocation, s_rayonProvocation, s_activationProvocation),
                     s_armureMelee, s_rayonVision),
          Personnage(placePersonnage(2), s_couleurTireur, attaqueDeBase(s_porteeTireur, s_rechargeTireurMin, s_rechargeTireurMax),
                     Skill(TypeSkill::Soin, s_porteeSoin, s_puissanceSoin, s_rechargeSoin, s_rechargeSoin, 0.0f, s_activationSoin), s_armureTireur, s_rayonVision),
          Personnage(placePersonnage(3), s_couleurTireur, attaqueDeBase(s_porteeTireur, s_rechargeTireurMin, s_rechargeTireurMax),
                     Skill(TypeSkill::Zone, s_porteeZone, s_degatsZone, s_rechargeZone, s_rechargeZone, s_rayonZone, s_activationZone), s_armureTireur, s_rayonVision),
      }),
      m_cibleVerrouillee(),
      m_skillEnAttente(),
      m_cibleCombat(),
      m_places({placePersonnage(0), placePersonnage(1), placePersonnage(2), placePersonnage(3)}),
      m_monstreAPortee(),
      m_placesDAttaque(),
      m_pointsVises({placePersonnage(0), placePersonnage(1), placePersonnage(2), placePersonnage(3)}),
      m_agresseurs(),
      m_provocations()
{
}

sf::Vector2f Escouade::placePersonnage(std::size_t indice) const {
    return getCentre() + m_decalages[indice].rotatedBy(m_orientation);
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
    m_pointARegarder.reset();
    m_mobile.allerVers(destination, carte);
}

void Escouade::regarderVers(sf::Vector2f point) {
    m_pointARegarder = point;
}

void Escouade::annulerAttaquesEngagees() {
    for (Personnage& personnage : m_personnages) {
        personnage.annulerAttaque();
    }
}

std::vector<Coup> Escouade::attaquer(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    actualiserMemoires(monstres);
    engagerSkillDeZoneEnAttente(carte);

    if (const std::optional<Cercle> verrou = approcherVerrou(monstres, carte)) {
        engagerSkillDeDegatsEnAttente(*verrou, carte);
    }

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
// à portée et en vue du point fixé au clic. Il ne dépend pas du verrou. La
// mort du lanceur l'annule (avancerSkillEnAttente).
void Escouade::engagerSkillDeZoneEnAttente(const TileMap& carte) {
    if (!m_skillEnAttente.has_value() || !std::holds_alternative<CiblePoint>(m_skillEnAttente->cible)) {
        return;
    }
    const CiblePoint point = std::get<CiblePoint>(m_skillEnAttente->cible);
    Personnage& lanceur = personnageDeLaPlace(m_skillEnAttente->place);
    if (!lanceur.estVivant() || !peutLancerAuPoint(m_skillEnAttente->place, point.point, carte)) {
        return;
    }
    static_cast<void>(lanceur.engagerSkill(m_skillEnAttente->cible));
    m_skillEnAttente = std::nullopt;
}

bool Escouade::tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const {
    return std::all_of(m_personnages.begin(), m_personnages.end(), [&](const Personnage& personnage) {
        return !personnage.estVivant() || peutAtteindre(personnage, TypeAction::AttaqueDeBase, cible, carte);
    });
}

// Approche du monstre verrouillé, s'il y en a un dans la liste : mémorise son
// cercle, et l'ancre avance tant qu'un vivant ne l'atteint pas. Renvoie ce
// cercle ; vide sans verrou ou si le monstre est absent de la liste.
std::optional<Cercle> Escouade::approcherVerrou(const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    if (!m_cibleVerrouillee.has_value()) {
        return std::nullopt;
    }
    const EntiteVue* verrou = trouverEntite(monstres, *m_cibleVerrouillee);
    if (verrou == nullptr) {
        return std::nullopt;
    }
    const Cercle cible = verrou->cercle;
    m_cibleCombat = cible;

    if (tousLesVivantsAtteignent(cible, carte)) {
        m_mobile.arreter();
    } else {
        m_pointARegarder.reset();
        m_mobile.allerVers(cible.centre, carte);  // pas allerVers : ce n'est pas un ordre du joueur
    }
    return cible;
}

// Skill de dégâts en attente : son activation commence dès que son lanceur
// est à portée du cercle à jour de la cible.
void Escouade::engagerSkillDeDegatsEnAttente(Cercle cible, const TileMap& carte) {
    if (!skillEnAttenteDeDegats()) {
        return;
    }
    Personnage& lanceur = personnageDeLaPlace(m_skillEnAttente->place);
    if (!peutAtteindre(lanceur, TypeAction::Skill, cible, carte)) {
        return;
    }
    static_cast<void>(lanceur.engagerSkill(m_skillEnAttente->cible));
    m_skillEnAttente = std::nullopt;
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
    return effetDuSkill(indice + 1, action->cible, puissance, monstres, carte);
}

// Effet d'un skill qui part, évalué au moment du départ. La provocation et le
// soin sont appliqués ici ; les dégâts et la zone sortent en `Coup`.
std::vector<Coup> Escouade::effetDuSkill(std::size_t place, const std::optional<Cible>& cible, int puissance, const std::vector<EntiteVue>& monstres, const TileMap& carte) {
    const Personnage& lanceur = personnageDeLaPlace(place);

    // Provocation (sans cible) : zone centrée sur le lanceur ; touche les
    // monstres de la zone et en vue du lanceur (centre à centre). La puissance
    // est la durée.
    if (!cible.has_value()) {
        const Cercle zone = lanceur.zoneDuSkill();
        for (const EntiteVue& monstre : monstres) {
            if (distanceBordABord(zone, monstre.cercle) == 0.0f && carte.ligneDeVue(lanceur.getCentre(), monstre.cercle.centre)) {
                provoquer(monstre.id, place, static_cast<float>(puissance));
            }
        }
        return {};
    }

    return std::visit(
        Surcharges{
            [&](const CibleMonstre& monstre) -> std::vector<Coup> { return {{monstre.id, puissance}}; },
            [&](const CibleAllie& allie) -> std::vector<Coup> {
                personnageDeLaPlace(allie.place).recevoirSoin(puissance);
                return {};
            },
            // Zone : tous les monstres dont le bord est dans la zone autour du
            // point, et en vue du point d'impact (centre à centre) : le sort ne traverse pas les murs.
            [&](const CiblePoint& point) -> std::vector<Coup> {
                const Cercle zone{point.point, lanceur.zoneDuSkill().rayon};
                std::vector<Coup> coups;
                for (const EntiteVue& monstre : monstres) {
                    if (distanceBordABord(zone, monstre.cercle) == 0.0f && carte.ligneDeVue(point.point, monstre.cercle.centre)) {
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
            personnage.getCercle(), [&](Cercle c) { return peutAtteindre(personnage, TypeAction::AttaqueDeBase, c, carte); }, monstres, m_cibleVerrouillee);
        m_monstreAPortee[i] = id.has_value();
        if (const std::optional<int> idEngage = personnage.cibleEngagee()) {
            if (!personnage.estVivant() || trouverEntite(monstres, *idEngage) == nullptr) {
                personnage.annulerAttaque();
            } else if (const int degats = personnage.avancerAttaque(dt, generateur); degats > 0) {
                coups.push_back({*idEngage, degats});
            }
            continue;
        }
        if (id.has_value()) {
            personnage.engagerAttaque(*id);
        }
    }
    return coups;
}

std::vector<Coup> Escouade::declencherSkill(std::size_t place, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    if (!placeValide(place)) {
        return {};
    }

    Personnage& lanceur = personnageDeLaPlace(place);
    const std::optional<TypeSkill> type = lanceur.typeSkill();
    if (!type.has_value() || m_skillEnAttente.has_value() || !lanceur.skillEstPret()) {
        return {};
    }

    // Provocation : zone centrée sur le lanceur, sans cible. Son activation
    // commence tout de suite ; elle touche les monstres au départ, même s'il
    // n'y en a aucun (la recharge est alors consommée).
    if (*type == TypeSkill::Provocation) {
        if (!lanceur.engagerSkill(std::nullopt)) {
            return {};
        }
        return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
    }

    // Zone : elle se lance à un point choisi (declencherSkillDeZone), pas ici.
    if (*type == TypeSkill::Zone) {
        return {};
    }

    const std::optional<Cible> cible = choisirCible(*type, place, monstres, carte);
    if (!cible.has_value()) {
        return {};
    }

    return std::visit(
        Surcharges{
            // Dégâts : l'activation commence si la cible est atteinte (les
            // coups sont renvoyés au départ, ici ou dans attaquer()) ; sinon
            // le skill est mis en attente.
            [&](const CibleMonstre& monstre) -> std::vector<Coup> {
                if (peutAtteindre(lanceur, TypeAction::Skill, monstre.cercle, carte)) {
                    static_cast<void>(lanceur.engagerSkill(*cible));
                    return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
                }
                m_skillEnAttente = SkillEnAttente{place, *cible};
                return {};
            },
            // Soin : appliqué au départ, il ne sort pas d'Escouade.
            [&](const CibleAllie& allie) -> std::vector<Coup> {
                if (peutAtteindre(lanceur, TypeAction::Skill, personnageDeLaPlace(allie.place).getCercle(), carte)) {
                    static_cast<void>(lanceur.engagerSkill(*cible));
                    return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
                }
                m_skillEnAttente = SkillEnAttente{place, *cible};
                return {};
            },
            // choisirCible ne renvoie jamais de CiblePoint (réservée au skill de zone).
            [](const CiblePoint&) -> std::vector<Coup> { return {}; },
        },
        *cible);
}

bool Escouade::skillEstPret(std::size_t place) const {
    return placeValide(place) && personnageDeLaPlace(place).skillEstPret();
}

std::optional<EtatSkill> Escouade::etatDuSkill(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return personnageDeLaPlace(place).etatDuSkill();
}

float Escouade::rayonZoneDuSkill(std::size_t place) const {
    if (!placeValide(place)) {
        return 0.0f;
    }
    return personnageDeLaPlace(place).zoneDuSkill().rayon;
}

Cercle Escouade::cercleDePorteeDuSkill(std::size_t place) const {
    if (!placeValide(place)) {
        return {{0.0f, 0.0f}, 0.0f};
    }
    return personnageDeLaPlace(place).cercleDePorteeDuSkill();
}

bool Escouade::peutAtteindre(const Personnage& personnage, TypeAction action, Cercle cible, const TileMap& carte) const {
    // À portée (bord à bord) ET en vue (centre à centre) : la seule règle d'atteinte d'Escouade.
    const bool aPortee = (action == TypeAction::Skill) ? personnage.skillEstAPortee(cible) : personnage.estAPortee(cible);
    return aPortee && carte.ligneDeVue(personnage.getCentre(), cible.centre);
}

bool Escouade::peutLancerAuPoint(std::size_t place, sf::Vector2f point, const TileMap& carte) const {
    // Le point est un cercle de rayon 0 pour la portée (bord à bord).
    return peutAtteindre(personnageDeLaPlace(place), TypeAction::Skill, Cercle{point, 0.0f}, carte);
}

std::vector<Coup> Escouade::declencherSkillDeZone(std::size_t place, sf::Vector2f point, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur) {
    if (!placeValide(place) || !skillEstPret(place) || m_skillEnAttente.has_value()) {
        return {};
    }

    Personnage& lanceur = personnageDeLaPlace(place);
    if (lanceur.typeSkill() != TypeSkill::Zone) {
        return {};
    }

    // Un point dans un mur est refusé : ni départ, ni attente, ni recharge.
    const sf::Vector2i coordCase = carte.caseContenant(point);
    if (estBloquante(carte.getTile(coordCase.x, coordCase.y))) {
        return {};
    }

    // À portée du point et en vue : l'activation commence tout de suite. Sinon il
    // est mis en attente, et le lanceur marche vers le point.
    if (peutLancerAuPoint(place, point, carte)) {
        static_cast<void>(lanceur.engagerSkill(Cible(CiblePoint{point})));
        return avancerSkillDe(indiceDeLaPlace(place), 0.0f, monstres, carte, generateur);
    }
    m_skillEnAttente = SkillEnAttente{place, Cible(CiblePoint{point})};
    return {};
}

std::optional<sf::Vector2f> Escouade::avancerSkillEnAttente(const TileMap& carte) {
    if (!m_skillEnAttente.has_value()) {
        return std::nullopt;
    }

    const std::size_t placeLanceur = m_skillEnAttente->place;
    const Personnage& lanceur = personnageDeLaPlace(placeLanceur);

    if (!lanceur.estVivant()) {
        m_skillEnAttente = std::nullopt;
        return std::nullopt;
    }

    // Copie : les lambdas ci-dessous peuvent vider m_skillEnAttente.
    const Cible cible = m_skillEnAttente->cible;
    return std::visit(
        Surcharges{
            // Dégâts : le skill part dans attaquer(), qui reçoit le cercle à jour du
            // monstre. Ici, le lanceur marche vers ce cercle. L'annulation (combat
            // rompu) se fait dans verrouiller() et deverrouiller().
            [&](const CibleMonstre&) -> std::optional<sf::Vector2f> {
                if (!m_cibleCombat.has_value()) {
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

                if (peutAtteindre(lanceur, TypeAction::Skill, allie.getCercle(), carte)) {
                    static_cast<void>(personnageDeLaPlace(placeLanceur).engagerSkill(cible));
                    m_skillEnAttente = std::nullopt;
                    return std::nullopt;
                }

                return allie.getCentre();
            },
        },
        cible);
}

float Escouade::porteeDe(const Personnage& personnage, TypeAction action) const {
    if (action == TypeAction::AttaqueDeBase) {
        return personnage.getPorteeAttaqueDeBase();
    }
    return personnage.cercleDePorteeDuSkill().rayon - personnage.getCercle().rayon;
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
sf::Vector2f Escouade::pointVersMonstre(std::size_t indice, int idMonstre, Cercle monstre, TypeAction action,
                                        const std::optional<PlaceDAttaque>& ancienne, const TileMap& carte) {
    const Personnage& personnage = m_personnages[indice];
    const float rayonPersonnage = personnage.getCercle().rayon;
    const float rayonAnneau = monstre.rayon + rayonPersonnage + s_fractionPorteeAnneau * porteeDe(personnage, action);

    const auto angleVers = [&](sf::Vector2f point) {
        const sf::Vector2f direction = point - monstre.centre;
        return direction.length() > 0.0f ? direction.angle() : sf::degrees(0.0f);
    };
    const auto estValide = [&](sf::Angle angle) {
        const sf::Vector2f place = positionDePlace(monstre.centre, rayonAnneau, angle);
        const sf::Vector2f demiCote(rayonPersonnage, rayonPersonnage);
        return !carte.chevaucheUnMur(sf::FloatRect(place - demiCote, 2.0f * demiCote)) &&
               carte.ligneDeVue(place, monstre.centre);
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
        m_personnages[indice].getCercle(), m_places[indice], s_laisseAgresseur, agresseurs);
    if (!cible.has_value()) {
        return std::nullopt;
    }
    if (m_monstreAPortee[indice]) {
        if (ancienne.has_value() && ancienne->idMonstre == cible->id) {
            m_placesDAttaque[indice] = ancienne;  // il reste sur place : sa place d'attaque est gardée
        }
        return m_personnages[indice].getCentre();
    }
    return pointVersMonstre(indice, cible->id, cible->cercle, TypeAction::AttaqueDeBase, ancienne, carte);
}

// Point que vise un personnage : la première règle qui s'applique gagne.
// 0. un personnage qui exécute un coup ou active un skill reste sur place ;
// 1. le lanceur d'un skill en attente vise sa cible (pour un skill de dégâts,
//    sa place d'attaque autour du monstre) ;
// 2. en combat, un vivant déjà à portée de la cible reste sur place (les
//    tireurs s'arrêtent à distance pendant que les mêlées vont jusqu'au contact) ;
//    un vivant pas encore à portée marche vers sa place d'attaque ;
// 3. ancre à l'arrêt, un vivant qui a un agresseur dans sa laisse va vers lui,
//    à sa place d'attaque (l'ordre du joueur gagne : ancre en mouvement, il garde sa place) ;
// 4. sinon sa place en formation.
// La place d'attaque retenue est oubliée dès qu'aucune règle ne la renouvelle.
sf::Vector2f Escouade::pointVoulu(std::size_t indice, const std::optional<sf::Vector2f>& pointSkill, bool ancreArretee,
                                  const std::vector<EntiteVue>& agresseurs, const TileMap& carte) {
    const Personnage& personnage = m_personnages[indice];
    const std::optional<PlaceDAttaque> ancienne = m_placesDAttaque[indice];
    m_placesDAttaque[indice].reset();

    if (personnage.estEnExecution()) {
        m_placesDAttaque[indice] = ancienne;  // le coup en cours ne fait pas oublier la place
        return personnage.getCentre();        // un coup ou un skill en exécution : il reste sur place
    }
    if (pointSkill.has_value() && indice == indiceDeLaPlace(m_skillEnAttente->place)) {
        if (skillEnAttenteDeDegats()) {
            const CibleMonstre& monstre = std::get<CibleMonstre>(m_skillEnAttente->cible);
            return pointVersMonstre(indice, monstre.id, *m_cibleCombat, TypeAction::Skill, ancienne, carte);
        }
        return *pointSkill;
    }
    const bool enCombat = m_cibleVerrouillee.has_value() && m_cibleCombat.has_value();
    if (enCombat && personnage.estVivant()) {
        if (peutAtteindre(personnage, TypeAction::AttaqueDeBase, *m_cibleCombat, carte)) {
            if (ancienne.has_value() && ancienne->idMonstre == *m_cibleVerrouillee) {
                m_placesDAttaque[indice] = ancienne;
            }
            return personnage.getCentre();
        }
        return pointVersMonstre(indice, *m_cibleVerrouillee, *m_cibleCombat, TypeAction::AttaqueDeBase, ancienne, carte);
    }
    if (ancreArretee && personnage.estVivant()) {
        if (const std::optional<sf::Vector2f> pointAgresseur = pointVersAgresseur(indice, agresseurs, ancienne, carte)) {
            return *pointAgresseur;
        }
    }
    return m_places[indice];
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

// Rotation progressive vers l'orientation voulue, par le plus court
// côté (wrapSigned ramène l'écart entre -180° et 180°), à vitesse
// angulaire bornée, que l'ancre bouge ou non.
void Escouade::tournerFormation(float dt) {
    const sf::Angle ecart = (m_orientationVoulue - m_orientation).wrapSigned();
    const float pasRotation = m_vitesseRotation * dt;
    if (std::abs(ecart.asDegrees()) <= pasRotation) {
        m_orientation = m_orientationVoulue;
    } else {
        const float signe = (ecart.asDegrees() > 0.0f) ? 1.0f : -1.0f;
        m_orientation += sf::degrees(pasRotation * signe);
    }
}

// Une place théorique dans un mur ou derrière un coin est ramenée
// vers l'ancre jusqu'à un point libre. Si la place ramenée tombe à moins de
// `s_ecartEntrePersonnages` d'une place déjà calculée, ou dans un monstre (centre à moins de
// rayon du monstre + rayon du personnage), elle est remplacée par le
// premier point valide (trajet libre depuis l'ancre, hors mur, hors monstre) et libre sur des
// anneaux autour de l'ancre, les candidates partant de la direction de la place
// théorique ; sans point, elle est au centre de l'ancre (ou, si seul un monstre la gênait, reste où le repli l'a mise).
void Escouade::calculerPlaces(const TileMap& carte, const std::vector<EntiteVue>& monstres) {
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        const float rayonPersonnage = m_personnages[i].getCercle().rayon;
        const float diametre = 2.0f * rayonPersonnage;
        const sf::Vector2f taille(diametre, diametre);
        const sf::Vector2f theorique = placePersonnage(i);
        m_places[i] = replierVersOrigine(carte, getCentre(), theorique, taille);

        const auto estLibre = [&](sf::Vector2f point) {
            for (std::size_t autre = 0; autre < i; ++autre) {
                if ((point - m_places[autre]).length() < s_ecartEntrePersonnages) {
                    return false;
                }
            }
            return true;
        };
        const auto estDansUnMonstre = [&](sf::Vector2f point) {
            return std::ranges::any_of(monstres, [&](const EntiteVue& monstre) {
                return (point - monstre.cercle.centre).length() < monstre.cercle.rayon + rayonPersonnage;
            });
        };
        const sf::Vector2f replie = m_places[i];
        const bool replieEstLibre = estLibre(replie);
        if (replieEstLibre && !estDansUnMonstre(replie)) {
            continue;
        }

        const sf::Vector2f direction = theorique - getCentre();
        const sf::Angle depart = direction.length() > 0.0f ? direction.angle() : sf::degrees(0.0f);
        const auto estValide = [&](sf::Vector2f point) {
            return estLibre(point) && !estDansUnMonstre(point) &&
                   !carte.chevaucheUnMur(sf::FloatRect(point - taille / 2.0f, taille)) &&
                   carte.trajetLibre(sf::FloatRect(getCentre() - taille / 2.0f, taille), point - getCentre());
        };
        bool trouve = false;
        m_places[i] = replieEstLibre ? replie : getCentre();
        for (const float rayon : s_rayonsDeSecoursPlaces) {
            for (const sf::Angle angle : anglesCandidats(depart, rayon, s_ecartEntrePersonnages)) {
                const sf::Vector2f point = positionDePlace(getCentre(), rayon, angle);
                if (estValide(point)) {
                    m_places[i] = point;
                    trouve = true;
                    break;
                }
            }
            if (trouve) {
                break;
            }
        }
    }
}

std::vector<Cercle> Escouade::monstresAContourner(std::size_t indice, const std::vector<EntiteVue>& monstres) const {
    std::optional<int> approche;
    if (m_placesDAttaque[indice].has_value()) {
        approche = m_placesDAttaque[indice]->idMonstre;
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

void Escouade::mettreAJour(float dt, const TileMap& carte, const std::vector<EntiteVue>& monstres) {
    std::vector<Cercle> obstacles;
    for (const EntiteVue& monstre : monstres) {
        obstacles.push_back(monstre.cercle);
    }

    if (const std::optional<sf::Vector2f> direction = m_mobile.directionVoulue()) {
        m_orientationVoulue = direction->angle();
    } else if (m_pointARegarder.has_value()) {
        const sf::Vector2f vers = *m_pointARegarder - getCentre();
        if (vers.length() > 0.0f) {
            m_orientationVoulue = vers.angle();
        }
    }

    ecoulerMemoires(dt);
    const std::vector<EntiteVue> agresseurs = m_agresseurs.monstresVus();  // une fois par image, pour tous les personnages

    // L'ancre n'est pas bloquée par les obstacles : seuls les personnages le sont.
    m_mobile.mettreAJour(dt, carte, {});
    const bool ancreArretee = !m_mobile.directionVoulue().has_value();

    tournerFormation(dt);
    calculerPlaces(carte, monstres);

    const std::optional<sf::Vector2f> pointSkill = avancerSkillEnAttente(carte);
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        m_pointsVises[i] = pointVoulu(i, pointSkill, ancreArretee, agresseurs, carte);
        m_personnages[i].mettreAJour(dt, m_pointsVises[i], carte, obstacles, monstresAContourner(i, monstres));
    }
    separerPersonnages(dt, carte, obstacles);
}

// Séparation douce : chaque paire de vivants à moins de s_ecartEntrePersonnages est écartée le
// long de la droite de leurs centres, chacun de la moitié du manque. Un
// personnage en exécution n'est pas poussé (l'autre prend toute la poussée ;
// si les deux le sont, rien). La poussée de chacun est plafonnée à
// s_vitesseSeparation * dt, et passe par la collision habituelle (murs, monstres).
void Escouade::separerPersonnages(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    std::vector<sf::Vector2f> poussees(m_personnages.size(), sf::Vector2f(0.0f, 0.0f));

    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        for (std::size_t j = i + 1; j < m_personnages.size(); ++j) {
            const Personnage& a = m_personnages[i];
            const Personnage& b = m_personnages[j];
            if (!a.estVivant() || !b.estVivant()) {
                continue;
            }
            const sf::Vector2f ecart = b.getCentre() - a.getCentre();
            const float distance = ecart.length();
            const float chevauchement = s_ecartEntrePersonnages - distance;
            if (chevauchement <= 0.0f) {
                continue;
            }
            // Centres confondus : repli fixe sur l'axe des x, le plus petit indice vers la gauche.
            const sf::Vector2f direction = distance > 0.0f ? ecart / distance : sf::Vector2f(1.0f, 0.0f);
            const bool aBouge = !a.estEnExecution();
            const bool bBouge = !b.estEnExecution();
            if (!aBouge && !bBouge) {
                continue;
            }
            const float partA = bBouge ? 0.5f : 1.0f;  // si l'autre est immobile, on prend tout
            const float partB = aBouge ? 0.5f : 1.0f;
            if (aBouge) {
                poussees[i] -= direction * (chevauchement * partA);
            }
            if (bBouge) {
                poussees[j] += direction * (chevauchement * partB);
            }
        }
    }

    const float pousseeMax = s_vitesseSeparation * dt;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        const float longueur = poussees[i].length();
        if (longueur <= 0.0f) {
            continue;
        }
        const sf::Vector2f poussee = longueur > pousseeMax ? poussees[i] * (pousseeMax / longueur) : poussees[i];
        m_personnages[i].decaler(poussee, carte, obstacles);
    }
}

std::optional<sf::Vector2f> Escouade::pointVise(std::size_t place) const {
    if (!placeValide(place)) {
        return std::nullopt;
    }
    return m_pointsVises[indiceDeLaPlace(place)];
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
    return m_places;
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

void Escouade::recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite) {
    const std::optional<EntiteVue> proie = proieDuMonstre(idMonstre, origine);
    if (!proie.has_value()) {
        return;
    }
    recevoirCoup(idMonstre, static_cast<std::size_t>(proie->id), origine, quantite);
}

void Escouade::recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite) {
    if (!placeEstVivante(place)) {
        return;
    }
    personnageDeLaPlace(place).recevoirDegats(quantite);

    // Un coup qui fait des dégâts désigne son auteur comme agresseur.
    if (quantite <= 0) {
        return;
    }
    m_agresseurs.enregistrer(idMonstre, origine);
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

std::optional<Cible> Escouade::choisirCible(TypeSkill type, std::size_t place, const std::vector<EntiteVue>& monstres, const TileMap& carte) const {
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
        if (!placeValide(place)) {
            return std::nullopt;
        }
        const Personnage& lanceur = personnageDeLaPlace(place);
        const std::optional<int> id = choisirCibleAFrapper(
            lanceur.getCercle(), [&](Cercle c) { return peutAtteindre(lanceur, TypeAction::Skill, c, carte); }, monstres, std::nullopt);
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
    return Cible(CibleAllie{*indiceChoisi + 1});  // place de 1 à 4, comme declencherSkill
}

void Escouade::verrouiller(int idCible) {
    // Un skill de dégâts en attente et les coups en exécution ne survivent
    // pas à un changement de cible.
    if (m_cibleVerrouillee != idCible) {
        annulerSkillDeDegatsEnAttente();
        annulerAttaquesEngagees();
    }
    m_cibleVerrouillee = idCible;
    m_pointARegarder.reset();
}

void Escouade::deverrouiller() {
    annulerSkillDeDegatsEnAttente();
    m_cibleVerrouillee = std::nullopt;
}

std::optional<int> Escouade::getCibleVerrouillee() const {
    return m_cibleVerrouillee;
}
