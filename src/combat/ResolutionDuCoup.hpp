// ./src/combat/ResolutionDuCoup.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/Degats.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// Ce que le défenseur oppose à un coup : son armure (qui réduit la portion physique) et ses statistiques de combat
// (seules les défensives sont lues : capacité défensive, esquives, blocage, résistances).
struct Defenses {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int armure;
    StatistiquesDeCombat statistiques;  // capaciteDefensive > 0
};

// Ce qu'un coup (ou un soin) a donné. Un résultat « rien » (montant 0, ni critique ni esquive) est celui d'un
// mort, d'un monstre invulnérable et d'un montant nul.
struct ResultatDuCoup {
    int montant = 0;         // ce que le défenseur subit (toutes portions, après toutes les réductions), ou le soin à rendre ; 0 : rien
    bool critique = false;
    bool esquive = false;    // le coup a été esquivé : il s'arrête là, montant 0 (l'attaque a pourtant eu lieu)
    int bloque = 0;          // points absorbés par un blocage ; > 0 : il y a eu blocage (total si montant == 0)
    Degats portions = {};    // ce que chaque élément a donné : la somme est `montant` (vide pour un soin et quand rien n'est subi)
};

// Point d'entrée unique d'un coup : ce que le défenseur subit pour cette attaque, portion par portion. Enchaîne des
// fonctions d'étape dans l'ordre de la section 6 de `docs/classes-et-combat.md` :
//   1. esquive          (`coupEsquive` : selon la nature de l'attaque, pas de l'élément ; un coup esquivé s'arrête là)
//   2. critique         (`coupCritique` : le multiplicateur s'applique à **toutes** les portions)
//   3. blocage          (`appliquerBlocage` : retire un montant fixe de la portion **physique** seule ; les portions
//                        élémentaires passent toujours. Tenté seulement si l'attaque est de contact ou un projectile
//                        et a une portion physique : sinon ni tirage ni récupération. Si tout le coup est absorbé,
//                        il s'arrête là, sans armure ni plancher)
//   4. armure           (`armureAppliquee`, réduite par la pénétration de l'attaque : portion physique seule)
//   5. résistances      (`appliquerResistance` : chaque portion, physique comprise, par la résistance de son élément)
//   6. arrondi          (un seul par portion, ici ; aucune étape avant n'arrondit)
// Le total est la somme des portions arrondies ; un coup ni esquivé ni entièrement bloqué inflige au moins 1.
// Seul endroit du moteur du coup qui tire au hasard (un nombre par coup, dans `generateur`, passé par
// référence, un par étape dont la chance est non nulle) ; aucun tirage quand la chance est nulle (esquive nulle
// ou zone, capacité offensive nulle, pas de bouclier ou bouclier en récupération, pas de portion physique) ni quand
// le total est nul. `blocagePret` : le blocage du défenseur n'est pas en récupération (l'état vit chez lui,
// `Vitalite` ; le résultat dit `bloque > 0` quand il faut lancer la récupération).
// Précondition : aucune portion de attaque.degats n'est négative.
[[nodiscard]] ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, bool blocagePret,
                                          Generateur& generateur);

// Capacité défensive contre laquelle un soin tente son critique. Elle suivra le niveau de la zone quand les
// niveaux existeront. Provisoire, à tester.
inline constexpr int s_capaciteDefensiveDeReference = 100;

// Un soin (décrit comme une attaque : son champ `soin` et la capacité offensive de qui le lance, sans élément) peut être
// critique, contre `s_capaciteDefensiveDeReference`, par la même courbe et la même règle de tirage que
// `resoudreCoup`. Le résultat est le soin à rendre, avant le plafond aux points de vie maximum, qui se fait
// chez celui qui est soigné. Précondition : attaque.soin >= 0.
[[nodiscard]] ResultatDuCoup resoudreSoin(const Attaque& attaque, Generateur& generateur);
