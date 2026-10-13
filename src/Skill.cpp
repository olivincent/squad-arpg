// ./src/Skill.cpp
#include "Skill.hpp"

Skill::Skill(TypeSkill type, float portee, int puissance, float tempsRecharge, float rayonZone)
    : m_type(type),
      m_portee(portee),
      m_puissance(puissance),
      m_tempsRecharge(tempsRecharge),
      m_recharge(0.0f),
      m_rayonZone(rayonZone)
{
}

void Skill::mettreAJour(float dt) {
    m_recharge -= dt;
    if (m_recharge < 0.0f) {
        m_recharge = 0.0f;
    }
}

bool Skill::estAPortee(Cercle origine, Cercle cible) const {
    return distanceBordABord(origine, cible) <= m_portee;
}

int Skill::declencher(Cercle origine, Cercle cible) {
    if (!estAPortee(origine, cible) || m_recharge > 0.0f) {
        return 0;
    }

    m_recharge = m_tempsRecharge;
    return m_puissance;
}

void Skill::relancerRecharge() {
    m_recharge = m_tempsRecharge;
}

float Skill::getProgressionRecharge() const {
    if (m_tempsRecharge <= 0.0f) {
        return 1.0f;
    }
    return 1.0f - m_recharge / m_tempsRecharge;
}

TypeSkill Skill::getType() const {
    return m_type;
}

float Skill::getPortee() const {
    return m_portee;
}

float Skill::getRayonZone() const {
    return m_rayonZone;
}
