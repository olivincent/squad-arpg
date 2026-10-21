// ./src/combat/Vitalite.hpp
#pragma once
#include "combat/PointsDeVie.hpp"

// La vitalité d'une entité, commune aux deux camps : ses points de vie et son armure. Les règles
// propres à un camp (invulnérabilité du monstre qui rentre à son poste, régénération…) restent
// dans la classe de l'entité.
class Vitalite {
public:
    // Commence pleine. Préconditions : pointsDeVieMax > 0, armure >= 0.
    Vitalite(int pointsDeVieMax, int armure);
    // Remplace l'armure (celle d'un personnage est recalculée depuis son équipement). Précondition : armure >= 0.
    void definirArmure(int armure);
    // Applique l'armure (`appliquerArmure`) puis retire les points de vie. Renvoie les dégâts après
    // armure, non plafonnés par les points de vie restants ; 0 pour un mort.
    int recevoirDegats(int montant);
    // Renvoie les points réellement rendus (plafonnés au maximum ; 0 pour un mort).
    int soigner(int montant);
    [[nodiscard]] bool estVivant() const;
    [[nodiscard]] int getValeur() const;
    [[nodiscard]] int getMaximum() const;
    [[nodiscard]] float getRatio() const;

private:
    PointsDeVie m_pointsDeVie;
    int m_armure;
};
