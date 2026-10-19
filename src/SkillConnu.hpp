// ./src/SkillConnu.hpp
#pragma once
#include "IdSkill.hpp"
#include "Skill.hpp"

// Un skill qu'un personnage connaît, avec son identifiant. `Skill` reste
// générique (il sert aussi d'attaque de base) et ne porte pas d'identifiant.
struct SkillConnu {
    IdSkill id;
    Skill skill;
};
