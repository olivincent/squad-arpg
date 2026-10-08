// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cmath>
#include <SFML/Graphics/Color.hpp>
#include <variant>
#include "Personnage.hpp"
#include "Attaque.hpp"
#include "Autodefense.hpp"
#include "Chemin.hpp"

namespace {
const sf::Color s_couleurMelee = sf::Color::Red;
const sf::Color s_couleurTireur = sf::Color::Cyan;
constexpr float s_porteeMelee = 20.0f;    // bord à bord
constexpr float s_porteeTireur = 120.0f;  // bord à bord
constexpr int s_degatsSkill = 25;
constexpr float s_rechargeSkill = 5.0f;   // en secondes
constexpr float s_porteeSoin = 40.0f;     // bord à bord
constexpr int s_puissanceSoin = 30;
constexpr float s_rechargeSoin = 8.0f;    // en secondes
}  // namespace

Escouade::Escouade(sf::Vector2f positionDepart)
    : m_mobile(positionDepart + sf::Vector2f(10.0f, 10.0f), {20.0f, 20.0f}, 180.0f),
      m_decalages({{25.0f, -25.0f}, {25.0f, 25.0f}, {-25.0f, -25.0f}, {-25.0f, 25.0f}}),
      m_orientation(),
      m_orientationVoulue(),
      m_vitesseRotation(360.0f),
      m_personnages({
          Personnage(placePersonnage(0), s_couleurMelee, Attaque(s_porteeMelee, 5, 0.5f),
                     Skill(TypeSkill::Degats, Attaque(s_porteeMelee, s_degatsSkill, s_rechargeSkill))),
          Personnage(placePersonnage(1), s_couleurMelee, Attaque(s_porteeMelee, 5, 0.5f), std::nullopt),
          Personnage(placePersonnage(2), s_couleurTireur, Attaque(s_porteeTireur, 5, 0.5f),
                     Skill(TypeSkill::Soin, Attaque(s_porteeSoin, s_puissanceSoin, s_rechargeSoin))),
          Personnage(placePersonnage(3), s_couleurTireur, Attaque(s_porteeTireur, 5, 0.5f), std::nullopt),
      }),
      m_cibleVerrouillee(),
      m_skillEnAttente(),
      m_cibleCombat(),
      m_places({placePersonnage(0), placePersonnage(1), placePersonnage(2), placePersonnage(3)})
{
}

sf::Vector2f Escouade::placePersonnage(std::size_t indice) const {
    return getCentre() + m_decalages[indice].rotatedBy(m_orientation);
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
    std::vector<Coup> coups;

    // Approche du monstre verrouillé, s'il y en a un dans la liste.
    if (m_cibleVerrouillee.has_value()) {
        const auto verrou = std::find_if(monstres.begin(), monstres.end(), [&](const MonstreVu& m) {
            return m.id == *m_cibleVerrouillee;
        });
        if (verrou != monstres.end()) {
            const Cercle cible = verrou->cercle;
            m_cibleCombat = cible;

            bool tousAPortee = true;
            for (const Personnage& personnage : m_personnages) {
                if (personnage.estVivant() && !personnage.estAPortee(cible)) {
                    tousAPortee = false;
                    break;
                }
            }

            if (tousAPortee) {
                m_mobile.arreter();
            } else {
                allerVers(cible.centre, carte);
            }

            // Skill de dégâts en attente : il part dès que son lanceur est à
            // portée du cercle à jour de la cible, avant l'attaque de base
            // (dont il relance la recharge).
            if (m_skillEnAttente.has_value() && std::holds_alternative<CibleMonstre>(m_skillEnAttente->cible)) {
                Personnage& lanceur = m_personnages[m_skillEnAttente->place - 1];
                if (lanceur.skillEstAPortee(cible)) {
                    const int degats = lanceur.declencherSkill(cible);
                    if (degats > 0) {
                        coups.push_back({std::get<CibleMonstre>(m_skillEnAttente->cible).id, degats});
                    }
                    m_skillEnAttente = std::nullopt;
                }
            }
        }
    }

    // Chaque personnage choisit pour lui-même le monstre qu'il frappe.
    for (Personnage& personnage : m_personnages) {
        const std::optional<int> id = choisirMonstreAFrapper(
            personnage.getCercle(), [&](Cercle c) { return personnage.estAPortee(c); }, monstres, m_cibleVerrouillee);
        if (!id.has_value()) {
            continue;
        }
        const auto cible = std::find_if(monstres.begin(), monstres.end(), [&](const MonstreVu& m) {
            return m.id == *id;
        });
        const int degats = personnage.frapper(cible->cercle);
        if (degats > 0) {
            coups.push_back({*id, degats});
        }
    }
    return coups;
}

std::vector<Coup> Escouade::declencherSkill(std::size_t place, const std::vector<MonstreVu>& monstres) {
    if (place < 1 || place > m_personnages.size()) {
        return {};
    }

    Personnage& lanceur = m_personnages[place - 1];
    const std::optional<TypeSkill> type = lanceur.typeSkill();
    if (!type.has_value() || m_skillEnAttente.has_value() || !lanceur.skillEstPret()) {
        return {};
    }

    const std::optional<Cible> cible = choisirCible(*type, place, monstres);
    if (!cible.has_value()) {
        return {};
    }

    // Dégâts : renvoyés à l'appelant, ou ajoutés aux coups d'attaquer()
    // si le skill part après une attente.
    if (const CibleMonstre* monstre = std::get_if<CibleMonstre>(&*cible)) {
        if (lanceur.skillEstAPortee(monstre->cercle)) {
            const int degats = lanceur.declencherSkill(monstre->cercle);
            return degats > 0 ? std::vector<Coup>{{monstre->id, degats}} : std::vector<Coup>{};
        }
        m_skillEnAttente = SkillEnAttente{place, *cible};
        return {};
    }

    // Soin : appliqué ici, il ne sort pas d'Escouade.
    const std::size_t placeAllie = std::get<CibleAllie>(*cible).place;
    if (lanceur.skillEstAPortee(m_personnages[placeAllie - 1].getCercle())) {
        soigner(place, placeAllie);
    } else {
        m_skillEnAttente = SkillEnAttente{place, *cible};
    }
    return {};
}

void Escouade::soigner(std::size_t placeLanceur, std::size_t placeAllie) {
    // Le skill rend 0 s'il ne part pas (lanceur mort, hors de portée, en recharge).
    const int soin = m_personnages[placeLanceur - 1].declencherSkill(m_personnages[placeAllie - 1].getCercle());
    m_personnages[placeAllie - 1].recevoirSoin(soin);
}

std::optional<sf::Vector2f> Escouade::avancerSkillEnAttente() {
    if (!m_skillEnAttente.has_value()) {
        return std::nullopt;
    }

    const std::size_t placeLanceur = m_skillEnAttente->place;
    const Personnage& lanceur = m_personnages[placeLanceur - 1];

    if (!lanceur.estVivant()) {
        m_skillEnAttente = std::nullopt;
        return std::nullopt;
    }

    // Dégâts : le skill part dans attaquer(), qui reçoit le cercle à jour du
    // monstre. Ici, le lanceur marche vers ce cercle. L'annulation (combat
    // rompu) se fait dans verrouiller() et deverrouiller().
    if (std::holds_alternative<CibleMonstre>(m_skillEnAttente->cible)) {
        if (!m_cibleCombat.has_value()) {
            return std::nullopt;
        }
        return m_cibleCombat->centre;
    }

    // Soin : annulation si la cible est morte ou remontée à pleine vie.
    const std::size_t placeAllie = std::get<CibleAllie>(m_skillEnAttente->cible).place;
    const Personnage& allie = m_personnages[placeAllie - 1];
    if (!allie.estVivant() || allie.getPointsDeVie() >= allie.getPointsDeVieMax()) {
        m_skillEnAttente = std::nullopt;
        return std::nullopt;
    }

    if (lanceur.skillEstAPortee(allie.getCercle())) {
        soigner(placeLanceur, placeAllie);
        m_skillEnAttente = std::nullopt;
        return std::nullopt;
    }

    return allie.getCentre();
}

void Escouade::mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    if (const std::optional<sf::Vector2f> direction = m_mobile.directionVoulue()) {
        m_orientationVoulue = direction->angle();
    }

    // L'ancre n'est pas bloquée par les obstacles : seuls les personnages le sont.
    m_mobile.mettreAJour(dt, carte, {});

    // Rotation progressive vers l'orientation voulue, par le plus court
    // côté (wrapSigned ramène l'écart entre -180° et 180°), à vitesse
    // angulaire bornée, que l'ancre bouge ou non.
    const sf::Angle ecart = (m_orientationVoulue - m_orientation).wrapSigned();
    const float pasRotation = m_vitesseRotation * dt;
    if (std::abs(ecart.asDegrees()) <= pasRotation) {
        m_orientation = m_orientationVoulue;
    } else {
        const float signe = (ecart.asDegrees() > 0.0f) ? 1.0f : -1.0f;
        m_orientation += sf::degrees(pasRotation * signe);
    }

    // En combat (verrou posé), un personnage déjà à portée de la cible
    // reste sur place plutôt que de suivre sa place en formation, pour
    // que les tireurs s'arrêtent à distance pendant que les mêlées
    // continuent jusqu'au contact.
    const bool enCombat = m_cibleVerrouillee.has_value() && m_cibleCombat.has_value();

    // Un lanceur dont le skill est en attente vise sa cible au lieu de sa place.
    const std::size_t placeLanceur = m_skillEnAttente.has_value() ? m_skillEnAttente->place : 0;
    const std::optional<sf::Vector2f> pointSkill = avancerSkillEnAttente();

    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        // Une place théorique dans un mur ou derrière un coin est ramenée
        // vers l'ancre jusqu'à un point libre.
        const float diametre = 2.0f * m_personnages[i].getCercle().rayon;
        m_places[i] = replierVersOrigine(carte, getCentre(), placePersonnage(i), {diametre, diametre});
        sf::Vector2f pointVoulu = m_places[i];
        if (enCombat && m_personnages[i].estVivant() && m_personnages[i].estAPortee(*m_cibleCombat)) {
            pointVoulu = m_personnages[i].getCentre();
        }
        if (pointSkill.has_value() && i == placeLanceur - 1) {
            pointVoulu = *pointSkill;
        }
        m_personnages[i].mettreAJour(dt, pointVoulu, carte, obstacles);
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

std::optional<Cercle> Escouade::personnageLePlusProche(sf::Vector2f point) const {
    const std::optional<std::size_t> indice = indicePersonnageLePlusProche(point);
    if (!indice.has_value()) {
        return std::nullopt;
    }
    return m_personnages[*indice].getCercle();
}

void Escouade::recevoirDegats(sf::Vector2f origine, int quantite) {
    const std::optional<std::size_t> indice = indicePersonnageLePlusProche(origine);
    if (!indice.has_value()) {
        return;
    }
    m_personnages[*indice].recevoirDegats(quantite);
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

std::optional<Cible> Escouade::choisirCible(TypeSkill type, std::size_t place, const std::vector<MonstreVu>& monstres) const {
    if (type == TypeSkill::Degats) {
        // Avec un verrou : le monstre verrouillé, à portée ou non.
        if (m_cibleVerrouillee.has_value()) {
            const auto verrou = std::find_if(monstres.begin(), monstres.end(), [&](const MonstreVu& m) {
                return m.id == *m_cibleVerrouillee;
            });
            if (verrou == monstres.end()) {
                return std::nullopt;
            }
            return Cible(CibleMonstre{verrou->id, verrou->cercle});
        }

        // Sans verrou : le monstre que le lanceur frapperait en autodéfense,
        // mais avec la portée de son skill.
        if (place < 1 || place > m_personnages.size()) {
            return std::nullopt;
        }
        const Personnage& lanceur = m_personnages[place - 1];
        const std::optional<int> id = choisirMonstreAFrapper(
            lanceur.getCercle(), [&](Cercle c) { return lanceur.skillEstAPortee(c); }, monstres, std::nullopt);
        if (!id.has_value()) {
            return std::nullopt;
        }
        const auto cible = std::find_if(monstres.begin(), monstres.end(), [&](const MonstreVu& m) {
            return m.id == *id;
        });
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
    if (m_cibleVerrouillee != idCible && m_skillEnAttente.has_value()
        && std::holds_alternative<CibleMonstre>(m_skillEnAttente->cible)) {
        m_skillEnAttente = std::nullopt;
    }
    m_cibleVerrouillee = idCible;
}

void Escouade::deverrouiller() {
    if (m_skillEnAttente.has_value() && std::holds_alternative<CibleMonstre>(m_skillEnAttente->cible)) {
        m_skillEnAttente = std::nullopt;
    }
    m_cibleVerrouillee = std::nullopt;
}

std::optional<int> Escouade::getCibleVerrouillee() const {
    return m_cibleVerrouillee;
}
