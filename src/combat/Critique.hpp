// ./src/combat/Critique.hpp
#pragma once

// Le critique (`docs/classes-et-combat.md`, sections 6 et 7) : sa chance et sa force viennent du rapport entre
// la capacité offensive de l'attaquant et la capacité défensive du défenseur, jamais d'un pourcentage fixe.
// Que des fonctions pures ; le tirage est fait ailleurs (`resoudreCoup`) et reçu en paramètre.

// Chance de critique = `s_chanceParRapportCarre` × rapport², plafonnée à `s_chanceMaximale`. Valeurs à tester.
inline constexpr float s_chanceParRapportCarre = 0.05f;
inline constexpr float s_chanceMaximale = 0.50f;
// Multiplicateur = `s_multiplicateurDeBase` + `s_multiplicateurParRapport` × (rapport − 1), borné à
// [`s_multiplicateurDeBase`, `s_multiplicateurMaximal`]. Valeurs à tester.
inline constexpr float s_multiplicateurDeBase = 1.5f;
inline constexpr float s_multiplicateurParRapport = 0.25f;
inline constexpr float s_multiplicateurMaximal = 2.0f;

// Le rapport des capacités. Précondition : `capaciteDefensive > 0`.
[[nodiscard]] float rapportDeCapacites(int capaciteOffensive, int capaciteDefensive);

// La chance (de 0 à `s_chanceMaximale`) et le multiplicateur d'un critique, pour ce rapport (>= 0).
[[nodiscard]] float chanceDeCritique(float rapport);
[[nodiscard]] float multiplicateurDeCritique(float rapport);

struct ResultatCritique {
    int montant;     // multiplié, arrondi au plus proche, si critique ; le montant d'entrée sinon
    bool critique;
};

// L'étape « critique » du coup : `tirage` (de 0 inclus à 1 exclu) sous la chance du rapport, c'est un critique.
// Précondition : montant >= 0.
[[nodiscard]] ResultatCritique appliquerCritique(int montant, float rapport, float tirage);
