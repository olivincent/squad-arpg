// ./src/combat/ResultatDEffet.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <variant>
#include "combat/Coup.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/IdSkill.hpp"
#include "combat/Place.hpp"

// Ce qu'un skill (ou une attaque de base) a produit, en données : l'unique canal par lequel l'escouade rend ses résultats à la
// partie, qui en fait des événements de combat. Un `std::visit` par lecteur ; un nouveau résultat est une alternative de plus, et le
// compilateur signale chaque `std::visit` à compléter.
// Les effets sur les personnages ont déjà eu lieu quand le résultat est rendu (le soin est rendu, la provocation est posée) ; les
// dégâts sur un monstre sont à infliger : le monstre n'est pas à l'escouade.

// Le skill vient de partir (fin de son activation), au-dessus de son lanceur ; toujours le premier résultat du skill.
struct SkillParti {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    Place lanceur;
    IdSkill skill;
    sf::Vector2f position;  // le centre du lanceur à ce moment
};

// Un personnage a été soigné : les points réellement rendus (jamais 0), son centre à ce moment et si le soin était un critique.
struct SoinSurAllie {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    Place place;
    int montant;
    sf::Vector2f position;
    bool critique;
};

// Ce monstre vient d'être provoqué (de nouveau, s'il l'était déjà). Pas de position : `Partie` la lit sur le monstre.
struct ProvocationPosee {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    IdMonstre monstre;
};

// Les dégâts sur un monstre sont un `Coup` : une attaque à infliger à ce monstre (l'attaque de base, un skill de dégâts, de frappe
// ou de zone, un coup d'opportunité).
using ResultatDEffet = std::variant<SkillParti, Coup, SoinSurAllie, ProvocationPosee>;
