// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "Personnage.hpp"

class TileMap;
class Escouade {
public:
    explicit Escouade(sf::Vector2f positionDepart);
    void dessiner(sf::RenderTarget& rendu) const;
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    int attaquer(sf::Vector2f cible, const TileMap& carte);
    void mettreAJour(float dt, const TileMap& carte);
    sf::Vector2f getCentre() const;
    std::optional<sf::Vector2f> personnageLePlusProche(sf::Vector2f point) const;
    void recevoirDegats(sf::Vector2f origine, int quantite);

private:
    sf::Vector2f placePersonnage(std::size_t indice) const;
    std::optional<std::size_t> indicePersonnageLePlusProche(sf::Vector2f point) const;

    sf::Vector2f m_position;               // coin haut-gauche, en pixels du monde
    sf::Vector2f m_taille;                 // en pixels
    std::vector<sf::Vector2f> m_chemin;    // points (centre de l'escouade) à atteindre, en pixels du monde
    std::size_t m_indexChemin;             // indice du prochain point de m_chemin à atteindre
    float m_vitesse;                       // en pixels par seconde
    std::vector<sf::Vector2f> m_decalages; // décalages de formation, pour une escouade orientée vers +x
    sf::Angle m_orientation;               // 0 = orientée vers +x
    sf::Angle m_orientationVoulue;         // direction voulue de l'ancre, vers laquelle m_orientation tourne
    float m_vitesseRotation;               // en degrés par seconde
    std::vector<Personnage> m_personnages;
};