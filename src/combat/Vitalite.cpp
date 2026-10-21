// ./src/combat/Vitalite.cpp
#include "combat/Vitalite.hpp"
#include "combat/Defense.hpp"
#include <cassert>

Vitalite::Vitalite(int pointsDeVieMax, int armure)
    : m_pointsDeVie(pointsDeVieMax),
      m_armure(armure)
{
    assert(armure >= 0);
}

void Vitalite::definirArmure(int armure) {
    assert(armure >= 0);
    m_armure = armure;
}

int Vitalite::recevoirDegats(int montant) {
    if (!estVivant()) {
        return 0;
    }
    const int degats = appliquerArmure(montant, m_armure);
    m_pointsDeVie.subirDegats(degats);
    return degats;
}

int Vitalite::soigner(int montant) {
    return m_pointsDeVie.soigner(montant);
}

bool Vitalite::estVivant() const {
    return m_pointsDeVie.estVivant();
}

int Vitalite::getValeur() const {
    return m_pointsDeVie.getValeur();
}

int Vitalite::getMaximum() const {
    return m_pointsDeVie.getMaximum();
}

float Vitalite::getRatio() const {
    return m_pointsDeVie.getRatio();
}
