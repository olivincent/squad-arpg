// ./src/Cible.hpp
#pragma once
#include <cstddef>
#include <variant>
#include "Cercle.hpp"

struct CibleAllie {
    std::size_t place;  // de 1 à 4, comme la touche du skill
};

struct CibleMonstre {
    Cercle cercle;
};

// L'absence de cible est un std::optional<Cible> vide, pas un troisième cas.
using Cible = std::variant<CibleAllie, CibleMonstre>;
