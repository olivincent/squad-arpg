// ./src/combat/EffetActif.hpp
#pragma once
#include <variant>
#include "combat/Element.hpp"
#include "combat/SourceDEffet.hpp"

// Un effet actif (étape 111) : ce qu'un skill a posé sur une entité pour une durée, et qui expire. Buffs, affaiblissements, dégâts dans
// le temps, contrôle, Aegis et Ward seront tous des effets actifs ; chaque sorte est une alternative de `SorteDEffetActif`, et le
// compilateur signale chaque `std::visit` à compléter. Pour l'instant : le modificateur de statistique, les dégâts dans le temps, le
// soin dans le temps (étape 112) et le contrôle (étape 113).
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

// Les états de contrôle qu'un effet actif peut infliger (étape 113). Ce n'est pas `EtatDeControle` (`EtatDeControle.hpp`), qui nomme les
// états dont on a une RÉSISTANCE : l'entrave n'en a pas, elle est réduite par la résistance au ralentissement (`etatQuiResiste`,
// `Controle.hpp`, la seule fonction qui dit quelle résistance joue). Le charme et la peur viendront avec leur étape.
//   Etourdissement : ni déplacement, ni attaque, ni skill (Stun).
//   Ralentissement : la vitesse de déplacement est multipliée par (1 − pourcentage) (Slowed).
//   Entrave        : la vitesse d'attaque de base est multipliée par (1 − pourcentage) (Hampered).
//   Disruption     : aucun skill ne peut être déclenché ; l'attaque de base reste permise (Disrupted).
enum class EtatInflige { Etourdissement, Ralentissement, Entrave, Disruption, Nombre };  // `Nombre` : la sentinelle, comme `Element`

// Un état de contrôle (étape 113) : `etat` pendant la durée de l'effet. `pourcentage` (de 0 à 100) ne sert qu'au ralentissement et à
// l'entrave ; il vaut 0 pour les deux autres. Les règles de lecture (peut-il bouger, attaquer, lancer un skill, à quelle vitesse) sont
// dans `Controle.hpp` ; elles lisent la liste des effets actifs, ce qui donne le cumul : deux étourdissements ou deux Disruptions de
// skills différents durent tant qu'il en reste un, deux ralentissements (deux entraves) gardent le plus fort et non la somme.
struct Controle {
    EtatInflige etat;
    int pourcentage = 0;

    constexpr bool operator==(const Controle&) const = default;
};

using SorteDEffetActif = std::variant<ModificateurDeStatistique, DegatsDansLeTemps, SoinDansLeTemps, Controle>;

struct EffetActif {
    // L'identité de l'effet est (source, rang) : le skill ou la capacité de monstre qui l'a posé et le rang de l'effet dans la liste
    // d'effets de ce skill (`DefinitionDeSkill::effets`, 0 pour le principal). Une source n'a jamais deux effets de même identité sur
    // une entité ; elle peut en avoir deux de rangs différents. Elle ne dépend pas du lanceur : deux lanceurs du même skill n'en font qu'un.
    SourceDEffet source;
    int rang = 0;
    float dureeRestante;     // en secondes ; l'effet disparaît à 0
    SorteDEffetActif sorte;
    // Les piles, communes à toutes les sortes : une nouvelle application du même effet ajoute une pile, jusqu'à `pilesMax` ; avec 1 (le
    // défaut) il n'y a jamais qu'une pile. La valeur de la sorte est celle d'UNE pile : l'effet vaut cette valeur fois `piles`. Toutes
    // les piles partagent la durée. Voir `EffetsActifs::poser`.
    int pilesMax = 1;
    int piles = 1;
};

// Un contrôle (jaune à l'écran) n'est ni un bonus ni un malus de statistique : il a sa propre rangée de marques.
[[nodiscard]] inline bool estUnControle(const EffetActif& effet) {
    return std::holds_alternative<Controle>(effet.sorte);
}

// Un bonus (vert à l'écran) aide l'entité qui le porte ; sinon c'est un malus (rouge). Un modificateur de 0 % est un bonus ; un soin
// dans le temps est un bonus, des dégâts dans le temps et un contrôle des malus (mais la marque d'un contrôle est jaune : `estUnControle`).
[[nodiscard]] inline bool estUnBonus(const EffetActif& effet) {
    struct Visiteur {
        bool operator()(const ModificateurDeStatistique& modificateur) const { return modificateur.pourcentage >= 0; }
        bool operator()(const DegatsDansLeTemps&) const { return false; }
        bool operator()(const SoinDansLeTemps&) const { return true; }
        bool operator()(const Controle&) const { return false; }
    };
    return std::visit(Visiteur{}, effet.sorte);
}
