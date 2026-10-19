// ./src/AdresseSkill.hpp
#pragma once
#include "IdSkill.hpp"
#include <cstddef>

// Où se trouve un skill : le personnage qui le connaît (sa place, de 1 à N) et
// l'identifiant du skill. Une adresse dont la place est invalide ou dont le
// skill est inconnu de ce personnage est traitée comme une place invalide :
// refus, liste vide, optional vide.
struct AdresseSkill {
    std::size_t place;
    IdSkill id;

    bool operator==(const AdresseSkill&) const = default;
};
