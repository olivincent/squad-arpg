// ./src/monstres/RoleDeMonstre.hpp
#pragma once
#include <variant>

// Ce qu'un monstre fait EN PLUS de sa poursuite et de sa frappe : son rôle, une alternative par sorte de rôle, chacune avec ses
// seuls champs (sur le modèle d'`EffetDeSkill`). Le rôle a deux moitiés : la DONNÉE, dans la définition, immuable (`RoleDeMonstre`) ;
// l'ÉTAT d'exécution, dans le monstre, construit depuis elle (`EtatDuRole`, de même forme : il garde une copie des données de son
// rôle, et y ajoute ce qui change). Un rôle est une donnée d'équilibrage, pas une identité de type : `Monstre` n'a pas de classe
// dérivée. Un nouveau rôle est une alternative de plus dans les deux variants, et le compilateur signale chaque `std::visit` à
// compléter.

// Les monstres d'avant les rôles : ils poursuivent et frappent au contact, rien d'autre.
struct RoleMelee {};

// Un tireur frappe de loin et recule quand on le colle : un personnage à `distanceDeContact` ou moins (bord à bord) le fait reculer de
// `distanceDeRecul`, dans la direction opposée, au plus une fois par `delaiEntreReculs`.
struct RoleTireur {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    float distanceDeContact;  // en pixels, bord à bord
    float distanceDeRecul;    // en pixels
    float delaiEntreReculs;   // en secondes ; le premier recul est disponible tout de suite
};

// Le soigneur (étape 107.4) n'a pas encore de champs : il les recevra avec son comportement.
struct RoleSoigneur {};

using RoleDeMonstre = std::variant<RoleMelee, RoleTireur, RoleSoigneur>;

// L'état d'exécution de chaque rôle, dans le monstre. Une alternative par rôle, dans le même ordre.
struct EtatMelee {};

struct EtatTireur {  // NOLINT(cppcoreguidelines-pro-type-member-init): `role` n'a pas de valeur par défaut exprès
    RoleTireur role;               // les données, copiées de la définition
    float delaiRestant = 0.0f;     // en secondes, avant que le prochain recul soit permis ; 0 : permis
    bool enRecul = false;          // un recul est en cours (son ordre est donné à `Mobile`)
};

struct EtatSoigneur {};

using EtatDuRole = std::variant<EtatMelee, EtatTireur, EtatSoigneur>;

// L'état d'un monstre qui vient d'être créé avec ce rôle : l'alternative de même rang que le rôle.
[[nodiscard]] EtatDuRole etatInitialDuRole(const RoleDeMonstre& role);
