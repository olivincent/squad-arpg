// ./src/combat/EtatDeControle.hpp
#pragma once
#include <array>
#include <cstddef>

// Les états de contrôle qu'une entité peut subir (`docs/classes-et-combat.md`, section 13) : le contrôle physique (étourdissement,
// ralentissement), que résiste la Constitution, et le contrôle magique (charme, peur, Disruption), que résiste la Resolve.
// Aucune mécanique ne les inflige encore (étape 110) : seules existent les résistances. Comme `Element`, `Nombre` est la
// sentinelle qui donne la taille de `ParEtat` ; ce qui dépend de l'état (nom, résistance) se lit dans un `ParEtat`, qui se parcourt
// avec `s_etatsDeControle`.
enum class EtatDeControle { Etourdissement, Ralentissement, Charme, Peur, Disruption, Nombre };

inline constexpr std::size_t s_nombreDEtatsDeControle = static_cast<std::size_t>(EtatDeControle::Nombre);

// Tous les états, dans l'ordre de l'énumération : pour parcourir un `ParEtat`.
inline constexpr std::array<EtatDeControle, s_nombreDEtatsDeControle> s_etatsDeControle = [] {
    std::array<EtatDeControle, s_nombreDEtatsDeControle> etats = {};
    for (std::size_t i = 0; i < s_nombreDEtatsDeControle; ++i) {
        etats[i] = static_cast<EtatDeControle>(i);
    }
    return etats;
}();

// Une valeur par état de contrôle, toutes à zéro par défaut, indexée par `EtatDeControle` (comme `ParElement`).
template <typename T>
struct ParEtat {
    std::array<T, s_nombreDEtatsDeControle> valeurs = {};

    [[nodiscard]] constexpr T& operator[](EtatDeControle etat) {
        return valeurs[static_cast<std::size_t>(etat)];
    }
    [[nodiscard]] constexpr const T& operator[](EtatDeControle etat) const {
        return valeurs[static_cast<std::size_t>(etat)];
    }

    bool operator==(const ParEtat&) const = default;
};
