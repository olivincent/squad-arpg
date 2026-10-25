// ./src/monde/CategorieDeMonstre.hpp
#pragma once
#include <array>
#include <cstddef>

// La catégorie d'un monstre : ce que le niveau dit d'un monstre à placer (`MonstreDuNiveau`). Le nom seul est
// ici, dans `monde/`, parce que le niveau et le générateur ne peuvent pas connaître les statistiques ; les
// valeurs de chaque catégorie sont le catalogue de `monstres/DefinitionDeMonstre.hpp`. L'ordre de l'enum est
// l'index dans ce catalogue.
enum class CategorieDeMonstre { Infanterie };

inline constexpr std::size_t s_nombreDeCategoriesDeMonstre = 1;

// Toutes les catégories, dans l'ordre de l'enum, pour les parcourir.
inline constexpr std::array<CategorieDeMonstre, s_nombreDeCategoriesDeMonstre> s_categoriesDeMonstre = {
    CategorieDeMonstre::Infanterie};
