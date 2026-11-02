// ./src/combat/Controle.hpp
#pragma once
#include "combat/EffetActif.hpp"
#include "combat/EffetsActifs.hpp"
#include "combat/EtatDeControle.hpp"
#include "combat/StatistiquesDeCombat.hpp"

// Les règles du contrôle (étape 113), des fonctions pures qui lisent les effets actifs d'une entité. Personnages et monstres les
// appellent de la même façon : ni l'un ni l'autre ne connaît la liste des effets, seulement ces questions.
//
// Le cumul vient de la lecture : la liste des effets garde un contrôle par (skill, rang), et chaque règle regarde TOUS ceux qui sont là.
// Deux étourdissements (ou deux Disruptions) de skills différents durent donc tant qu'il en reste un ; deux ralentissements (deux
// entraves) donnent le plus fort des deux et non leur somme ; le même skill reposé rafraîchit (`EffetsActifs::poser`).

// La résistance qui s'applique à un état inflige (étape 113) : l'étourdissement, le ralentissement et la Disruption ont la leur ;
// l'entrave n'en a pas et suit celle du ralentissement. Seule fonction qui fasse ce lien.
[[nodiscard]] EtatDeControle etatQuiResiste(EtatInflige etat);

// Cette résistance telle qu'elle joue sur l'état, en pour cent, ramenée entre les bornes (`resistanceAuControleEffective`).
[[nodiscard]] float resistanceAppliquee(const StatistiquesDeCombat& statistiques, EtatInflige etat);

// La durée qu'un contrôle pose sur sa cible, en secondes (étape 113) : la durée avant résistance (celle du skill, déjà majorée par le
// bonus de durée de l'Intellect du lanceur selon le coefficient du skill) × (1 − résistance de la cible) × le facteur de rendement
// décroissant. La résistance est lue À LA POSE (contrairement à celle d'un tick de dégâts, lue au tick) ; `resistance` est en pour cent,
// déjà ramenée entre les bornes (`resistanceAppliquee`). Négative (vulnérabilité), elle allonge la durée. Seul endroit du calcul.
[[nodiscard]] float dureePosee(float dureeAvantResistance, float resistance, float rendement);

// Ce qu'une pose de contrôle a donné, rendu par `Vitalite::prendreLesControles` : l'état, et `immunise` quand le rendement décroissant a
// tout bloqué (rien n'est posé). `Partie` en fait des événements (« Stunned », « Immune »).
struct ControleApplique {
    EtatInflige etat;
    bool immunise;
};

// Un effet de cet état est-il actif ?
[[nodiscard]] bool estSousEtat(const EffetsActifs& effets, EtatInflige etat);

// Étourdi : ni déplacement, ni attaque, ni skill.
[[nodiscard]] bool peutBouger(const EffetsActifs& effets);
[[nodiscard]] bool peutAttaquer(const EffetsActifs& effets);
// Ni étourdi ni en Disruption. Un monstre y compte aussi ses capacités de rôle (le soin du soigneur, le coup assommant).
[[nodiscard]] bool peutUtiliserUnSkill(const EffetsActifs& effets);

// Le facteur de la vitesse de déplacement : 1 sans ralentissement, sinon 1 − le pourcentage du plus fort des ralentissements (jamais
// sous 0). Celui de la vitesse de l'attaque de base : la même chose pour l'entrave ; il multiplie le temps qui s'écoule pour cette
// attaque, donc allonge son intervalle (et son temps d'exécution).
[[nodiscard]] float facteurDeDeplacement(const EffetsActifs& effets);
[[nodiscard]] float facteurDAttaque(const EffetsActifs& effets);
