// ./src/Attaque.cpp
#include "Attaque.hpp"

Attaque::Attaque(float portee, int degats, float tempsRecharge)
    : m_portee(portee),
      m_degats(degats),
      m_tempsRecharge(tempsRecharge),
      m_recharge(0.0f)
{
}

bool Attaque::estAPortee(Cercle origine, Cercle cible) const {
    return distanceBordABord(origine, cible) <= m_portee;
}

void Attaque::mettreAJour(float dt) {
    m_recharge -= dt;
    if (m_recharge < 0.0f) {
        m_recharge = 0.0f;
    }
}

int Attaque::frapper(Cercle origine, Cercle cible) {
    if (!estAPortee(origine, cible) || m_recharge > 0.0f) {
        return 0;
    }

    m_recharge = m_tempsRecharge;
    return m_degats;
}

void Attaque::relancerRecharge() {
    m_recharge = m_tempsRecharge;
}

float Attaque::getPortee() const {
    return m_portee;
}

float Attaque::getProgressionRecharge() const {
    if (m_tempsRecharge <= 0.0f) {
        return 1.0f;
    }
    return 1.0f - m_recharge / m_tempsRecharge;
}
