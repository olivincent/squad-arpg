// ./src/jeu/ObjetAuSol.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "objets/Objet.hpp"

class TileMap;

// Un objet posé au sol, dans le monde. Possédé par la `Partie`, qui lui donne un identifiant jamais réutilisé : tout ce
// qui désigne un objet au sol (survol, clic, ramassage) passe par lui, jamais par un indice du conteneur, qui change
// quand on retire un élément.
struct ObjetAuSol {
    int id;
    Objet objet;
    sf::Vector2f position;  // le centre de sa case, en pixels du monde

    bool operator==(const ObjetAuSol&) const = default;
};

// Le rayon (en pixels du monde) de la zone où le curseur désigne un objet au sol : un peu plus que son dessin (un losange de
// 8 px de rayon, `interface/AffichageObjetsAuSol`), pour ne pas exiger la précision au pixel.
inline constexpr float s_rayonDeSurvolDObjet = 14.0f;

// La distance (centre à centre, en pixels du monde) de l'ancre de l'escouade à un objet visé en deçà de laquelle il est ramassé
// (« à tester »).
inline constexpr float s_distanceDeRamassage = 30.0f;

// Où poser un objet de plus, à partir de `origine` : le centre de la case de `origine` si aucun objet de `auSol` n'y est,
// sinon celui de la case voisine la plus proche qui est praticable (ni mur ni porte fermée) et sans objet. Les voisines
// sont essayées par couronnes de cases autour de l'origine (jusqu'à trois cases), les plus proches d'abord ; si toutes sont
// prises ou bloquées, l'objet se pose sur la case de l'origine, avec celui qui y est. `origine` est un point praticable
// (un monstre, l'ancre de l'escouade). Une fonction pure : même résultat pour les mêmes arguments, aucun hasard.
[[nodiscard]] sf::Vector2f positionPourUnObjet(const TileMap& carte, sf::Vector2f origine,
                                               const std::vector<ObjetAuSol>& auSol);
