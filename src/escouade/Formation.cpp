// ./src/escouade/Formation.cpp
#include "escouade/Formation.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <utility>
#include "escouade/EcartEntrePersonnages.hpp"
#include "monde/Chemin.hpp"
#include "monde/Encerclement.hpp"
#include "monde/TileMap.hpp"

namespace {
constexpr std::array<float, 3> s_rayonsDeSecoursPlaces = {34.0f, 68.0f, 102.0f};  // anneaux autour de l'ancre pour une place repliée sur une autre
}  // namespace

Formation::Formation(std::vector<sf::Vector2f> decalages, sf::Vector2f centre, float vitesseRotation)
    : m_decalages(std::move(decalages)),
      m_orientation(),
      m_orientationVoulue(),
      m_pointARegarder(),
      m_vitesseRotation(vitesseRotation),
      m_places()
{
    for (std::size_t i = 0; i < m_decalages.size(); ++i) {
        m_places.push_back(placeTheorique(i, centre));
    }
}

sf::Vector2f Formation::placeTheorique(std::size_t indice, sf::Vector2f centre) const {
    return centre + m_decalages[indice].rotatedBy(m_orientation);
}

void Formation::regarderVers(sf::Vector2f point) {
    m_pointARegarder = point;
}

void Formation::oublierPointARegarder() {
    m_pointARegarder.reset();
}

void Formation::choisirOrientation(std::optional<sf::Vector2f> directionDeMarche, sf::Vector2f centre) {
    if (directionDeMarche.has_value()) {
        m_orientationVoulue = directionDeMarche->angle();
    } else if (m_pointARegarder.has_value()) {
        const sf::Vector2f vers = *m_pointARegarder - centre;
        if (vers.length() > 0.0f) {
            m_orientationVoulue = vers.angle();
        }
    }
}

void Formation::mettreAJour(float dt, sf::Vector2f centre, const std::vector<float>& rayons, const TileMap& carte,
                            const std::vector<Cercle>& monstres) {
    tourner(dt);
    calculerPlaces(centre, rayons, carte, monstres);
}

const std::vector<sf::Vector2f>& Formation::getPlaces() const {
    return m_places;
}

// Rotation progressive vers l'orientation voulue, par le plus court
// côté (wrapSigned ramène l'écart entre -180° et 180°), à vitesse
// angulaire bornée, que l'ancre bouge ou non.
void Formation::tourner(float dt) {
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
void Formation::calculerPlaces(sf::Vector2f centre, const std::vector<float>& rayons, const TileMap& carte,
                               const std::vector<Cercle>& monstres) {
    assert(rayons.size() == m_places.size());
    for (std::size_t i = 0; i < m_places.size(); ++i) {
        const float rayonPersonnage = rayons[i];
        const float diametre = 2.0f * rayonPersonnage;
        const sf::Vector2f taille(diametre, diametre);
        const sf::Vector2f theorique = placeTheorique(i, centre);
        m_places[i] = replierVersOrigine(carte, centre, theorique, taille);

        const auto estLibre = [&](sf::Vector2f point) {
            for (std::size_t autre = 0; autre < i; ++autre) {
                if ((point - m_places[autre]).length() < s_ecartEntrePersonnages) {
                    return false;
                }
            }
            return true;
        };
        const auto estDansUnMonstre = [&](sf::Vector2f point) {
            return std::ranges::any_of(monstres, [&](const Cercle& monstre) {
                return (point - monstre.centre).length() < monstre.rayon + rayonPersonnage;
            });
        };
        const sf::Vector2f replie = m_places[i];
        const bool replieEstLibre = estLibre(replie);
        if (replieEstLibre && !estDansUnMonstre(replie)) {
            continue;
        }

        const sf::Vector2f direction = theorique - centre;
        const sf::Angle depart = direction.length() > 0.0f ? direction.angle() : sf::degrees(0.0f);
        const auto estValide = [&](sf::Vector2f point) {
            return estLibre(point) && !estDansUnMonstre(point) &&
                   !carte.chevaucheUnMur(sf::FloatRect(point - taille / 2.0f, taille)) &&
                   carte.trajetLibre(sf::FloatRect(centre - taille / 2.0f, taille), point - centre);
        };
        bool trouve = false;
        m_places[i] = replieEstLibre ? replie : centre;
        for (const float rayon : s_rayonsDeSecoursPlaces) {
            for (const sf::Angle angle : anglesCandidats(depart, rayon, s_ecartEntrePersonnages)) {
                const sf::Vector2f point = positionDePlace(centre, rayon, angle);
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
