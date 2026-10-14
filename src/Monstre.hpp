// ./src/Monstre.hpp
#pragma once
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
    [[nodiscard]] int frapper(Cercle cible);
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
};
