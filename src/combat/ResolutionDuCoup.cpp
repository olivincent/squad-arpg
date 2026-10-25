// ./src/combat/ResolutionDuCoup.cpp
#include "combat/ResolutionDuCoup.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include "combat/Blocage.hpp"
#include "combat/Critique.hpp"
#include "combat/Defense.hpp"
#include "combat/Element.hpp"
#include "combat/Esquive.hpp"

namespace {
// Un nombre de 0 inclus à 1 exclu. Le seul tirage du moteur du coup.
float tirer(Generateur& generateur) {
    const float tirage = std::uniform_real_distribution<float>(0.0f, 1.0f)(generateur);
    return std::min(tirage, std::nextafter(1.0f, 0.0f));  // l'arrondi en flottant peut rendre 1
}

// L'étape critique d'un montant seul (un soin), tirage compris : sans chance, aucun tirage (le `0` passé n'est
// jamais lu).
ResultatCritique critiqueDe(int montant, int capaciteOffensive, int capaciteDefensive, Generateur& generateur) {
    if (montant == 0) {
        return {.montant = 0, .critique = false};
    }
    const float rapport = rapportDeCapacites(capaciteOffensive, capaciteDefensive);
    const float tirage = chanceDeCritique(rapport) > 0.0f ? tirer(generateur) : 0.0f;
    return appliquerCritique(montant, rapport, tirage);
}

// L'étape critique d'un coup, tirage compris : le multiplicateur à appliquer à chaque portion, 1 sans critique.
struct CritiqueDuCoup {
    bool critique;
    float multiplicateur;
};

CritiqueDuCoup critiqueDuCoup(int capaciteOffensive, int capaciteDefensive, Generateur& generateur) {
    const float rapport = rapportDeCapacites(capaciteOffensive, capaciteDefensive);
    const float tirage = chanceDeCritique(rapport) > 0.0f ? tirer(generateur) : 0.0f;
    if (!coupCritique(rapport, tirage)) {
        return {.critique = false, .multiplicateur = 1.0f};
    }
    return {.critique = true, .multiplicateur = multiplicateurDeCritique(rapport)};
}

// L'étape esquive, tirage compris : sans chance, aucun tirage.
bool esquiveDe(NatureDAttaque nature, const StatistiquesDeCombat& defenseur, Generateur& generateur) {
    const float chance = chanceDEsquive(nature, defenseur);
    const float tirage = chance > 0.0f ? tirer(generateur) : 0.0f;
    return coupEsquive(chance, tirage);
}

// L'étape blocage de la portion physique, tirage compris : sans chance (récupération, pas de bouclier, zone) ou
// sans portion physique, aucun tirage.
ResultatDuBlocage blocageDe(float physique, NatureDAttaque nature, const StatistiquesDeCombat& defenseur, bool blocagePret,
                            Generateur& generateur) {
    const float chance = physique > 0.0f ? chanceDeBlocage(nature, defenseur, blocagePret) : 0.0f;
    const float tirage = chance > 0.0f ? tirer(generateur) : 0.0f;
    return appliquerBlocage(physique, defenseur.montantBloque, chance, tirage);
}

// Un seul arrondi par portion, au plus proche. Si tout arrondit à 0 (le coup n'a pas été entièrement bloqué : il
// inflige au moins 1), le point va à la plus grosse portion (à égalité, la première de l'énumération).
Degats arrondir(const Portions& portions) {
    Degats arrondies;
    for (const Element element : s_elements) {
        arrondies[element] = static_cast<int>(std::lround(portions[element]));
    }
    if (arrondies.total() < 1) {
        arrondies[portions.elementDominant()] = 1;
    }
    return arrondies;
}
}  // namespace

ResultatDuCoup resoudreCoup(const Attaque& attaque, const Defenses& defenses, bool blocagePret, Generateur& generateur) {
    if (attaque.degats.total() == 0) {
        return {};
    }
    if (esquiveDe(attaque.nature, defenses.statistiques, generateur)) {
        return {.montant = 0, .critique = false, .esquive = true};
    }
    Portions portions = enPortions(attaque.degats);
    const CritiqueDuCoup critique = critiqueDuCoup(attaque.capaciteOffensive, defenses.statistiques.capaciteDefensive, generateur);
    for (const Element element : s_elements) {
        portions[element] *= critique.multiplicateur;
    }
    // Le bouclier ne retient que le physique : les portions élémentaires passent toujours.
    const ResultatDuBlocage blocage =
        blocageDe(portions[Element::Physique], attaque.nature, defenses.statistiques, blocagePret, generateur);
    portions[Element::Physique] = blocage.montant;
    const int bloque = static_cast<int>(std::lround(blocage.bloque));
    if (blocage.bloque > 0.0f && portions.total() <= 0.0f) {
        // Tout est absorbé (il n'y avait que du physique) : le coup s'arrête, l'armure et son plancher ne s'appliquent pas.
        return {.montant = 0, .critique = critique.critique, .esquive = false, .bloque = bloque};
    }
    portions[Element::Physique] = armureAppliquee(portions[Element::Physique], defenses.armure, attaque.penetration);
    for (const Element element : s_elements) {
        portions[element] = appliquerResistance(portions[element], defenses.statistiques.resistances[element]);
    }
    const Degats subies = arrondir(portions);
    return {.montant = subies.total(), .critique = critique.critique, .esquive = false, .bloque = bloque, .portions = subies};
}

ResultatDuCoup resoudreSoin(const Attaque& attaque, Generateur& generateur) {
    const ResultatCritique critique =
        critiqueDe(attaque.soin, attaque.capaciteOffensive, s_capaciteDefensiveDeReference, generateur);
    return {.montant = critique.montant, .critique = critique.critique};
}
