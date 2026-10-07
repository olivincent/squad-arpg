// ./src/Monstre.hpp
#pragma once
#include "Attaque.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

class Monstre {
public:
    Monstre(sf::Vector2f centre, int id);
    void dessiner(sf::RenderTarget& rendu, bool estCible) const;
    void mettreAJour(float dt);
    int frapper(sf::Vector2f cible);
    sf::Vector2f getCentre() const;
    int getId() const;
    bool contient(sf::Vector2f point) const;
    void recevoirDegats(int quantite);
    bool estVivant() const;

private:
    sf::Vector2f m_centre;  // pixels du monde
    sf::Vector2f m_taille;  // en pixels
    int m_pointsDeVie;
    int m_pointsDeVieMax;
    int m_id;
    Attaque m_attaque;
};
