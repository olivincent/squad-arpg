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

std::string ligneDEffetActif(const EffetActif& effet) {
    const std::string valeur = std::visit(
        Surcharges{[](const ModificateurDeStatistique& modificateur) {
            const std::string signe = modificateur.pourcentage < 0 ? "−" : "+";
            return signe + std::to_string(std::abs(modificateur.pourcentage)) + " % " + nomDeLaStatistique(modificateur.statistique);
        }},
        effet.sorte);
    const int secondes = static_cast<int>(std::ceil(effet.dureeRestante));
    return nomDuSkill(effet.source) + ": " + valeur + " (" + std::to_string(secondes) + " s)";
}
