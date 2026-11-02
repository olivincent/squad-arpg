// ./src/combat/Skill.cpp
#include "combat/Skill.hpp"
#include <algorithm>
#include <cassert>
#include <random>

namespace {
// Un effet dans le temps a une durée, au moins une pile et un montant positif ou nul (lu seulement par un `assert`).
[[maybe_unused]] bool effetsDansLeTempsValides(const std::vector<EffetDeSkill>& effets) {
    return std::ranges::all_of(effets, [](const EffetDeSkill& effet) {
        if (const auto* degats = std::get_if<EffetDegatsDansLeTemps>(&effet)) {
            return degats->duree > 0 && degats->pilesMax >= 1 && degats->montant >= 0;
        }
        if (const auto* soin = std::get_if<EffetSoinDansLeTemps>(&effet)) {
            return soin->duree > 0 && soin->pilesMax >= 1 && soin->montant >= 0;
        }
        return true;
    });
}

// Un contrôle a une durée, et un pourcentage de 1 à 100 pour le ralentissement et l'entrave seulement (lu seulement par un `assert`).
[[maybe_unused]] bool controlesValides(const std::vector<EffetDeSkill>& effets) {
    return std::ranges::all_of(effets, [](const EffetDeSkill& effet) {
        const auto* controle = std::get_if<EffetControle>(&effet);
        if (controle == nullptr) {
            return true;
        }
        const bool aUnPourcentage = controle->etat == EtatInflige::Ralentissement || controle->etat == EtatInflige::Entrave;
        return controle->duree > 0.0f && (aUnPourcentage ? controle->pourcentage >= 1 && controle->pourcentage <= 100 : controle->pourcentage == 0);
    });
}
}  // namespace

Skill::Skill(const DefinitionDeSkill& definition)
    : m_mode(definition.mode),
      m_portee(definition.portee),
      m_rechargeMin(definition.rechargeMin),
      m_rechargeMax(definition.rechargeMax),
      m_tempsRecharge(definition.rechargeMax),
      m_recharge(0.0f),
      m_effets(definition.effets),
      m_rayonZone(definition.rayonZone),
      m_coefficientDAire(definition.coefficientDAire),
      m_tempsExecution(definition.tempsExecution),
      m_armesExigees(definition.armesExigees),
      m_cout(definition.cout),
      m_executionRestante(0.0f),
      m_phase(PhaseSkill::Pret)
{
    assert(m_rechargeMin >= 0.0f && m_rechargeMin <= m_rechargeMax);
    assert(m_tempsExecution >= 0.0f);
    assert(m_cout >= 0);
    assert(!m_effets.empty());
    assert(!m_rayonZone.has_value() || *m_rayonZone > 0.0f);
    assert(m_coefficientDAire >= 0.0f && m_coefficientDAire <= 1.0f);
    assert(m_mode != ModeDeCible::Point || m_rayonZone.has_value());  // un skill à point touche une zone
    // On vise un monstre pour lui faire des dégâts ou le provoquer, un allié pour le soigner.
    assert(m_mode != ModeDeCible::Monstre || campTouche(m_effets.front()) == CampTouche::Monstres);
    assert(m_mode != ModeDeCible::Allie || campTouche(m_effets.front()) == CampTouche::Allies);
    // Toute l'escouade : un skill sans zone ni portée qui touche les alliés (un buff).
    assert(m_mode != ModeDeCible::Escouade || (campTouche(m_effets.front()) == CampTouche::Allies && !m_rayonZone.has_value()));
    // Un modificateur principal est un buff, posé sur les alliés ; un affaiblissement n'existe que derrière un coup.
    assert(!std::holds_alternative<EffetModificateur>(m_effets.front()) || campTouche(m_effets.front()) == CampTouche::Allies);
    // Des dégâts dans le temps ne sont jamais l'effet principal : ils suivent un coup, qui seul les fait toucher une cible.
    assert(!std::holds_alternative<EffetDegatsDansLeTemps>(m_effets.front()));
    // Les effets secondaires sont des modificateurs, des dégâts dans le temps ou des contrôles qui suivent des dégâts ou une frappe et touchent
    // le même camp que lui : `Escouade` et `Personnage` n'en posent pas d'autres.
    assert(m_effets.size() == 1 || std::holds_alternative<EffetDegats>(m_effets.front()) ||
           std::holds_alternative<EffetFrappe>(m_effets.front()));
    for (std::size_t i = 1; i < m_effets.size(); ++i) {
        assert(std::holds_alternative<EffetModificateur>(m_effets[i]) || std::holds_alternative<EffetDegatsDansLeTemps>(m_effets[i]) ||
               std::holds_alternative<EffetControle>(m_effets[i]));
        assert(campTouche(m_effets[i]) == campTouche(m_effets.front()));
    }
    assert(effetsDansLeTempsValides(m_effets));
    assert(controlesValides(m_effets));
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

float Skill::getRechargeMin() const {
    return m_rechargeMin;
}

const EffetDeSkill& Skill::getEffet() const {
    return m_effets.front();
}

const std::vector<EffetDeSkill>& Skill::getEffets() const {
    return m_effets;
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

int Skill::getCout() const {
    return m_cout;
}

bool Skill::convientA(std::optional<TypeDArme> arme) const {
    return m_armesExigees.empty() || (arme.has_value() && std::ranges::find(m_armesExigees, *arme) != m_armesExigees.end());
}

float Skill::getCoefficientDAire() const {
    return m_coefficientDAire;
}
