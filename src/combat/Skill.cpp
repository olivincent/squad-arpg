// ./src/combat/Skill.cpp
#include "combat/Skill.hpp"
#include <algorithm>
#include <cassert>
#include <random>

Skill::Skill(const DefinitionDeSkill& definition)
    : m_mode(definition.mode),
      m_portee(definition.portee),
      m_rechargeMin(definition.rechargeMin),
      m_rechargeMax(definition.rechargeMax),
      m_tempsRecharge(definition.rechargeMax),
      m_recharge(0.0f),
      m_effet(definition.effet),
      m_rayonZone(definition.rayonZone),
      m_tempsExecution(definition.tempsExecution),
      m_armesExigees(definition.armesExigees),
      m_executionRestante(0.0f),
      m_phase(PhaseSkill::Pret)
{
    assert(m_rechargeMin >= 0.0f && m_rechargeMin <= m_rechargeMax);
    assert(m_tempsExecution >= 0.0f);
    assert(!m_rayonZone.has_value() || *m_rayonZone > 0.0f);
    assert(m_mode != ModeDeCible::Point || m_rayonZone.has_value());  // un skill à point touche une zone
    // On vise un monstre pour lui faire des dégâts ou le provoquer, un allié pour le soigner.
    assert(m_mode != ModeDeCible::Monstre || campTouche(m_effet) == CampTouche::Monstres);
    assert(m_mode != ModeDeCible::Allie || campTouche(m_effet) == CampTouche::Allies);
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

ModeDeCible Skill::getMode() const {
    return m_mode;
}

const EffetDeSkill& Skill::getEffet() const {
    return m_effet;
}

std::optional<float> Skill::getRayonZone() const {
    return m_rayonZone;
}

float Skill::getPortee() const {
    return m_portee;
}

const std::vector<TypeDArme>& Skill::getArmesExigees() const {
    return m_armesExigees;
}

bool Skill::convientA(std::optional<TypeDArme> arme) const {
    return m_armesExigees.empty() || (arme.has_value() && std::ranges::find(m_armesExigees, *arme) != m_armesExigees.end());
}
