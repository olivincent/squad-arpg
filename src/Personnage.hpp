// ./src/Personnage.hpp
#pragma once
#include "Aleatoire.hpp"
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
    // Attaque de base en trois temps : engager (vivant, immobile, attaque
    // prête) ; avancer l'exécution, qui rend les dégâts au moment où le coup
    // part ; annuler, sans recharge consommée.
    bool engagerAttaque(int idCible);
    std::optional<int> cibleEngagee() const;
    [[nodiscard]] int avancerAttaque(float dt, Generateur& generateur);
    void annulerAttaque();
    std::optional<TypeSkill> typeSkill() const;
    bool skillEstPret() const;
    std::optional<EtatSkill> etatDuSkill() const;
    [[nodiscard]] bool skillEstAPortee(Cercle cible) const;
    [[nodiscard]] Cercle zoneDuSkill() const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill() const;
    [[nodiscard]] int declencherSkill(Cercle cible, Generateur& generateur);
    [[nodiscard]] int declencherSkillSansCible(Generateur& generateur);
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
    std::optional<int> m_idCibleEngagee; // monstre visé par le coup en exécution ; lu seulement pendant l'exécution
};
