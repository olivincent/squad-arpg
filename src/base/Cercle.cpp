// ./src/base/Cercle.cpp
#include "base/Cercle.hpp"
#include <algorithm>

Cercle cercleDuRectangle(sf::Vector2f coin, sf::Vector2f taille) {
    return Cercle{coin + taille / 2.0f, taille.x / 2.0f};
}

float distanceBordABord(Cercle a, Cercle b) {
    const float distanceCentres = (b.centre - a.centre).length();
    const float ecart = distanceCentres - a.rayon - b.rayon;
    return (ecart < 0.0f) ? 0.0f : ecart;
}

float distancePointSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) {
    const sf::Vector2f segment = b - a;
    const float longueurCarree = segment.lengthSquared();
    if (longueurCarree == 0.0f) {
        return (p - a).length();
    }
    // Fraction du segment où tombe la projection de p, ramenée dans [0, 1].
    const float t = std::clamp((p - a).dot(segment) / longueurCarree, 0.0f, 1.0f);
    return (p - (a + segment * t)).length();
}

float distanceCercleRectangle(Cercle cercle, sf::FloatRect rectangle) {
    // Point du rectangle le plus proche du centre ; le centre dedans donne une distance nulle.
    const sf::Vector2f plusProche(
        std::clamp(cercle.centre.x, rectangle.position.x, rectangle.position.x + rectangle.size.x),
        std::clamp(cercle.centre.y, rectangle.position.y, rectangle.position.y + rectangle.size.y));
    const float ecart = (cercle.centre - plusProche).length() - cercle.rayon;
    return (ecart < 0.0f) ? 0.0f : ecart;
}
