// ./src/combat/Reserve.cpp
#include "combat/Reserve.hpp"
#include <cassert>

Reserve::Reserve(int maximum, float regenerationParSeconde)
    : m_jauge(maximum),
      m_regeneration(regenerationParSeconde),
      m_accumule(0.0f)
{
    assert(regenerationParSeconde >= 0.0f);
}

void Reserve::regenerer(float dt) {
    m_accumule += m_regeneration * dt;
    const int entier = static_cast<int>(m_accumule);
    static_cast<void>(m_jauge.ajouter(entier));  // plafonné : seule compte la valeur qui en résulte
    m_accumule -= static_cast<float>(entier);
    if (m_jauge.getValeur() == m_jauge.getMaximum()) {
        m_accumule = 0.0f;
    }
}

bool Reserve::suffit(int cout) const {
    return m_jauge.getValeur() >= cout;
}

void Reserve::payer(int cout) {
    assert(suffit(cout));
    static_cast<void>(m_jauge.retirer(cout));  // tout a été retiré, par la précondition
}

void Reserve::changerMaximum(int nouveauMaximum) {
    m_jauge.changerMaximum(nouveauMaximum);
}

void Reserve::definirMaximum(int nouveauMaximum) {
    m_jauge.definirMaximum(nouveauMaximum);
}

void Reserve::definirRegeneration(float regenerationParSeconde) {
    assert(regenerationParSeconde >= 0.0f);
    m_regeneration = regenerationParSeconde;
}

int Reserve::getValeur() const {
    return m_jauge.getValeur();
}

int Reserve::getMaximum() const {
    return m_jauge.getMaximum();
}

float Reserve::getRatio() const {
    return m_jauge.getRatio();
}

float Reserve::getRegeneration() const {
    return m_regeneration;
}
