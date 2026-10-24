// ./src/combat/Skill.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/TypeSkill.hpp"

// Un skill : un type, une portée, une puissance et une recharge (un
// intervalle [rechargeMin, rechargeMax] en secondes, tiré au hasard à chaque
// déclenchement ; fixe si les deux bornes sont égales, sans aucun tirage). La
// « puissance » est le nombre de dégâts pour un skill de Degats, le nombre
// de points de vie rendus pour un skill de Soin, la durée en secondes pour
// un skill de Provocation, le nombre de dégâts pour un skill de Zone. Le
// rayon de zone est la taille de la zone touchée par un skill de zone ; il
// vaut 0 et n'est pas lu pour un skill à cible unique (c'est le type qui
// décide).
//
// Machine à états (PhaseSkill) d'un coup à temps d'exécution :
//   Pret --commencer--> EnExecution --avancer (fin de l'exécution)--> EnRecharge
//   EnRecharge --mettreAJour/avancer (recharge écoulée)--> Pret
//   EnExecution --annuler--> Pret   (aucune recharge consommée)
// Le coup part à la fin de l'exécution : c'est à ce moment seulement que la
// recharge est tirée. Avec un temps d'exécution nul, le coup part dès le
// premier `avancer` (Pret -> EnExecution -> EnRecharge).
// Le skill ne sait pas sur qui porte le coup engagé : c'est son propriétaire
// qui le retient.
enum class PhaseSkill { Pret, EnExecution, EnRecharge };

// Les données d'un skill, nommées à la construction (initialisation désignée) : sept valeurs de
// types voisins qu'on ne confond plus. Voir `Skill` pour leur sens.
struct DefinitionDeSkill {
    TypeSkill type;
    float portee;        // en pixels, écart bord à bord
    int puissance;
    float rechargeMin;   // en secondes
    float rechargeMax;   // en secondes
    float rayonZone = 0.0f;       // en pixels
    float tempsExecution = 0.0f;  // en secondes, entre l'engagement du coup et son départ
};

class Skill {
public:
    explicit Skill(const DefinitionDeSkill& definition);
    void mettreAJour(float dt);
    // Pret -> EnExecution ; faux (sans rien changer) dans toute autre phase.
    [[nodiscard]] bool commencer();
    // Fait avancer l'exécution (EnExecution) ou la recharge (autres phases).
    // Vrai une seule fois, au moment où le coup part : la recharge est alors
    // tirée et la phase passe à EnRecharge.
    [[nodiscard]] bool avancer(float dt, Generateur& generateur);
    // EnExecution -> Pret, sans recharge ; sans effet dans toute autre phase.
    void annuler();
    [[nodiscard]] PhaseSkill getPhase() const;
    [[nodiscard]] bool estAPortee(Cercle origine, Cercle cible) const;
    void relancerRecharge(Generateur& generateur);
    [[nodiscard]] float getProgressionRecharge() const;
    [[nodiscard]] float getRechargeRestante() const;
    [[nodiscard]] TypeSkill getType() const;
    [[nodiscard]] float getRayonZone() const;
    [[nodiscard]] float getPortee() const;
    [[nodiscard]] int getPuissance() const;

private:
    TypeSkill m_type;
    float m_portee;         // en pixels, écart bord à bord
    int m_puissance;
    float m_rechargeMin;    // en secondes
    float m_rechargeMax;    // en secondes
    float m_tempsRecharge;  // en secondes, durée de la recharge en cours (dernier tirage)
    float m_recharge;       // en secondes, avant le prochain déclenchement possible
    float m_rayonZone;      // en pixels
    float m_tempsExecution; // en secondes, entre l'engagement du coup et son départ
    float m_executionRestante; // en secondes, lue seulement en phase EnExecution
    PhaseSkill m_phase;

    float tirerRecharge(Generateur& generateur) const;
};
