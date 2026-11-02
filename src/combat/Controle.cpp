// ./src/combat/Controle.cpp
#include "combat/Controle.hpp"
#include <algorithm>
#include "combat/Defense.hpp"

namespace {
constexpr float s_pourCent = 100.0f;

// Le plus fort pourcentage parmi les contrôles de cet état (0 s'il n'y en a aucun).
int pourcentageLePlusFort(const EffetsActifs& effets, EtatInflige etat) {
    int plusFort = 0;
    for (const EffetActif& effet : effets.liste()) {
        if (const auto* controle = std::get_if<Controle>(&effet.sorte); controle != nullptr && controle->etat == etat) {
            plusFort = std::max(plusFort, controle->pourcentage);
        }
    }
    return plusFort;
}

// 1 − pourcentage, jamais sous 0.
float facteurDuPourcentage(int pourcentage) {
    return std::max(0.0f, 1.0f - static_cast<float>(pourcentage) / s_pourCent);
}
}  // namespace

EtatDeControle etatQuiResiste(EtatInflige etat) {
    switch (etat) {
        case EtatInflige::Etourdissement: return EtatDeControle::Etourdissement;
        case EtatInflige::Ralentissement:
        case EtatInflige::Entrave:  // l'entrave n'a pas de résistance propre : celle du ralentissement
            return EtatDeControle::Ralentissement;
        case EtatInflige::Disruption: return EtatDeControle::Disruption;
        case EtatInflige::Nombre: return EtatDeControle::Etourdissement;
    }
    return EtatDeControle::Etourdissement;
}

float resistanceAppliquee(const StatistiquesDeCombat& statistiques, EtatInflige etat) {
    return resistanceAuControleEffective(statistiques.resistancesAuControle[etatQuiResiste(etat)]);
}

float dureePosee(float dureeAvantResistance, float resistance, float rendement) {
    return dureeAvantResistance * (1.0f - resistance / s_pourCent) * rendement;
}

bool estSousEtat(const EffetsActifs& effets, EtatInflige etat) {
    return std::ranges::any_of(effets.liste(), [etat](const EffetActif& effet) {
        const auto* controle = std::get_if<Controle>(&effet.sorte);
        return controle != nullptr && controle->etat == etat;
    });
}

bool peutBouger(const EffetsActifs& effets) {
    return !estSousEtat(effets, EtatInflige::Etourdissement);
}

bool peutAttaquer(const EffetsActifs& effets) {
    return !estSousEtat(effets, EtatInflige::Etourdissement);
}

bool peutUtiliserUnSkill(const EffetsActifs& effets) {
    return !estSousEtat(effets, EtatInflige::Etourdissement) && !estSousEtat(effets, EtatInflige::Disruption);
}

float facteurDeDeplacement(const EffetsActifs& effets) {
    return facteurDuPourcentage(pourcentageLePlusFort(effets, EtatInflige::Ralentissement));
}

float facteurDAttaque(const EffetsActifs& effets) {
    return facteurDuPourcentage(pourcentageLePlusFort(effets, EtatInflige::Entrave));
}
