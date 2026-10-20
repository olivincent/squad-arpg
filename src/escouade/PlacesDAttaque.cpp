// ./src/escouade/PlacesDAttaque.cpp
#include "escouade/PlacesDAttaque.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <cassert>
#include "monde/ChampDeVision.hpp"
#include "monde/Encerclement.hpp"
#include "monde/TileMap.hpp"

namespace {
constexpr float s_fractionPorteeAnneau = 0.9f;    // anneau d'attaque : part de la portée, pour que la place soit à portée
}  // namespace

PlacesDAttaque::PlacesDAttaque(std::size_t nombreDAssaillants, float ecartMinimal)
    : m_places(nombreDAssaillants),
      m_ecartMinimal(ecartMinimal)
{
}

std::optional<sf::Vector2f> PlacesDAttaque::placer(const Approche& approche, const std::optional<Place>& ancienne,
                                                   const std::vector<std::optional<sf::Vector2f>>& occupants,
                                                   const TileMap& carte) {
    const std::optional<Place> place = trouverPlace(approche, ancienne, occupants, carte);
    if (!place.has_value()) {
        return std::nullopt;
    }
    m_places[approche.assaillant] = place;
    return positionDePlace(approche.cible.cercle.centre, place->rayon, place->angle);
}

bool PlacesDAttaque::peutPlacer(const Approche& approche, const std::optional<Place>& ancienne,
                                const std::vector<std::optional<sf::Vector2f>>& occupants, const TileMap& carte) const {
    return trouverPlace(approche, ancienne, occupants, carte).has_value();
}

std::optional<PlacesDAttaque::Place> PlacesDAttaque::placeRetenue(std::size_t assaillant) const {
    return m_places.at(assaillant);
}

void PlacesDAttaque::oublierSaufAutourDe(std::size_t assaillant, std::optional<int> idCible) {
    std::optional<Place>& place = m_places.at(assaillant);
    if (place.has_value() && place->idCible != idCible) {
        place.reset();
    }
}

// La place est un point où l'on marche : l'assaillant s'arrête dès que la cible est à portée et en
// vue (règle de l'appelant). Seule la position suit la cible ; l'angle est gardé tant que la place
// reste valide et pas occupée. Les candidates partent de la direction de la cible vers `reference`.
std::optional<PlacesDAttaque::Place> PlacesDAttaque::trouverPlace(
    const Approche& approche, const std::optional<Place>& ancienne,
    const std::vector<std::optional<sf::Vector2f>>& occupants, const TileMap& carte) const {
    assert(approche.assaillant < m_places.size());
    assert(occupants.size() == m_places.size());
    const std::size_t indice = approche.assaillant;
    const int idCible = approche.cible.id;
    const Cercle cible = approche.cible.cercle;
    const float rayonAssaillant = approche.cercleAssaillant.rayon;
    const float rayonAnneau = cible.rayon + rayonAssaillant + s_fractionPorteeAnneau * approche.portee;

    const auto angleVers = [&](sf::Vector2f point) {
        const sf::Vector2f direction = point - cible.centre;
        return direction.length() > 0.0f ? direction.angle() : sf::degrees(0.0f);
    };
    const auto estValide = [&](sf::Angle angle) {
        const sf::Vector2f place = positionDePlace(cible.centre, rayonAnneau, angle);
        const sf::Vector2f demiCote(rayonAssaillant, rayonAssaillant);
        return !carte.chevaucheUnMur(sf::FloatRect(place - demiCote, 2.0f * demiCote)) &&
               ligneDeVue(carte, place, cible.centre);
    };

    // Occupée : un autre occupant se tient déjà à cet endroit.
    const auto estOccupee = [&](sf::Vector2f place) {
        for (std::size_t autre = 0; autre < occupants.size(); ++autre) {
            const std::optional<sf::Vector2f>& occupant = occupants[autre];
            if (autre != indice && occupant.has_value() && (*occupant - place).length() <= m_ecartMinimal) {
                return true;
            }
        }
        return false;
    };

    std::optional<sf::Angle> angle;
    if (ancienne.has_value() && ancienne->idCible == idCible && estValide(ancienne->angle) &&
        !estOccupee(positionDePlace(cible.centre, rayonAnneau, ancienne->angle))) {
        angle = ancienne->angle;
    } else {
        const auto estLibreEtValide = [&](sf::Angle candidat) {
            if (!estValide(candidat)) {
                return false;
            }
            const sf::Vector2f place = positionDePlace(cible.centre, rayonAnneau, candidat);
            if (estOccupee(place)) {
                return false;
            }
            for (std::size_t autre = 0; autre < m_places.size(); ++autre) {
                const std::optional<Place>& placeDeLAutre = m_places[autre];
                if (autre == indice || !placeDeLAutre.has_value() || placeDeLAutre->idCible != idCible) {
                    continue;
                }
                const sf::Vector2f positionDeLAutre = positionDePlace(cible.centre, placeDeLAutre->rayon, placeDeLAutre->angle);
                if ((place - positionDeLAutre).length() <= m_ecartMinimal) {
                    return false;
                }
            }
            return true;
        };
        angle = choisirAngleLibre(anglesCandidats(angleVers(approche.reference), rayonAnneau, m_ecartMinimal),
                                  angleVers(approche.cercleAssaillant.centre), estLibreEtValide);
    }

    if (!angle.has_value()) {
        return std::nullopt;
    }
    return Place{idCible, rayonAnneau, *angle};
}
