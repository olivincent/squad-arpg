// ./src/Skill.cpp
#include "Skill.hpp"

Skill::Skill(TypeSkill type, Attaque attaque)
    : m_type(type),
      m_attaque(attaque)
{
}

void Skill::mettreAJour(float dt) {
    m_attaque.mettreAJour(dt);
}

int Skill::declencher(Cercle origine, Cercle cible) {
    return m_attaque.frapper(origine, cible);
}

float Skill::getProgressionRecharge() const {
    return m_attaque.getProgressionRecharge();
}

TypeSkill Skill::getType() const {
    return m_type;
}
