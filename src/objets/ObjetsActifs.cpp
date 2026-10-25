// ./src/objets/ObjetsActifs.cpp
#include "objets/ObjetsActifs.hpp"
#include <cstddef>

namespace {
// Les caractéristiques de la base et des bonus des objets actifs autres que celui de l'indice `sauf`.
Caracteristiques sansLui(const Caracteristiques& base, const std::vector<std::optional<ExigencesEtBonus>>& portes,
                         const std::vector<bool>& actifs, std::size_t sauf) {
    Caracteristiques total = base;
    for (std::size_t i = 0; i < portes.size(); ++i) {
        const std::optional<ExigencesEtBonus>& porte = portes[i];
        if (i != sauf && actifs[i] && porte.has_value()) {
            total += porte->bonus;
        }
    }
    return total;
}

bool seuilsAtteints(const std::vector<SeuilDeCaracteristique>& seuils, const Caracteristiques& caracteristiques) {
    for (const SeuilDeCaracteristique& seuil : seuils) {
        if (caracteristiques.valeur(seuil.caracteristique) < seuil.valeur) {
            return false;
        }
    }
    return true;
}
}  // namespace

std::vector<bool> objetsActifs(const Caracteristiques& base, const std::vector<std::optional<ExigencesEtBonus>>& portes) {
    std::vector<bool> actifs(portes.size());
    for (std::size_t i = 0; i < portes.size(); ++i) {
        actifs[i] = portes[i].has_value();
    }
    bool retire = true;
    while (retire) {
        retire = false;
        for (std::size_t i = 0; i < portes.size(); ++i) {
            const std::optional<ExigencesEtBonus>& porte = portes[i];
            if (actifs[i] && porte.has_value() && !seuilsAtteints(porte->seuils, sansLui(base, portes, actifs, i))) {
                actifs[i] = false;
                retire = true;
            }
        }
    }
    return actifs;
}
