// ./src/combat/EffetsActifs.hpp
#pragma once
#include <variant>
#include <vector>
#include "combat/EffetActif.hpp"
#include "combat/Element.hpp"
#include "combat/RendementsDeControle.hpp"
#include "combat/ResolutionDuCoup.hpp"

// Un tick qui vient d'avoir lieu (étape 112) : des dégâts d'un élément ou un soin, du montant d'UNE pile fois ses piles. Au sortir d'
// `EffetsActifs::mettreAJour` c'est le montant avant réduction ; `Vitalite::prendreLesTicks` rend les mêmes types avec ce qui a vraiment été
// subi ou rendu.
struct TickDeDegats {
    Element element;
    int montant;
};
struct TickDeSoin {
    int montant;
};
using Tick = std::variant<TickDeDegats, TickDeSoin>;

// Les effets actifs d'une entité (étape 111), dans l'ordre où ils ont été posés. Possédés par `Vitalite`, donc par les personnages comme
// par les monstres.
//
// Cumul : un skill n'a jamais deux effets actifs de même identité sur une même entité (l'identité d'un effet est son skill et son rang
// dans la liste d'effets du skill, quel que soit le lanceur ; `EffetActif`).
// Reposer le même effet rafraîchit la durée (la plus longue des deux : un rafraîchissement ne raccourcit jamais) et garde la valeur la plus
// forte ; deux effets d'identités différentes se cumulent. Piles (étape 112) : chaque effet déclare un maximum de piles (`EffetActif::pilesMax`,
// 1 par défaut). Avec 1, rien d'autre ne change. Avec plus, chaque nouvelle application ajoute une pile jusqu'au maximum, en plus de rafraîchir
// la durée (partagée par toutes les piles) ; la valeur par pile est la plus forte des applications, et l'effet vaut cette valeur fois les piles.
class EffetsActifs {
public:
    // Ajoute l'effet, ou rafraîchit celui de même identité (et lui ajoute une pile). Préconditions : dureeRestante > 0, 1 <= piles <= pilesMax.
    void poser(const EffetActif& effet);
    // Fait couler le temps : chaque durée diminue de `dt`, et l'effet qui atteint 0 disparaît ; le rendement décroissant du contrôle
    // s'écoule aussi. Rend les ticks des effets dans le temps qui
    // ont eu lieu pendant ce pas, effet par effet dans l'ordre où ils ont été posés, tous les `s_intervalleDeTick` : le premier une seconde
    // après la pose, jamais après la fin de l'effet (une durée de N secondes donne N ticks, même quand l'imprécision des flottants fait
    // atteindre 0 à la durée un instant avant le dernier tick). Un tick n'a lieu qu'une fois : l'effet qui disparaît a rendu les siens.
    std::vector<Tick> mettreAJour(float dt);
    // Retire tous les effets (la mort de l'entité), et remet à zéro le rendement décroissant du contrôle.
    void vider();
    // Compte une application de cet état de contrôle et rend le facteur de sa durée (1, 0,5, 0,25, puis 0 : l'immunité ;
    // `RendementsDeControle::appliquer`). Le compteur vit ici, avec les effets : il s'écoule avec eux (`mettreAJour`) et disparaît avec eux.
    [[nodiscard]] float compterUneApplicationDeControle(EtatInflige etat);
    [[nodiscard]] const RendementsDeControle& rendements() const;
    [[nodiscard]] const std::vector<EffetActif>& liste() const;
    // Les points d'absorption que ces effets donnent, par famille (tous les boucliers de la famille additionnés).
    [[nodiscard]] PointsDAbsorption pointsDAbsorption() const;
    // Retire `points` aux boucliers de cette famille : celui qui expire le premier d'abord (la durée restante la plus courte ; à égalité,
    // le plus anciennement posé), puis le suivant. Un bouclier tombé à 0 disparaît. Précondition : `points` ne dépasse pas ce que la
    // famille a (`pointsDAbsorption`).
    void consommerLAbsorption(FamilleDAbsorption famille, int points);
    // Les piles de chaque effet actif qui est un bonus (`estUnBonus`), puis de chaque malus, dans l'ordre où ils ont été posés : une marque
    // verte ou rouge par élément à l'écran, suivie du nombre de piles quand il dépasse 1.
    // Les malus qui ne sont pas des contrôles (un contrôle a sa rangée jaune : `nombreDeControles`).
    [[nodiscard]] std::vector<int> pilesDesBonus() const;
    [[nodiscard]] std::vector<int> pilesDesMalus() const;
    // Combien d'effets de contrôle sont actifs : une marque jaune chacun.
    [[nodiscard]] int nombreDeControles() const;

private:
    std::vector<EffetActif> m_effets;
    RendementsDeControle m_rendements;
};

// Les défenses une fois les effets pris en compte, quelle que soit leur source (personnage ou monstre) : la couche des effets s'applique
// après les caractéristiques et l'équipement. Les pourcentages de tous les effets sur une même statistique s'additionnent, puis
// s'appliquent une seule fois : `statistique × (100 + somme) / 100`, arrondie au plus proche, jamais sous 0. Seul endroit qui lit les
// effets pour changer une statistique. Il remplit aussi `Defenses::absorption` (étape 114), les points d'absorption que ces effets donnent.
[[nodiscard]] Defenses appliquerLesEffets(const Defenses& defenses, const EffetsActifs& effets);
