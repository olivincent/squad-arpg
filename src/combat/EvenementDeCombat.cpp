// ./src/combat/EvenementDeCombat.cpp
#include "combat/EvenementDeCombat.hpp"

std::vector<DetailEvenement> detailsDuCoup(const ResultatDuCoup& resultat, bool opportunite) {
    if (resultat.esquive) {
        return {Esquive{}};
    }
    std::vector<DetailEvenement> details;
    if (resultat.montant > 0) {
        details.push_back(DegatsSubis{resultat.montant, opportunite, resultat.critique, resultat.portions.elementDominant()});
    }
    if (resultat.bloque > 0) {
        details.push_back(Bloque{resultat.bloque, resultat.montant == 0});
    }
    // Un événement par famille qui a retenu quelque chose : la portion physique pour l'Aegis, tout le reste pour le Ward.
    const int parAegis = resultat.absorbees[Element::Physique];
    const int parWard = resultat.absorbees.total() - parAegis;
    if (parAegis > 0) {
        details.push_back(Absorbe{parAegis, FamilleDAbsorption::Aegis, resultat.montant == 0});
    }
    if (parWard > 0) {
        details.push_back(Absorbe{parWard, FamilleDAbsorption::Ward, resultat.montant == 0});
    }
    return details;
}
