// ./src/base/Aleatoire.hpp
#pragma once
#include <random>

// Le générateur de nombres aléatoires du jeu. Le reste du code écrit
// `Generateur` et ne dépend pas de l'algorithme choisi ici.
//
// Un `Generateur` se passe TOUJOURS par référence (`Generateur&`), jamais
// par valeur : une copie repart du même état et redonnerait la même suite
// de nombres. La `Partie` en possède un seul ; il descend en paramètre
// jusqu'au tirage, et personne d'autre n'en garde de référence.
using Generateur = std::mt19937;

// Tirage dans [min, max], bornes INCLUSES (comme std::uniform_int_distribution).
[[nodiscard]] inline int tirerEntre(Generateur& generateur, int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generateur);
}
