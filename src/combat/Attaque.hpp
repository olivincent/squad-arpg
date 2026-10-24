// ./src/combat/Attaque.hpp
#pragma once
#include "combat/NatureDAttaque.hpp"

// Ce qui part de l'attaquant quand un coup ou un skill part, et qui circule jusqu'au défenseur à la place d'un
// montant seul : le montant effectif et ce que le moteur du coup lit de l'attaquant. Le défenseur fournit
// de son côté ses propres statistiques (`resoudreCoup`).
struct Attaque {
    int montant;             // dégâts avant toute réduction (ou soin, ou durée d'une provocation), caractéristiques comprises
    int capaciteOffensive;   // celle de l'attaquant (`StatistiquesDeCombat`), contre la capacité défensive du défenseur
    NatureDAttaque nature;   // contact, projectile ou zone : une donnée de l'attaque, jamais déduite ; lue par l'esquive et le blocage
};
