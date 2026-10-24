// ./src/combat/ResolutionDuCoup.hpp
#pragma once

// Ce que le défenseur oppose à un coup. Aujourd'hui l'armure seule ; l'esquive, le blocage et les
// résistances s'y ajouteront avec leurs étapes.
struct Defenses {
    int armure;
};

// Point d'entrée unique d'un coup : ce que le défenseur subit pour un coup de `montant` dégâts bruts.
// Fonction pure, qui enchaîne des fonctions d'étape dans l'ordre de la section 6 de
// `docs/classes-et-combat.md` :
//   1. esquive          (à venir)
//   2. critique         (à venir : s'applique avant toute réduction)
//   3. blocage          (à venir)
//   4. armure           (`appliquerArmure`)
//   5. résistances      (à venir)
// Précondition : montant >= 0.
[[nodiscard]] int resoudreCoup(int montant, const Defenses& defenses);
