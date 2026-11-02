// ./src/combat/EffetActif.hpp
#pragma once
#include <variant>
#include "combat/IdSkill.hpp"

// Un effet actif (étape 111) : ce qu'un skill a posé sur une entité pour une durée, et qui expire. Buffs, affaiblissements, dégâts dans
// le temps, contrôle, Aegis et Ward seront tous des effets actifs ; chaque sorte est une alternative de `SorteDEffetActif`, et le
// compilateur signale chaque `std::visit` à compléter. Pour l'instant : le modificateur de statistique.
//
// À ne pas confondre avec `EffetDeSkill` (`EffetDeSkill.hpp`) : celui-là est la donnée d'un skill, ce qu'il fait quand il part ; l'effet
// actif est ce qui reste sur l'entité touchée, avec son temps restant. Les `Effet*` de skill qui posent un effet actif en sont la source.

// Les statistiques qu'un modificateur peut changer. Ajouter une statistique : une valeur ici et un cas dans `appliquerLesEffets`
// (`EffetsActifs.hpp`), le seul endroit qui les lit.
enum class StatistiqueModifiable { Armure };

// Un pourcentage entier ajouté à une statistique : positif pour un buff, négatif pour un affaiblissement. Les pourcentages de tous les
// effets actifs sur une même statistique s'additionnent, puis s'appliquent une seule fois.
struct ModificateurDeStatistique {
    StatistiqueModifiable statistique;
    int pourcentage;

    constexpr bool operator==(const ModificateurDeStatistique&) const = default;
};

using SorteDEffetActif = std::variant<ModificateurDeStatistique>;

struct EffetActif {
    IdSkill source;          // l'identité de l'effet : le skill qui l'a posé (jamais deux effets du même skill sur une entité)
    float dureeRestante;     // en secondes ; l'effet disparaît à 0
    SorteDEffetActif sorte;
};
