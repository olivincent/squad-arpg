// ./src/combat/PlageDeDegats.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Degats.hpp"

// Les dégâts d'une arme avant le coup : pour chaque élément, un minimum et un maximum (bornes incluses). Chaque coup en
// tire des dégâts précis (`tirerLesDegats`) ; la Puissance et la défense du coup viennent après, sur ce tirage.
struct PlageDeDegats {
    Degats min;
    Degats max;

    bool operator==(const PlageDeDegats&) const = default;
};

// Une plage sans écart : le minimum est le maximum, le coup ne tire rien (les mains nues, les portions que les affixes
// ajoutent à une arme).
[[nodiscard]] constexpr PlageDeDegats plageFixe(const Degats& degats) {
    return {degats, degats};
}

// Des dégâts tirés dans la plage : pour chaque élément, dans l'ordre de `s_elements`, un entier uniforme entre son minimum et son
// maximum. AUCUN tirage quand le minimum est le maximum (comme une recharge fixe), donc le `Generateur` n'avance pas pour une
// plage fixe. `assert` : le minimum d'aucun élément ne dépasse son maximum.
[[nodiscard]] Degats tirerLesDegats(Generateur& generateur, const PlageDeDegats& plage);
