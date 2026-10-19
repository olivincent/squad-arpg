// ./src/EntiteVue.hpp
#pragma once
#include <vector>
#include "Cercle.hpp"

// Une entité telle que l'autre camp la voit : son identifiant (celui d'un
// monstre, ou la place 1 à N d'un personnage) et sa forme.
struct EntiteVue {
    int id;
    Cercle cercle;
};

// L'entité portant cet identifiant, nullptr si elle n'est pas dans la liste.
// Le pointeur reste valide tant que la liste n'est pas modifiée.
[[nodiscard]] const EntiteVue* trouverEntite(const std::vector<EntiteVue>& entites, int id);
