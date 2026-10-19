// ./src/Escouade.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <vector>
#include "ActionEngagee.hpp"
#include "AdresseSkill.hpp"
#include "Agresseurs.hpp"
#include "Aleatoire.hpp"
#include "Cible.hpp"
#include "Cercle.hpp"
#include "Composition.hpp"
#include "Coup.hpp"
#include "EtatSkill.hpp"
#include "IdSkill.hpp"
#include "Mobile.hpp"
#include "EntiteVue.hpp"
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
    // `composition` : un personnage par définition, aux places 1 à N dans l'ordre (assert : non vide).
    Escouade(sf::Vector2f positionDepart, std::vector<DefinitionPersonnage> composition);
    // `afficherAncre` faux : le carré vert de l'ancre n'est pas dessiné (débogage).
    void dessiner(sf::RenderTarget& rendu, bool afficherAncre) const;
    void allerVers(sf::Vector2f destination, const TileMap& carte);
    // Une fois l'ancre arrêtée, la formation se tourne vers ce point. Tout nouvel ordre à
    // l'ancre (allerVers, approche du verrou, verrouiller) l'efface.
    void regarderVers(sf::Vector2f point);
    void annulerAttaquesEngagees();
    [[nodiscard]] std::vector<Coup> attaquer(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] std::vector<Coup> declencherSkill(AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] bool skillEstPret(AdresseSkill adresse) const;
    [[nodiscard]] std::optional<EtatSkill> etatDuSkill(AdresseSkill adresse) const;
    // Type du skill à cette adresse ; vide si la place est invalide ou le skill inconnu de ce personnage.
    [[nodiscard]] std::optional<TypeSkill> typeDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] float rayonZoneDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] std::vector<Coup> declencherSkillDeZone(AdresseSkill adresse, sf::Vector2f point, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    // `monstres` bloquent les personnages ; chaque personnage contourne en plus tous ceux
    // qu'il n'approche pas (voir monstresAContourner).
    void mettreAJour(float dt, const TileMap& carte, const std::vector<EntiteVue>& monstres);
    sf::Vector2f getCentre() const;
    // Personnage que ce monstre poursuit : l'identifiant est sa place (1 à N), le cercle le sien.
    [[nodiscard]] std::optional<EntiteVue> proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const;
    // Personnages que ce monstre peut frapper (identifiant = place) : seulement son
    // provocateur s'il est provoqué et vivant, sinon tous les personnages vivants.
    [[nodiscard]] std::vector<EntiteVue> candidatsDeFrappe(int idMonstre) const;
    [[nodiscard]] bool placeEstVivante(std::size_t place) const;
    void recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite);
    // Comme recevoirDegats, mais sur le personnage de cette place (un coup engagé).
    void recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite);
    bool estVivante() const;
    std::vector<Cercle> cerclesDesVivants() const;
    // Un cercle (centre du personnage, rayon de vision) par personnage vivant : la vision de
    // l'escouade est l'union de leurs champs.
    [[nodiscard]] std::vector<Cercle> cerclesDeVision() const;
    // Un personnage vivant est-il à `ecart` ou moins (bord à bord) de cette zone ?
    [[nodiscard]] bool unVivantPresDe(sf::FloatRect zone, float ecart) const;
    [[nodiscard]] std::optional<Cible> choisirCible(TypeSkill type, AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte) const;
    void verrouiller(int idCible);
    void deverrouiller();
    std::optional<int> getCibleVerrouillee() const;
    const std::vector<sf::Vector2f>& getPlacesDeFormation() const;
    // Point vers lequel le personnage de cette place (de 1 à N) marche, calculé par le dernier
    // mettreAJour : place de formation, place d'attaque, cible du skill en attente, ou sa
    // propre position s'il reste sur place. Vide si la place est invalide.
    [[nodiscard]] std::optional<sf::Vector2f> pointVise(std::size_t place) const;
    // Débogage : un trait fin du centre de chaque personnage vivant à son point visé, et un petit
    // cercle à ce point.
    void dessinerPointsVises(sf::RenderTarget& rendu) const;
    const std::vector<Agresseurs::Agresseur>& getAgresseurs() const;
    void provoquer(int idMonstre, std::size_t place, float duree);
    const std::vector<Provocations::Provocation>& getProvocations() const;

private:
    sf::Vector2f placePersonnage(std::size_t indice) const;
    // Place de 1 à N : seule conversion vers l'indice 0 à N-1.
    bool placeValide(std::size_t place) const;
    Personnage& personnageDeLaPlace(std::size_t place);              // place valide exigée
    const Personnage& personnageDeLaPlace(std::size_t place) const;  // place valide exigée
    bool skillEnAttenteDeDegats() const;
    void annulerSkillDeDegatsEnAttente();
    std::optional<std::size_t> indicePersonnageLePlusProche(sf::Vector2f point) const;
    std::optional<std::size_t> indiceProieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const;
    // Avec quoi un personnage atteint une cible : son attaque de base ou son skill.
    // `skill` vide : l'attaque de base.
    bool peutAtteindre(const Personnage& personnage, std::optional<IdSkill> skill, Cercle cible, const TileMap& carte) const;
    bool peutLancerAuPoint(AdresseSkill adresse, sf::Vector2f point, const TileMap& carte) const;
    std::vector<Coup> avancerSkillDe(std::size_t indice, float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    std::vector<Coup> effetDuSkill(std::size_t place, IdSkill id, const std::optional<Cible>& cible, int puissance, const std::vector<EntiteVue>& monstres, const TileMap& carte);
    std::optional<sf::Vector2f> avancerSkillEnAttente(const TileMap& carte);
    // Étapes d'attaquer(), dans l'ordre où elles sont appelées.
    void actualiserMemoires(const std::vector<EntiteVue>& monstres);
    void engagerSkillDeZoneEnAttente(const TileMap& carte);
    bool tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const;
    std::optional<Cercle> approcherVerrou(const std::vector<EntiteVue>& monstres, const TileMap& carte);
    void engagerSkillDeDegatsEnAttente(Cercle cible, const TileMap& carte);
    std::vector<Coup> avancerSkills(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    std::vector<Coup> autodefense(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    // Place d'attaque : voir la définition de pointVersMonstre.
    struct PlaceDAttaque {
        int idMonstre;
        float rayon;     // de l'anneau, en pixels
        sf::Angle angle; // autour du centre du monstre
    };
    float porteeDe(const Personnage& personnage, std::optional<IdSkill> skill) const;
    sf::Vector2f pointVersMonstre(std::size_t indice, int idMonstre, Cercle monstre, std::optional<IdSkill> skill,
                                  const std::optional<PlaceDAttaque>& ancienne, const TileMap& carte);
    std::optional<sf::Vector2f> pointVersAgresseur(std::size_t indice, const std::vector<EntiteVue>& agresseurs,
                                                   const std::optional<PlaceDAttaque>& ancienne, const TileMap& carte);
    // Cercles des monstres sauf celui que ce personnage approche (sa place d'attaque, ou le
    // verrou en combat) ; tous s'il n'en approche aucun.
    std::vector<Cercle> monstresAContourner(std::size_t indice, const std::vector<EntiteVue>& monstres) const;
    // Étapes de mettreAJour().
    void ecoulerMemoires(float dt);
    void tournerFormation(float dt);
    void calculerPlaces(const TileMap& carte, const std::vector<EntiteVue>& monstres);
    void separerPersonnages(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles);
    sf::Vector2f pointVoulu(std::size_t indice, const std::optional<sf::Vector2f>& pointSkill, bool ancreArretee,
                            const std::vector<EntiteVue>& agresseurs, const TileMap& carte);

    struct SkillEnAttente {
        AdresseSkill adresse;  // du skill en attente (place du lanceur et identifiant)
        Cible cible;        // fixée à l'appui (pour un monstre, le cercle lu en route est m_cibleCombat)
    };

    Mobile m_mobile;
    std::vector<sf::Vector2f> m_decalages; // décalages de formation, pour une escouade orientée vers +x
    sf::Angle m_orientation;               // 0 = orientée vers +x
    sf::Angle m_orientationVoulue;         // direction voulue de l'ancre, vers laquelle m_orientation tourne
    std::optional<sf::Vector2f> m_pointARegarder; // orientation voulue de l'ancre à l'arrêt (voir regarderVers)
    float m_vitesseRotation;               // en degrés par seconde
    std::vector<Personnage> m_personnages;
    std::optional<int> m_cibleVerrouillee;
    std::optional<SkillEnAttente> m_skillEnAttente; // un seul skill en attente pour toute l'escouade
    std::optional<Cercle> m_cibleCombat; // dernière cible donnée à attaquer(), pour figer les personnages déjà à portée
    std::vector<sf::Vector2f> m_places;  // place de formation de chaque personnage, repliée si besoin (recalculée par mettreAJour)
    std::vector<bool> m_monstreAPortee;  // par personnage : a-t-il un monstre à portée de son attaque (calculé par attaquer, lu par mettreAJour)
    // Par personnage : place d'attaque retenue autour du monstre qu'il approche (vide = il n'en approche aucun).
    std::vector<std::optional<PlaceDAttaque>> m_placesDAttaque;
    std::vector<sf::Vector2f> m_pointsVises;  // par personnage, calculé par mettreAJour
    Agresseurs m_agresseurs; // monstres qui ont frappé l'escouade récemment
    Provocations m_provocations; // monstres provoqués par un personnage
};