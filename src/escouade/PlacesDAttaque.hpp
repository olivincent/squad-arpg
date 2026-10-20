// ./src/escouade/PlacesDAttaque.hpp
#pragma once
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <vector>
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"

class TileMap;

// Les places d'attaque : pour chaque assaillant, la place retenue sur un anneau autour de la cible
// qu'il approche, pour que les assaillants arrivent chacun de leur côté. Ne connaît ni Personnage ni
// Escouade : elle reçoit la cible, l'indice, le cercle et la portée de l'assaillant, un point de
// référence, les occupants et la carte.
class PlacesDAttaque {
public:
    // Une place retenue : l'anneau et l'angle autour de la cible. La position se recalcule depuis le
    // centre actuel de la cible.
    struct Place {
        int idCible;
        float rayon;      // de l'anneau, en pixels
        sf::Angle angle;  // autour du centre de la cible
    };
    // Un assaillant qui approche une cible.
    struct Approche {
        EntiteVue cible;
        std::size_t assaillant;   // indice, de 0 au nombre d'assaillants - 1
        Cercle cercleAssaillant;
        float portee;             // de l'action avec laquelle il approche, bord à bord
        sf::Vector2f reference;   // la première candidate est dans la direction de la cible vers ce point
    };

    // `ecartMinimal` : distance en dessous de laquelle une place n'est pas libre (en pixels).
    PlacesDAttaque(std::size_t nombreDAssaillants, float ecartMinimal);
    // Point de la place d'attaque de cet assaillant autour de la cible, sur un anneau de rayon : rayon de
    // la cible + rayon de l'assaillant + 90 % de la portée. L'angle de `ancienne` est gardé si elle est
    // autour de la même cible, valide et pas occupée ; sinon, la place valide et libre la plus proche de
    // l'angle actuel de l'assaillant. Valide : le carré de l'assaillant ne chevauche aucun mur et la place
    // voit le centre de la cible. Libre : à plus de `ecartMinimal` des places retenues des autres
    // assaillants autour de cette cible, et pas occupée (à plus de `ecartMinimal` d'un autre occupant).
    // `occupants` : par assaillant, son centre s'il occupe sa position (vide sinon) ; celui de
    // l'assaillant est ignoré. Trouvée, la place devient la place retenue de l'assaillant ; vide si
    // aucune place n'est libre (la place retenue ne change pas).
    [[nodiscard]] std::optional<sf::Vector2f> placer(const Approche& approche, const std::optional<Place>& ancienne,
                                                     const std::vector<std::optional<sf::Vector2f>>& occupants,
                                                     const TileMap& carte);
    // Oubli des places (mécanisme actuel, remplacé par les intentions en 82.4) : l'appelant retire la
    // place retenue d'un assaillant (elle est renvoyée), puis la remet avec `retenir` ou en calcule une
    // nouvelle avec `placer` ; sinon elle reste oubliée.
    std::optional<Place> retirer(std::size_t assaillant);
    void retenir(std::size_t assaillant, const std::optional<Place>& place);
    // Identifiant de la cible de la place retenue de cet assaillant ; vide s'il n'en a pas.
    // Provisoire, remplacé par les intentions en 82.4.
    [[nodiscard]] std::optional<int> cibleApprochee(std::size_t assaillant) const;

private:
    std::vector<std::optional<Place>> m_places;  // par assaillant (vide = il n'approche aucune cible)
    float m_ecartMinimal;                        // en pixels
};
