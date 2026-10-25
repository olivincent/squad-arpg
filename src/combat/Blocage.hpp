// ./src/combat/Blocage.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// Le blocage au bouclier (`docs/classes-et-combat.md`, sections 6 et 7), à la *Titan Quest* : une chance de
// bloquer, un montant fixe absorbé (pas tout le coup), puis une récupération pendant laquelle le bouclier ne
// bloque plus. Que des fonctions pures ; le tirage est fait ailleurs (`resoudreCoup`) et l'état de récupération
// vit chez le défenseur (`Vitalite`).

// Après un blocage réussi, le bouclier ne bloque plus pendant ce temps, en secondes : le même pour tous les boucliers
// et les défenseurs (ce n'est ni une donnée d'objet ni une statistique). Provisoire, à tester.
inline constexpr float s_recuperationDuBlocage = 3.0f;

// La chance (de 0 à 1) que ce défenseur bloque une attaque de cette nature. Nulle quand le blocage est en
// récupération (`blocagePret` faux), quand il n'absorbe rien (pas de bouclier) et contre une zone, qui ne se
// bloque jamais. Contact et projectile se bloquent de la même façon.
[[nodiscard]] float chanceDeBlocage(NatureDAttaque nature, const StatistiquesDeCombat& defenseur, bool blocagePret);

struct ResultatDuBlocage {
    float montant;  // ce qui reste de la portion après le blocage
    float bloque;   // ce que le blocage a absorbé ; 0 : pas de blocage (donc pas de récupération à lancer)
};

// L'étape « blocage » du coup : `tirage` (de 0 inclus à 1 exclu) sous `chance`, le bouclier absorbe
// `montantBloque` points, sans dépasser la portion. Le bouclier ne retient que la portion **physique** d'un coup
// (étape 94) : `montant` est cette portion, en flottant (l'arrondi vient à la fin du coup). Précondition : montant >= 0.
[[nodiscard]] ResultatDuBlocage appliquerBlocage(float montant, int montantBloque, float chance, float tirage);
