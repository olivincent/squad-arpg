// ./src/jeu/Partie.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "combat/AdresseSkill.hpp"
#include "base/Aleatoire.hpp"
#include "escouade/Composition.hpp"
#include "combat/Coup.hpp"
#include "combat/EntiteVue.hpp"
#include "combat/Engagement.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "monde/ObjetInteractif.hpp"
#include "monde/TileMap.hpp"
#include "monde/Vision.hpp"

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
    // `composition` : les personnages de l'escouade (elle n'appartient pas au niveau : c'est le choix du joueur).
    Partie(const Niveau& niveau, std::vector<DefinitionPersonnage> composition, Generateur::result_type graine);

    // Verrouille le monstre ciblable qui contient ce point (Monstre) ; sinon, sur une porte
    // fermée (Objet), lève le verrou, mémorise l'interaction et envoie l'escouade devant elle
    // (Sol si aucun côté n'est libre) ; sinon lève le verrou (Sol) : un monstre qui rentre à son poste compte comme le sol.
    // Ignore si l'escouade est vaincue : rien ne change.
    ResultatClic cliquer(sf::Vector2f point);
    // Skill à cette adresse, tenté dans mettreAJour.
    void demanderSkill(AdresseSkill adresse);
    // Skill de zone à cette adresse lancé au point (monde), dans mettreAJour.
    void lancerSkillDeZone(AdresseSkill adresse, sf::Vector2f point);
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
    // Débogage (F3) : un cercle fin autour de chaque entité qui a une zone de contrôle (rayon de
    // l'entité + zone), personnages vivants et monstres qui ne rentrent pas à leur poste.
    void dessinerZonesDeControle(sf::RenderTarget& rendu) const;

    // Vrai quand tous les personnages sont morts.
    [[nodiscard]] bool estPerdue() const;
    // Vrai quand il ne reste aucun monstre ET que la partie n'est pas perdue
    // (la défaite l'emporte si les derniers meurent dans la même image).
    [[nodiscard]] bool estGagnee() const;
    // Nombre de monstres vivants.
    [[nodiscard]] int monstresRestants() const;

    // Les dégâts et les soins réellement appliqués depuis le dernier appel (voir EvenementDeCombat.hpp),
    // dans l'ordre où ils ont eu lieu ; la file est vidée. Une partie figée n'en produit plus.
    [[nodiscard]] std::vector<EvenementDeCombat> prendreEvenements();

    // Ce point (pixels du monde) est-il dans une case actuellement visible ? Sert à l'affichage :
    // ne pas montrer ce qui se passe dans le brouillard.
    [[nodiscard]] bool estVisible(sf::Vector2f point) const;

    [[nodiscard]] const Escouade& getEscouade() const;
    // Pour l'inventaire, qui déplace les objets de l'escouade (`docs/ecrans.md`).
    [[nodiscard]] Escouade& getEscouade();
    [[nodiscard]] const std::vector<Monstre>& getMonstres() const;

private:
    struct LancerDeZone {
        AdresseSkill adresse;
        sf::Vector2f point;
    };

    [[nodiscard]] Monstre* trouverMonstre(int id);
    // Objet interactif encore actif qui contient ce point (une porte fermée), nullptr sinon.
    [[nodiscard]] const ObjetInteractif* trouverObjetActif(sf::Vector2f point) const;
    // Recalcule la vision à partir des personnages vivants, à leur position actuelle.
    void mettreAJourLaVision();
    // Objet dont l'interaction est en cours (vide : nullptr).
    [[nodiscard]] const ObjetInteractif* objetEnCours() const;
    // Effet d'une interaction : ouvre la porte.
    void ouvrir(const ObjetInteractif& objet);
    // Ouvre l'objet en cours d'interaction si un personnage vivant est assez près.
    void interagirSiAPortee();
    // Deux vues des monstres pour l'escouade : les CIBLES (ceux qu'on peut frapper, provoquer,
    // viser ou verrouiller : pas ceux qui rentrent à leur poste) et les PHYSIQUES (tous :
    // obstacles bloquants et à contourner).
    [[nodiscard]] std::vector<EntiteVue> entitesCiblables() const;
    [[nodiscard]] std::vector<EntiteVue> entitesPhysiques() const;
    // Applique les coups aux monstres et signale chacun (`opportunite` : coups gratuits).
    void appliquer(const std::vector<Coup>& coups, bool opportunite = false);
    // Une attaque de monstre sur le personnage de cette place (Escouade::recevoirCoup), signalée.
    void infligerAUnPersonnage(int idMonstre, std::size_t place, const Attaque& attaque, bool opportunite);
    // Les faits qu'Escouade a produits depuis le dernier appel, signalés.
    void signalerLesFaits();
    // Le seul endroit où un événement naît ; des dégâts ou un soin de montant nul ne produisent rien.
    void signaler(Camp campCible, int idCible, sf::Vector2f position, DetailEvenement detail);
    // Étapes de mettreAJour(), dans l'ordre où elles sont appelées.
    void resoudreLesCoups(float dt);
    void faireTournerLesMonstres(float dt);
    void faireFrapperLeMonstre(Monstre& monstre, std::optional<EntiteVue> proie,
                               const std::vector<EntiteVue>& candidats, float dt);
    // Les deux camps tels que la règle des zones de contrôle les voit (Engagement.hpp) : les personnages
    // vivants et les monstres vivants qui ne rentrent pas à leur poste (invulnérables, ils n'engagent
    // personne et ne sont engagés par personne).
    struct CampsDeControle {
        std::vector<EntiteDeControle> monstres;
        std::vector<EntiteDeControle> personnages;
    };
    [[nodiscard]] CampsDeControle photographierLesCamps() const;
    // Compare la photo du début de l'image à celle de la fin et applique les coups gratuits dans les
    // deux sens (calculés tous deux avant d'en appliquer un) ; retire les monstres tués et lève leur verrou.
    void appliquerLesCoupsDOpportunite(const CampsDeControle& avant);
    // `enPoursuiteAvant` : statut de chaque monstre de `m_monstres` avant leur mise à jour.
    void alerterLesGroupes(const std::vector<bool>& enPoursuiteAvant);

    TileMap m_carte;
    Escouade m_escouade;
    std::vector<Monstre> m_monstres;
    std::vector<ObjetInteractif> m_objets;
    std::optional<int> m_interactionEnCours;  // identifiant de l'objet que l'escouade va activer
    std::optional<LancerDeZone> m_lancerDeZoneDemande;
    std::optional<AdresseSkill> m_skillDemande;
    std::optional<sf::Vector2f> m_deplacementOrdonne;
    Generateur m_generateur;  // passé par référence jusqu'au tirage, jamais copié
    Vision m_vision;          // ce que le joueur sait de la carte (l'union de ce que voient les personnages vivants)
    std::vector<EvenementDeCombat> m_evenements;  // produits par mettreAJour, vidés par prendreEvenements
};
