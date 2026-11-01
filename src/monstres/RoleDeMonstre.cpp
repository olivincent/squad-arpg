// ./src/monstres/RoleDeMonstre.cpp
#include "monstres/RoleDeMonstre.hpp"
#include "combat/Cible.hpp"

EtatDuRole etatInitialDuRole(const RoleDeMonstre& role) {
    return std::visit(Surcharges{[](const RoleMelee&) -> EtatDuRole { return EtatMelee{}; },
                                 [](const RoleTireur& tireur) -> EtatDuRole { return EtatTireur{.role = tireur}; },
                                 [](const RoleSoigneur& soigneur) -> EtatDuRole {
                                     return EtatSoigneur{.role = soigneur, .soin = Skill(soigneur.soin), .cibleDuSoin = std::nullopt};
                                 }},
                      role);
}

namespace {
// `a` est-il plus blessé que `b` en proportion (a.pointsDeVie / a.pointsDeVieMax < b.pointsDeVie / b.pointsDeVieMax) ? Produit en
// croix, en entiers : aucune division. À proportion égale, le plus petit identifiant.
bool estPlusBlesse(const AllieVu& a, const AllieVu& b) {
    const int proportionA = a.pointsDeVie * b.pointsDeVieMax;
    const int proportionB = b.pointsDeVie * a.pointsDeVieMax;
    return proportionA < proportionB || (proportionA == proportionB && a.id < b.id);
}
}  // namespace

std::optional<AllieVu> choisirLeBlesse(const std::vector<AllieVu>& allies, int seuilDeBlessure) {
    const AllieVu* blesse = nullptr;
    for (const AllieVu& allie : allies) {
        const bool assezBlesse = allie.pointsDeVie > 0 && allie.pointsDeVie * 100 < seuilDeBlessure * allie.pointsDeVieMax;
        if (assezBlesse && (blesse == nullptr || estPlusBlesse(allie, *blesse))) {
            blesse = &allie;
        }
    }
    return blesse == nullptr ? std::nullopt : std::optional<AllieVu>(*blesse);
}
