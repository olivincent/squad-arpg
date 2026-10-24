// ./src/combat/EvenementDeCombat.cpp
#include "combat/EvenementDeCombat.hpp"

std::vector<DetailEvenement> detailsDuCoup(const ResultatDuCoup& resultat, bool opportunite) {
    if (resultat.esquive) {
        return {Esquive{}};
    }
    std::vector<DetailEvenement> details;
    if (resultat.montant > 0) {
        details.push_back(Degats{resultat.montant, opportunite, resultat.critique});
    }
    if (resultat.bloque > 0) {
        details.push_back(Bloque{resultat.bloque, resultat.montant == 0});
    }
    return details;
}
