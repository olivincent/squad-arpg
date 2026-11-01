// ./src/combat/AllieVu.hpp
#pragma once
#include "base/Cercle.hpp"
#include "combat/IdMonstre.hpp"

// Un monstre tel que ses alliés le voient (ceux de son groupe) : de quoi le trouver, l'atteindre et mesurer sa blessure. Distinct de
// `MonstreVu`, que voient les personnages et qui n'a ni points de vie ni maximum : le soigneur a besoin de ces deux nombres, pas
// l'escouade. Une structure à part plutôt qu'un champ de plus dans `EntiteVue`, que les deux camps partagent.
struct AllieVu {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    IdMonstre id;
    Cercle cercle;
    int pointsDeVie;
    int pointsDeVieMax;
};
