// ./src/combat/ResolutionDuCoup.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// Ce que le défenseur oppose à un coup : son armure et ses statistiques de combat (seules les défensives sont
// lues : capacité défensive, esquives). Le blocage et les résistances s'y ajouteront avec leurs étapes.
struct Defenses {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int armure;
    StatistiquesDeCombat statistiques;  // capaciteDefensive > 0
};

// Ce qu'un coup (ou un soin) a donné. Un résultat « rien » (montant 0, ni critique ni esquive) est celui d'un
// mort, d'un monstre invulnérable et d'un montant nul.
struct ResultatDuCoup {
    int montant = 0;         // ce que le défenseur subit (après armure), ou le soin à rendre ; 0 : rien
    bool critique = false;
    bool esquive = false;    // le coup a été esquivé : il s'arrête là, montant 0 (l'attaque a pourtant eu lieu)
    int bloque = 0;          // points absorbés par un blocage ; > 0 : il y a eu blocage (total si montant == 0)
};

// Point d'entrée unique d'un coup : ce que le défenseur subit pour cette attaque. Enchaîne des fonctions
// d'étape dans l'ordre de la section 6 de `docs/classes-et-combat.md` :
//   1. esquive          (`coupEsquive` : selon la nature de l'attaque ; un coup esquivé s'arrête là)
//   2. critique         (`appliquerCritique` : s'applique avant toute réduction)
//   3. blocage          (`appliquerBlocage` : retire un montant fixe ; si tout est absorbé, le coup s'arrête là,
//                        sans armure ni plancher)
//   4. armure           (`appliquerArmure`, réduite par la pénétration de l'attaque)
//   5. résistances      (à venir)
// Seul endroit du moteur du coup qui tire au hasard (un nombre par coup, dans `generateur`, passé par
// référence, un par étape dont la chance est non nulle) ; aucun tirage quand la chance est nulle (esquive nulle
// ou zone, capacité offensive nulle, pas de bouclier ou bouclier en récupération) ni quand le montant est nul.
// `blocagePret` : le blocage du défenseur n'est pas en récupération (l'état vit chez lui, `Vitalite` ; le
// résultat dit `bloque > 0` quand il faut lancer la récupération). Précondition : attaque.degats >= 0.
[[nodiscard]] ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, bool blocagePret,
                                          Generateur& generateur);

// Capacité défensive contre laquelle un soin tente son critique. Elle suivra le niveau de la zone quand les
// niveaux existeront. Provisoire, à tester.
inline constexpr int s_capaciteDefensiveDeReference = 100;

// Un soin (décrit comme une attaque : son champ `soin` et la capacité offensive de qui le lance) peut être
// critique, contre `s_capaciteDefensiveDeReference`, par la même courbe et la même règle de tirage que
// `resoudreCoup`. Le résultat est le soin à rendre, avant le plafond aux points de vie maximum, qui se fait
// chez celui qui est soigné. Précondition : attaque.soin >= 0.
[[nodiscard]] ResultatDuCoup resoudreSoin(const Attaque& attaque, Generateur& generateur);
