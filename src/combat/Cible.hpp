// ./src/combat/Cible.hpp
#pragma once
#include <variant>
#include <SFML/System/Vector2.hpp>
#include "base/Cercle.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/Place.hpp"

struct CibleAllie {
    Place place;  // comme la place d'une AdresseSkill
};

struct CibleMonstre {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    IdMonstre id;   // pour attribuer les dégâts
    Cercle cercle;
};

struct CiblePoint {
    sf::Vector2f point;  // en pixels du monde, fixé au moment du lancer
};

// L'absence de cible est un std::optional<Cible> vide, pas un quatrième cas.
using Cible = std::variant<CibleAllie, CibleMonstre, CiblePoint>;

// Pour std::visit : rassemble plusieurs lambdas en un seul objet appelable,
// une lambda par alternative de Cible. Si une alternative n'a pas sa lambda
// (par exemple après l'ajout d'un quatrième cas), la compilation échoue.
template <class... Ts>
struct Surcharges : Ts... {
    using Ts::operator()...;
};
