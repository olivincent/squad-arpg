// ./src/combat/EffetsActifs.hpp
#pragma once
#include <vector>
#include "combat/EffetActif.hpp"
#include "combat/ResolutionDuCoup.hpp"

// Les effets actifs d'une entité (étape 111), dans l'ordre où ils ont été posés. Possédés par `Vitalite`, donc par les personnages comme
// par les monstres.
//
// Cumul : un skill n'a jamais deux effets actifs sur une même entité (l'identité d'un effet est son skill, quel que soit le lanceur).
// Reposer le même skill rafraîchit la durée (la plus longue des deux : un rafraîchissement ne raccourcit jamais) et garde la valeur
// la plus forte ; deux skills différents se cumulent. Les piles d'intensité n'existent pas.
class EffetsActifs {
public:
    // Ajoute l'effet, ou rafraîchit celui du même skill. Précondition : dureeRestante > 0.
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
