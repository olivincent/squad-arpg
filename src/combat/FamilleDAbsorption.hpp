// ./src/combat/FamilleDAbsorption.hpp
#pragma once
#include "combat/Degats.hpp"
#include "combat/Element.hpp"
#include "combat/EtatDeControle.hpp"

// Les deux familles d'absorption (étape 114) : ce que protège un bouclier d'absorption, et contre quel contrôle il immunise tant qu'il lui
// reste des points (`docs/classes-et-combat.md`, section 9).
//   Aegis : absorbe le physique ; immunise contre le contrôle physique (Stun, Slow).
//   Ward  : absorbe tout le non-physique (feu, sacré, et le poison, la foudre… à venir) ; immunise contre le contrôle magique (Charm,
//           Fear, Disruption).
enum class FamilleDAbsorption { Aegis, Ward };

// La famille qui absorbe un élément : le physique va aux Aegis, tout autre élément aux Ward. Seule fonction qui fasse ce lien.
[[nodiscard]] FamilleDAbsorption familleQuiAbsorbe(Element element);

// La famille qui immunise contre une catégorie de résistance : Aegis pour l'étourdissement et le ralentissement, Ward pour le charme, la
// peur et la Disruption. Seule fonction qui fasse ce lien (l'entrave suit la résistance au ralentissement : `etatQuiResiste`).
[[nodiscard]] FamilleDAbsorption familleQuiImmunise(EtatDeControle etat);

// Les points d'absorption qu'une entité a, par famille, tous ses boucliers additionnés. Ce que `resoudreCoup` et `resoudreTick` lisent
// (`Defenses::absorption`) et ce que la barre de vie montre.
struct PointsDAbsorption {
    int aegis = 0;
    int ward = 0;

    [[nodiscard]] constexpr int deLaFamille(FamilleDAbsorption famille) const {
        return famille == FamilleDAbsorption::Aegis ? aegis : ward;
    }

    constexpr bool operator==(const PointsDAbsorption&) const = default;
};

// Ce que les boucliers retiennent de ces portions, élément par élément : pour chaque famille, le plus petit de ce qu'elle a et de ce que
// ses éléments lui apportent, pris dans l'ordre de `s_elements` (le feu avant le sacré quand un Ward ne suffit pas pour les deux).
// Précondition : aucune portion n'est négative.
[[nodiscard]] Degats portionsAbsorbees(const Degats& portions, const PointsDAbsorption& points);
