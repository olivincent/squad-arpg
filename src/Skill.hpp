// ./src/Skill.hpp
#pragma once
#include "Attaque.hpp"
#include "Cercle.hpp"

enum class TypeSkill { Degats, Soin, Provocation, Zone };

// Un skill : un type et une Attaque, qui fournit portée, puissance et
// recharge. La « puissance » est le nombre de dégâts pour un skill de
// Degats, le nombre de points de vie rendus pour un skill de Soin, la
// durée en secondes pour un skill de Provocation, le nombre de dégâts
// pour un skill de Zone. Le rayon de zone est la
// taille de la zone touchée par un skill de zone ; il vaut 0 et n'est pas
// lu pour un skill à cible unique (c'est le type qui décide).
class Skill {
public:
    Skill(TypeSkill type, Attaque attaque, float rayonZone);
    void mettreAJour(float dt);
    bool estAPortee(Cercle origine, Cercle cible) const;
    int declencher(Cercle origine, Cercle cible);
    float getProgressionRecharge() const;
    TypeSkill getType() const;
    float getRayonZone() const;
    float getPortee() const;

private:
    TypeSkill m_type;
    Attaque m_attaque;
    float m_rayonZone;  // en pixels
};
