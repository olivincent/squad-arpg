// ./src/combat/ActionEngagee.hpp
#pragma once
#include "combat/Cible.hpp"
#include "combat/IdSkill.hpp"
#include <optional>

// Ce qu'un personnage est en train de préparer : son attaque de base ou son skill.
enum class TypeAction { AttaqueDeBase, Skill };

// L'action engagée d'un personnage et sa cible. La cible est vide pour une
// provocation. Le personnage la stocke sans l'interpréter : c'est Escouade
// qui la résout (place d'un allié, identifiant d'un monstre, point).
// `skill` est l'identifiant du skill engagé ; vide pour une attaque de base.
struct ActionEngagee {
    TypeAction action;
    std::optional<Cible> cible;
    std::optional<IdSkill> skill;
};
