// ./src/combat/EffetsActifs.hpp
#pragma once
#include <vector>
#include "combat/EffetActif.hpp"
#include "combat/ResolutionDuCoup.hpp"

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
    // Fait couler le temps : chaque durée diminue de `dt`, et l'effet qui atteint 0 disparaît.
    void mettreAJour(float dt);
    // Retire tous les effets (la mort de l'entité).
    void vider();
    [[nodiscard]] const std::vector<EffetActif>& liste() const;
    // Combien d'effets actifs sont des bonus (`estUnBonus`), combien sont des malus : une marque verte ou rouge chacun, à l'écran.
    [[nodiscard]] int nombreDeBonus() const;
    [[nodiscard]] int nombreDeMalus() const;

private:
    std::vector<EffetActif> m_effets;
};

// Les défenses une fois les effets pris en compte, quelle que soit leur source (personnage ou monstre) : la couche des effets s'applique
// après les caractéristiques et l'équipement. Les pourcentages de tous les effets sur une même statistique s'additionnent, puis
// s'appliquent une seule fois : `statistique × (100 + somme) / 100`, arrondie au plus proche, jamais sous 0. Seul endroit qui lit les
// effets pour changer une statistique.
[[nodiscard]] Defenses appliquerLesEffets(const Defenses& defenses, const EffetsActifs& effets);
