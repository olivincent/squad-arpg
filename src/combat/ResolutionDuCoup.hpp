// ./src/combat/ResolutionDuCoup.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"

// Ce que le défenseur oppose à un coup. L'esquive, le blocage et les résistances s'y ajouteront avec leurs étapes.
struct Defenses {
    int armure;
    int capaciteDefensive;  // > 0 (`StatistiquesDeCombat`)
};

// Ce qu'un coup (ou un soin) a donné.
struct ResultatDuCoup {
    int montant;     // ce que le défenseur subit (après armure), ou le soin à rendre ; 0 : rien
    bool critique;
};

// Point d'entrée unique d'un coup : ce que le défenseur subit pour cette attaque. Enchaîne des fonctions
// d'étape dans l'ordre de la section 6 de `docs/classes-et-combat.md` :
//   1. esquive          (à venir)
//   2. critique         (`appliquerCritique` : s'applique avant toute réduction)
//   3. blocage          (à venir)
//   4. armure           (`appliquerArmure`)
//   5. résistances      (à venir)
// Seul endroit du moteur du coup qui tire au hasard (un nombre par coup, dans `generateur`, passé par
// référence) ; aucun tirage quand la chance de critique est nulle (capacité offensive nulle) ni quand le
// montant est nul. Précondition : attaque.montant >= 0.
[[nodiscard]] ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, Generateur& generateur);

// Capacité défensive contre laquelle un soin tente son critique. Elle suivra le niveau de la zone quand les
// niveaux existeront. Provisoire, à tester.
inline constexpr int s_capaciteDefensiveDeReference = 100;

// Un soin (décrit comme une attaque : son montant et la capacité offensive de qui le lance) peut être
// critique, contre `s_capaciteDefensiveDeReference`, par la même courbe et la même règle de tirage que
// `resoudreCoup`. Le résultat est le soin à rendre, avant le plafond aux points de vie maximum, qui se fait
// chez celui qui est soigné. Précondition : soin.montant >= 0.
[[nodiscard]] ResultatDuCoup resoudreSoin(const Attaque& soin, Generateur& generateur);
