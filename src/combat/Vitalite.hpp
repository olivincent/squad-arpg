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
    // Commence pleine. Préconditions : pointsDeVieMax > 0, defenses.armure >= 0, defenses.statistiques.capaciteDefensive > 0.
    Vitalite(int pointsDeVieMax, const Defenses& defenses);
    // Remplace les défenses (celles d'un personnage sont recalculées depuis son équipement et ses
    // caractéristiques). Mêmes préconditions.
    void definirDefenses(const Defenses& defenses);
    // Résout le coup (`resoudreCoup`, qui tire dans `generateur`) puis retire les points de vie. Le
    // résultat porte les dégâts après armure, non plafonnés par les points de vie restants ; rien (0, sans
    // tirage) pour un mort. Un blocage réussi lance la récupération du blocage (le temps pendant lequel le
    // bouclier ne bloque plus), qui est un état de la vitalité : elle s'écoule par `mettreAJour`.
    ResultatDuCoup recevoirDegats(const Attaque& attaque, Generateur& generateur);
    // Fait couler le temps : la récupération du blocage diminue de `dt`, sans descendre sous 0.
    void mettreAJour(float dt);
    // Le blocage n'est pas en récupération : un blocage est tenté au prochain coup.
    [[nodiscard]] bool blocagePret() const;
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
    float m_recuperationDuBlocage = 0.0f;  // en secondes ; 0 : le blocage est prêt
};
