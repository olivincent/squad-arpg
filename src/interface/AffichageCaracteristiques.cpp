// ./src/interface/AffichageCaracteristiques.cpp
#include "interface/AffichageCaracteristiques.hpp"

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
