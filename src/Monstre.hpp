// ./src/Monstre.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

class Monstre {
public:
    explicit Monstre(sf::Vector2f centre);
    void dessiner(sf::RenderTarget& rendu) const;
    bool contient(sf::Vector2f point) const;
    void recevoirDegats(int quantite);
    bool estVivant() const;

private:
    sf::Vector2f m_centre;  // pixels du monde
    sf::Vector2f m_taille;  // en pixels
    int m_pointsDeVie;
};
