// ./src/escouade/Specialite.hpp
#pragma once
#include <string>
#include <vector>
#include "escouade/Composition.hpp"

// Les classes que le joueur peut choisir. Une spécialité est une recette de départ :
// deux personnages de la même classe pourront devenir différents plus tard.
enum class IdSpecialite { Guerrier, Pretre, Mage, Paladin, Rodeur };

struct Specialite {
    IdSpecialite id;
    std::string nom;
    DefinitionPersonnage definition;  // couleur de la classe, sans nuance
};

// Toutes les spécialités, dans l'ordre de `IdSpecialite` : l'unique source de leurs valeurs.
[[nodiscard]] const std::vector<Specialite>& catalogue();

// La spécialité de cet identifiant.
[[nodiscard]] const Specialite& specialite(IdSpecialite id);

// Une définition par identifiant, dans l'ordre reçu (c'est l'ordre des places). La même classe
// peut revenir : sa k-ième occurrence a la couleur de la classe assombrie de 0 %, 20 %, 40 % puis 60 %.
// `assert` : une classe revient 4 fois au plus (`s_tailleDeLEscouade`).
[[nodiscard]] std::vector<DefinitionPersonnage> composer(const std::vector<IdSpecialite>& identifiants);
