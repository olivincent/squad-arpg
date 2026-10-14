// ./src/PointsDeVie.hpp
#pragma once

// Les points de vie d'une entité. Invariants, garantis par la classe :
//   - 0 <= valeur <= maximum ;
//   - maximum > 0 ;
//   - une valeur à 0 ne remonte plus (un mort ne se soigne pas : la mort
//     est définitive).
// Commence plein. Ne dépend d'aucun autre module du projet.
class PointsDeVie {
public:
    explicit PointsDeVie(int maximum);
    void subirDegats(int montant);
    void soigner(int montant);
    [[nodiscard]] bool estVivant() const;
    int getValeur() const;
    int getMaximum() const;
    float getRatio() const;

private:
    int m_valeur;
    int m_maximum;
};
