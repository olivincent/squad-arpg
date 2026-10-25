// ./src/combat/Critique.cpp
#include "combat/Critique.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

float rapportDeCapacites(int capaciteOffensive, int capaciteDefensive) {
    assert(capaciteDefensive > 0);
    return static_cast<float>(capaciteOffensive) / static_cast<float>(capaciteDefensive);
}

float chanceDeCritique(float rapport) {
    return std::min(s_chanceParRapportCarre * rapport * rapport, s_chanceMaximale);
}

float multiplicateurDeCritique(float rapport) {
    return std::clamp(s_multiplicateurDeBase + s_multiplicateurParRapport * (rapport - 1.0f), s_multiplicateurDeBase,
                      s_multiplicateurMaximal);
}

bool coupCritique(float rapport, float tirage) {
    return tirage < chanceDeCritique(rapport);
}

ResultatCritique appliquerCritique(int montant, float rapport, float tirage) {
    assert(montant >= 0);
    if (!coupCritique(rapport, tirage)) {
        return {.montant = montant, .critique = false};
    }
    const float amplifie = static_cast<float>(montant) * multiplicateurDeCritique(rapport);
    return {.montant = static_cast<int>(std::lround(amplifie)), .critique = true};
}
