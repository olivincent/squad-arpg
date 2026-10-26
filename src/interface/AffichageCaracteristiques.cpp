// ./src/interface/AffichageCaracteristiques.cpp
#include "interface/AffichageCaracteristiques.hpp"

#include <cmath>

#include "combat/StatistiquesDeCombat.hpp"

// Le `switch` n'a pas de `default` : une caractéristique ajoutée sans nom fait avertir le compilateur.
std::string nomDeLaCaracteristique(Caracteristique caracteristique) {
    switch (caracteristique) {
        case Caracteristique::Puissance: return "Might";
        case Caracteristique::Endurance: return "Constitution";
        case Caracteristique::Adresse: return "Dexterity";
        case Caracteristique::Savoir: return "Intellect";
        case Caracteristique::Volonte: return "Resolve";
    }
    return "";
}

std::string ligneDeCaracteristique(Caracteristique caracteristique, int valeurEffective, int bonus) {
    std::string ligne = nomDeLaCaracteristique(caracteristique) + " " + std::to_string(valeurEffective);
    if (bonus != 0) {
        ligne += " (" + std::string(bonus > 0 ? "+" : "") + std::to_string(bonus) + ")";
    }
    return ligne;
}

std::string ligneDePointsLibres(int points) {
    return "Points: " + std::to_string(points);
}

std::string textePointsEnAttente(int points) {
    return points > 0 ? "(+" + std::to_string(points) + ")" : "";
}

std::string ligneDEffetDeLaCaracteristique(Caracteristique caracteristique) {
    const std::string nom = nomDeLaCaracteristique(caracteristique) + ": ";
    // Pas de `default` : une caractéristique ajoutée sans effet décrit fait avertir le compilateur.
    switch (caracteristique) {
        case Caracteristique::Puissance:
            return nom + "+" + std::to_string(std::lround(s_bonusParPoint * 100.0f)) + "% damage and healing per point";
        case Caracteristique::Endurance:
            return nom + "+" + std::to_string(s_pvParPointDEndurance) + " health per point";
        case Caracteristique::Adresse:
            return nom + "+" + std::to_string(s_capaciteParPointDAdresse) + " offensive and defensive capability per point";
        case Caracteristique::Savoir:
        case Caracteristique::Volonte:
            return nom + "no effect yet";
    }
    return "";
}
