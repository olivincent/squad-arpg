// ./src/Mobile.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>

class TileMap;

class Mobile {
public:
    Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse);
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    void arreter();
    void mettreAJour(float dt, const TileMap& carte);
    sf::Vector2f getCentre() const;
    sf::Vector2f getTaille() const;
    std::optional<sf::Vector2f> directionVoulue() const;

private:
    sf::Vector2f m_position;            // coin haut-gauche, en pixels du monde
    sf::Vector2f m_taille;              // en pixels
    std::vector<sf::Vector2f> m_chemin; // points (centre) à atteindre, en pixels du monde
    std::size_t m_indexChemin;          // indice du prochain point de m_chemin à atteindre
    float m_vitesse;                    // en pixels par seconde
};
