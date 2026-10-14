// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <vector>
#include "Agresseurs.hpp"
#include "Aleatoire.hpp"
#include "Cible.hpp"
#include "Cercle.hpp"
#include "Coup.hpp"
#include "EtatSkill.hpp"
#include "Mobile.hpp"
#include "MonstreVu.hpp"
#include "Personnage.hpp"
#include "Provocations.hpp"
#include "TypeSkill.hpp"

class TileMap;
// Ordre d'appel dans une même image : attaquer() AVANT mettreAJour().
// attaquer() calcule m_monstreAPortee et m_cibleCombat, que mettreAJour()
// lit ; dans l'ordre inverse, mettreAJour() verrait les valeurs de l'image
// précédente (les personnages réagiraient avec une image de retard).
class Escouade {
public:
    explicit Escouade(sf::Vector2f positionDepart);
    void dessiner(sf::RenderTarget& rendu) const;
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    void annulerAttaquesEngagees();
    [[nodiscard]] std::vector<Coup> attaquer(float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] std::vector<Coup> declencherSkill(std::size_t place, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] bool skillEstPret(std::size_t place) const;
    [[nodiscard]] std::optional<EtatSkill> etatDuSkill(std::size_t place) const;
    [[nodiscard]] float rayonZoneDuSkill(std::size_t place) const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill(std::size_t place) const;
    [[nodiscard]] std::vector<Coup> declencherSkillDeZone(std::size_t place, sf::Vector2f point, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles, Generateur& generateur);
    sf::Vector2f getCentre() const;
    [[nodiscard]] std::optional<Cercle> proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const;
    // Place (de 1 à 4) du personnage que ce monstre vise : même règle que proieDuMonstre.
    [[nodiscard]] std::optional<std::size_t> placeDeLaProie(int idMonstre, sf::Vector2f positionMonstre) const;
    [[nodiscard]] bool placeEstVivante(std::size_t place) const;
    void recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite);
    // Comme recevoirDegats, mais sur le personnage de cette place (un coup engagé).
    void recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite);
    bool estVivante() const;
    std::vector<Cercle> cerclesDesVivants() const;
    [[nodiscard]] std::optional<Cible> choisirCible(TypeSkill type, std::size_t place, const std::vector<MonstreVu>& monstres, const TileMap& carte) const;
    void verrouiller(int idCible);
    void deverrouiller();
    std::optional<int> getCibleVerrouillee() const;
    const std::vector<sf::Vector2f>& getPlacesDeFormation() const;
    const std::vector<Agresseurs::Agresseur>& getAgresseurs() const;
    void provoquer(int idMonstre, std::size_t place, float duree);
    const std::vector<Provocations::Provocation>& getProvocations() const;

private:
    sf::Vector2f placePersonnage(std::size_t indice) const;
    // Place de 1 à 4 (comme la touche du skill) : seule conversion vers l'indice 0 à 3.
    bool placeValide(std::size_t place) const;
    Personnage& personnageDeLaPlace(std::size_t place);              // place valide exigée
    const Personnage& personnageDeLaPlace(std::size_t place) const;  // place valide exigée
    bool skillEnAttenteDeDegats() const;
    void annulerSkillDeDegatsEnAttente();
    std::optional<std::size_t> indicePersonnageLePlusProche(sf::Vector2f point) const;
    std::optional<std::size_t> indiceProieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const;
    // Avec quoi un personnage atteint une cible : son attaque de base ou son skill.
    enum class Arme { AttaqueDeBase, Skill };
    bool peutAtteindre(const Personnage& personnage, Arme arme, Cercle cible, const TileMap& carte) const;
    bool peutLancerAuPoint(std::size_t place, sf::Vector2f point, const TileMap& carte) const;
    std::vector<Coup> lancerSkillDeZone(std::size_t place, sf::Vector2f point, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    void soigner(std::size_t placeLanceur, std::size_t placeAllie, Generateur& generateur);
    std::optional<sf::Vector2f> avancerSkillEnAttente(const TileMap& carte, Generateur& generateur);
    // Étapes d'attaquer(), dans l'ordre où elles sont appelées.
    void actualiserMemoires(const std::vector<MonstreVu>& monstres);
    std::vector<Coup> departSkillDeZoneEnAttente(const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    bool tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const;
    std::optional<Cercle> approcherVerrou(const std::vector<MonstreVu>& monstres, const TileMap& carte);
    std::optional<Coup> departSkillDeDegatsEnAttente(Cercle cible, const TileMap& carte, Generateur& generateur);
    std::vector<Coup> autodefense(float dt, const std::vector<MonstreVu>& monstres, const TileMap& carte, Generateur& generateur);
    std::optional<sf::Vector2f> pointVersAgresseur(std::size_t indice, const std::vector<MonstreVu>& agresseurs) const;
    // Étapes de mettreAJour().
    void ecoulerMemoires(float dt);
    void tournerFormation(float dt);
    void calculerPlaces(const TileMap& carte);
    sf::Vector2f pointVoulu(std::size_t indice, const std::optional<sf::Vector2f>& pointSkill, bool ancreArretee,
                            const std::vector<MonstreVu>& agresseurs, const TileMap& carte) const;

    struct SkillEnAttente {
        std::size_t place;  // du lanceur, de 1 à 4
        Cible cible;        // fixée à l'appui (pour un monstre, le cercle lu en route est m_cibleCombat)
    };

    Mobile m_mobile;
    std::vector<sf::Vector2f> m_decalages; // décalages de formation, pour une escouade orientée vers +x
    sf::Angle m_orientation;               // 0 = orientée vers +x
    sf::Angle m_orientationVoulue;         // direction voulue de l'ancre, vers laquelle m_orientation tourne
    float m_vitesseRotation;               // en degrés par seconde
    static constexpr std::size_t s_nombrePersonnages = 4;
    std::vector<Personnage> m_personnages;
    std::optional<int> m_cibleVerrouillee;
    std::optional<SkillEnAttente> m_skillEnAttente; // un seul skill en attente pour toute l'escouade
    std::optional<Cercle> m_cibleCombat; // dernière cible donnée à attaquer(), pour figer les personnages déjà à portée
    std::vector<sf::Vector2f> m_places;  // place de formation de chaque personnage, repliée si besoin (recalculée par mettreAJour)
    std::array<bool, s_nombrePersonnages> m_monstreAPortee;  // par personnage : a-t-il un monstre à portée de son attaque (calculé par attaquer, lu par mettreAJour)
    Agresseurs m_agresseurs; // monstres qui ont frappé l'escouade récemment
    Provocations m_provocations; // monstres provoqués par un personnage
};