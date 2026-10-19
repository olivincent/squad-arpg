// ./src/monde/Mobile.hpp
#pragma once
#include "base/Cercle.hpp"
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>

class TileMap;

class Mobile {
public:
    Mobile(sf::Vector2f centre, sf::Vector2f taille, float vitesse);
    // `aContourner` : cercles que le chemin évite (coût ajouté dans le A*, lissage qui ne les
    // traverse pas). Distinct des obstacles bloquants de mettreAJour. Vide = murs seulement.
    void allerVers(sf::Vector2f destination, const TileMap& carte, const std::vector<Cercle>& aContourner = {});
    void allerToutDroit(sf::Vector2f destination);
    void arreter();
    void mettreAJour(float dt, const TileMap& carte, const std::vector<Cercle>& obstacles);
    // Pousse le mobile de `decalage` (murs et obstacles respectés, avec glissement),
    // sans toucher au chemin ni à estEnMouvement().
    void decaler(sf::Vector2f decalage, const TileMap& carte, const std::vector<Cercle>& obstacles);
    [[nodiscard]] sf::Vector2f getCentre() const;
    [[nodiscard]] sf::Vector2f getTaille() const;
    // Cercle de collision, voir cercleDuRectangle.
    [[nodiscard]] Cercle getCercle() const;
    [[nodiscard]] std::optional<sf::Vector2f> directionVoulue() const;
    // Vrai si, au dernier tic, le mobile avait un point à atteindre à plus de
    // quelques centièmes de pixel, qu'il ait pu avancer ou non : un mobile
    // bloqué qui essaie d'avancer est en mouvement, un mobile qui vient
    // d'arriver l'est encore pour ce tic. Faux avant le premier tic et après
    // arreter().
    [[nodiscard]] bool estEnMouvement() const;
    // Vrai quand, depuis au moins 0,3 s, le mobile a un point à atteindre mais avance de moins de
    // 10 % de son pas. Remis à zéro par allerVers, allerToutDroit et arreter ; faux à l'arrêt.
    // C'est au propriétaire de recalculer le chemin (il connaît les obstacles à contourner).
    [[nodiscard]] bool estBloque() const;
    // Points du chemin restant à parcourir (le prochain en premier), pour le débogage et les tests.
    [[nodiscard]] std::vector<sf::Vector2f> cheminRestant() const;

private:
    sf::Vector2f m_position;            // coin haut-gauche, en pixels du monde
    sf::Vector2f m_taille;              // en pixels
    std::vector<sf::Vector2f> m_chemin; // points (centre) à atteindre, en pixels du monde
    std::size_t m_indexChemin;          // indice du prochain point de m_chemin à atteindre
    float m_vitesse;                    // en pixels par seconde
    bool m_enMouvement;                 // voir estEnMouvement()
    float m_tempsBloque;                // secondes consécutives sans avancer vraiment, voir estBloque()
};
