// ./src/Escouade.cpp
#include "Escouade.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>
#include <SFML/Graphics/Color.hpp>
#include "Personnage.hpp"
#include "Attaque.hpp"

namespace {
const sf::Color s_couleurMelee = sf::Color::Red;
const sf::Color s_couleurTireur = sf::Color::Cyan;
constexpr float s_porteeMelee = 10.0f;    // bord à bord
constexpr float s_porteeTireur = 120.0f;  // bord à bord
constexpr int s_degatsSkill = 25;
constexpr float s_rechargeSkill = 5.0f;   // en secondes
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
          Personnage(placePersonnage(2), s_couleurTireur, Attaque(s_porteeTireur, 5, 0.5f), std::nullopt),
          Personnage(placePersonnage(3), s_couleurTireur, Attaque(s_porteeTireur, 5, 0.5f), std::nullopt),
      }),
      m_cibleVerrouillee(),
      m_cibleCombat()
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

int Escouade::attaquer(Cercle cible, const TileMap& carte) {
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

    int degats = 0;
    for (Personnage& personnage : m_personnages) {
        degats += personnage.frapper(cible);
    }
    return degats;
}

int Escouade::declencherSkill(std::size_t place, Cercle cible) {
    if (!m_cibleVerrouillee.has_value() || place < 1 || place > m_personnages.size()) {
        return 0;
    }

    // Un personnage sans skill renvoie 0.
    return m_personnages[place - 1].declencherSkill(cible);
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
    for (std::size_t i = 0; i < m_personnages.size(); ++i) {
        sf::Vector2f pointVoulu = placePersonnage(i);
        if (enCombat && m_personnages[i].estVivant() && m_personnages[i].estAPortee(*m_cibleCombat)) {
            pointVoulu = m_personnages[i].getCentre();
        }
        m_personnages[i].mettreAJour(dt, pointVoulu, carte, obstacles);
    }
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

std::optional<std::size_t> Escouade::choisirAlliePourSoin() const {
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
    return *indiceChoisi + 1;  // place de 1 à 4, comme declencherSkill
}

void Escouade::verrouiller(int idCible) {
    m_cibleVerrouillee = idCible;
}

void Escouade::deverrouiller() {
    m_cibleVerrouillee = std::nullopt;
}

std::optional<int> Escouade::getCibleVerrouillee() const {
    return m_cibleVerrouillee;
}
