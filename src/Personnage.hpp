// ./src/Personnage.hpp
#pragma once
#include "Attaque.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

class TileMap;

class Personnage {
public:
    Personnage(sf::Vector2f centre, sf::Color couleur, Attaque attaque);
    void dessiner(sf::RenderTarget& rendu) const;
    void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte);
    bool estAPortee(sf::Vector2f cible) const;
    int frapper(sf::Vector2f cible);
    void recevoirDegats(int quantite);
    bool estVivant() const;
    sf::Vector2f getCentre() const;

private:
    sf::Vector2f m_centre;  // pixels du monde
    sf::Vector2f m_taille;  // en pixels
    sf::Color m_couleur;
    float m_vitesse;        // en pixels par seconde
    Attaque m_attaque;
    int m_pointsDeVie;
    int m_pointsDeVieMax;
};
