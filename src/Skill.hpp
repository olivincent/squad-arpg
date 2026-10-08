// ./src/Skill.hpp
#pragma once
#include "Attaque.hpp"
#include "Cercle.hpp"

enum class TypeSkill { Degats, Soin };

// Un skill : un type et une Attaque, qui fournit portée, puissance et
// recharge. La « puissance » est le nombre de dégâts pour un skill de
// Degats, le nombre de points de vie rendus pour un skill de Soin.
class Skill {
public:
    Skill(TypeSkill type, Attaque attaque);
    void mettreAJour(float dt);
    int declencher(Cercle origine, Cercle cible);
    float getProgressionRecharge() const;
    TypeSkill getType() const;

private:
    TypeSkill m_type;
    Attaque m_attaque;
};
