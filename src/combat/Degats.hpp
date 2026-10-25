// ./src/combat/Degats.hpp
#pragma once
#include "combat/Element.hpp"

// Les dégâts d'un coup en portions : une quantité par élément, chacune réduite par sa propre défense chez le
// défenseur (`resoudreCoup`). Le total est la somme des portions. Ne pas confondre avec `DegatsSubis`, l'événement
// de combat qui dit ce qu'une cible a perdu.
using Degats = ParElement<int>;

// Des dégâts d'un seul élément : `montant` points de cet élément, rien ailleurs.
[[nodiscard]] constexpr Degats degatsDe(Element element, int montant) {
    Degats degats;
    degats[element] = montant;
    return degats;
}
