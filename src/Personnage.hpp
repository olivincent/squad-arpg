// ./src/Personnage.hpp
#pragma once
#include "Attaque.hpp"
#include "Cercle.hpp"
#include "Mobile.hpp"
#include "Skill.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

class TileMap;

class Personnage {
public:
    Personnage(sf::Vector2f centre, sf::Color couleur, Attaque attaque, std::optional<Skill> skill);
    void dessiner(sf::RenderTarget& rendu) const;
    void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles);
    bool estAPortee(Cercle cible) const;
    int frapper(Cercle cible);
    std::optional<TypeSkill> typeSkill() const;
    bool skillEstPret() const;
    bool skillEstAPortee(Cercle cible) const;
    int declencherSkill(Cercle cible);
    void recevoirDegats(int quantite);
    void recevoirSoin(int quantite);
    bool estVivant() const;
    int getPointsDeVie() const;
    int getPointsDeVieMax() const;
    sf::Vector2f getCentre() const;
    Cercle getCercle() const;

private:
    Mobile m_mobile;        // position (centre en pixels du monde) et déplacement
    sf::Vector2f m_taille;  // en pixels
    sf::Color m_couleur;
    Attaque m_attaque;
    std::optional<Skill> m_skill; // vide = pas de skill
    std::optional<sf::Vector2f> m_cibleChemin; // cible du dernier A* ; vide = pas de chemin en cours
    int m_pointsDeVie;
    int m_pointsDeVieMax;
};
