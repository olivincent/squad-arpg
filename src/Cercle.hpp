// ./src/Cercle.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

struct Cercle {
    sf::Vector2f centre;
    float rayon;
};

[[nodiscard]] float distanceBordABord(Cercle a, Cercle b);
