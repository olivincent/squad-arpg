// ./src/Cercle.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

struct Cercle {
    sf::Vector2f centre;
    float rayon;
};

[[nodiscard]] float distanceBordABord(Cercle a, Cercle b);

// Distance du point `p` au segment [a, b] (au point le plus proche du segment, extrémités comprises).
[[nodiscard]] float distancePointSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b);
