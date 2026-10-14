// ./src/PointsDeVie.cpp
#include "PointsDeVie.hpp"
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

void PointsDeVie::soigner(int montant) {
    assert(montant >= 0);
    if (!estVivant()) {
        return;
    }
    m_valeur += montant;
    if (m_valeur > m_maximum) {
        m_valeur = m_maximum;
    }
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
