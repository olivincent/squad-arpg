// ./src/EtatSkill.hpp
#pragma once
#include "Skill.hpp"

// Le skill d'une place tel que l'affichage en a besoin.
struct EtatSkill {
    TypeSkill type;
    float progression;    // de 0 (juste lancé) à 1 (prêt)
    float tempsRestant;   // en secondes
    bool lanceurVivant;
};
