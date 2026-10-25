// ./src/combat/Attaque.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"

// Ce qui part de l'attaquant quand un coup ou un skill part, et qui circule jusqu'au défenseur à la place d'un
// montant seul : le montant effectif et ce que le moteur du coup lit de l'attaquant. Le défenseur fournit
// de son côté ses propres statistiques (`resoudreCoup`).
struct Attaque {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès (sauf la pénétration), un oubli est signalé par le compilateur
    int montant;             // dégâts avant toute réduction (ou soin, ou durée d'une provocation), caractéristiques comprises
    int capaciteOffensive;   // celle de l'attaquant (`StatistiquesDeCombat`), contre la capacité défensive du défenseur
    NatureDAttaque nature;   // contact, projectile ou zone : une donnée de l'attaque, jamais déduite ; lue par l'esquive et le blocage
    int penetration = 0;     // en pour cent de l'armure du défenseur ignorée (plafonnée : `s_penetrationMaximale`) ; l'attaque de base d'une arme en a, un skill n'en a pas
};
