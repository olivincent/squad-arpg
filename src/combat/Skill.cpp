// ./src/combat/Skill.cpp
#include "combat/Skill.hpp"
#include <algorithm>
#include <cassert>
#include <random>

Skill::Skill(const DefinitionDeSkill& definition)
    : m_type(definition.type),
      m_portee(definition.portee),
      m_puissance(definition.puissance),
      m_rechargeMin(definition.rechargeMin),
      m_rechargeMax(definition.rechargeMax),
      m_tempsRecharge(definition.rechargeMax),
      m_recharge(0.0f),
      m_rayonZone(definition.rayonZone),
      m_tempsExecution(definition.tempsExecution),
      m_coefficients(definition.coefficients),
      m_nature(definition.nature),
      m_armesExigees(definition.armesExigees),
      m_element(definition.element),
      m_bonusDeFrappe(definition.bonusDeFrappe),
      m_executionRestante(0.0f),
      m_phase(PhaseSkill::Pret)
{
    assert(m_rechargeMin >= 0.0f && m_rechargeMin <= m_rechargeMax);
    assert(m_tempsExecution >= 0.0f);
    assert((m_type == TypeSkill::Zone) == (m_nature == NatureDAttaque::Zone));  // la zone est une nature de type Zone
    assert(!m_bonusDeFrappe.has_value() || (m_type == TypeSkill::DegatsSurCible && m_puissance == 0));  // un skill de frappe : dégâts de l'arme
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

TypeSkill Skill::getType() const {
    return m_type;
}

float Skill::getPortee() const {
    return m_portee;
}

const Coefficients& Skill::getCoefficients() const {
    return m_coefficients;
}

NatureDAttaque Skill::getNature() const {
    return m_nature;
}

Element Skill::getElement() const {
    return m_element;
}

const std::optional<Degats>& Skill::getBonusDeFrappe() const {
    return m_bonusDeFrappe;
}

const std::vector<TypeDArme>& Skill::getArmesExigees() const {
    return m_armesExigees;
}

bool Skill::convientA(std::optional<TypeDArme> arme) const {
    return m_armesExigees.empty() || (arme.has_value() && std::ranges::find(m_armesExigees, *arme) != m_armesExigees.end());
}

int Skill::getPuissance() const {
    return m_puissance;
}

float Skill::getRayonZone() const {
    return m_rayonZone;
}
