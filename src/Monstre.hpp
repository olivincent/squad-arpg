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
    // `groupe` est un identifiant opaque : des monstres de même groupe « vont
    // ensemble ». Le monstre ne sait pas ce qu'il désigne.
    Monstre(sf::Vector2f centre, int id, int armure, int groupe);
    void dessiner(sf::RenderTarget& rendu, bool estCible) const;
    void mettreAJour(float dt, std::optional<Cercle> cible, const TileMap& carte, const std::vector<Cercle>& obstacles);
    // Attaque en trois temps, comme celle d'un personnage : engager (cible à
    // portée et en vue, monstre immobile, attaque prête) ; avancer l'exécution, qui rend
    // les dégâts au départ du coup ; annuler, sans recharge consommée.
    // `idProie` est l'identification de la cible chez l'appelant (la place du
    // personnage) ; le monstre la retient jusqu'au départ du coup.
    // La proie est atteignable à portée ET en vue (ligne de vue centre à centre) ;
    // `engagerAttaque` l'exige. Même règle que `Escouade::peutAtteindre`.
    [[nodiscard]] bool peutAtteindre(Cercle proie, const TileMap& carte) const;
    bool engagerAttaque(Cercle proie, int idProie, const TileMap& carte);
    std::optional<int> proieEngagee() const;
    [[nodiscard]] int avancerAttaque(float dt, Generateur& generateur);
    void annulerAttaque();
    sf::Vector2f getCentre() const;
    Cercle getCercle() const;
    int getId() const;
    [[nodiscard]] int getGroupe() const;
    bool contient(sf::Vector2f point) const;
    void recevoirDegats(int quantite);
    // Demande au monstre d'entrer en poursuite de sa proie habituelle, même
    // hors de la détection, s'il est dans son territoire. Prise en compte au
    // prochain `mettreAJour` ; sans effet sur un monstre déjà en poursuite.
    void alerter();
    // Vrai quand le monstre est en poursuite de sa proie (le statut « attaque »).
    [[nodiscard]] bool estEnPoursuite() const;
    bool estVivant() const;
    int getPointsDeVie() const;

private:
    enum class Etat { EnGarde, Poursuite, Retour };

    bool proieDetectee(std::optional<Cercle> cible, const TileMap& carte) const;

    Mobile m_mobile;
    PointsDeVie m_pointsDeVie;
    int m_armure;
    int m_id;
    int m_groupe;
    Skill m_attaque;
    float m_rayonDetection;
    float m_rayonTerritoire;
    Etat m_etat;
    sf::Vector2f m_poste;
    bool m_alerte; // posée par `alerter`, consommée par `mettreAJour`
    std::optional<int> m_idProieEngagee; // lu seulement pendant l'exécution
};
