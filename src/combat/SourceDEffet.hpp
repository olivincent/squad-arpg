// ./src/combat/SourceDEffet.hpp
#pragma once
#include <variant>
#include "combat/IdSkill.hpp"

// Les capacités des monstres qui posent un effet actif (étape 114) : leur identifiant stable, comme `IdSkill` pour les skills d'un
// personnage. Une capacité de monstre n'est pas un skill (ni barre, ni recharge de personnage, ni énergie) : elle n'a pas de valeur dans
// `IdSkill`. Ajouter une capacité : une valeur ici, son nom dans `AffichageSkills`.
enum class IdCapacite {
    CoupAssommant,   // l'infanterie lourde (étape 113) : l'étourdissement qu'elle pose
    WardDuSoigneur   // le soigneur (étape 114) : le Ward qu'il pose sur un allié de son groupe
};

// D'où vient un effet actif : un skill OU une capacité de monstre. Avec le rang, c'est l'identité de l'effet (`EffetActif`) : deux
// sources différentes ne se confondent jamais, même si leurs valeurs numériques étaient égales.
struct SourceDEffet {
    std::variant<IdSkill, IdCapacite> valeur;

    // Implicites : `{.source = IdSkill::Soin}` et `{.source = IdCapacite::CoupAssommant}` s'écrivent tels quels.
    SourceDEffet(IdSkill skill) : valeur(skill) {}
    SourceDEffet(IdCapacite capacite) : valeur(capacite) {}

    friend bool operator==(const SourceDEffet&, const SourceDEffet&) = default;
};
