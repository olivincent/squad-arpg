// ./src/Monstre.hpp
#pragma once
#include "Aleatoire.hpp"
#include "Skill.hpp"
#include "Cercle.hpp"
#include "Mobile.hpp"
#include "PointsDeVie.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

class TileMap;

class Monstre {
public:
    Monstre(sf::Vector2f centre, int id, int armure);
    void dessiner(sf::RenderTarget& rendu, bool estCible) const;
    void mettreAJour(float dt, std::optional<Cercle> cible, const TileMap& carte, const std::vector<Cercle>& obstacles);
    // Attaque en trois temps, comme celle d'un personnage : engager (cible à
    // portée, monstre immobile, attaque prête) ; avancer l'exécution, qui rend
    // les dégâts au départ du coup ; annuler, sans recharge consommée.
    // `idProie` est l'identification de la cible chez l'appelant (la place du
    // personnage) ; le monstre la retient jusqu'au départ du coup.
    bool engagerAttaque(Cercle proie, int idProie);
    std::optional<int> proieEngagee() const;
    [[nodiscard]] int avancerAttaque(float dt, Generateur& generateur);
    void annulerAttaque();
    sf::Vector2f getCentre() const;
    Cercle getCercle() const;
    int getId() const;
    bool contient(sf::Vector2f point) const;
    void recevoirDegats(int quantite);
    bool estVivant() const;
    int getPointsDeVie() const;

private:
    enum class Etat { EnGarde, Poursuite, Retour };

    bool proieDetectee(std::optional<Cercle> cible) const;

    Mobile m_mobile;
    PointsDeVie m_pointsDeVie;
    int m_armure;
    int m_id;
    Skill m_attaque;
    float m_rayonDetection;
    float m_rayonTerritoire;
    Etat m_etat;
    sf::Vector2f m_poste;
    bool m_vientDEtreTouche;
    std::optional<int> m_idProieEngagee; // lu seulement pendant l'exécution
};
