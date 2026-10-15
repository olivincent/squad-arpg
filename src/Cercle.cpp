// ./src/Cercle.cpp
#include "Cercle.hpp"
#include <algorithm>

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
