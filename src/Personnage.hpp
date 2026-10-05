// ./src/Personnage.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

class Personnage {
public:
    Personnage(sf::Vector2f centre, sf::Color couleur);
    void dessiner(sf::RenderTarget& rendu) const;
    void mettreAJour(float dt, sf::Vector2f cible);

private:
    sf::Vector2f m_centre;  // pixels du monde
    sf::Vector2f m_taille;  // en pixels
    sf::Color m_couleur;
    float m_vitesse;        // en pixels par seconde
};
