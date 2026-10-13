// ./src/Skill.cpp
#include "Skill.hpp"

Skill::Skill(TypeSkill type, Attaque attaque, float rayonZone)
    : m_type(type),
      m_attaque(attaque),
      m_rayonZone(rayonZone)
{
}

void Skill::mettreAJour(float dt) {
    m_attaque.mettreAJour(dt);
}

bool Skill::estAPortee(Cercle origine, Cercle cible) const {
    return m_attaque.estAPortee(origine, cible);
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

float Skill::getPortee() const {
    return m_attaque.getPortee();
}

float Skill::getRayonZone() const {
    return m_rayonZone;
}
