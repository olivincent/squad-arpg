// ./src/Attaque.cpp
#include "Attaque.hpp"

Attaque::Attaque(float portee, int degats, float tempsRecharge)
    : m_portee(portee),
      m_degats(degats),
      m_tempsRecharge(tempsRecharge),
      m_recharge(0.0f)
{
}

bool Attaque::estAPortee(sf::Vector2f origine, sf::Vector2f cible) const {
    return (cible - origine).length() <= m_portee;
}

void Attaque::mettreAJour(float dt) {
    m_recharge -= dt;
    if (m_recharge < 0.0f) {
        m_recharge = 0.0f;
    }
}

int Attaque::frapper(sf::Vector2f origine, sf::Vector2f cible) {
    if (!estAPortee(origine, cible) || m_recharge > 0.0f) {
        return 0;
    }

    m_recharge = m_tempsRecharge;
    return m_degats;
}
