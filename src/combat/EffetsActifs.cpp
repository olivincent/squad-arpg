// ./src/combat/EffetsActifs.cpp
#include "combat/EffetsActifs.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include "combat/Cible.hpp"

namespace {
// De deux sorts du même skill, celle dont la valeur est la plus forte (la première en cas d'égalité). Le visiteur à deux variants veut
// une surcharge par paire : une sorte de plus ne compile pas tant qu'on n'a pas dit comment la comparer.
SorteDEffetActif lePlusFort(const SorteDEffetActif& present, const SorteDEffetActif& nouveau) {
    return std::visit(Surcharges{
                          [](const ModificateurDeStatistique& a, const ModificateurDeStatistique& b) -> SorteDEffetActif {
                              return std::abs(b.pourcentage) > std::abs(a.pourcentage) ? b : a;
                          },
                      },
                      present, nouveau);
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

void EffetsActifs::mettreAJour(float dt) {
    for (EffetActif& effet : m_effets) {
        effet.dureeRestante -= dt;
    }
    std::erase_if(m_effets, [](const EffetActif& effet) { return effet.dureeRestante <= 0.0f; });
}

void EffetsActifs::vider() {
    m_effets.clear();
}

const std::vector<EffetActif>& EffetsActifs::liste() const {
    return m_effets;
}

int EffetsActifs::nombreDeBonus() const {
    return static_cast<int>(std::ranges::count_if(m_effets, [](const EffetActif& effet) { return estUnBonus(effet); }));
}

int EffetsActifs::nombreDeMalus() const {
    return static_cast<int>(m_effets.size()) - nombreDeBonus();
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
                   },
                   effet.sorte);
    }
    Defenses resultat = defenses;
    const double armure = static_cast<double>(defenses.armure) * (100 + pourcentageDArmure) / 100.0;
    resultat.armure = std::max(0, static_cast<int>(std::lround(armure)));
    return resultat;
}
