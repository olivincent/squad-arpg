// ./src/monstres/RoleDeMonstre.hpp
#pragma once
#include <variant>

// Ce qu'un monstre fait EN PLUS de sa poursuite et de sa frappe : son rôle, une alternative par sorte de rôle, chacune avec ses
// seuls champs (sur le modèle d'`EffetDeSkill`). Le rôle a deux moitiés : la DONNÉE, dans la définition, immuable (`RoleDeMonstre`) ;
// l'ÉTAT d'exécution, dans le monstre, construit depuis elle (`EtatDuRole`, de même forme). Un rôle est une donnée d'équilibrage,
// pas une identité de type : `Monstre` n'a pas de classe dérivée. Un nouveau rôle est une alternative de plus dans les deux
// variants, et le compilateur signale chaque `std::visit` à compléter.

// Les monstres d'avant les rôles : ils poursuivent et frappent au contact, rien d'autre.
struct RoleMelee {};

// Le tireur et le soigneur (étapes 107.2 et 107.4) n'ont pas encore de champs : ils les recevront avec leur comportement.
struct RoleTireur {};
struct RoleSoigneur {};

using RoleDeMonstre = std::variant<RoleMelee, RoleTireur, RoleSoigneur>;

// L'état d'exécution de chaque rôle, dans le monstre. Une alternative par rôle, dans le même ordre.
struct EtatMelee {};
struct EtatTireur {};
struct EtatSoigneur {};

using EtatDuRole = std::variant<EtatMelee, EtatTireur, EtatSoigneur>;

// L'état d'un monstre qui vient d'être créé avec ce rôle : l'alternative de même rang que le rôle.
[[nodiscard]] EtatDuRole etatInitialDuRole(const RoleDeMonstre& role);
