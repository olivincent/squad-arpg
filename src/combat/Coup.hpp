// ./src/combat/Coup.hpp
#pragma once

#include "combat/Attaque.hpp"

// Une attaque à infliger au monstre portant cet identifiant.
struct Coup {
    int idMonstre;
    Attaque attaque;
};
