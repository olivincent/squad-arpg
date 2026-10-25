// ./src/objets/ObjetsActifs.hpp
#pragma once
#include <optional>
#include <vector>
#include "combat/Caracteristiques.hpp"

// Une valeur minimale de caractéristique que le porteur doit avoir pour équiper un objet (étape 92).
struct SeuilDeCaracteristique {
    Caracteristique caracteristique;
    int valeur;

    bool operator==(const SeuilDeCaracteristique&) const = default;
};

// Ce qu'un objet porté exige et ce qu'il donne, pour le calcul des objets actifs : séparé du catalogue pour que ce
// calcul, qui est une règle pure, se teste sur des objets inventés (une chaîne de dépendances que le catalogue n'a pas).
struct ExigencesEtBonus {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    std::vector<SeuilDeCaracteristique> seuils;
    Caracteristiques bonus;
};

// Quels objets portés sont actifs pour un porteur de ces caractéristiques de base (`portes` : un objet ou rien par
// emplacement). Un objet dont le seuil n'est plus atteint reste porté mais n'a plus aucun effet. Calcul par point fixe :
// on part de tous les objets, on retire ceux dont le seuil n'est pas atteint par la base et les bonus des **autres**
// objets encore actifs, et on recommence jusqu'à stabilité (la liste ne fait que diminuer, donc le calcul se termine ;
// l'ordre des retraits ne change pas le résultat). Suppose des bonus positifs ou nuls, comme tous ceux du catalogue.
// Rend un booléen par emplacement de `portes` (faux pour un emplacement vide).
[[nodiscard]] std::vector<bool> objetsActifs(const Caracteristiques& base, const std::vector<std::optional<ExigencesEtBonus>>& portes);
