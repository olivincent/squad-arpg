// ./src/Skill.cpp
#include "Skill.hpp"
#include <cassert>
#include <random>

Skill::Skill(TypeSkill type, float portee, int puissance, float rechargeMin, float rechargeMax, float rayonZone,
             float tempsExecution)
    : m_type(type),
      m_portee(portee),
      m_puissance(puissance),
      m_rechargeMin(rechargeMin),
      m_rechargeMax(rechargeMax),
      m_tempsRecharge(rechargeMax),
      m_recharge(0.0f),
      m_rayonZone(rayonZone),
      m_tempsExecution(tempsExecution),
      m_executionRestante(0.0f),
      m_phase(PhaseSkill::Pret)
{
    assert(rechargeMin >= 0.0f && rechargeMin <= rechargeMax);
    assert(tempsExecution >= 0.0f);
}

// Recharge fixe (aucun tirage) si les deux bornes sont égales ; sinon tirage
// uniforme dans l'intervalle.
float Skill::tirerRecharge(Generateur& generateur) const {
    if (m_rechargeMin == m_rechargeMax) {
        return m_rechargeMin;
    }
    return std::uniform_real_distribution<float>(m_rechargeMin, m_rechargeMax)(generateur);
}

void Skill::mettreAJour(float dt) {
    m_recharge -= dt;
    if (m_recharge < 0.0f) {
        m_recharge = 0.0f;
    }
    if (m_phase == PhaseSkill::EnRecharge && m_recharge <= 0.0f) {
        m_phase = PhaseSkill::Pret;
    }
}

bool Skill::commencer() {
    if (m_phase != PhaseSkill::Pret) {
        return false;
    }
    m_phase = PhaseSkill::EnExecution;
    m_executionRestante = m_tempsExecution;
    return true;
}

bool Skill::avancer(float dt, Generateur& generateur) {
    if (m_phase != PhaseSkill::EnExecution) {
        mettreAJour(dt);
        return false;
    }

    m_executionRestante -= dt;
    if (m_executionRestante > 0.0f) {
        return false;
    }

    relancerRecharge(generateur);
    return true;
}

void Skill::annuler() {
    if (m_phase == PhaseSkill::EnExecution) {
        m_phase = PhaseSkill::Pret;
    }
}

PhaseSkill Skill::getPhase() const {
    return m_phase;
}

bool Skill::estAPortee(Cercle origine, Cercle cible) const {
    return distanceBordABord(origine, cible) <= m_portee;
}

int Skill::declencher(Cercle origine, Cercle cible, Generateur& generateur) {
    if (!estAPortee(origine, cible) || m_phase != PhaseSkill::Pret) {
        return 0;
    }

    relancerRecharge(generateur);
    return m_puissance;
}

int Skill::declencherSansCible(Generateur& generateur) {
    if (m_phase != PhaseSkill::Pret) {
        return 0;
    }

    relancerRecharge(generateur);
    return m_puissance;
}

void Skill::relancerRecharge(Generateur& generateur) {
    m_tempsRecharge = tirerRecharge(generateur);
    m_recharge = m_tempsRecharge;
    m_phase = m_recharge > 0.0f ? PhaseSkill::EnRecharge : PhaseSkill::Pret;
}

float Skill::getProgressionRecharge() const {
    if (m_tempsRecharge <= 0.0f) {
        return 1.0f;
    }
    return 1.0f - m_recharge / m_tempsRecharge;
}

float Skill::getRechargeRestante() const {
    return m_recharge;
}

TypeSkill Skill::getType() const {
    return m_type;
}

float Skill::getPortee() const {
    return m_portee;
}

int Skill::getPuissance() const {
    return m_puissance;
}

float Skill::getRayonZone() const {
    return m_rayonZone;
}
