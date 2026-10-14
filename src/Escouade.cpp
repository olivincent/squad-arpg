// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cmath>
#include <variant>
#include "MonstreVu.hpp"
#include "Agresseurs.hpp"
#include "Personnage.hpp"
#include "Provocations.hpp"
#include "Skill.hpp"
#include "Autodefense.hpp"
#include "Chemin.hpp"
#include "TileMap.hpp"

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
constexpr int s_degatsAttaqueDeBase = 5;
constexpr float s_rechargeAttaqueDeBase = 0.5f;  // en secondes
constexpr int s_degatsSkill = 25;
constexpr float s_rechargeSkill = 5.0f;   // en secondes
constexpr float s_porteeSoin = 40.0f;     // bord à bord
constexpr int s_puissanceSoin = 30;
constexpr float s_rechargeSoin = 8.0f;    // en secondes
constexpr float s_rayonProvocation = 150.0f;  // rayon de zone, depuis le centre du provocateur
constexpr int s_dureeProvocation = 4;         // en secondes (la puissance de ce skill)
constexpr float s_rechargeProvocation = 10.0f;  // en secondes
constexpr float s_porteeZone = 250.0f;        // portée de lancement du skill de zone, bord à bord
constexpr int s_degatsZone = 15;
constexpr float s_rechargeZone = 8.0f;        // en secondes
constexpr float s_rayonZone = 60.0f;          // rayon de zone, depuis le point visé
constexpr float s_laisseAgresseur = 100.0f;      // en pixels, centre à centre, mesurée depuis la place de formation

// Attaque de base d'un personnage, mêlée ou tireur : seule la portée change.
Skill attaqueDeBase(float portee) {
    return Skill(TypeSkill::Degats, portee, s_degatsAttaqueDeBase, s_rechargeAttaqueDeBase, 0.0f);
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
          Personnage(placePersonnage(0), s_couleurMelee, attaqueDeBase(s_porteeMelee),
                     Skill(TypeSkill::Degats, s_porteeMelee, s_degatsSkill, s_rechargeSkill, 0.0f), s_armureMelee),
          Personnage(placePersonnage(1), s_couleurMelee, attaqueDeBase(s_porteeMelee),
                     Skill(TypeSkill::Provocation, 0.0f, s_dureeProvocation, s_rechargeProvocation, s_rayonProvocation),
                     s_armureMelee),
          Personnage(placePersonnage(2), s_couleurTireur, attaqueDeBase(s_porteeTireur),
                     Skill(TypeSkill::Soin, s_porteeSoin, s_puissanceSoin, s_rechargeSoin, 0.0f), s_armureTireur),
          Personnage(placePersonnage(3), s_couleurTireur, attaqueDeBase(s_porteeTireur),
                     Skill(TypeSkill::Zone, s_porteeZone, s_degatsZone, s_rechargeZone, s_rayonZone), s_armureTireur),
      }),
      m_cibleVerrouillee(),
      m_skillEnAttente(),
      m_cibleCombat(),
      m_places({placePersonnage(0), placePersonnage(1), placePersonnage(2), placePersonnage(3)}),
      m_monstreAPortee(),
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

void Escouade::dessiner(sf::RenderTarget& rendu) const {
    const sf::Vector2f taille = m_mobile.getTaille();
    sf::RectangleShape forme(taille);
    forme.setPosition(getCentre() - taille / 2.0f);
    forme.setFillColor(sf::Color::Green);
    rendu.draw(forme);

    for (const Personnage& personnage : m_personnages) {
        personnage.dessiner(rendu);
    }
}

void Escouade::allerVers(sf::Vector2f destination, const TileMap& carte) {
    m_mobile.allerVers(destination, carte);
}

std::vector<Coup> Escouade::attaquer(const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    actualiserMemoires(monstres);
    std::vector<Coup> coups = departSkillDeZoneEnAttente(monstres, carte);

    if (const std::optional<Cercle> verrou = approcherVerrou(monstres, carte)) {
        if (const std::optional<Coup> coup = departSkillDeDegatsEnAttente(*verrou, carte)) {
            coups.push_back(*coup);
        }
    }

    const std::vector<Coup> coupsDAutodefense = autodefense(monstres, carte);
    coups.insert(coups.end(), coupsDAutodefense.begin(), coupsDAutodefense.end());
    return coups;
}

// Un agresseur ou un monstre provoqué absent de la liste est mort : on
// l'oublie ; le cercle des agresseurs restants est mis à jour.
void Escouade::actualiserMemoires(const std::vector<MonstreVu>& monstres) {
    m_agresseurs.actualiser(monstres);
    m_provocations.retirerMonstresAbsents(monstres);
}

// Skill de zone en attente : il part dès que son lanceur est à portée
// du point fixé au clic, sur les monstres présents à ce moment. Il
// part avant les attaques de base (dont il relance la recharge). Il ne
// dépend pas du verrou. La mort du lanceur l'annule (avancerSkillEnAttente).
std::vector<Coup> Escouade::departSkillDeZoneEnAttente(const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    if (!m_skillEnAttente.has_value()) {
        return {};
    }
    const CiblePoint* point = std::get_if<CiblePoint>(&m_skillEnAttente->cible);
    if (point == nullptr) {
        return {};
    }
    const std::size_t place = m_skillEnAttente->place;
    if (!personnageDeLaPlace(place).estVivant() || !peutLancerAuPoint(place, point->point, carte)) {
        return {};
    }
    const std::vector<Coup> coups = lancerSkillDeZone(place, point->point, monstres, carte);
    m_skillEnAttente = std::nullopt;
    return coups;
}

bool Escouade::tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const {
    return std::all_of(m_personnages.begin(), m_personnages.end(), [&](const Personnage& personnage) {
        return !personnage.estVivant() || peutAtteindre(personnage, Arme::AttaqueDeBase, cible, carte);
    });
}

// Approche du monstre verrouillé, s'il y en a un dans la liste : mémorise son
// cercle, et l'ancre avance tant qu'un vivant ne l'atteint pas. Renvoie ce
// cercle ; vide sans verrou ou si le monstre est absent de la liste.
std::optional<Cercle> Escouade::approcherVerrou(const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    if (!m_cibleVerrouillee.has_value()) {
        return std::nullopt;
    }
    const MonstreVu* verrou = trouverMonstre(monstres, *m_cibleVerrouillee);
    if (verrou == nullptr) {
        return std::nullopt;
    }
    const Cercle cible = verrou->cercle;
    m_cibleCombat = cible;

    if (tousLesVivantsAtteignent(cible, carte)) {
        m_mobile.arreter();
    } else {
        allerVers(cible.centre, carte);
    }
    return cible;
}

// Skill de dégâts en attente : il part dès que son lanceur est à
// portée du cercle à jour de la cible, avant l'attaque de base
// (dont il relance la recharge).
std::optional<Coup> Escouade::departSkillDeDegatsEnAttente(Cercle cible, const TileMap& carte) {
    if (!skillEnAttenteDeDegats()) {
        return std::nullopt;
    }
    Personnage& lanceur = personnageDeLaPlace(m_skillEnAttente->place);
    if (!peutAtteindre(lanceur, Arme::Skill, cible, carte)) {
        return std::nullopt;
    }
    const int degats = lanceur.declencherSkill(cible);
    const int idCible = std::get<CibleMonstre>(m_skillEnAttente->cible).id;
    m_skillEnAttente = std::nullopt;
    if (degats <= 0) {
        return std::nullopt;
    }
    return Coup{idCible, degats};
}

// Chaque personnage choisit pour lui-même le monstre qu'il frappe, parmi
// ceux qu'il atteint (à portée et en vue).
std::vector<Coup> Escouade::autodefense(const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    std::vector<Coup> coups;
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        Personnage& personnage = m_personnages[i];
        const std::optional<int> id = choisirMonstreAFrapper(
            personnage.getCercle(), [&](Cercle c) { return peutAtteindre(personnage, Arme::AttaqueDeBase, c, carte); }, monstres, m_cibleVerrouillee);
        m_monstreAPortee[i] = id.has_value();
        if (!id.has_value()) {
            continue;
        }
        const MonstreVu* cible = trouverMonstre(monstres, *id);
        if (cible == nullptr) {
            continue;
        }
        const int degats = personnage.frapper(cible->cercle);
        if (degats > 0) {
            coups.push_back({*id, degats});
        }
    }
    return coups;
}

std::vector<Coup> Escouade::declencherSkill(std::size_t place, const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    if (!placeValide(place)) {
        return {};
    }

    Personnage& lanceur = personnageDeLaPlace(place);
    const std::optional<TypeSkill> type = lanceur.typeSkill();
    if (!type.has_value() || m_skillEnAttente.has_value() || !lanceur.skillEstPret()) {
        return {};
    }

    // Provocation : skill de zone centrée sur le lanceur, sans cible. Il
    // part dans l'image de l'appui sur tous les monstres touchés par la
    // zone et en vue du lanceur (centre à centre), même s'il n'y en a aucun
    // (la recharge est alors consommée).
    if (*type == TypeSkill::Provocation) {
        const Cercle zone = lanceur.zoneDuSkill();
        std::vector<MonstreVu> touches;
        for (const MonstreVu& monstre : monstres) {
            if (distanceBordABord(zone, monstre.cercle) == 0.0f && carte.ligneDeVue(lanceur.getCentre(), monstre.cercle.centre)) {
                touches.push_back(monstre);
            }
        }
        // Un seul déclenchement pour la recharge ; la puissance est la
        // durée.
        const int duree = lanceur.declencherSkillSansCible();
        for (const MonstreVu& monstre : touches) {
            provoquer(monstre.id, place, static_cast<float>(duree));
        }
        return {};
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
            // Dégâts : renvoyés à l'appelant, ou ajoutés aux coups d'attaquer()
            // si le skill part après une attente.
            [&](const CibleMonstre& monstre) -> std::vector<Coup> {
                if (peutAtteindre(lanceur, Arme::Skill, monstre.cercle, carte)) {
                    const int degats = lanceur.declencherSkill(monstre.cercle);
                    return degats > 0 ? std::vector<Coup>{{monstre.id, degats}} : std::vector<Coup>{};
                }
                m_skillEnAttente = SkillEnAttente{place, *cible};
                return {};
            },
            // Soin : appliqué ici, il ne sort pas d'Escouade.
            [&](const CibleAllie& allie) -> std::vector<Coup> {
                if (peutAtteindre(lanceur, Arme::Skill, personnageDeLaPlace(allie.place).getCercle(), carte)) {
                    soigner(place, allie.place);
                } else {
                    m_skillEnAttente = SkillEnAttente{place, *cible};
                }
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

bool Escouade::peutAtteindre(const Personnage& personnage, Arme arme, Cercle cible, const TileMap& carte) const {
    // À portée (bord à bord) ET en vue (centre à centre) : la seule règle d'atteinte d'Escouade.
    const bool aPortee = (arme == Arme::Skill) ? personnage.skillEstAPortee(cible) : personnage.estAPortee(cible);
    return aPortee && carte.ligneDeVue(personnage.getCentre(), cible.centre);
}

bool Escouade::peutLancerAuPoint(std::size_t place, sf::Vector2f point, const TileMap& carte) const {
    // Le point est un cercle de rayon 0 pour la portée (bord à bord).
    return peutAtteindre(personnageDeLaPlace(place), Arme::Skill, Cercle{point, 0.0f}, carte);
}

std::vector<Coup> Escouade::declencherSkillDeZone(std::size_t place, sf::Vector2f point, const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    if (!placeValide(place) || !skillEstPret(place) || m_skillEnAttente.has_value()) {
        return {};
    }

    Personnage& lanceur = personnageDeLaPlace(place);
    if (lanceur.typeSkill() != TypeSkill::Zone) {
        return {};
    }

    // Un point dans un mur est refusé : ni départ, ni attente, ni recharge.
    const sf::Vector2i coordCase = carte.caseContenant(point);
    if (carte.getTile(coordCase.x, coordCase.y) == Tile::Wall) {
        return {};
    }

    // À portée du point et en vue : le sort part tout de suite. Sinon il
    // est mis en attente, et le lanceur marche vers le point.
    if (peutLancerAuPoint(place, point, carte)) {
        return lancerSkillDeZone(place, point, monstres, carte);
    }
    m_skillEnAttente = SkillEnAttente{place, Cible(CiblePoint{point})};
    return {};
}

std::vector<Coup> Escouade::lancerSkillDeZone(std::size_t place, sf::Vector2f point, const std::vector<MonstreVu>& monstres, const TileMap& carte) {
    if (!placeValide(place)) {
        return {};
    }
    Personnage& lanceur = personnageDeLaPlace(place);

    // Un seul déclenchement pour la recharge ; la puissance est le nombre
    // de dégâts. La portée a déjà été vérifiée.
    const int degats = lanceur.declencherSkillSansCible();
    const Cercle zone{point, lanceur.zoneDuSkill().rayon};

    std::vector<Coup> coups;
    for (const MonstreVu& monstre : monstres) {
        // Dans la zone, et en vue du point d'impact (centre à centre) : le sort ne traverse pas les murs.
        if (distanceBordABord(zone, monstre.cercle) == 0.0f && carte.ligneDeVue(point, monstre.cercle.centre)) {
            coups.push_back({monstre.id, degats});
        }
    }
    return coups;
}

void Escouade::soigner(std::size_t placeLanceur, std::size_t placeAllie) {
    // Le skill rend 0 s'il ne part pas (lanceur mort, hors de portée, en recharge).
    const int soin = personnageDeLaPlace(placeLanceur).declencherSkill(personnageDeLaPlace(placeAllie).getCercle());
    personnageDeLaPlace(placeAllie).recevoirSoin(soin);
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

                if (peutAtteindre(lanceur, Arme::Skill, allie.getCercle(), carte)) {
                    soigner(placeLanceur, cibleAllie.place);
                    m_skillEnAttente = std::nullopt;
                    return std::nullopt;
                }

                return allie.getCentre();
            },
        },
        cible);
}

// Point à viser pour un personnage vivant qui a un agresseur dans sa laisse :
// son propre centre s'il a déjà un monstre à portée (il reste sur place),
// sinon le centre de l'agresseur le plus proche. Vide : il vise sa place.
std::optional<sf::Vector2f> Escouade::pointVersAgresseur(std::size_t indice, const std::vector<MonstreVu>& agresseurs) const {
    const std::optional<Cercle> cible = choisirAgresseurAViser(
        m_personnages[indice].getCercle(), m_places[indice], s_laisseAgresseur, agresseurs);
    if (!cible.has_value()) {
        return std::nullopt;
    }
    if (m_monstreAPortee[indice]) {
        return m_personnages[indice].getCentre();
    }
    return cible->centre;
}

// Point que vise un personnage : la première règle qui s'applique gagne.
// 1. le lanceur d'un skill en attente vise sa cible ;
// 2. en combat, un vivant déjà à portée de la cible reste sur place (les
//    tireurs s'arrêtent à distance pendant que les mêlées vont jusqu'au contact) ;
// 3. ancre à l'arrêt, un vivant qui a un agresseur dans sa laisse va vers lui
//    (l'ordre du joueur gagne : ancre en mouvement, il garde sa place) ;
// 4. sinon sa place en formation.
sf::Vector2f Escouade::pointVoulu(std::size_t indice, const std::optional<sf::Vector2f>& pointSkill, bool ancreArretee,
                                  const std::vector<MonstreVu>& agresseurs, const TileMap& carte) const {
    const Personnage& personnage = m_personnages[indice];
    if (pointSkill.has_value() && indice == indiceDeLaPlace(m_skillEnAttente->place)) {
        return *pointSkill;
    }
    const bool enCombat = m_cibleVerrouillee.has_value() && m_cibleCombat.has_value();
    if (enCombat && personnage.estVivant() && peutAtteindre(personnage, Arme::AttaqueDeBase, *m_cibleCombat, carte)) {
        return personnage.getCentre();
    }
    if (ancreArretee && personnage.estVivant()) {
        if (const std::optional<sf::Vector2f> pointAgresseur = pointVersAgresseur(indice, agresseurs)) {
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
// vers l'ancre jusqu'à un point libre.
void Escouade::calculerPlaces(const TileMap& carte) {
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        const float diametre = 2.0f * m_personnages[i].getCercle().rayon;
        m_places[i] = replierVersOrigine(carte, getCentre(), placePersonnage(i), {diametre, diametre});
    }
}

void Escouade::mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    if (const std::optional<sf::Vector2f> direction = m_mobile.directionVoulue()) {
        m_orientationVoulue = direction->angle();
    }

    ecoulerMemoires(dt);
    const std::vector<MonstreVu> agresseurs = m_agresseurs.monstresVus();  // une fois par image, pour tous les personnages

    // L'ancre n'est pas bloquée par les obstacles : seuls les personnages le sont.
    m_mobile.mettreAJour(dt, carte, {});
    const bool ancreArretee = !m_mobile.directionVoulue().has_value();

    tournerFormation(dt);
    calculerPlaces(carte);

    const std::optional<sf::Vector2f> pointSkill = avancerSkillEnAttente(carte);
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        m_personnages[i].mettreAJour(dt, pointVoulu(i, pointSkill, ancreArretee, agresseurs, carte), carte, obstacles);
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

std::optional<Cercle> Escouade::proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const {
    const std::optional<std::size_t> indice = indiceProieDuMonstre(idMonstre, positionMonstre);
    if (!indice.has_value()) {
        return std::nullopt;
    }
    return m_personnages[*indice].getCercle();
}

void Escouade::recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite) {
    const std::optional<std::size_t> indice = indiceProieDuMonstre(idMonstre, origine);
    if (!indice.has_value()) {
        return;
    }
    m_personnages[*indice].recevoirDegats(quantite);

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

std::optional<Cible> Escouade::choisirCible(TypeSkill type, std::size_t place, const std::vector<MonstreVu>& monstres, const TileMap& carte) const {
    // Skills de zone, sans cible.
    if (type == TypeSkill::Provocation || type == TypeSkill::Zone) {
        return std::nullopt;
    }
    if (type == TypeSkill::Degats) {
        // Avec un verrou : le monstre verrouillé, à portée ou non.
        if (m_cibleVerrouillee.has_value()) {
            const MonstreVu* verrou = trouverMonstre(monstres, *m_cibleVerrouillee);
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
        const std::optional<int> id = choisirMonstreAFrapper(
            lanceur.getCercle(), [&](Cercle c) { return peutAtteindre(lanceur, Arme::Skill, c, carte); }, monstres, std::nullopt);
        if (!id.has_value()) {
            return std::nullopt;
        }
        const MonstreVu* cible = trouverMonstre(monstres, *id);
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
    // Un skill de dégâts en attente ne survit pas à un changement de cible.
    if (m_cibleVerrouillee != idCible) {
        annulerSkillDeDegatsEnAttente();
    }
    m_cibleVerrouillee = idCible;
}

void Escouade::deverrouiller() {
    annulerSkillDeDegatsEnAttente();
    m_cibleVerrouillee = std::nullopt;
}

std::optional<int> Escouade::getCibleVerrouillee() const {
    return m_cibleVerrouillee;
}
