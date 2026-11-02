// ./src/interface/AffichageEffets.cpp
#include "interface/AffichageEffets.hpp"

#include <cmath>
#include <cstdlib>
#include <variant>

#include "combat/Cible.hpp"
#include "interface/AffichageSkills.hpp"

std::string titreDesEffetsActifs() {
    return "Active effects";
}

// Pas de `default` : une statistique ajoutée sans nom fait avertir le compilateur.
std::string nomDeLaStatistique(StatistiqueModifiable statistique) {
    switch (statistique) {
        case StatistiqueModifiable::Armure: return "Armor";
    }
    return "";
}

namespace {
// Le nom d'un effet de dégâts dans le temps : celui de son élément (le saignement est du physique dans le temps). Pas de `default` : un
// élément ajouté sans nom fait avertir le compilateur.
std::string nomDesDegatsDansLeTemps(Element element) {
    switch (element) {
        case Element::Physique: return "Bleeding";
        case Element::Feu: return "Burning";
        case Element::Sacre: return "Holy Fire";
        case Element::Nombre: return "";
    }
    return "";
}

std::string nomDeLEffetActif(const EffetActif& effet) {
    return std::visit(Surcharges{[&](const ModificateurDeStatistique&) { return nomDuSkill(effet.source); },
                                 [](const DegatsDansLeTemps& degats) { return nomDesDegatsDansLeTemps(degats.element); },
                                 // Le soin dans le temps porte le nom de son skill (Renew).
                                 [&](const SoinDansLeTemps&) { return nomDuSkill(effet.source); }},
                      effet.sorte);
}
}  // namespace

std::string ligneDEffetActif(const EffetActif& effet) {
    const std::string valeur = std::visit(
        Surcharges{[&](const ModificateurDeStatistique& modificateur) {
                       const int total = modificateur.pourcentage * effet.piles;
                       return std::string(total < 0 ? "−" : "+") + std::to_string(std::abs(total)) + " % " + nomDeLaStatistique(modificateur.statistique);
                   },
                   [&](const DegatsDansLeTemps& degats) { return std::to_string(degats.montant * effet.piles) + " per s"; },
                   [&](const SoinDansLeTemps& soin) { return std::to_string(soin.montant * effet.piles) + " per s"; }},
        effet.sorte);
    const std::string piles = effet.piles > 1 ? " ×" + std::to_string(effet.piles) : "";
    const int secondes = static_cast<int>(std::ceil(effet.dureeRestante));
    return nomDeLEffetActif(effet) + piles + ": " + valeur + " (" + std::to_string(secondes) + " s)";
}
