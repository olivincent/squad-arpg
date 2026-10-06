// ./src/Monstre.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

class Monstre {
public:
    explicit Monstre(sf::Vector2f centre);

private:
    sf::Vector2f m_centre;  // pixels du monde
    sf::Vector2f m_taille;  // en pixels
    int m_pointsDeVie;
};
