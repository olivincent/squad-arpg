// ./src/combat/ResolutionDuCoup.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/Degats.hpp"
#include "combat/Element.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// Ce que le défenseur oppose à un coup : son armure (qui réduit la portion physique) et ses statistiques de combat
// (seules les défensives sont lues : capacité défensive, esquives, blocage, résistances).
struct Defenses {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int armure;
    StatistiquesDeCombat statistiques;  // capaciteDefensive > 0
    PointsDAbsorption absorption = {};  // les boucliers d'absorption qu'il a en ce moment (étape 114) ; `appliquerLesEffets` les remplit
};

// Ce qu'un coup (ou un soin) a donné. Un résultat « rien » (montant 0, ni critique ni esquive) est celui d'un
// mort, d'un monstre invulnérable et d'un montant nul.
struct ResultatDuCoup {
    int montant = 0;         // ce que le défenseur subit (toutes portions, après toutes les réductions, absorption comprise), ou le soin à rendre ; 0 : rien
    bool critique = false;
    bool esquive = false;    // le coup a été esquivé : il s'arrête là, montant 0 (l'attaque a pourtant eu lieu)
    int bloque = 0;          // points absorbés par un blocage ; > 0 : il y a eu blocage (total si montant == 0)
    Degats portions = {};    // ce que chaque élément a donné : la somme est `montant` (vide pour un soin et quand rien n'est subi)
    Degats absorbees = {};   // ce que les boucliers d'absorption ont retenu, par élément (étape 114) : Aegis pour le physique, Ward pour le reste ; en plus de `montant`
};

// Ce qu'un tick de dégâts a donné (étape 114) : `montant` est ce que la cible subit, `absorbe` ce que ses boucliers d'absorption ont retenu
// (le tick, après sa résistance, vaut leur somme).
struct ResultatDuTick {
    int montant = 0;
    int absorbe = 0;
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
//   7. absorption       (étape 114, `portionsAbsorbees` : les Aegis retiennent la portion physique, les Ward les autres ; sur les portions
//                        arrondies, donc en points entiers. Ce qui est retenu est dans `absorbees`, le reste dans `montant` et `portions`)
// Le total est la somme des portions arrondies ; un coup ni esquivé ni entièrement bloqué inflige au moins 1 AVANT l'absorption (un coup
// que les boucliers retiennent tout entier inflige 0).
// Seul endroit du moteur du coup qui tire au hasard (un nombre par coup, dans `generateur`, passé par
// référence, un par étape dont la chance est non nulle) ; aucun tirage quand la chance est nulle (esquive nulle
// ou zone, capacité offensive nulle, pas de bouclier ou bouclier en récupération, pas de portion physique) ni quand
// le total est nul. `blocagePret` : le blocage du défenseur n'est pas en récupération (l'état vit chez lui,
// `Vitalite` ; le résultat dit `bloque > 0` quand il faut lancer la récupération).
// Précondition : aucune portion de attaque.degats n'est négative.
[[nodiscard]] ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, bool blocagePret,
                                          Generateur& generateur);

// Un tick de dégâts dans le temps (étape 112) : `montant` points de cet élément, ce que la cible subit. Pas d'esquive, pas de critique,
// pas de blocage et pas d'armure : un tick n'est pas un coup, il ne vise personne. Seule la résistance de l'élément joue (celle du
// physique aussi : le saignement), lue chez la cible à l'instant du tick et non à la pose. Un seul arrondi, au plus proche ; aucun
// tirage. Peut rendre 0 (un petit tick très résisté). Précondition : montant >= 0.
[[nodiscard]] int resoudreTick(Element element, int montant, const StatistiquesDeCombat& defenseur);

// Le même tick avec les boucliers d'absorption (étape 114) : après la résistance, les Aegis retiennent un tick physique et les Ward un tick
// d'un autre élément (`portionsAbsorbees`). Le résultat dit ce que la cible subit et ce qu'ont retenu ses boucliers. Aucun tirage.
[[nodiscard]] ResultatDuTick resoudreTick(Element element, int montant, const Defenses& defenses);

// Capacité défensive contre laquelle un soin tente son critique. Elle suivra le niveau de la zone quand les
// niveaux existeront. Provisoire, à tester.
inline constexpr int s_capaciteDefensiveDeReference = 100;

// Un soin (décrit comme une attaque : son champ `soin` et la capacité offensive de qui le lance, sans élément) peut être
// critique, contre `s_capaciteDefensiveDeReference`, par la même courbe et la même règle de tirage que
// `resoudreCoup`. Le résultat est le soin à rendre, avant le plafond aux points de vie maximum, qui se fait
// chez celui qui est soigné. Précondition : attaque.soin >= 0.
[[nodiscard]] ResultatDuCoup resoudreSoin(const Attaque& attaque, Generateur& generateur);
