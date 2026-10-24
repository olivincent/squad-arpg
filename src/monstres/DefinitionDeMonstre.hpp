// ./src/monstres/DefinitionDeMonstre.hpp
#pragma once
#include "combat/Skill.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// Les valeurs d'équilibrage d'un monstre, nommées à la construction (initialisation désignée) : un monstre
// ne choisit rien, il reçoit sa définition. Aujourd'hui un seul modèle (`monstreOrdinaire`) ; les types de
// monstres seront d'autres définitions.
struct DefinitionDeMonstre {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    float cote;                // du carré, en pixels
    float vitesse;             // en pixels par seconde
    int pointsDeVieMax;
    int armure;                // le niveau donne la sienne (`Niveau::armureMonstres`)
    float rayonDetection;      // en pixels
    float rayonTerritoire;     // distance maximale au poste au-delà de laquelle le monstre abandonne, en pixels
    float zoneDeControle;      // en pixels, bord à bord : tout monstre engage ce qui est à cette distance
    float regenerationRetour;  // fraction des points de vie maximum rendue par seconde pendant le retour au poste
    DefinitionDeSkill attaque; // portée bord à bord, dégâts avant l'armure de la cible, recharge, exécution
    StatistiquesDeCombat statistiques;  // un monstre n'a pas de caractéristiques : il reçoit ses statistiques
};

// Le monstre du jeu aujourd'hui, sans armure (celle du niveau s'y ajoute).
[[nodiscard]] DefinitionDeMonstre monstreOrdinaire();
