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
#include "jeu/ObjetAuSol.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monde/Niveau.hpp"
#include "monde/ObjetInteractif.hpp"
#include "monde/TileMap.hpp"
#include "monde/Vision.hpp"
#include "objets/PositionDObjet.hpp"

// Ce que le clic du joueur a désigné : un monstre, un objet (une porte fermée ou un objet au sol), le sol.
enum class ResultatClic { Monstre, Objet, Sol, Ignore };

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
    // La graine initialise le générateur aléatoire de la partie, qui est à elle seule.
    // `composition` : les personnages de l'escouade (elle n'appartient pas au niveau : c'est le choix du joueur).
    // `catalogue` : les définitions des monstres, une par catégorie ; chaque monstre du niveau reçoit celle de sa
    // catégorie. Le jeu ne le précise pas ; les tests y substituent un catalogue sans hasard.
    Partie(const Niveau& niveau, std::vector<DefinitionPersonnage> composition, Generateur::result_type graine,
           const CatalogueDeMonstres& catalogue = catalogueDeMonstres());

    // Verrouille le monstre ciblable qui contient ce point (Monstre) ; sinon, sur un objet au sol (`objetAuSolSous`), lève le
    // verrou et envoie l'escouade le ramasser (Objet : `ramasserSiAPortee` le ramasse quand l'ancre arrive, et tout autre
    // ordre remplace cette visée) ; sinon, sur une porte
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

    // Jette l'objet de cette position (case du sac ou emplacement d'un personnage, `Escouade::retirerObjet`) : il est retiré de
    // l'escouade, qui recalcule ce que son retrait change, et posé au sol près de l'ancre (`positionPourUnObjet`) avec un
    // identifiant neuf. Un objet n'est jamais perdu ni dupliqué : il quitte l'escouade et arrive au sol dans le même appel.
    // Faux, sans rien changer, si la position est vide.
    [[nodiscard]] bool jeterUnObjet(const PositionDObjet& position);

    // Tire `nombre` objets au hasard (`genererObjet`, avec le générateur de la partie) et les met dans le sac de l'escouade,
    // tout ou rien : sans `nombre` cases libres, rien n'est ajouté, aucun tirage n'est fait (le générateur n'avance pas) et
    // le résultat est faux. Pour la touche de débogage F7.
    [[nodiscard]] bool ramasserDesObjetsAleatoires(std::size_t nombre);

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
    // Les objets posés au sol, dans l'ordre où ils sont tombés.
    [[nodiscard]] const std::vector<ObjetAuSol>& getObjetsAuSol() const;
    // L'objet au sol que ce point (pixels du monde) désigne : le plus proche à moins de `s_rayonDeSurvolDObjet`, nullptr s'il
    // n'y en a pas. Un objet est comme le terrain : sur une case jamais vue (inconnue), il est caché, donc introuvable ;
    // dans une case explorée, même hors de vue, il se désigne.
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
    // Retire les monstres morts ; chacun, dans l'ordre du vecteur, laisse son butin (un tirage de chance, puis les objets) à
    // l'endroit de sa mort. Le seul endroit où un monstre quitte le vecteur.
    void retirerLesMorts();
    // Pose cet objet au sol, à la place libre la plus proche de `origine` (`positionPourUnObjet`), avec un identifiant neuf.
    void poserAuSol(const Objet& objet, sf::Vector2f origine);
    // `enPoursuiteAvant` : statut de chaque monstre de `m_monstres` avant leur mise à jour.
    void alerterLesGroupes(const std::vector<bool>& enPoursuiteAvant);

    TileMap m_carte;
    Escouade m_escouade;
    std::vector<Monstre> m_monstres;
    std::vector<ObjetInteractif> m_objets;
    std::vector<ObjetAuSol> m_objetsAuSol;
    int m_prochainIdObjetAuSol = 1;  // jamais réutilisé dans la partie
    std::optional<int> m_interactionEnCours;  // identifiant de l'objet que l'escouade va activer
    std::optional<int> m_objetAuSolVise;      // identifiant de l'objet au sol que l'escouade va ramasser (jamais un indice)
    std::optional<LancerDeZone> m_lancerDeZoneDemande;
    std::optional<AdresseSkill> m_skillDemande;
    std::optional<sf::Vector2f> m_deplacementOrdonne;
    Generateur m_generateur;  // passé par référence jusqu'au tirage, jamais copié
    Vision m_vision;          // ce que le joueur sait de la carte (l'union de ce que voient les personnages vivants)
    std::vector<EvenementDeCombat> m_evenements;  // produits par mettreAJour, vidés par prendreEvenements
};
