// ./src/combat/EffetDeSkill.hpp
#pragma once
#include <variant>
#include "combat/Caracteristiques.hpp"
#include "combat/Degats.hpp"
#include "combat/EffetActif.hpp"
#include "combat/Element.hpp"
#include "combat/NatureDAttaque.hpp"

// CE QUE FAIT un skill, une alternative par sorte d'effet, chacune avec ses champs et seulement les siens : un champ n'existe que là
// où il a un sens (une provocation n'a ni élément ni coefficients, un soin ni nature). Où il s'applique (une cible ou une zone) est
// l'affaire du rayon de zone du skill ; comment il vise, celle de son `ModeDeCible`. Un skill porte une liste d'effets (le premier est
// le principal) ; un nouveau type d'effet est une alternative de plus, et le compilateur signale chaque `std::visit` à compléter.
//
// Les coefficients disent ce que le montant doit aux caractéristiques du lanceur (`montantEffectif`) ; sans eux, le montant est
// celui de la définition.

// Des dégâts propres au skill : un montant d'un élément, d'une nature (contact, projectile ou zone : lue par l'esquive et le blocage).
// Une explosion est du feu ; la nature `Zone` ne s'esquive ni ne se bloque.
struct EffetDegats {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int montant;
    NatureDAttaque nature;
    Element element = Element::Physique;
    Coefficients coefficients = {};
};

// Un **skill de frappe** (étape 94) : l'arme portée, plus ce bonus fixe et typé (la frappe sacrée : du sacré). Les dégâts physiques, la
// nature et la pénétration sont ceux de l'arme (`DescriptionDAttaque`) ; la Puissance joue sur chaque portion.
struct EffetFrappe {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    Degats bonus;
    Coefficients coefficients = {};
};

// Des points de vie rendus.
struct EffetSoin {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int montant;
    Coefficients coefficients = {};
};

// Les monstres touchés prennent pour proie le lanceur pendant cette durée (en secondes ; ce n'est pas un montant : les coefficients
// de montant n'y jouent pas, seul le bonus de durée de l'Intellect, selon `coefficientDeDuree`).
struct EffetProvocation {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int duree;
    float coefficientDeDuree = 0.0f;  // sa part du bonus de durée du lanceur (`dureeEffective`), de 0 à 1 ; la provocation du guerrier : 0
};

// Pose un modificateur de statistique (`ModificateurDeStatistique`, un effet actif) sur chaque entité touchée, pour cette durée (en
// secondes, majorée par le bonus de durée du lanceur selon `coefficientDeDuree`, de 0 à 1 ; 0 pour ce qui n'est pas un sort). Le
// pourcentage est positif pour un buff, négatif pour un affaiblissement. Effet principal d'un skill, il touche les alliés et est donc
// un buff ; effet secondaire, il suit des dégâts ou une frappe, touche les monstres que le coup atteint, et n'est posé que si le coup
// n'est pas esquivé.
struct EffetModificateur {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    StatistiqueModifiable statistique;
    int pourcentage;
    int duree;
    float coefficientDeDuree = 0.0f;
};

// Pose des dégâts dans le temps (`DegatsDansLeTemps`, un effet actif) sur chaque monstre touché : `montant` points de cet élément par
// seconde pendant `duree` secondes, ceux d'une pile, au plus `pilesMax` piles (le saignement s'empile, la brûlure non). Le montant se
// fige à la pose, avec les coefficients et les caractéristiques du lanceur à ce moment-là. Toujours un effet secondaire : il suit des
// dégâts ou une frappe, n'est posé que si le coup n'est pas esquivé, et touche les monstres que le coup atteint. Le coefficient de
// durée vaut 0 (le défaut) : le montant gagne déjà par le Savoir ou la Puissance, la durée ne gagnerait pas une seconde fois.
struct EffetDegatsDansLeTemps {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int montant;
    int duree;
    Element element = Element::Physique;
    int pilesMax = 1;
    Coefficients coefficients = {};
    float coefficientDeDuree = 0.0f;
};

// Pose un soin dans le temps (`SoinDansLeTemps`) sur chaque allié touché : `montant` points de vie par seconde pendant `duree` secondes,
// figés à la pose. Effet principal d'un skill (il touche les alliés, comme un buff). Même coefficient de durée de 0 par convention.
struct EffetSoinDansLeTemps {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    int montant;
    int duree;
    int pilesMax = 1;
    Coefficients coefficients = {};
    float coefficientDeDuree = 0.0f;
};

// Pose un état de contrôle (`Controle`, un effet actif) sur chaque monstre touché, pour cette durée (étape 113) : un étourdissement, un
// ralentissement (`pourcentage` de la vitesse de déplacement en moins), une entrave (de la vitesse d'attaque de base) ou une Disruption.
// `duree` est la durée avant la résistance de la cible et le rendement décroissant (`dureePosee`), majorée par le bonus de durée du lanceur
// selon `coefficientDeDuree` (0 pour une technique ou une frappe, un coefficient pour un sort). Effet principal d'un skill de mode `Monstre` ou
// `Point` (un sort sans dégâts : le contresort), il touche les monstres ; effet secondaire, il suit des dégâts ou une frappe et n'est posé
// que si le coup n'est pas esquivé.
struct EffetControle {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    EtatInflige etat;
    float duree;
    int pourcentage = 0;  // de 1 à 100 pour le ralentissement et l'entrave, 0 pour les deux autres
    float coefficientDeDuree = 0.0f;
};

using EffetDeSkill = std::variant<EffetDegats, EffetFrappe, EffetSoin, EffetProvocation, EffetModificateur, EffetDegatsDansLeTemps,
                                  EffetSoinDansLeTemps, EffetControle>;

// Le camp que touche un effet quand il a un rayon de zone : les dégâts, les dégâts dans le temps et la provocation touchent les monstres,
// le soin et le soin dans le temps les alliés, un modificateur les alliés si c'est un buff et les monstres si c'est un affaiblissement, un
// contrôle les monstres.
// Seul endroit où le camp se déduit de l'effet.
enum class CampTouche { Monstres, Allies };
[[nodiscard]] CampTouche campTouche(const EffetDeSkill& effet);
