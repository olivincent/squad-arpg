// ./src/combat/Progression.cpp
#include "combat/Progression.hpp"
#include <cassert>
#include <cstddef>

int experiencePourAtteindre(int niveau) {
    assert(niveau >= 1);
    return s_experienceDeLaCourbe * niveau * (niveau - 1);
}

Caracteristiques croissanceAuNiveau(int niveau, const PaireDeCroissance& paire) {
    assert(niveau >= 1);
    Caracteristiques croissance;
    for (int gagne = 2; gagne <= niveau; ++gagne) {
        croissance += Caracteristiques::seule(paire[static_cast<std::size_t>((gagne - 2) % 2)], 1);
    }
    return croissance;
}

int niveauPour(int experience) {
    assert(experience >= 0);
    int niveau = 1;
    while (experiencePourAtteindre(niveau + 1) <= experience) {
        ++niveau;
    }
    return niveau;
}
