// ./src/Cercle.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

struct Cercle {
    sf::Vector2f centre;
    float rayon;
};

[[nodiscard]] float distanceBordABord(Cercle a, Cercle b);

// Distance du point `p` au segment [a, b] (au point le plus proche du segment, extrémités comprises).
[[nodiscard]] float distancePointSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b);

// Écart bord à bord entre un cercle et un rectangle plein : 0 en cas de contact ou de chevauchement, jamais négatif.
[[nodiscard]] float distanceCercleRectangle(Cercle cercle, sf::FloatRect rectangle);
