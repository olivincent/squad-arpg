// ./src/combat/Degats.hpp
#pragma once
#include "combat/Element.hpp"

// Les dégâts d'un coup en portions : une quantité par élément, chacune réduite par sa propre défense chez le
// défenseur (`resoudreCoup`). Le total est la somme des portions. Ne pas confondre avec `DegatsSubis`, l'événement
// de combat qui dit ce qu'une cible a perdu.
using Degats = ParElement<int>;

// Les portions d'un coup en cours de résolution : en flottants, pour qu'aucune étape n'arrondisse. Le moteur du coup
// n'arrondit qu'une fois par portion, à la fin (`resoudreCoup`), comme la pénétration d'armure ne s'arrondit pas.
using Portions = ParElement<float>;

// Des dégâts d'un seul élément : `montant` points de cet élément, rien ailleurs.
[[nodiscard]] constexpr Degats degatsDe(Element element, int montant) {
    Degats degats;
    degats[element] = montant;
    return degats;
}

// Des dégâts purement physiques : ceux de toute source qui n'a pas d'élément (attaque de base, skill ordinaire,
// coup d'un monstre).
[[nodiscard]] constexpr Degats degatsPhysiques(int montant) {
    return degatsDe(Element::Physique, montant);
}

// Ces dégâts, en portions flottantes : le point de départ de la résolution d'un coup.
[[nodiscard]] constexpr Portions enPortions(const Degats& degats) {
    Portions portions;
    for (const Element element : s_elements) {
        portions[element] = static_cast<float>(degats[element]);
    }
    return portions;
}
