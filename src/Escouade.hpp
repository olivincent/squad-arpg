// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cstddef>
#include <vector>
#include "Personnage.hpp"

class TileMap;
class Escouade {
public:
    explicit Escouade(sf::Vector2f positionDepart);
    void dessiner(sf::RenderTarget& rendu) const;
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    void mettreAJour(float dt, const TileMap& carte);
    sf::Vector2f getCentre() const;

private:
    sf::Vector2f placePersonnage(std::size_t indice) const;

    sf::Vector2f m_position;               // coin haut-gauche, en pixels du monde
    sf::Vector2f m_taille;                 // en pixels
    std::vector<sf::Vector2f> m_chemin;    // points (centre de l'escouade) à atteindre, en pixels du monde
    std::size_t m_indexChemin;             // indice du prochain point de m_chemin à atteindre
    float m_vitesse;                       // en pixels par seconde
    std::vector<sf::Vector2f> m_decalages; // décalages de formation, pour une escouade orientée vers +x
    sf::Angle m_orientation;               // 0 = orientée vers +x
    std::vector<Personnage> m_personnages;
};