// ./src/combat/ResolutionDuCoup.cpp
#include "combat/ResolutionDuCoup.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include "combat/Blocage.hpp"
#include "combat/Critique.hpp"
#include "combat/Defense.hpp"
#include "combat/Esquive.hpp"

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

// L'étape esquive, tirage compris : sans chance, aucun tirage.
bool esquiveDe(NatureDAttaque nature, const StatistiquesDeCombat& defenseur, Generateur& generateur) {
    const float chance = chanceDEsquive(nature, defenseur);
    const float tirage = chance > 0.0f ? tirer(generateur) : 0.0f;
    return coupEsquive(chance, tirage);
}

// L'étape blocage, tirage compris : sans chance (récupération, pas de bouclier, zone), aucun tirage.
ResultatDuBlocage blocageDe(int montant, NatureDAttaque nature, const StatistiquesDeCombat& defenseur, bool blocagePret,
                            Generateur& generateur) {
    const float chance = chanceDeBlocage(nature, defenseur, blocagePret);
    const float tirage = chance > 0.0f ? tirer(generateur) : 0.0f;
    return appliquerBlocage(montant, defenseur.montantBloque, chance, tirage);
}
}  // namespace

ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, bool blocagePret, Generateur& generateur) {
    if (attaque.montant == 0) {
        return {};
    }
    if (esquiveDe(attaque.nature, defenses.statistiques, generateur)) {
        return {.montant = 0, .critique = false, .esquive = true};
    }
    const ResultatCritique critique =
        critiqueDe(attaque.montant, attaque.capaciteOffensive, defenses.statistiques.capaciteDefensive, generateur);
    const ResultatDuBlocage blocage =
        blocageDe(critique.montant, attaque.nature, defenses.statistiques, blocagePret, generateur);
    if (blocage.bloque > 0 && blocage.montant == 0) {
        // Tout est absorbé : le coup s'arrête, l'armure et son plancher ne s'appliquent pas.
        return {.montant = 0, .critique = critique.critique, .esquive = false, .bloque = blocage.bloque};
    }
    return {.montant = appliquerArmure(blocage.montant, defenses.armure),
            .critique = critique.critique,
            .esquive = false,
            .bloque = blocage.bloque};
}

ResultatDuCoup resoudreSoin(const Attaque& soin, Generateur& generateur) {
    const ResultatCritique critique =
        critiqueDe(soin.montant, soin.capaciteOffensive, s_capaciteDefensiveDeReference, generateur);
    return {.montant = critique.montant, .critique = critique.critique};
}
