// ./src/Cercle.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

struct Cercle {
    sf::Vector2f centre;
    float rayon;
};

float distanceBordABord(const Cercle& a, const Cercle& b);
