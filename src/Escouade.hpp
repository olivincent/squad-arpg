// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class Escouade {
public:
    explicit Escouade(sf::Vector2f positionDepart);
    void dessiner(sf::RenderTarget& rendu) const;

private:
    sf::Vector2f m_position;    // coin haut-gauche, en pixels du monde
    sf::Vector2f m_taille;      // en pixels
    sf::Vector2f m_cible;       // point du monde vers lequel on se déplace
    float m_vitesse;            // en pixels par seconde
};