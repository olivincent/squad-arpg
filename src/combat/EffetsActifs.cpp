// ./src/combat/EffetsActifs.cpp
#include "combat/EffetsActifs.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include "combat/Cible.hpp"

namespace {
// Marge sous laquelle un tick en retard d'un rien est quand même rendu à la fin de l'effet : la durée et le compte à rebours du tick
// s'usent du même `dt`, mais le compte à rebours repart de 1 à chaque tick, et deux sommes de flottants ne tombent pas toujours ensemble.
constexpr float s_toleranceDeTick = 1.0e-3f;

// De deux valeurs d'un même effet, celle qui est la plus forte (la première en cas d'égalité), et pour un effet dans le temps le rythme
// des ticks de celle qui était déjà posée. Une surcharge par sorte : une sorte de plus ne compile pas tant qu'on n'a pas dit comment
// comparer ses valeurs.
ModificateurDeStatistique lePlusFort(const ModificateurDeStatistique& present, const ModificateurDeStatistique& nouveau) {
    return std::abs(nouveau.pourcentage) > std::abs(present.pourcentage) ? nouveau : present;
}

DegatsDansLeTemps lePlusFort(const DegatsDansLeTemps& present, const DegatsDansLeTemps& nouveau) {
    DegatsDansLeTemps retenu = nouveau.montant > present.montant ? nouveau : present;
    retenu.avantLeTick = present.avantLeTick;
    return retenu;
}

SoinDansLeTemps lePlusFort(const SoinDansLeTemps& present, const SoinDansLeTemps& nouveau) {
    SoinDansLeTemps retenu = nouveau.montant > present.montant ? nouveau : present;
    retenu.avantLeTick = present.avantLeTick;
    return retenu;
}

// Un contrôle posé de nouveau garde le pourcentage le plus fort (le même état, puisque l'identité est la même).
Controle lePlusFort(const Controle& present, const Controle& nouveau) {
    return nouveau.pourcentage > present.pourcentage ? nouveau : present;
}

// Deux sortes de même identité ont la même alternative (une identité est un skill et un rang, et un rang est une sorte).
SorteDEffetActif lePlusFort(const SorteDEffetActif& present, const SorteDEffetActif& nouveau) {
    assert(present.index() == nouveau.index());
    return std::visit(
        [&](const auto& valeurNouvelle) -> SorteDEffetActif {
            return lePlusFort(std::get<std::decay_t<decltype(valeurNouvelle)>>(present), valeurNouvelle);
        },
        nouveau);
}

// Combien de ticks ont lieu quand `ecoule` secondes passent sur un compte à rebours de tick ; `ecoule` ne dépasse jamais ce qui restait à
// l'effet, et `fin` dit qu'il vient de s'épuiser.
int ticksEchus(float& avantLeTick, float ecoule, bool fin) {
    int ticks = 0;
    avantLeTick -= ecoule;
    while (avantLeTick <= 0.0f) {
        ++ticks;
        avantLeTick += s_intervalleDeTick;
    }
    if (fin && avantLeTick <= s_toleranceDeTick) {
        ++ticks;
        avantLeTick += s_intervalleDeTick;
    }
    return ticks;
}
}  // namespace

void EffetsActifs::poser(const EffetActif& effet) {
    assert(effet.dureeRestante > 0.0f);
    assert(effet.piles >= 1 && effet.piles <= effet.pilesMax);
    const auto present = std::ranges::find_if(
        m_effets, [&](const EffetActif& autre) { return autre.source == effet.source && autre.rang == effet.rang; });
    if (present == m_effets.end()) {
        m_effets.push_back(effet);
        return;
    }
    present->dureeRestante = std::max(present->dureeRestante, effet.dureeRestante);
    present->sorte = lePlusFort(present->sorte, effet.sorte);
    // Une pile de plus, jusqu'au maximum ; avec un maximum de 1, la règle de l'étape 111 (rien que la durée et la valeur).
    present->piles = std::min(present->piles + effet.piles, effet.pilesMax);
    present->pilesMax = effet.pilesMax;
}

std::vector<Tick> EffetsActifs::mettreAJour(float dt) {
    m_rendements.mettreAJour(dt);
    std::vector<Tick> ticks;
    for (EffetActif& effet : m_effets) {
        const float ecoule = std::min(dt, effet.dureeRestante);
        effet.dureeRestante -= dt;
        const bool fin = effet.dureeRestante <= 0.0f;
        std::visit(Surcharges{
                       [](ModificateurDeStatistique&) {},
                       [](Controle&) {},
                       [&](DegatsDansLeTemps& degats) {
                           for (int i = ticksEchus(degats.avantLeTick, ecoule, fin); i > 0; --i) {
                               ticks.push_back(TickDeDegats{degats.element, degats.montant * effet.piles});
                           }
                       },
                       [&](SoinDansLeTemps& soin) {
                           for (int i = ticksEchus(soin.avantLeTick, ecoule, fin); i > 0; --i) {
                               ticks.push_back(TickDeSoin{soin.montant * effet.piles});
                           }
                       },
                   },
                   effet.sorte);
    }
    std::erase_if(m_effets, [](const EffetActif& effet) { return effet.dureeRestante <= 0.0f; });
    return ticks;
}

void EffetsActifs::vider() {
    m_effets.clear();
    m_rendements.vider();
}

float EffetsActifs::compterUneApplicationDeControle(EtatInflige etat) {
    return m_rendements.appliquer(etat);
}

const RendementsDeControle& EffetsActifs::rendements() const {
    return m_rendements;
}

const std::vector<EffetActif>& EffetsActifs::liste() const {
    return m_effets;
}

std::vector<int> EffetsActifs::pilesDesBonus() const {
    std::vector<int> piles;
    for (const EffetActif& effet : m_effets) {
        if (estUnBonus(effet)) {
            piles.push_back(effet.piles);
        }
    }
    return piles;
}

int EffetsActifs::nombreDeControles() const {
    return static_cast<int>(std::ranges::count_if(m_effets, [](const EffetActif& effet) { return estUnControle(effet); }));
}

std::vector<int> EffetsActifs::pilesDesMalus() const {
    std::vector<int> piles;
    for (const EffetActif& effet : m_effets) {
        if (!estUnBonus(effet) && !estUnControle(effet)) {
            piles.push_back(effet.piles);
        }
    }
    return piles;
}

Defenses appliquerLesEffets(const Defenses& defenses, const EffetsActifs& effets) {
    int pourcentageDArmure = 0;
    for (const EffetActif& effet : effets.liste()) {
        std::visit(Surcharges{
                       [&](const ModificateurDeStatistique& modificateur) {
                           switch (modificateur.statistique) {
                               case StatistiqueModifiable::Armure:
                                   pourcentageDArmure += modificateur.pourcentage * effet.piles;
                                   break;
                           }
                       },
                       // Les effets dans le temps ne changent aucune statistique : ils agissent par leurs ticks. Un contrôle non plus : il
                       // se lit par ses règles (`Controle.hpp`).
                       [](const DegatsDansLeTemps&) {},
                       [](const SoinDansLeTemps&) {},
                       [](const Controle&) {},
                   },
                   effet.sorte);
    }
    Defenses resultat = defenses;
    const double armure = static_cast<double>(defenses.armure) * (100 + pourcentageDArmure) / 100.0;
    resultat.armure = std::max(0, static_cast<int>(std::lround(armure)));
    return resultat;
}
