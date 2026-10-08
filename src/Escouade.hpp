// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "Cible.hpp"
#include "Cercle.hpp"
#include "Coup.hpp"
#include "Mobile.hpp"
#include "Personnage.hpp"
#include "Skill.hpp"

class TileMap;
class Escouade {
public:
    // Monstre qui vient de frapper l'escouade, retenu jusqu'à ce que son
    // temps restant tombe à zéro.
    struct Agresseur {
        int id;
        Cercle cercle;       // à jour à chaque attaquer() ; centre seul (rayon nul) à l'enregistrement
        float tempsRestant;  // en secondes, remis au maximum à chaque coup reçu
    };

    explicit Escouade(sf::Vector2f positionDepart);
    void dessiner(sf::RenderTarget& rendu) const;
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    std::vector<Coup> attaquer(const std::vector<MonstreVu>& monstres, const TileMap& carte);
    std::vector<Coup> declencherSkill(std::size_t place, const std::vector<MonstreVu>& monstres);
    void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles);
    sf::Vector2f getCentre() const;
    std::optional<Cercle> personnageLePlusProche(sf::Vector2f point) const;
    void recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite);
    bool estVivante() const;
    std::vector<Cercle> cerclesDesVivants() const;
    std::optional<Cible> choisirCible(TypeSkill type, std::size_t place, const std::vector<MonstreVu>& monstres) const;
    void verrouiller(int idCible);
    void deverrouiller();
    std::optional<int> getCibleVerrouillee() const;
    const std::vector<sf::Vector2f>& getPlacesDeFormation() const;
    const std::vector<Agresseur>& getAgresseurs() const;

private:
    sf::Vector2f placePersonnage(std::size_t indice) const;
    std::optional<std::size_t> indicePersonnageLePlusProche(sf::Vector2f point) const;
    void soigner(std::size_t placeLanceur, std::size_t placeAllie);
    std::optional<sf::Vector2f> avancerSkillEnAttente();
    std::optional<sf::Vector2f> pointVersAgresseur(std::size_t indice) const;

    struct SkillEnAttente {
        std::size_t place;  // du lanceur, de 1 à 4
        Cible cible;        // fixée à l'appui (pour un monstre, le cercle lu en route est m_cibleCombat)
    };

    Mobile m_mobile;
    std::vector<sf::Vector2f> m_decalages; // décalages de formation, pour une escouade orientée vers +x
    sf::Angle m_orientation;               // 0 = orientée vers +x
    sf::Angle m_orientationVoulue;         // direction voulue de l'ancre, vers laquelle m_orientation tourne
    float m_vitesseRotation;               // en degrés par seconde
    std::vector<Personnage> m_personnages;
    std::optional<int> m_cibleVerrouillee;
    std::optional<SkillEnAttente> m_skillEnAttente; // un seul skill en attente pour toute l'escouade
    std::optional<Cercle> m_cibleCombat; // dernière cible donnée à attaquer(), pour figer les personnages déjà à portée
    std::vector<sf::Vector2f> m_places;  // place de formation de chaque personnage, repliée si besoin (recalculée par mettreAJour)
    std::vector<bool> m_monstreAPortee;  // par personnage : a-t-il un monstre à portée de son attaque (calculé par attaquer)
    std::vector<Agresseur> m_agresseurs; // monstres qui ont frappé l'escouade récemment
};