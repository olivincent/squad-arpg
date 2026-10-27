// ./src/combat/IdMonstre.hpp
#pragma once

// L'identifiant d'un monstre dans une partie (distinct d'une place de personnage : les deux sont des entiers, et on ne
// les confond plus). Donné par `Partie` à la création, jamais réutilisé pendant l'étage.
struct IdMonstre {
    int valeur;

    auto operator<=>(const IdMonstre&) const = default;
};
