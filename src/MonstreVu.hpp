// ./src/MonstreVu.hpp
#pragma once
#include <vector>
#include "Cercle.hpp"

// Un monstre tel que l'escouade le voit : son identifiant et sa forme.
struct MonstreVu {
    int id;
    Cercle cercle;
};

// Le monstre portant cet identifiant, nullptr s'il n'est pas dans la liste.
// Le pointeur reste valide tant que la liste n'est pas modifiée.
[[nodiscard]] const MonstreVu* trouverMonstre(const std::vector<MonstreVu>& monstres, int id);
