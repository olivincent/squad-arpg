// ./src/combat/Reserve.hpp
#pragma once
#include "combat/Jauge.hpp"

// Une réserve qui se dépense et se régénère : une `Jauge` de valeurs entières (l'énergie d'un personnage, demain l'Élan de l'escouade)
// et un débit de régénération en points par seconde. Seule la partie entière de ce qui s'est régénéré est ajoutée : le reste est gardé
// (`m_accumule`), sinon un débit de 2 points par seconde à 60 images par seconde ajouterait 0,03 point, tronqué à 0 à chaque image.
// Commence pleine. Ne sait pas qui la possède : celui qui l'a décide si elle se régénère (`regenerer`) et ce qu'elle paie (`payer`).
class Reserve {
public:
    // Préconditions : `maximum` > 0, `regenerationParSeconde` >= 0.
    Reserve(int maximum, float regenerationParSeconde);
    // Fait couler `dt` secondes de régénération. Pleine, elle ne garde aucun reste : la régénération ne se met pas de côté.
    void regenerer(float dt);
    // La réserve contient au moins `cout` points.
    [[nodiscard]] bool suffit(int cout) const;
    // Retire `cout` points. Précondition : `suffit(cout)`.
    void payer(int cout);
    // Voir `Jauge::changerMaximum` (la valeur change de la même différence) et `Jauge::definirMaximum` (elle ne change pas).
    void changerMaximum(int nouveauMaximum);
    void definirMaximum(int nouveauMaximum);
    // Change le débit, en points par seconde. Précondition : `regenerationParSeconde` >= 0.
    void definirRegeneration(float regenerationParSeconde);
    [[nodiscard]] int getValeur() const;
    [[nodiscard]] int getMaximum() const;
    [[nodiscard]] float getRatio() const;
    [[nodiscard]] float getRegeneration() const;

private:
    Jauge m_jauge;
    float m_regeneration;  // en points par seconde
    float m_accumule;      // points régénérés pas encore entiers, 0 <= m_accumule < 1
};
