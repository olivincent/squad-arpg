// ./src/combat/Vitalite.cpp
#include "combat/Vitalite.hpp"
#include "combat/Blocage.hpp"
#include "combat/ResolutionDuCoup.hpp"
#include <cassert>
#include <utility>
#include "combat/Cible.hpp"

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
            poserEffet(effet);
        }
    }
    return resultat;
}

void Vitalite::mettreAJour(float dt) {
    m_recuperationDuBlocage = m_recuperationDuBlocage > dt ? m_recuperationDuBlocage - dt : 0.0f;
    if (!estVivant()) {
        return;
    }
    for (const Tick& tick : m_effets.mettreAJour(dt)) {
        if (!estVivant()) {
            break;  // mort d'un tick précédent de ce pas : les suivants n'ont pas lieu
        }
        std::visit(Surcharges{
                       [&](const TickDeDegats& degats) {
                           const int subi = resoudreTick(degats.element, degats.montant, m_defenses.statistiques);
                           m_pointsDeVie.subirDegats(subi);
                           if (subi > 0) {
                               m_ticksAppliques.push_back(TickDeDegats{degats.element, subi});
                           }
                           if (!estVivant()) {
                               m_effets.vider();
                           }
                       },
                       [&](const TickDeSoin& soin) {
                           if (const int rendu = m_pointsDeVie.soigner(soin.montant); rendu > 0) {
                               m_ticksAppliques.push_back(TickDeSoin{rendu});
                           }
                       },
                   },
                   tick);
    }
}

std::vector<Tick> Vitalite::prendreLesTicks() {
    return std::exchange(m_ticksAppliques, {});
}

void Vitalite::viderLesEffets() {
    m_effets.vider();
}

void Vitalite::poserEffet(const EffetActif& effet) {
    if (!estVivant()) {
        return;
    }
    const auto* controle = std::get_if<Controle>(&effet.sorte);
    if (controle == nullptr) {
        m_effets.poser(effet);
        return;
    }
    // Un contrôle : le rendement décroissant de cet état d'abord (une application bloquée ne pose rien), puis la durée selon la
    // résistance de la cible, lue maintenant.
    const float rendement = m_effets.compterUneApplicationDeControle(controle->etat);
    if (rendement <= 0.0f) {
        m_controlesAppliques.push_back({controle->etat, true});
        return;
    }
    EffetActif pose = effet;
    pose.dureeRestante = dureePosee(effet.dureeRestante, resistanceAppliquee(m_defenses.statistiques, controle->etat), rendement);
    m_effets.poser(pose);
    m_controlesAppliques.push_back({controle->etat, false});
}

std::vector<ControleApplique> Vitalite::prendreLesControles() {
    return std::exchange(m_controlesAppliques, {});
}

const EffetsActifs& Vitalite::getEffets() const {
    return m_effets;
}

int Vitalite::getArmure() const {
    return appliquerLesEffets(m_defenses, m_effets).armure;
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
