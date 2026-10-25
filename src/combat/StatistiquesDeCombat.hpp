// ./src/combat/StatistiquesDeCombat.hpp
#pragma once
#include "combat/Caracteristiques.hpp"
#include "combat/Element.hpp"

// Les résistances d'une entité, en pour cent par élément : un coup en perd cette part, élément par élément
// (`appliquerResistance`, bornée par `s_resistanceMinimale` et `s_resistanceMaximale`). Négative : vulnérabilité.
// La résistance physique est 0 partout pour l'instant : l'armure fait le travail du physique.
using Resistances = ParElement<int>;

// Ce que le moteur du coup lit d'une entité (`docs/classes-et-combat.md`, sections 6, 7 et 13). Le moteur
// ne connaît pas les caractéristiques : un personnage en tire ses statistiques (`statistiquesDuPersonnage`,
// seul endroit où elles se calculent), un monstre les reçoit de sa définition. Type valeur ; un champ
// n'existe que quand une étape du coup s'en sert. Le même type sert de **bonus d'un objet**
// (`DefinitionObjet::bonusDeCombat`) : les bonus de l'équipement s'additionnent aux statistiques du personnage.
struct StatistiquesDeCombat {
    int capaciteOffensive = 0;  // chance et force du critique, contre la capacité défensive du défenseur
    int capaciteDefensive = 0;  // réduit le critique subi
    int esquiveContact = 0;     // chance en pour cent d'éviter un coup de contact (plafonnée : `esquiveEffective`)
    int esquiveProjectile = 0;  // idem pour un projectile
    int chanceDeBlocage = 0;    // chance en pour cent de bloquer un coup de contact ou un projectile (bouclier)
    int montantBloque = 0;      // points que le blocage absorbe, sans dépasser le coup ; 0 : pas de bouclier
    Resistances resistances = {};  // en pour cent par élément ; toutes à 0 par défaut
    // La récupération qui suit un blocage réussi est la même pour tous (`s_recuperationDuBlocage`, `Blocage.hpp`) :
    // ce n'est pas une statistique.

    // Les bonus s'additionnent.
    constexpr StatistiquesDeCombat& operator+=(const StatistiquesDeCombat& autre) {
        capaciteOffensive += autre.capaciteOffensive;
        capaciteDefensive += autre.capaciteDefensive;
        esquiveContact += autre.esquiveContact;
        esquiveProjectile += autre.esquiveProjectile;
        chanceDeBlocage += autre.chanceDeBlocage;
        montantBloque += autre.montantBloque;
        for (const Element element : s_elements) {
            resistances[element] += autre.resistances[element];
        }
        return *this;
    }

    [[nodiscard]] friend constexpr StatistiquesDeCombat operator+(StatistiquesDeCombat a, const StatistiquesDeCombat& b) {
        a += b;
        return a;
    }

    constexpr bool operator==(const StatistiquesDeCombat&) const = default;
};

// Capacités d'un personnage sans Adresse (celle du point de référence 10 : 100). Provisoire, à tester.
inline constexpr int s_capaciteDeBase = 50;
// Capacité (offensive et défensive) que donne chaque point d'Adresse. Provisoire, à tester.
inline constexpr int s_capaciteParPointDAdresse = 5;

// Les statistiques d'un personnage : `s_capaciteDeBase` plus `s_capaciteParPointDAdresse` par point d'Adresse
// effective (base de classe plus équipement) pour les deux capacités, plus les bonus de combat de son équipement
// (`bonusDEquipement`, la somme de ceux de ses objets). La capacité offensive est bornée à 0 par le bas ; la
// défensive à 1, parce que le rapport des capacités la divise.
[[nodiscard]] StatistiquesDeCombat statistiquesDuPersonnage(const Caracteristiques& caracteristiques,
                                                            const StatistiquesDeCombat& bonusDEquipement);
