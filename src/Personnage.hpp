// ./src/Personnage.hpp
#pragma once
#include "Attaque.hpp"
#include "Cercle.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>

class TileMap;

class Personnage {
public:
    Personnage(sf::Vector2f centre, sf::Color couleur, Attaque attaque, std::optional<Attaque> skill);
    void dessiner(sf::RenderTarget& rendu) const;
    void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte);
    bool estAPortee(Cercle cible) const;
    int frapper(Cercle cible);
    int declencherSkill(Cercle cible);
    void recevoirDegats(int quantite);
    bool estVivant() const;
    sf::Vector2f getCentre() const;
    Cercle getCercle() const;

private:
    sf::Vector2f m_centre;  // pixels du monde
    sf::Vector2f m_taille;  // en pixels
    sf::Color m_couleur;
    float m_vitesse;        // en pixels par seconde
    Attaque m_attaque;
    std::optional<Attaque> m_skill; // vide = pas de skill
    int m_pointsDeVie;
    int m_pointsDeVieMax;
};
