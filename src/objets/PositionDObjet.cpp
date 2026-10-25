// ./src/objets/PositionDObjet.cpp
#include "objets/PositionDObjet.hpp"

#include <type_traits>

#include "objets/CatalogueObjets.hpp"

namespace {
bool objetPeutAller(const Objet& objet, Emplacement emplacement) {
    return peutAller(definitionObjet(objet.modele).categorie, emplacement);
}

// L'emplacement de cette position, vide si c'est une case du sac. (`objets/` ne dépend que de
// `base/` : pas de `Surcharges`, qui est dans `combat/`.)
std::optional<Emplacement> emplacementDe(const PositionDObjet& position) {
    return std::visit(
        [](const auto& precise) -> std::optional<Emplacement> {
            if constexpr (std::is_same_v<std::decay_t<decltype(precise)>, PositionPortee>) {
                return precise.emplacement;
            } else {
                return std::nullopt;
            }
        },
        position);
}
}  // namespace

bool deplacementPermis(const PositionDObjet& de, const std::optional<Objet>& objetDe, const PositionDObjet& vers,
                       const std::optional<Objet>& objetVers) {
    if (!objetDe.has_value()) {
        return false;
    }
    if (de == vers) {
        return true;
    }
    const std::optional<Emplacement> arrivee = emplacementDe(vers);
    if (arrivee.has_value() && !objetPeutAller(*objetDe, *arrivee)) {
        return false;
    }
    // Échange : l'objet d'arrivée prend la place de départ, qui n'a de contrainte que si c'est un emplacement.
    const std::optional<Emplacement> depart = emplacementDe(de);
    if (objetVers.has_value() && depart.has_value() && !objetPeutAller(*objetVers, *depart)) {
        return false;
    }
    return true;
}
