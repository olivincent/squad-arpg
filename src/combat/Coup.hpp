// ./src/combat/Coup.hpp
#pragma once

#include "combat/Attaque.hpp"
#include "combat/IdMonstre.hpp"

// Une attaque à infliger au monstre portant cet identifiant.
struct Coup {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    IdMonstre idMonstre;
    Attaque attaque;
};
