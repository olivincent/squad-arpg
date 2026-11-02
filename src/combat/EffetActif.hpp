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
// effets actifs sur une même statistique s'additionnent (chacun fois ses piles), puis s'appliquent une seule fois.
struct ModificateurDeStatistique {
    StatistiqueModifiable statistique;
    int pourcentage;

    constexpr bool operator==(const ModificateurDeStatistique&) const = default;
};

using SorteDEffetActif = std::variant<ModificateurDeStatistique>;

struct EffetActif {
    // L'identité de l'effet est (source, rang) : le skill qui l'a posé et le rang de l'effet dans la liste d'effets de ce skill
    // (`DefinitionDeSkill::effets`, 0 pour le principal). Un skill n'a jamais deux effets de même identité sur une entité ; il peut en
    // avoir deux de rangs différents. Elle ne dépend pas du lanceur : deux lanceurs du même skill n'en font qu'un.
    IdSkill source;
    int rang = 0;
    float dureeRestante;     // en secondes ; l'effet disparaît à 0
    SorteDEffetActif sorte;
    // Les piles, communes à toutes les sortes : une nouvelle application du même effet ajoute une pile, jusqu'à `pilesMax` ; avec 1 (le
    // défaut) il n'y a jamais qu'une pile. La valeur de la sorte est celle d'UNE pile : l'effet vaut cette valeur fois `piles`. Toutes
    // les piles partagent la durée. Voir `EffetsActifs::poser`.
    int pilesMax = 1;
    int piles = 1;
};

// Un bonus (vert à l'écran) aide l'entité qui le porte ; sinon c'est un malus (rouge). Un modificateur de 0 % est un bonus.
[[nodiscard]] inline bool estUnBonus(const EffetActif& effet) {
    return std::visit([](const ModificateurDeStatistique& modificateur) { return modificateur.pourcentage >= 0; }, effet.sorte);
}
