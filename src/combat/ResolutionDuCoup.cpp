// ./src/combat/ResolutionDuCoup.cpp
#include "combat/ResolutionDuCoup.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include "combat/Critique.hpp"
#include "combat/Defense.hpp"

namespace {
// Un nombre de 0 inclus à 1 exclu. Le seul tirage du moteur du coup.
float tirer(Generateur& generateur) {
    const float tirage = std::uniform_real_distribution<float>(0.0f, 1.0f)(generateur);
    return std::min(tirage, std::nextafter(1.0f, 0.0f));  // l'arrondi en flottant peut rendre 1
}

// L'étape critique, tirage compris : sans chance, aucun tirage (le `0` passé n'est jamais lu).
ResultatCritique critiqueDe(int montant, int capaciteOffensive, int capaciteDefensive, Generateur& generateur) {
    if (montant == 0) {
        return {.montant = 0, .critique = false};
    }
    const float rapport = rapportDeCapacites(capaciteOffensive, capaciteDefensive);
    const float tirage = chanceDeCritique(rapport) > 0.0f ? tirer(generateur) : 0.0f;
    return appliquerCritique(montant, rapport, tirage);
}
}  // namespace

ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, Generateur& generateur) {
    const ResultatCritique critique =
        critiqueDe(attaque.montant, attaque.capaciteOffensive, defenses.capaciteDefensive, generateur);
    return {.montant = appliquerArmure(critique.montant, defenses.armure), .critique = critique.critique};
}

ResultatDuCoup resoudreSoin(const Attaque& soin, Generateur& generateur) {
    const ResultatCritique critique =
        critiqueDe(soin.montant, soin.capaciteOffensive, s_capaciteDefensiveDeReference, generateur);
    return {.montant = critique.montant, .critique = critique.critique};
}
