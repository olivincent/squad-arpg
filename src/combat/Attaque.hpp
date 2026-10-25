// ./src/combat/Attaque.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"

// Ce qui part de l'attaquant quand un coup ou un skill part, et qui circule jusqu'au défenseur à la place d'un
// montant seul : ce que le skill produit et ce que le moteur du coup lit de l'attaquant. Le défenseur fournit
// de son côté ses propres statistiques (`resoudreCoup`).
// Un skill produit une seule de trois choses, selon son type : des dégâts (skills de dégâts et de zone, attaque de
// base), un soin ou une durée de provocation. Chacune a son champ, que le moteur du coup ne confond plus ; les deux
// autres restent à 0.
struct Attaque {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès (sauf la pénétration et le soin), un oubli est signalé par le compilateur
    int degats;              // dégâts avant toute réduction, caractéristiques comprises ; 0 pour un soin ou une provocation
    int capaciteOffensive;   // celle de l'attaquant (`StatistiquesDeCombat`), contre la capacité défensive du défenseur
    NatureDAttaque nature;   // contact, projectile ou zone : une donnée de l'attaque, jamais déduite ; lue par l'esquive et le blocage
    int penetration = 0;     // en pour cent de l'armure du défenseur ignorée (plafonnée : `s_penetrationMaximale`) ; l'attaque de base d'une arme en a, un skill n'en a pas
    int soin = 0;            // points de vie à rendre, caractéristiques comprises ; seulement pour un skill de soin
    int dureeDeProvocation = 0;  // en secondes ; seulement pour un skill de provocation
};
