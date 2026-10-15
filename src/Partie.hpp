// ./src/Partie.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "Aleatoire.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "Monstre.hpp"
#include "Niveau.hpp"
#include "TileMap.hpp"

// Ce que le clic du joueur a désigné.
enum class ResultatClic { Monstre, Sol, Ignore };

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

    // Verrouille le monstre qui contient ce point (Monstre), sinon lève le
    // verrou (Sol). Ignore si l'escouade est vaincue : rien ne change.
    ResultatClic cliquer(sf::Vector2f point);
    // Skill du personnage de cette place (de 1 à 3), tenté dans mettreAJour.
    void demanderSkill(std::size_t place);
    // Skill de zone de cette place lancé au point (monde), dans mettreAJour.
    void lancerSkillDeZone(std::size_t place, sf::Vector2f point);
    // Envoie l'escouade vers ce point, dans mettreAJour, si elle est vivante.
    void ordonnerDeplacement(sf::Vector2f point);

    // Fige le monde (rien ne bouge) une fois la partie perdue ou gagnée.
    void mettreAJour(float dt);
    void dessiner(sf::RenderTarget& rendu) const;

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
    void appliquer(const std::vector<Coup>& coups);
    // Étapes de mettreAJour(), dans l'ordre où elles sont appelées.
    void resoudreLesCoups(float dt);
    void faireTournerLesMonstres(float dt);
    void faireFrapperLeMonstre(Monstre& monstre, std::optional<Cercle> proie, float dt);

    TileMap m_carte;
    Escouade m_escouade;
    std::vector<Monstre> m_monstres;
    std::optional<LancerDeZone> m_lancerDeZoneDemande;
    std::optional<std::size_t> m_skillDemande;
    std::optional<sf::Vector2f> m_deplacementOrdonne;
    Generateur m_generateur;  // passé par référence jusqu'au tirage, jamais copié
};
