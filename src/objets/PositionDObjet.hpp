// ./src/objets/PositionDObjet.hpp
#pragma once
#include <cstddef>
#include <optional>
#include <variant>

#include "objets/Emplacement.hpp"
#include "objets/Objet.hpp"

// Où se trouve un objet : une case du sac (indice à partir de 0) ou un emplacement d'un personnage
// (place de 1 à N, comme partout). `objets/` ne connaît pas `Escouade` : la place n'est qu'un entier.
struct PositionDansLeSac {
    std::size_t caseDuSac;

    bool operator==(const PositionDansLeSac&) const = default;
};
struct PositionPortee {
    std::size_t place;
    Emplacement emplacement;

    bool operator==(const PositionPortee&) const = default;
};
using PositionDObjet = std::variant<PositionDansLeSac, PositionPortee>;

// Un objet peut-il aller de `de` à `vers` ? `objetDe` et `objetVers` sont ce qui s'y trouve (vide si
// la position l'est). Seule source de la règle des déplacements :
// - faux si `objetDe` est vide ; vrai si `de == vers` (rien ne bouge) ;
// - `vers` est un emplacement : l'objet doit pouvoir y aller (`peutAller`) ;
// - `vers` occupé et `de` un emplacement : échange, donc l'objet de `vers` doit aussi pouvoir aller à
//   l'emplacement de `de` ; si `de` est une case du sac, l'objet de `vers` peut toujours y aller.
[[nodiscard]] bool deplacementPermis(const PositionDObjet& de, const std::optional<Objet>& objetDe,
                                     const PositionDObjet& vers, const std::optional<Objet>& objetVers);
