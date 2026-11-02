// ./src/combat/Vitalite.cpp
#include "combat/Vitalite.hpp"
#include "combat/Blocage.hpp"
#include "combat/ResolutionDuCoup.hpp"
#include <cassert>

Vitalite::Vitalite(int pointsDeVieMax, const Defenses& defenses)
    : m_pointsDeVie(pointsDeVieMax),
      m_defenses(defenses)
{
    assert(defenses.armure >= 0 && defenses.statistiques.capaciteDefensive > 0);
}

void Vitalite::definirDefenses(const Defenses& defenses) {
    assert(defenses.armure >= 0 && defenses.statistiques.capaciteDefensive > 0);
    m_defenses = defenses;
}

ResultatDuCoup Vitalite::recevoirDegats(const Attaque& attaque, Generateur& generateur) {
    if (!estVivant()) {
        return {.montant = 0, .critique = false};
    }
    const ResultatDuCoup resultat = resoudreCoup(attaque, appliquerLesEffets(m_defenses, m_effets), blocagePret(), generateur);
    if (resultat.bloque > 0) {
        m_recuperationDuBlocage = s_recuperationDuBlocage;
    }
    m_pointsDeVie.subirDegats(resultat.montant);
    if (!estVivant()) {
        m_effets.vider();
    } else if (!resultat.esquive) {
        // Les effets du skill ne se posent que si le coup atteint : un coup esquivé n'applique rien.
        for (const EffetActif& effet : attaque.effets) {
            m_effets.poser(effet);
        }
    }
    return resultat;
}

void Vitalite::mettreAJour(float dt) {
    m_recuperationDuBlocage = m_recuperationDuBlocage > dt ? m_recuperationDuBlocage - dt : 0.0f;
    m_effets.mettreAJour(dt);
}

void Vitalite::poserEffet(const EffetActif& effet) {
    if (estVivant()) {
        m_effets.poser(effet);
    }
}

const EffetsActifs& Vitalite::getEffets() const {
    return m_effets;
}

bool Vitalite::blocagePret() const {
    return m_recuperationDuBlocage <= 0.0f;
}

void Vitalite::definirMaximum(int maximum) {
    m_pointsDeVie.changerMaximum(maximum);
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
