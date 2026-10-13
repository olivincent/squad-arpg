// ./src/Skill.hpp
#pragma once
#include "Cercle.hpp"

enum class TypeSkill { Degats, Soin, Provocation, Zone };

// Un skill : un type, une portée, une puissance et une recharge. La
// « puissance » est le nombre de dégâts pour un skill de Degats, le nombre
// de points de vie rendus pour un skill de Soin, la durée en secondes pour
// un skill de Provocation, le nombre de dégâts pour un skill de Zone. Le
// rayon de zone est la taille de la zone touchée par un skill de zone ; il
// vaut 0 et n'est pas lu pour un skill à cible unique (c'est le type qui
// décide).
class Skill {
public:
    Skill(TypeSkill type, float portee, int puissance, float tempsRecharge, float rayonZone);
    void mettreAJour(float dt);
    bool estAPortee(Cercle origine, Cercle cible) const;
    int declencher(Cercle origine, Cercle cible);
    void relancerRecharge();
    float getProgressionRecharge() const;
    float getRechargeRestante() const;
    TypeSkill getType() const;
    float getRayonZone() const;
    float getPortee() const;

private:
    TypeSkill m_type;
    float m_portee;         // en pixels, écart bord à bord
    int m_puissance;
    float m_tempsRecharge;  // en secondes
    float m_recharge;       // en secondes, avant le prochain déclenchement possible
    float m_rayonZone;      // en pixels
};
