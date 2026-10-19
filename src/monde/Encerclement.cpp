// ./src/monde/Encerclement.cpp
#include "monde/Encerclement.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

std::vector<sf::Angle> anglesCandidats(sf::Angle depart, float rayon, float ecartMin) {
    // Corde entre voisines = 2 r sin(π / n) >= ecartMin  <=>  n <= π / asin(ecartMin / 2r).
    std::size_t nombre = 1;
    const float rapport = ecartMin / (2.0f * rayon);
    if (rapport <= 1.0f) {
        nombre = static_cast<std::size_t>(std::numbers::pi_v<float> / std::asin(rapport));
    }
    const sf::Angle pas = sf::degrees(360.0f / static_cast<float>(nombre));

    std::vector<sf::Angle> angles;
    angles.reserve(nombre);
    for (std::size_t i = 0; i < nombre; ++i) {
        const float rang = static_cast<float>((i + 1) / 2);
        angles.push_back((i % 2 == 1) ? depart + pas * rang : depart - pas * rang);
    }
    return angles;
}

sf::Vector2f positionDePlace(sf::Vector2f centre, float rayon, sf::Angle angle) {
    return centre + sf::Vector2f(rayon * std::cos(angle.asRadians()), rayon * std::sin(angle.asRadians()));
}

std::optional<sf::Angle> choisirAngleLibre(const std::vector<sf::Angle>& candidats,
                                           sf::Angle angleActuel,
                                           const std::function<bool(sf::Angle)>& estLibreEtValide) {
    std::optional<sf::Angle> meilleur;
    float meilleurEcart = 0.0f;
    for (const sf::Angle candidat : candidats) {
        if (!estLibreEtValide(candidat)) {
            continue;
        }
        const float ecart = std::abs((candidat - angleActuel).wrapSigned().asDegrees());
        if (!meilleur || ecart < meilleurEcart) {
            meilleur = candidat;
            meilleurEcart = ecart;
        }
    }
    return meilleur;
}
