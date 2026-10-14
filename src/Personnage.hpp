// ./src/Personnage.hpp
#pragma once
#include "Cercle.hpp"
#include "EtatSkill.hpp"
#include "Mobile.hpp"
#include "PointsDeVie.hpp"
#include "Skill.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

class TileMap;

class Personnage {
public:
    Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::optional<Skill> skill, int armure);
    void dessiner(sf::RenderTarget& rendu) const;
    void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles);
    [[nodiscard]] bool estAPortee(Cercle cible) const;
    [[nodiscard]] int frapper(Cercle cible);
    std::optional<TypeSkill> typeSkill() const;
    bool skillEstPret() const;
    std::optional<EtatSkill> etatDuSkill() const;
    [[nodiscard]] bool skillEstAPortee(Cercle cible) const;
    [[nodiscard]] Cercle zoneDuSkill() const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill() const;
    [[nodiscard]] int declencherSkill(Cercle cible);
    [[nodiscard]] int declencherSkillSansCible();
    void recevoirDegats(int quantite);
    void recevoirSoin(int quantite);
    bool estVivant() const;
    int getPointsDeVie() const;
    int getPointsDeVieMax() const;
    sf::Vector2f getCentre() const;
    Cercle getCercle() const;

private:
    Mobile m_mobile;        // position (centre en pixels du monde) et déplacement
    sf::Color m_couleur;
    Skill m_attaqueDeBase;
    std::optional<Skill> m_skill; // vide = pas de skill
    std::optional<sf::Vector2f> m_cibleChemin; // cible du dernier A* ; vide = pas de chemin en cours
    PointsDeVie m_pointsDeVie;
    int m_armure;
};
