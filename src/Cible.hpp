// ./src/Cible.hpp
#pragma once
#include <cstddef>
#include <variant>
#include <SFML/System/Vector2.hpp>
#include "Cercle.hpp"

struct CibleAllie {
    std::size_t place;  // de 1 à 4, comme la touche du skill
};

struct CibleMonstre {
    int id;         // identifiant du monstre, pour attribuer les dégâts
    Cercle cercle;
};

struct CiblePoint {
    sf::Vector2f point;  // en pixels du monde, fixé au moment du lancer
};

// L'absence de cible est un std::optional<Cible> vide, pas un quatrième cas.
using Cible = std::variant<CibleAllie, CibleMonstre, CiblePoint>;

// Pour std::visit : rassemble plusieurs lambdas en un seul objet appelable,
// une lambda par alternative de Cible. Si une alternative n'a pas sa lambda
// (par exemple après l'ajout d'un quatrième cas), la compilation échoue.
template <class... Ts>
struct Surcharges : Ts... {
    using Ts::operator()...;
};
