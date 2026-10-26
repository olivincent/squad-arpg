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
#include "jeu/Expedition.hpp"
#include "jeu/ObjetAuSol.hpp"
#include "jeu/ObjetsAuSol.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "monde/ObjetInteractif.hpp"
#include "monde/TileMap.hpp"
#include "monde/Vision.hpp"
#include "objets/PositionDObjet.hpp"

// Ce que le clic du joueur a désigné : un monstre, une porte fermée, un objet au sol, l'escalier, le sol.
enum class ResultatClic { Monstre, Porte, ObjetAuSol, Escalier, Sol, Ignore };

// La distance (centre à centre, en pixels du monde) de l'ancre de l'escouade au centre de l'escalier visé en deçà de laquelle
// l'escouade le prend (« à tester »).
inline constexpr float s_distanceDeDescente = 30.0f;

// Les règles de jeu : possède la carte, l'escouade et les monstres, et sert
// d'intermédiaire entre Escouade et Monstre, qui ne se connaissent pas.
// Orchestre seulement : les règles propres à l'un des deux restent chez lui.
//
// Le joueur agit par des demandes. cliquer() s'applique tout de suite ; les
// autres sont mémorisées, exécutées par mettreAJour() à leur place dans
// l'image, puis oubliées en fin d'image.
//
// Invariant : entre deux appels, tout monstre du vecteur est vivant. Un monstre qui meurt peut laisser des objets au sol
// (`genererButin`, selon sa définition) ; ils appartiennent à la partie, donc au niveau : une nouvelle partie les efface.
class Partie {
public:
    // Un étage : le niveau, avec l'escouade que l'expédition amène. La partie est neuve : tous les personnages sont à pleine vie et
    // vivants, à la place de départ du niveau, et rien de l'étage précédent ne passe (ni monstre, ni objet au sol, ni combat) ;
    // seuls viennent de l'expédition les personnages (classe, caractéristiques de base, équipement) et le sac (`jeu/Expedition`).
    // La graine initialise le générateur aléatoire de la partie, qui est à elle seule (celle de l'étage : `graineDEtage`).
    // `catalogue` : les définitions des monstres, une par catégorie ; chaque monstre du niveau reçoit celle de sa
    // catégorie, renforcée par la profondeur de l'expédition (`renforcer`). Le jeu ne le précise pas ; les tests y substituent un catalogue sans hasard.
    Partie(const Niveau& niveau, Expedition expedition, Generateur::result_type graine,
           const CatalogueDeMonstres& catalogue = catalogueDeMonstres());

    // Verrouille le monstre ciblable qui contient ce point (Monstre) ; sinon, sur un objet au sol (`objetAuSolSous`), lève le
    // verrou et envoie l'escouade le ramasser (ObjetAuSol : `ramasserSiAPortee` le ramasse quand l'ancre arrive, et tout autre
    // ordre remplace cette visée) ; sinon, sur l'escalier d'une case connue (pas inconnue), lève le verrou et envoie l'escouade
    // au centre de la case (Escalier : `prendreLEscalierSiAPortee` la fait descendre quand l'ancre arrive, et tout autre ordre
    // remplace cette visée) ; sinon, sur une porte
    // fermée (Porte), lève le verrou, mémorise l'interaction et envoie l'escouade devant elle
    // (Sol si aucun côté n'est libre) ; sinon lève le verrou (Sol) : un monstre qui rentre à son poste compte comme le sol.
    // Ignore si l'escouade est vaincue : rien ne change.
    ResultatClic cliquer(sf::Vector2f point);
    // Skill à cette adresse, tenté dans mettreAJour.
    void demanderSkill(AdresseSkill adresse);
    // Skill de zone à cette adresse lancé au point (monde), dans mettreAJour.
    void lancerSkillDeZone(AdresseSkill adresse, sf::Vector2f point);
    // Envoie l'escouade vers ce point, dans mettreAJour, si elle est vivante. Annule l'interaction en cours.
    void ordonnerDeplacement(sf::Vector2f point);

    // Fige le monde (rien ne bouge) une fois la partie perdue, gagnée, ou l'escalier pris.
    void mettreAJour(float dt);
    // Le dessin d'une image se fait en deux temps, l'écran dessinant entre les deux ce qui n'est pas du noyau (les objets
    // au sol, `interface/AffichageObjetsAuSol`, sous les monstres et l'escouade) : la carte, puis les entités et le voile.
    void dessinerLaCarte(sf::RenderTarget& rendu) const;
    // `afficherAncre` est transmis à Escouade::dessiner, `marquerLeRetour` (débogage F3) à Monstre::dessiner.
    // `brouillard` (débogage F6) : vrai, seuls les monstres visibles sont dessinés et le voile de la
    // vision couvre l'inconnu et l'exploré ; faux, ni voile ni monstre caché. Visuel seulement : la
    // règle de clic (monstre hors de vue, porte inconnue) n'en dépend pas. À appeler après `dessinerLaCarte`.
    void dessinerLesEntites(sf::RenderTarget& rendu, bool afficherAncre, bool marquerLeRetour, bool brouillard) const;
    // Débogage (F3) : chemin restant de chaque monstre.
    void dessinerCheminsDesMonstres(sf::RenderTarget& rendu) const;
    // Débogage (F3) : un cercle fin autour de chaque entité qui a une zone de contrôle (rayon de
    // l'entité + zone), personnages vivants et monstres qui ne rentrent pas à leur poste.
    void dessinerZonesDeControle(sf::RenderTarget& rendu) const;

    // Tire `nombre` objets au hasard (`genererObjet`, avec le générateur de la partie) et les met dans le sac de l'escouade,
    // tout ou rien : sans `nombre` cases libres, rien n'est ajouté, aucun tirage n'est fait (le générateur n'avance pas) et
    // le résultat est faux. Pour la touche de débogage F7.
    [[nodiscard]] bool ramasserDesObjetsAleatoires(std::size_t nombre);

    // L'expédition telle qu'elle est maintenant : ce qui doit survivre à l'étage (équipement et points dépensés de chaque
    // personnage, sac) lu dans l'escouade, l'expérience gagnée pendant l'étage, la profondeur et la graine de celle que la partie a reçue. Rien de l'état de l'étage
    // (positions, combat, agresseurs, provocations, visées, recharges, objets au sol, monstres). Pour construire l'étage suivant.
    [[nodiscard]] Expedition expedition() const;

    // Vrai quand l'escouade a pris l'escalier d'un étage qui n'est pas le dernier (l'ancre est arrivée à l'escalier qu'elle
    // visait, ou `forcerLaDescente`) et n'est pas vaincue : l'étage est terminé et l'escouade descend. Reste vrai pour le reste
    // de la partie, qui est figée : celui qui la lit construit l'étage suivant (`expedition`, `etageSuivant`).
    [[nodiscard]] bool doitDescendre() const;
    // Débogage (F8) : l'escouade prend l'escalier d'où elle est, comme si l'ancre y était arrivée (donc, au dernier étage, la
    // partie est gagnée). Sans effet si l'escouade est vaincue.
    void forcerLaDescente();

    // L'expérience totale de l'expédition, et le niveau de l'escouade (celui de chacun de ses personnages) : ce que l'en-tête montre.
    [[nodiscard]] int getExperience() const;
    [[nodiscard]] int getNiveau() const;
    // Débogage (F9) : donne l'expérience qui manque pour le niveau suivant, donc exactement un niveau (`NiveauAtteint` est
    // signalé). Sans effet si l'escouade est vaincue.
    void gagnerUnNiveau();

    // Vrai quand tous les personnages sont morts.
    [[nodiscard]] bool estPerdue() const;
    // Vrai quand l'escouade a pris l'escalier du dernier étage (`s_profondeurMaximale`) ET que la partie n'est pas perdue
    // (la défaite l'emporte si les derniers personnages meurent dans la même image). Tuer tous les monstres ne suffit pas.
    [[nodiscard]] bool estGagnee() const;
    // L'étage où l'on est : la profondeur de l'expédition (1 au premier).
    [[nodiscard]] int getProfondeur() const;
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
    // La carte, pour ce qui doit trouver une place dans le monde (jeter un objet : `jeterUnObjet`).
    [[nodiscard]] const TileMap& getCarte() const;
    // Les objets posés au sol (`tous()` : dans l'ordre où ils sont tombés).
    [[nodiscard]] const ObjetsAuSol& getSol() const;
    // Pour l'inventaire, qui y jette les objets (`docs/ecrans.md`).
    [[nodiscard]] ObjetsAuSol& getSol();
    // L'objet au sol que ce point (pixels du monde) désigne (`ObjetsAuSol::sous`, avec la carte et la vision de la partie).
    [[nodiscard]] const ObjetAuSol* objetAuSolSous(sf::Vector2f point) const;

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
    // Ramasse l'objet au sol visé quand l'ANCRE est à moins de `s_distanceDeRamassage` de lui : il passe dans le premier
    // emplacement libre du sac et l'ordre est terminé. Sac plein : l'objet reste au sol, `SacPlein` est signalé et l'ordre est
    // terminé aussi. Un objet visé qui n'existe plus : la visée est abandonnée.
    void ramasserSiAPortee();
    // Prend l'escalier visé quand l'ANCRE est à moins de `s_distanceDeDescente` de son centre : la visée est terminée et
    // l'escalier est pris (`doitDescendre`, ou `estGagnee` au dernier étage).
    void prendreLEscalierSiAPortee();
    // Un nouvel ordre : l'escouade ne va plus ouvrir de porte, ramasser un objet ni prendre l'escalier.
    void abandonnerLesVisees();
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
    // Ce qu'un coup a donné, en événements (`detailsDuCoup` : « Esquivé », ou les dégâts puis le blocage).
    void signalerLeCoup(Camp campCible, int idCible, sf::Vector2f position, const ResultatDuCoup& resultat, bool opportunite);
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
    // Retire les monstres morts ; chacun, dans l'ordre du vecteur, rapporte son expérience à l'escouade puis laisse son butin
    // (un tirage de chance, puis les objets) à l'endroit de sa mort. Le seul endroit où un monstre quitte le vecteur.
    void retirerLesMorts();
    // Si l'expérience de l'expédition donne un niveau de plus que l'escouade n'en a, tous les niveaux gagnés s'appliquent d'un coup.
    void monterSiLExperienceLeDonne();
    // `enPoursuiteAvant` : statut de chaque monstre de `m_monstres` avant leur mise à jour.
    void alerterLesGroupes(const std::vector<bool>& enPoursuiteAvant);

    // Ce que la partie garde de l'expédition reçue, et seulement cela : la profondeur, la graine et la classe de chaque membre ne
    // changent pas pendant l'étage, l'expérience grandit ici. Ce qui change (équipement, points dépensés, sac) vit dans
    // l'escouade : l'équipement et le sac reçus ne sont PAS gardés (ils seraient périmés dès le premier changement), et
    // `expedition()` est le seul endroit où l'`Expedition` complète est construite, en les relisant.
    int m_profondeur;
    Generateur::result_type m_graineDExpedition;
    std::vector<DefinitionPersonnage> m_classes;  // une par membre, dans l'ordre des places
    int m_experience;
    TileMap m_carte;
    Escouade m_escouade;
    std::vector<Monstre> m_monstres;
    std::vector<ObjetInteractif> m_objets;
    ObjetsAuSol m_sol;
    std::optional<int> m_interactionEnCours;  // identifiant de l'objet que l'escouade va activer
    std::optional<int> m_objetAuSolVise;      // identifiant de l'objet au sol que l'escouade va ramasser (jamais un indice)
    bool m_escalierVise = false;              // l'escouade marche vers l'escalier pour le prendre
    bool m_escalierPris = false;              // l'ancre est arrivée à l'escalier visé
    std::optional<LancerDeZone> m_lancerDeZoneDemande;
    std::optional<AdresseSkill> m_skillDemande;
    std::optional<sf::Vector2f> m_deplacementOrdonne;
    Generateur m_generateur;  // passé par référence jusqu'au tirage, jamais copié
    Vision m_vision;          // ce que le joueur sait de la carte (l'union de ce que voient les personnages vivants)
    std::vector<EvenementDeCombat> m_evenements;  // produits par mettreAJour, vidés par prendreEvenements
};
