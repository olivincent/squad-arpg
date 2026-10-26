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

int pointsLibresRecus(int niveau) {
    assert(niveau >= 1);
    return s_pointsLibresParNiveau * (niveau - 1);
}

int pointsLibresDisponibles(int niveau, const Caracteristiques& depenses) {
    return pointsLibresRecus(niveau) - depenses.somme();
}

bool depensePermise(int niveau, const Caracteristiques& depenses, const Caracteristiques& depense) {
    for (const Caracteristique caracteristique : s_caracteristiques) {
        if (depense.valeur(caracteristique) < 0) {
            return false;
        }
    }
    return depense.somme() <= pointsLibresDisponibles(niveau, depenses);
}

float progressionVersLeNiveauSuivant(int experience) {
    const int niveau = niveauPour(experience);
    const int debut = experiencePourAtteindre(niveau);
    const int fin = experiencePourAtteindre(niveau + 1);
    return static_cast<float>(experience - debut) / static_cast<float>(fin - debut);
}

int experienceManquante(int experience) {
    return experiencePourAtteindre(niveauPour(experience) + 1) - experience;
}

int niveauPour(int experience) {
    assert(experience >= 0);
    int niveau = 1;
    while (experiencePourAtteindre(niveau + 1) <= experience) {
        ++niveau;
    }
    return niveau;
}
