// ./src/combat/PointsDeVie.cpp
#include "combat/PointsDeVie.hpp"

PointsDeVie::PointsDeVie(int maximum)
    : m_jauge(maximum)
{
}

void PointsDeVie::subirDegats(int montant) {
    static_cast<void>(m_jauge.retirer(montant));  // seule compte la valeur qui reste, pas ce qui a été retiré
}

int PointsDeVie::soigner(int montant) {
    if (!estVivant()) {
        return 0;
    }
    return m_jauge.ajouter(montant);
}

void PointsDeVie::changerMaximum(int nouveauMaximum) {
    if (estVivant()) {
        m_jauge.changerMaximum(nouveauMaximum, 1);
    } else {
        m_jauge.definirMaximum(nouveauMaximum);
    }
}

bool PointsDeVie::estVivant() const {
    return m_jauge.getValeur() > 0;
}

int PointsDeVie::getValeur() const {
    return m_jauge.getValeur();
}

int PointsDeVie::getMaximum() const {
    return m_jauge.getMaximum();
}

float PointsDeVie::getRatio() const {
    return m_jauge.getRatio();
}
