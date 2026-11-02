// ./src/monstres/RoleDeMonstre.hpp
#pragma once
#include <optional>
#include <variant>
#include <vector>
#include "combat/AllieVu.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/Skill.hpp"

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

// Un soigneur soigne les membres BLESSÉS de son groupe (jamais lui-même) avec un skill de mode `Allie` (`EffetSoin`). Quand il n'a
// personne à soigner ou que son soin recharge, il se comporte comme son attaque le décrit (un tireur faible, sans recul).
struct RoleSoigneur {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    DefinitionDeSkill soin;  // mode `Allie`, un `EffetSoin` : montant, portée, recharge, temps d'activation
    int seuilDeBlessure;     // en pour cent des points de vie maximum : un membre est blessé en dessous (strictement)
};

// Un assommeur (étape 113) frappe comme une mêlée, et son attaque de base étourdit le personnage qu'elle touche pendant `dureeDeLEtourdissement`
// (avant la résistance du personnage et le rendement décroissant), au plus une fois par `delaiEntreCoups`. Premier monstre qui pose un effet
// actif : l'effet voyage dans l'`Attaque` (`Attaque::effets`), comme ceux d'un skill de personnage. C'est une capacité de rôle : la Disruption
// l'empêche (elle compte comme un skill), l'attaque de base reste, sans étourdissement.
struct RoleAssommeur {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    float dureeDeLEtourdissement;  // en secondes
    float delaiEntreCoups;         // en secondes ; le premier coup assommant est disponible tout de suite
};

using RoleDeMonstre = std::variant<RoleMelee, RoleTireur, RoleSoigneur, RoleAssommeur>;

// L'état d'exécution de chaque rôle, dans le monstre. Une alternative par rôle, dans le même ordre.
struct EtatMelee {};

struct EtatTireur {  // NOLINT(cppcoreguidelines-pro-type-member-init): `role` n'a pas de valeur par défaut exprès
    RoleTireur role;               // les données, copiées de la définition
    float delaiRestant = 0.0f;     // en secondes, avant que le prochain recul soit permis ; 0 : permis
    bool enRecul = false;          // un recul est en cours (son ordre est donné à `Mobile`)
};

struct EtatSoigneur {  // NOLINT(cppcoreguidelines-pro-type-member-init): `role` et `soin` n'ont pas de valeur par défaut exprès
    RoleSoigneur role;                    // les données, copiées de la définition
    Skill soin;                           // la machine à états du soin (prêt, activation, recharge)
    std::optional<IdMonstre> cibleDuSoin; // le membre visé par le soin engagé, lu seulement pendant l'activation
};

struct EtatAssommeur {  // NOLINT(cppcoreguidelines-pro-type-member-init): `role` n'a pas de valeur par défaut exprès
    RoleAssommeur role;            // les données, copiées de la définition
    float delaiRestant = 0.0f;     // en secondes, avant que le prochain coup assommant soit permis ; 0 : permis
};

using EtatDuRole = std::variant<EtatMelee, EtatTireur, EtatSoigneur, EtatAssommeur>;

// L'état d'un monstre qui vient d'être créé avec ce rôle : l'alternative de même rang que le rôle.
[[nodiscard]] EtatDuRole etatInitialDuRole(const RoleDeMonstre& role);

// Le membre à soigner parmi ces alliés : le plus blessé EN PROPORTION de ses points de vie maximum, parmi ceux qui sont strictement
// sous `seuilDeBlessure` pour cent ; à égalité, le plus petit identifiant. Vide si personne n'est assez blessé. Les alliés sont déjà
// ceux qu'on peut soigner (vivants, ciblables, sans le soigneur lui-même) : cette fonction ne regarde que la blessure. Pure.
[[nodiscard]] std::optional<AllieVu> choisirLeBlesse(const std::vector<AllieVu>& allies, int seuilDeBlessure);
