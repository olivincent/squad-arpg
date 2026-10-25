// ./src/combat/Element.hpp
#pragma once
#include <array>
#include <cstddef>
#include <type_traits>

// Les éléments d'un coup : de quoi il est fait, donc ce qui le réduit chez le défenseur (l'armure pour le physique,
// une résistance pour chaque élément). Seulement ceux qui ont une source dans le jeu ; les autres (froid, foudre,
// poison, nécrotique, saignement) s'ajoutent avec leur premier skill ou monstre (`docs/classes-et-combat.md`).
// `Nombre` n'est pas un élément : c'est la sentinelle qui compte les autres. Elle reste la dernière, et c'est elle
// qui donne la taille de `ParElement` : ajouter un élément avant elle agrandit tous les tableaux, sans second endroit
// à tenir à jour (une taille écrite à la main, ou un `static_assert` à relire, serait un oubli possible). Le prix : un
// `switch` sur un `Element` devrait traiter la sentinelle, donc aucun n'existe ; ce qui dépend de l'élément (nom,
// couleur) se lit dans un `ParElement`, qui se parcourt avec `s_elements`.
enum class Element { Physique, Feu, Sacre, Nombre };

inline constexpr std::size_t s_nombreDElements = static_cast<std::size_t>(Element::Nombre);

// Tous les éléments, dans l'ordre de l'énumération : pour parcourir un `ParElement`.
inline constexpr std::array<Element, s_nombreDElements> s_elements = [] {
    std::array<Element, s_nombreDElements> elements = {};
    for (std::size_t i = 0; i < s_nombreDElements; ++i) {
        elements[i] = static_cast<Element>(i);
    }
    return elements;
}();

// Une valeur par élément, toutes à zéro par défaut, indexée par `Element` (le `static_cast` est caché ici). Une
// valeur, pas une référence : se copie comme un tableau. `Degats` et les résistances en sont deux usages.
template <typename T>
struct ParElement {
    std::array<T, s_nombreDElements> valeurs = {};

    [[nodiscard]] constexpr T& operator[](Element element) {
        return valeurs[static_cast<std::size_t>(element)];
    }
    [[nodiscard]] constexpr const T& operator[](Element element) const {
        return valeurs[static_cast<std::size_t>(element)];
    }

    // La somme des valeurs, pour un type numérique.
    [[nodiscard]] constexpr T total() const
        requires std::is_arithmetic_v<T>
    {
        T somme = {};
        for (const T& valeur : valeurs) {
            somme += valeur;
        }
        return somme;
    }

    bool operator==(const ParElement&) const = default;
};
