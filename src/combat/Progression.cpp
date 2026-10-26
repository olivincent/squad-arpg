// ./src/combat/Progression.cpp
#include "combat/Progression.hpp"
#include <cassert>

int experiencePourAtteindre(int niveau) {
    assert(niveau >= 1);
    return s_experienceDeLaCourbe * niveau * (niveau - 1);
}

int niveauPour(int experience) {
    assert(experience >= 0);
    int niveau = 1;
    while (experiencePourAtteindre(niveau + 1) <= experience) {
        ++niveau;
    }
    return niveau;
}
