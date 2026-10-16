// ./src/Partie.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "Aleatoire.hpp"
#include "Coup.hpp"
#include "EntiteVue.hpp"
#include "Escouade.hpp"
#include "Monstre.hpp"
#include "Niveau.hpp"
#include "ObjetInteractif.hpp"
#include "TileMap.hpp"
#include "Vision.hpp"

// Ce que le clic du joueur a désigné.
enum class ResultatClic { Monstre, Objet, Sol, Ignore };

// Les règles de jeu : possède la carte, l'escouade et les monstres, et sert
// d'intermédiaire entre Escouade et Monstre, qui ne se connaissent pas.
// Orchestre seulement : les règles propres à l'un des deux restent chez lui.
//
// Le joueur agit par des demandes. cliquer() s'applique tout de suite ; les
// autres sont mémorisées, exécutées par mettreAJour() à leur place dans
// l'image, puis oubliées en fin d'image.
//
// Invariant : entre deux appels, tout monstre du vecteur est vivant.
class Partie {
public:
    // La graine initialise le générateur aléatoire de la partie, qui est à elle seule.
    Partie(const Niveau& niveau, Generateur::result_type graine);

    // Verrouille le monstre ciblable qui contient ce point (Monstre) ; sinon, sur une porte
    // fermée (Objet), lève le verrou, mémorise l'interaction et envoie l'escouade devant elle
    // (Sol si aucun côté n'est libre) ; sinon lève le verrou (Sol) : un monstre qui rentre à son poste compte comme le sol.
    // Ignore si l'escouade est vaincue : rien ne change.
    ResultatClic cliquer(sf::Vector2f point);
    // Skill du personnage de cette place (de 1 à 3), tenté dans mettreAJour.
    void demanderSkill(std::size_t place);
    // Skill de zone de cette place lancé au point (monde), dans mettreAJour.
    void lancerSkillDeZone(std::size_t place, sf::Vector2f point);
    // Envoie l'escouade vers ce point, dans mettreAJour, si elle est vivante. Annule l'interaction en cours.
    void ordonnerDeplacement(sf::Vector2f point);

    // Fige le monde (rien ne bouge) une fois la partie perdue ou gagnée.
    void mettreAJour(float dt);
    // `afficherAncre` est transmis à Escouade::dessiner, `marquerLeRetour` (débogage F3) à Monstre::dessiner.
    // `brouillard` (débogage F6) : vrai, seuls les monstres visibles sont dessinés et le voile de la
    // vision couvre l'inconnu et l'exploré ; faux, ni voile ni monstre caché. Visuel seulement : la
    // règle de clic (monstre hors de vue, porte inconnue) n'en dépend pas.
    void dessiner(sf::RenderTarget& rendu, bool afficherAncre, bool marquerLeRetour, bool brouillard) const;
    // Débogage (F3) : chemin restant de chaque monstre.
    void dessinerCheminsDesMonstres(sf::RenderTarget& rendu) const;

    // Vrai quand tous les personnages sont morts.
    [[nodiscard]] bool estPerdue() const;
    // Vrai quand il ne reste aucun monstre ET que la partie n'est pas perdue
    // (la défaite l'emporte si les derniers meurent dans la même image).
    [[nodiscard]] bool estGagnee() const;
    // Nombre de monstres vivants.
    [[nodiscard]] int monstresRestants() const;

    const Escouade& getEscouade() const;
    const std::vector<Monstre>& getMonstres() const;

private:
    struct LancerDeZone {
        std::size_t place;
        sf::Vector2f point;
    };

    Monstre* trouverMonstre(int id);
    // Objet interactif encore actif qui contient ce point (une porte fermée), nullptr sinon.
    const ObjetInteractif* trouverObjetActif(sf::Vector2f point) const;
    // Recalcule la vision à partir des personnages vivants, à leur position actuelle.
    void mettreAJourLaVision();
    // Objet dont l'interaction est en cours (vide : nullptr).
    const ObjetInteractif* objetEnCours() const;
    // Effet d'une interaction : ouvre la porte.
    void ouvrir(const ObjetInteractif& objet);
    // Ouvre l'objet en cours d'interaction si un personnage vivant est assez près.
    void interagirSiAPortee();
    // Deux vues des monstres pour l'escouade : les CIBLES (ceux qu'on peut frapper, provoquer,
    // viser ou verrouiller : pas ceux qui rentrent à leur poste) et les PHYSIQUES (tous :
    // obstacles bloquants et à contourner).
    std::vector<EntiteVue> entitesCiblables() const;
    std::vector<EntiteVue> entitesPhysiques() const;
    void appliquer(const std::vector<Coup>& coups);
    // Étapes de mettreAJour(), dans l'ordre où elles sont appelées.
    void resoudreLesCoups(float dt);
    void faireTournerLesMonstres(float dt);
    void faireFrapperLeMonstre(Monstre& monstre, std::optional<EntiteVue> proie,
                               const std::vector<EntiteVue>& candidats, float dt);
    // `enPoursuiteAvant` : statut de chaque monstre de `m_monstres` avant leur mise à jour.
    void alerterLesGroupes(const std::vector<bool>& enPoursuiteAvant);

    TileMap m_carte;
    Escouade m_escouade;
    std::vector<Monstre> m_monstres;
    std::vector<ObjetInteractif> m_objets;
    std::optional<int> m_interactionEnCours;  // identifiant de l'objet que l'escouade va activer
    std::optional<LancerDeZone> m_lancerDeZoneDemande;
    std::optional<std::size_t> m_skillDemande;
    std::optional<sf::Vector2f> m_deplacementOrdonne;
    Generateur m_generateur;  // passé par référence jusqu'au tirage, jamais copié
    Vision m_vision;          // ce que le joueur sait de la carte (l'union de ce que voient les personnages vivants)
};
