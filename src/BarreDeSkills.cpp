// ./src/BarreDeSkills.cpp
#include "BarreDeSkills.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace {

// Point du bord du carré dans la direction d'un angle mesuré depuis midi,
// en sens horaire, en degrés (y vers le bas).
sf::Vector2f pointDuBord(sf::Vector2f centre, float demiCote, float angleDegres) {
    const float angle = angleDegres * std::numbers::pi_v<float> / 180.0f;
    const sf::Vector2f direction{std::sin(angle), -std::cos(angle)};
    const float facteur = demiCote / std::max(std::abs(direction.x), std::abs(direction.y));
    return centre + direction * facteur;
}

}  // namespace

char lettreDuSkill(TypeSkill type) {
    switch (type) {
        case TypeSkill::Degats:
            return 'D';
        case TypeSkill::Provocation:
            return 'P';
        case TypeSkill::Soin:
            return 'S';
        case TypeSkill::Zone:
            return 'Z';
    }
    return '?';
}

int secondesAffichees(float tempsRestant) {
    return static_cast<int>(std::ceil(tempsRestant));
}

std::vector<sf::Vector2f> pointsDuQuartier(sf::Vector2f centre, float cote, float fractionSombre) {
    const float fraction = std::clamp(fractionSombre, 0.0f, 1.0f);
    if (fraction <= 0.0f) {
        return {};
    }

    const float demiCote = cote / 2.0f;
    // La zone sombre va de l'aiguille jusqu'à midi (360°).
    const float angleAiguille = (1.0f - fraction) * 360.0f;

    std::vector<sf::Vector2f> points = {centre, pointDuBord(centre, demiCote, angleAiguille)};
    const std::array<float, 4> anglesDesCoins = {45.0f, 135.0f, 225.0f, 315.0f};
    for (const float angleCoin : anglesDesCoins) {
        if (angleCoin > angleAiguille) {
            points.push_back(pointDuBord(centre, demiCote, angleCoin));
        }
    }
    points.push_back({centre.x, centre.y - demiCote});
    return points;
}
