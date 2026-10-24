// ./src/combat/Vitalite.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/PointsDeVie.hpp"
#include "combat/ResolutionDuCoup.hpp"

// La vitalité d'une entité, commune aux deux camps : ses points de vie et ses défenses (ce qu'elle oppose
// à un coup, `Defenses`). Les règles
// propres à un camp (invulnérabilité du monstre qui rentre à son poste, régénération…) restent
// dans la classe de l'entité.
class Vitalite {
public:
    // Commence pleine. Préconditions : pointsDeVieMax > 0, defenses.armure >= 0, defenses.capaciteDefensive > 0.
    Vitalite(int pointsDeVieMax, const Defenses& defenses);
    // Remplace les défenses (celles d'un personnage sont recalculées depuis son équipement et ses
    // caractéristiques). Mêmes préconditions.
    void definirDefenses(const Defenses& defenses);
    // Résout le coup (`resoudreCoup`, qui tire dans `generateur`) puis retire les points de vie. Le
    // résultat porte les dégâts après armure, non plafonnés par les points de vie restants ; rien (0, sans
    // tirage) pour un mort.
    ResultatDuCoup recevoirDegats(const Attaque& attaque, Generateur& generateur);
    // Change le maximum des points de vie (`PointsDeVie::changerMaximum`). Précondition : `maximum > 0`.
    void definirMaximum(int maximum);
    // Renvoie les points réellement rendus (plafonnés au maximum ; 0 pour un mort).
    int soigner(int montant);
    [[nodiscard]] bool estVivant() const;
    [[nodiscard]] int getValeur() const;
    [[nodiscard]] int getMaximum() const;
    [[nodiscard]] float getRatio() const;

private:
    PointsDeVie m_pointsDeVie;
    Defenses m_defenses;
};
