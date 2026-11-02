// ./src/combat/EffetActif.hpp
#pragma once
#include <variant>
#include "combat/Element.hpp"
#include "combat/IdSkill.hpp"

// Un effet actif (étape 111) : ce qu'un skill a posé sur une entité pour une durée, et qui expire. Buffs, affaiblissements, dégâts dans
// le temps, contrôle, Aegis et Ward seront tous des effets actifs ; chaque sorte est une alternative de `SorteDEffetActif`, et le
// compilateur signale chaque `std::visit` à compléter. Pour l'instant : le modificateur de statistique, les dégâts dans le temps et le
// soin dans le temps (étape 112).
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

// Le temps entre deux ticks d'un effet dans le temps, en secondes : le premier a lieu une seconde après la pose, donc une durée de N
// secondes donne N ticks.
inline constexpr float s_intervalleDeTick = 1.0f;

// Des dégâts dans le temps (étape 112) : `montant` points d'un élément par tick, figé à la pose (les caractéristiques du lanceur à ce
// moment-là ; l'effet continue si le lanceur meurt), ceux d'UNE pile. Un tick ne connaît ni esquive, ni critique, ni armure, ni blocage :
// seule la résistance de l'élément joue, lue chez la cible au moment du tick (`resoudreTick`). Le saignement est du physique dans le temps.
struct DegatsDansLeTemps {
    Element element;
    int montant;
    float avantLeTick = s_intervalleDeTick;  // secondes avant le prochain tick ; un rafraîchissement ne le remet pas à zéro

    constexpr bool operator==(const DegatsDansLeTemps&) const = default;
};

// Un soin dans le temps (étape 112) : `montant` points de vie par tick, ceux d'UNE pile, figés à la pose. Ni critique, ni dépassement du
// maximum de points de vie. Un type distinct des dégâts, pas un montant signé.
struct SoinDansLeTemps {
    int montant;
    float avantLeTick = s_intervalleDeTick;

    constexpr bool operator==(const SoinDansLeTemps&) const = default;
};

using SorteDEffetActif = std::variant<ModificateurDeStatistique, DegatsDansLeTemps, SoinDansLeTemps>;

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

// Un bonus (vert à l'écran) aide l'entité qui le porte ; sinon c'est un malus (rouge). Un modificateur de 0 % est un bonus ; un soin
// dans le temps est un bonus, des dégâts dans le temps un malus.
[[nodiscard]] inline bool estUnBonus(const EffetActif& effet) {
    struct Visiteur {
        bool operator()(const ModificateurDeStatistique& modificateur) const { return modificateur.pourcentage >= 0; }
        bool operator()(const DegatsDansLeTemps&) const { return false; }
        bool operator()(const SoinDansLeTemps&) const { return true; }
    };
    return std::visit(Visiteur{}, effet.sorte);
}
