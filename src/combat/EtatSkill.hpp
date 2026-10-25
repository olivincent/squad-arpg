// ./src/combat/EtatSkill.hpp
#pragma once
#include "combat/IdSkill.hpp"
#include "combat/TypeSkill.hpp"

// Le skill d'une place tel que l'affichage en a besoin.
struct EtatSkill {
    IdSkill id;           // sert à l'étiquette de l'icône
    TypeSkill type;
    float progression;    // de 0 (juste lancé) à 1 (prêt)
    float tempsRestant;   // en secondes
    bool lanceurVivant;
    bool armeAbsente;     // le skill exige un type d'arme que le lanceur ne porte pas : refusé, icône grisée
    bool enActivation;    // le lanceur prépare le skill (il n'est pas encore parti)
};
