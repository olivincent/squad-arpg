// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

class Escouade {
private:
    sf::Vector2f m_position;    // coin haut-gauche, en pixels du monde
    sf::Vector2f m_taille;      // en pixels
    sf::Vector2f m_cible;       // point du monde vers lequel on se déplace
    float m_vitesse;            // en pixels par seconde
};