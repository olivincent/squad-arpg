// ./src/monde/Mobile.cpp
#include "monde/Mobile.hpp"
#include "monde/Chemin.hpp"
#include "monde/Deplacement.hpp"
#include "monde/TileMap.hpp"
#include <SFML/Graphics/Rect.hpp>

namespace {
constexpr float s_seuilMouvement = 0.01f;  // en pixels : en dessous, c'est de l'arrondi, pas un déplacement voulu
constexpr int s_penaliteCaseOccupee = 8;   // coût ajouté pour une case occupée : un détour d'environ 8 cases est préféré
constexpr float s_fractionDePasBloque = 0.1f;  // avancer de moins que cette fraction du pas = ne pas avancer
constexpr float s_tempsAvantBlocage = 0.3f;    // en secondes
}  // namespace

Mobile::Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse)
    : m_position(centre - taille / 2.0f),
      m_taille(taille),
      m_chemin({centre}),
      m_indexChemin(0),
      m_vitesse(vitesse),
      m_enMouvement(false),
      m_tempsBloque(0.0f)
{
}

sf::Vector2f Mobile::getCentre() const {
    return m_position + m_taille / 2.0f;
}

sf::Vector2f Mobile::getTaille() const {
    return m_taille;
}

Cercle Mobile::getCercle() const {
    return cercleDuRectangle(m_position, m_taille);
}

void Mobile::allerVers(sf::Vector2f destination, const TileMap& carte, const std::vector<Cercle>& aContourner) {
    // Même si aucun chemin n'existe : le propriétaire ne doit pas relancer le A* à chaque image.
    m_tempsBloque = 0.0f;

    const sf::Vector2i caseDepart = carte.caseContenant(getCentre());
    const sf::Vector2i caseArrivee = carte.caseContenant(destination);
    const std::vector<sf::Vector2i> chemin = trouverChemin(
        carte, caseDepart, caseArrivee, coutDesObstacles(carte, aContourner, m_taille.x / 2.0f, s_penaliteCaseOccupee));

    if (chemin.empty()) {
        return;
    }

    // La case de départ est ignorée : l'escouade est déjà dessus, la
    // viser d'abord la ferait reculer vers le centre de sa propre case.
    // Le dernier point est le point exact cliqué, pas le centre de la
    // case d'arrivée, pour que l'escouade s'arrête précisément dessus.
    std::vector<sf::Vector2f> points;
    for (std::size_t i = 1; i < chemin.size() - 1; ++i) {
        points.push_back(carte.centreDeCase(chemin[i]));
    }
    points.push_back(destination);

    m_chemin = lisserChemin(carte, getCentre(), points, m_taille, aContourner);
    m_indexChemin = 0;
}

void Mobile::allerToutDroit(sf::Vector2f destination) {
    m_chemin = {destination};
    m_indexChemin = 0;
    m_tempsBloque = 0.0f;
}

void Mobile::arreter() {
    m_chemin = {getCentre()};
    m_indexChemin = 0;
    m_enMouvement = false;
    m_tempsBloque = 0.0f;
}

void Mobile::mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    sf::Vector2f deplacement(0.0f, 0.0f);
    m_enMouvement = false;

    if (m_indexChemin < m_chemin.size()) {
        const sf::Vector2f pointCourant = m_chemin[m_indexChemin];
        const sf::Vector2f direction = pointCourant - getCentre();
        const float distance = direction.length();
        m_enMouvement = distance > s_seuilMouvement;
        const float pasDuTic = m_vitesse * dt;

        if (distance <= pasDuTic) {
            deplacement = direction;
            ++m_indexChemin;
        } else {
            deplacement = direction.normalized() * pasDuTic;
        }
    }

    const sf::Vector2f positionAvant = m_position;
    m_position = deplacerAvecGlissement(carte, sf::FloatRect(m_position, m_taille), deplacement, obstacles);

    const bool avance = (m_position - positionAvant).length() >= s_fractionDePasBloque * m_vitesse * dt;
    m_tempsBloque = (m_enMouvement && !avance) ? m_tempsBloque + dt : 0.0f;
}

void Mobile::decaler(sf::Vector2f decalage, const TileMap& carte, const std::vector<Cercle>& obstacles) {
    m_position = deplacerAvecGlissement(carte, sf::FloatRect(m_position, m_taille), decalage, obstacles);
}

std::optional<sf::Vector2f> Mobile::directionVoulue() const {
    if (m_indexChemin >= m_chemin.size()) {
        return std::nullopt;
    }

    const sf::Vector2f direction = m_chemin[m_indexChemin] - getCentre();
    if (direction.length() <= 0.0f) {
        return std::nullopt;
    }

    return direction;
}

bool Mobile::estEnMouvement() const {
    return m_enMouvement;
}

bool Mobile::estBloque() const {
    return m_tempsBloque >= s_tempsAvantBlocage;
}

std::vector<sf::Vector2f> Mobile::cheminRestant() const {
    if (m_indexChemin >= m_chemin.size()) {
        return {};
    }
    return {m_chemin.begin() + static_cast<std::ptrdiff_t>(m_indexChemin), m_chemin.end()};
}
