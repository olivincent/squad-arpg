// ./src/combat/Jauge.cpp
#include "combat/Jauge.hpp"
#include <algorithm>
#include <cassert>

Jauge::Jauge(int maximum)
    : m_valeur(maximum),
      m_maximum(maximum)
{
    assert(maximum > 0);
}

int Jauge::ajouter(int montant) {
    assert(montant >= 0);
    const int avant = m_valeur;
    m_valeur = std::min(m_valeur + montant, m_maximum);
    return m_valeur - avant;
}

int Jauge::retirer(int montant) {
    assert(montant >= 0);
    const int avant = m_valeur;
    m_valeur = std::max(m_valeur - montant, 0);
    return avant - m_valeur;
}

void Jauge::changerMaximum(int nouveauMaximum, int plancher) {
    assert(nouveauMaximum > 0);
    assert(plancher >= 0 && plancher <= nouveauMaximum);
    m_valeur = std::clamp(m_valeur + (nouveauMaximum - m_maximum), plancher, nouveauMaximum);
    m_maximum = nouveauMaximum;
}

void Jauge::definirMaximum(int nouveauMaximum) {
    assert(nouveauMaximum > 0);
    m_valeur = std::min(m_valeur, nouveauMaximum);
    m_maximum = nouveauMaximum;
}

int Jauge::getValeur() const {
    return m_valeur;
}

int Jauge::getMaximum() const {
    return m_maximum;
}

float Jauge::getRatio() const {
    return static_cast<float>(m_valeur) / static_cast<float>(m_maximum);
}
