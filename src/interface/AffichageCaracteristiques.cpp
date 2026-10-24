// ./src/interface/AffichageCaracteristiques.cpp
#include "interface/AffichageCaracteristiques.hpp"

// Le `switch` n'a pas de `default` : une caractéristique ajoutée sans nom fait avertir le compilateur.
std::string nomDeLaCaracteristique(Caracteristique caracteristique) {
    switch (caracteristique) {
        case Caracteristique::Puissance: return "Puissance";
        case Caracteristique::Endurance: return "Endurance";
        case Caracteristique::Adresse: return "Adresse";
        case Caracteristique::Savoir: return "Savoir";
        case Caracteristique::Volonte: return "Volonté";
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
