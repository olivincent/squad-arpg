// ./src/combat/PointsDeVie.cpp
#include "combat/PointsDeVie.hpp"
#include <algorithm>
#include <cassert>

PointsDeVie::PointsDeVie(int maximum)
    : m_valeur(maximum),
      m_maximum(maximum)
{
    assert(maximum > 0);
}

void PointsDeVie::subirDegats(int montant) {
    assert(montant >= 0);
    m_valeur -= montant;
    if (m_valeur < 0) {
        m_valeur = 0;
    }
}

int PointsDeVie::soigner(int montant) {
    assert(montant >= 0);
    if (!estVivant()) {
        return 0;
    }
    const int avant = m_valeur;
    m_valeur += montant;
    if (m_valeur > m_maximum) {
        m_valeur = m_maximum;
    }
    return m_valeur - avant;
}

void PointsDeVie::changerMaximum(int nouveauMaximum) {
    assert(nouveauMaximum > 0);
    if (estVivant()) {
        m_valeur = std::clamp(m_valeur + (nouveauMaximum - m_maximum), 1, nouveauMaximum);
    }
    m_maximum = nouveauMaximum;
}

bool PointsDeVie::estVivant() const {
    return m_valeur > 0;
}

int PointsDeVie::getValeur() const {
    return m_valeur;
}

int PointsDeVie::getMaximum() const {
    return m_maximum;
}

float PointsDeVie::getRatio() const {
    return static_cast<float>(m_valeur) / static_cast<float>(m_maximum);
}
