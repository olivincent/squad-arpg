// ./src/combat/Place.hpp
#pragma once
#include <cassert>
#include <cstddef>

// La place d'un personnage dans l'escouade : un numéro de 1 à N, celui que le joueur lit à l'écran. `indice` est la
// seule conversion vers la position (de 0 à N-1) dans un tableau, `depuisIndice` le chemin inverse : aucun « + 1 » ni
// « - 1 » n'est écrit ailleurs. Une place hors de 1 à N est représentable (« place invalide » : refus, liste vide) ;
// c'est l'`Escouade` qui sait combien elle en a.
struct Place {
    std::size_t numero;

    [[nodiscard]] static constexpr Place depuisIndice(std::size_t indice) {
        return Place{indice + 1};
    }
    [[nodiscard]] std::size_t indice() const {
        assert(numero >= 1);
        return numero - 1;
    }

    auto operator<=>(const Place&) const = default;
};
