// ./src/jeu/Expedition.cpp
#include "jeu/Expedition.hpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include "escouade/Escouade.hpp"

namespace {
// Les constantes du mélangeur « splitmix64 » : un mélange de bits connu, qui fait d'entiers voisins des entiers sans rapport.
constexpr std::uint64_t s_pasDeProfondeur = 0x9E3779B97F4A7C15ULL;
constexpr std::uint64_t s_multiplicateurA = 0xBF58476D1CE4E5B9ULL;
constexpr std::uint64_t s_multiplicateurB = 0x94D049BB133111EBULL;
constexpr unsigned int s_decalageA = 30;
constexpr unsigned int s_decalageB = 27;
constexpr unsigned int s_decalageC = 31;
}  // namespace

Expedition nouvelleExpedition(std::vector<DefinitionPersonnage> composition, Generateur::result_type graine) {
    assert(!composition.empty());
    Expedition expedition{.profondeur = 1, .graine = graine, .membres = {}, .sac = Inventaire(Escouade::s_capaciteDuSac)};
    for (DefinitionPersonnage& definition : composition) {
        Equipement equipement(definition.equipementDeDepart);
        definition.equipementDeDepart.clear();
        expedition.membres.push_back({std::move(definition), equipement});
    }
    return expedition;
}

std::vector<MembreDEscouade> compositionDeLExpedition(const Expedition& expedition) {
    std::vector<MembreDEscouade> membres;
    membres.reserve(expedition.membres.size());
    for (const MembreDExpedition& membre : expedition.membres) {
        membres.push_back({membre.definition, membre.equipement});
    }
    return membres;
}

Generateur::result_type graineDEtage(Generateur::result_type graineDExpedition, int profondeur) {
    std::uint64_t x = static_cast<std::uint64_t>(graineDExpedition) + static_cast<std::uint64_t>(profondeur) * s_pasDeProfondeur;
    x = (x ^ (x >> s_decalageA)) * s_multiplicateurA;
    x = (x ^ (x >> s_decalageB)) * s_multiplicateurB;
    x ^= x >> s_decalageC;
    return static_cast<Generateur::result_type>(x);
}

Expedition etageSuivant(Expedition expedition) {
    assert(expedition.profondeur < s_profondeurMaximale);
    ++expedition.profondeur;
    return expedition;
}
