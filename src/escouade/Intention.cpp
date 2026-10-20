// ./src/escouade/Intention.cpp
#include "escouade/Intention.hpp"
#include "combat/Cible.hpp"

namespace {
ApprocherMonstre approcher(EntiteVue monstre, float portee) {
    return {monstre.id, monstre.cercle, portee};
}
}  // namespace

Intention choisirIntention(const SituationDuPersonnage& situation) {
    if (situation.enExecution) {
        return ResterSurPlace{situation.monstreDeLaPlace};
    }
    if (situation.skillEnAttente.has_value()) {
        return *situation.skillEnAttente;
    }
    if (situation.degagement.has_value()) {
        return approcher(*situation.degagement, situation.porteeAttaqueDeBase);
    }
    if (situation.vivant && situation.verrou.has_value()) {
        if (situation.verrouAtteint) {
            return ResterSurPlace{situation.verrou->id};
        }
        return approcher(*situation.verrou, situation.porteeAttaqueDeBase);
    }
    if (situation.vivant && situation.ancreArretee && situation.agresseur.has_value()) {
        if (situation.monstreAPortee) {
            return ResterSurPlace{situation.agresseur->id};
        }
        return approcher(*situation.agresseur, situation.porteeAttaqueDeBase);
    }
    return AllerAuPoint{situation.placeDeFormation};
}

std::optional<int> monstreDeLIntention(const Intention& intention) {
    return std::visit(
        Surcharges{
            [](const ResterSurPlace& rester) { return rester.placeAutourDe; },
            [](const AllerAuPoint&) -> std::optional<int> { return std::nullopt; },
            [](const ApprocherMonstre& approche) -> std::optional<int> { return approche.idMonstre; },
        },
        intention);
}

std::vector<Cercle> cerclesAContourner(const std::vector<EntiteVue>& monstres, const Intention& intention) {
    const std::optional<int> vise = monstreDeLIntention(intention);
    std::vector<Cercle> aContourner;
    for (const EntiteVue& monstre : monstres) {
        if (monstre.id != vise) {
            aContourner.push_back(monstre.cercle);
        }
    }
    return aContourner;
}
