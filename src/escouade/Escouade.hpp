// ./src/escouade/Escouade.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <vector>
#include "combat/ActionEngagee.hpp"
#include "combat/AdresseSkill.hpp"
#include "escouade/Agresseurs.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Cible.hpp"
#include "base/Cercle.hpp"
#include "escouade/Composition.hpp"
#include "escouade/Formation.hpp"
#include "escouade/Intention.hpp"
#include "combat/Coup.hpp"
#include "combat/Engagement.hpp"
#include "combat/EtatSkill.hpp"
#include "combat/IdSkill.hpp"
#include "monde/Mobile.hpp"
#include "combat/EntiteVue.hpp"
#include "escouade/Personnage.hpp"
#include "escouade/PlacesDAttaque.hpp"
#include "escouade/Provocations.hpp"
#include "combat/TypeSkill.hpp"

class TileMap;
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
    // Les adresses de tous les skills de l'escouade : par place croissante, puis dans l'ordre de la
    // liste de chaque personnage.
    [[nodiscard]] std::vector<AdresseSkill> adressesDesSkills() const;
    // Couleur du personnage de cette place ; vide si la place est invalide.
    [[nodiscard]] std::optional<sf::Color> couleurDuPersonnage(std::size_t place) const;
    // Points de vie du personnage de cette place ; vide si la place est invalide.
    [[nodiscard]] std::optional<int> pointsDeVieDuPersonnage(std::size_t place) const;
    [[nodiscard]] float rayonZoneDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill(AdresseSkill adresse) const;
    [[nodiscard]] std::vector<Coup> declencherSkillDeZone(AdresseSkill adresse, sf::Vector2f point, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    // Les monstres tels que mettreAJour les reçoit, en trois listes nommées (on ne peut pas en
    // inverser deux). `ciblables` n'a pas de valeur par défaut : l'omettre dans une initialisation
    // désignée est signalé par le compilateur (-Wmissing-field-initializers).
    struct VueDesMonstres {
        // Tous les monstres, même ceux qui rentrent à leur poste : ils bloquent les personnages, et
        // chacun contourne en plus tous ceux qu'il n'approche pas.
        std::vector<EntiteVue> physiques;
        // Ceux qu'on peut viser (pas ceux qui rentrent à leur poste) : le verrou n'est vu, et un
        // monstre n'est « à portée », que s'il y est.
        std::vector<EntiteVue> ciblables;
        // Ceux que la règle des zones de contrôle voit, avec leur zone ; Partie y met la photo du
        // début de l'image. Vide : aucun personnage ne se dégage.
        std::vector<EntiteDeControle> deControle = {};
    };
    void mettreAJour(float dt, const TileMap& carte, const VueDesMonstres& monstres);
    [[nodiscard]] sf::Vector2f getCentre() const;
    // Personnage que ce monstre poursuit : l'identifiant est sa place (1 à N), le cercle le sien.
    [[nodiscard]] std::optional<EntiteVue> proieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const;
    // Personnages que ce monstre peut frapper (identifiant = place) : seulement son
    // provocateur s'il est provoqué et vivant, sinon tous les personnages vivants.
    [[nodiscard]] std::vector<EntiteVue> candidatsDeFrappe(int idMonstre) const;
    [[nodiscard]] bool placeEstVivante(std::size_t place) const;
    // Renvoient les dégâts réellement subis (après armure) ; 0 si personne n'est touché.
    int recevoirDegats(int idMonstre, sf::Vector2f origine, int quantite);
    // Comme recevoirDegats, mais sur le personnage de cette place (un coup engagé).
    int recevoirCoup(int idMonstre, std::size_t place, sf::Vector2f origine, int quantite);
    [[nodiscard]] bool estVivante() const;
    [[nodiscard]] std::vector<Cercle> cerclesDesVivants() const;
    // Une entité de contrôle (identifiant = place de 1 à N, cercle, zone de contrôle) par personnage
    // vivant : les morts n'engagent personne et ne sont pas engagés.
    [[nodiscard]] std::vector<EntiteDeControle> entitesDeControle() const;
    // Dégâts de l'attaque de base du personnage de cette place, avant l'armure de la cible ; vide si la place est invalide.
    [[nodiscard]] std::optional<int> puissanceDeLAttaqueDeBase(std::size_t place) const;
    // Un cercle (centre du personnage, rayon de vision) par personnage vivant : la vision de
    // l'escouade est l'union de leurs champs.
    [[nodiscard]] std::vector<Cercle> cerclesDeVision() const;
    // Un personnage vivant est-il à `ecart` ou moins (bord à bord) de cette zone ?
    [[nodiscard]] bool unVivantPresDe(sf::FloatRect zone, float ecart) const;
    [[nodiscard]] std::optional<Cible> choisirCible(TypeSkill type, AdresseSkill adresse, const std::vector<EntiteVue>& monstres, const TileMap& carte) const;
    void verrouiller(int idCible);
    void deverrouiller();
    [[nodiscard]] std::optional<int> getCibleVerrouillee() const;
    [[nodiscard]] const std::vector<sf::Vector2f>& getPlacesDeFormation() const;
    // Identifiant du monstre dont le personnage de cette place se dégage (voir degagementVoulu) ;
    // vide s'il ne se dégage pas ou si la place est invalide. Lu par les tests.
    [[nodiscard]] std::optional<int> degagementDuPersonnage(std::size_t place) const;
    // Centre du personnage de cette place (de 1 à N), mort ou vivant ; vide si la place est invalide.
    [[nodiscard]] std::optional<sf::Vector2f> centreDuPersonnage(std::size_t place) const;
    // Un soin appliqué : le personnage soigné (sa place), les points réellement rendus (jamais 0) et
    // son centre à ce moment. Données de sortie, comme les `Coup` : Escouade ne sait pas qui les lit.
    struct SoinApplique {
        std::size_t place;
        int montant;
        sf::Vector2f position;
    };
    // Les soins appliqués depuis le dernier appel (par `declencherSkill` et `attaquer`), dans l'ordre ;
    // la liste est vidée. `Partie` en fait des événements de combat.
    [[nodiscard]] std::vector<SoinApplique> prendreSoins();
    // Point vers lequel le personnage de cette place (de 1 à N) marche, calculé par le dernier
    // mettreAJour : place de formation, place d'attaque, cible du skill en attente, ou sa
    // propre position s'il reste sur place. Vide si la place est invalide.
    [[nodiscard]] std::optional<sf::Vector2f> pointVise(std::size_t place) const;
    // Débogage : un trait fin du centre de chaque personnage vivant à son point visé, et un petit
    // cercle à ce point.
    void dessinerPointsVises(sf::RenderTarget& rendu) const;
    [[nodiscard]] const std::vector<Agresseurs::Agresseur>& getAgresseurs() const;
    void provoquer(int idMonstre, std::size_t place, float duree);
    [[nodiscard]] const std::vector<Provocations::Provocation>& getProvocations() const;

private:
    // Place de 1 à N : seule conversion vers l'indice 0 à N-1.
    [[nodiscard]] bool placeValide(std::size_t place) const;
    [[nodiscard]] Personnage& personnageDeLaPlace(std::size_t place);              // place valide exigée
    [[nodiscard]] const Personnage& personnageDeLaPlace(std::size_t place) const;  // place valide exigée
    [[nodiscard]] bool skillEnAttenteDeDegats() const;
    void annulerSkillDeDegatsEnAttente();
    [[nodiscard]] std::optional<std::size_t> indicePersonnageLePlusProche(sf::Vector2f point) const;
    [[nodiscard]] std::optional<std::size_t> indiceProieDuMonstre(int idMonstre, sf::Vector2f positionMonstre) const;
    // Avec quoi un personnage atteint une cible : son attaque de base ou son skill.
    // `skill` vide : l'attaque de base.
    [[nodiscard]] bool peutAtteindre(const Personnage& personnage, std::optional<IdSkill> skill, Cercle cible, const TileMap& carte) const;
    [[nodiscard]] bool peutLancerAuPoint(AdresseSkill adresse, sf::Vector2f point, const TileMap& carte) const;
    std::vector<Coup> avancerSkillDe(std::size_t indice, float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    std::vector<Coup> effetDuSkill(std::size_t place, IdSkill id, const std::optional<Cible>& cible, int puissance, const std::vector<EntiteVue>& monstres, const TileMap& carte);
    // L'intention que le skill en attente donne à son lanceur (indice de 0 à N-1).
    struct IntentionDuSkill {
        std::size_t lanceur;
        Intention intention;
    };
    // `verrou` : le monstre verrouillé, s'il est ciblable (vide sinon).
    std::optional<IntentionDuSkill> avancerSkillEnAttente(const TileMap& carte, const std::optional<EntiteVue>& verrou);
    // Étapes d'attaquer(), dans l'ordre où elles sont appelées.
    void actualiserMemoires(const std::vector<EntiteVue>& monstres);
    void engagerSkillDeZoneEnAttente(const TileMap& carte);
    [[nodiscard]] bool tousLesVivantsAtteignent(Cercle cible, const TileMap& carte) const;
    void approcherVerrou(const std::vector<EntiteVue>& monstres, const TileMap& carte);
    void engagerSkillDeDegatsEnAttente(const std::vector<EntiteVue>& monstres, const TileMap& carte);
    std::vector<Coup> avancerSkills(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    std::vector<Coup> autodefense(float dt, const std::vector<EntiteVue>& monstres, const TileMap& carte, Generateur& generateur);
    [[nodiscard]] float porteeDe(const Personnage& personnage, std::optional<IdSkill> skill) const;
    // Places d'attaque (m_placesDAttaque) : ce qu'elles doivent savoir du personnage de cet indice
    // qui approche ce monstre avec une action de cette portée.
    [[nodiscard]] PlacesDAttaque::Approche approcheDe(std::size_t indice, EntiteVue monstre, float portee) const;
    // Par personnage : son centre s'il est vivant et immobile (il occupe l'endroit où il se tient), vide sinon.
    [[nodiscard]] std::vector<std::optional<sf::Vector2f>> occupants() const;
    // Étapes de mettreAJour().
    void ecoulerMemoires(float dt);
    void separerPersonnages(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles);
    // Ce qui vaut pour tous les personnages dans un mettreAJour, calculé avant la boucle des personnages.
    struct ContexteDesIntentions {
        bool ancreArretee;
        std::optional<EntiteVue> verrou;         // le monstre verrouillé, s'il est ciblable
        std::vector<EntiteVue> agresseurs;       // la mémoire des agresseurs
        std::optional<IntentionDuSkill> skill;   // l'intention du lanceur du skill en attente
    };
    [[nodiscard]] std::optional<EntiteVue> confirmerDegagement(std::size_t indice,
                                                               const std::optional<PlacesDAttaque::Place>& ancienne,
                                                               const std::vector<EntiteVue>& monstres,
                                                               const TileMap& carte);
    [[nodiscard]] SituationDuPersonnage situationDe(std::size_t indice,
                                                    const std::optional<PlacesDAttaque::Place>& ancienne,
                                                    const std::optional<EntiteVue>& degagement,
                                                    const ContexteDesIntentions& contexte,
                                                    const std::vector<EntiteVue>& ciblables,
                                                    const TileMap& carte) const;
    // Le point visé par une intention et, pour une approche, si une place d'attaque a été trouvée.
    struct PointVise {
        sf::Vector2f point;
        bool placeTrouvee;
    };
    PointVise pointDeLIntention(std::size_t indice, const Intention& intention,
                                const std::optional<PlacesDAttaque::Place>& ancienne, const TileMap& carte);
    // Dégagement : ces deux fonctions décident, avec m_degagements, qui se dégage de quel monstre ;
    // choisirIntention en fait sa règle 2 (le point de branchement des futurs gambits).
    void mettreAJourDegagements(bool ancreArretee, const std::vector<EntiteDeControle>& monstresDeControle);
    [[nodiscard]] std::optional<int> degagementVoulu(std::size_t indice, bool ancreArretee, const std::vector<EntiteDeControle>& personnages,
                                       const std::vector<EntiteDeControle>& monstresDeControle) const;

    struct SkillEnAttente {
        AdresseSkill adresse;  // du skill en attente (place du lanceur et identifiant)
        Cible cible;        // fixée à l'appui (pour un monstre, le cercle lu en route est celui du verrou, à jour)
    };

    Mobile m_mobile;
    Formation m_formation;  // décalages, orientation et places de formation autour de l'ancre
    std::vector<Personnage> m_personnages;
    std::optional<int> m_cibleVerrouillee;
    std::optional<SkillEnAttente> m_skillEnAttente; // un seul skill en attente pour toute l'escouade
    PlacesDAttaque m_placesDAttaque;  // par personnage : place d'attaque retenue autour du monstre qu'il approche
    std::vector<sf::Vector2f> m_pointsVises;  // par personnage, calculé par mettreAJour
    // Par personnage : identifiant du monstre dont il se dégage vers sa place d'attaque (vide = il ne se dégage pas).
    std::vector<std::optional<int>> m_degagements;
    std::vector<SoinApplique> m_soinsAppliques; // soins pas encore pris par prendreSoins
    Agresseurs m_agresseurs; // monstres qui ont frappé l'escouade récemment
    Provocations m_provocations; // monstres provoqués par un personnage
};