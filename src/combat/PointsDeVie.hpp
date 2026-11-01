// ./src/combat/PointsDeVie.hpp
#pragma once
#include "combat/Jauge.hpp"

// Les points de vie d'une entité. Invariants, garantis par la classe :
//   - 0 <= valeur <= maximum ;
//   - maximum > 0 ;
//   - une valeur à 0 ne remonte plus (un mort ne se soigne pas : la mort
//     est définitive).
// Commence plein. Le plafond et le plancher à 0 sont ceux de la `Jauge` qu'elle possède ; la mort est sa règle à elle.
class PointsDeVie {
public:
    explicit PointsDeVie(int maximum);
    void subirDegats(int montant);
    // Renvoie les points réellement rendus (plafonnés au maximum ; 0 pour un mort).
    int soigner(int montant);
    // Change le maximum (précondition : `nouveauMaximum > 0`). La valeur actuelle change de la même
    // différence, sans dépasser le nouveau maximum : monter le maximum soigne d'autant, le baisser
    // blesse d'autant. Un vivant garde au moins 1 point de vie (on ne meurt pas en perdant du
    // maximum) ; un mort reste à 0.
    void changerMaximum(int nouveauMaximum);
    [[nodiscard]] bool estVivant() const;
    [[nodiscard]] int getValeur() const;
    [[nodiscard]] int getMaximum() const;
    [[nodiscard]] float getRatio() const;

private:
    Jauge m_jauge;
};
