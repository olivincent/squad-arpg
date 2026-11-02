// ./src/combat/Vitalite.hpp
#pragma once
#include <vector>
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/Controle.hpp"
#include "combat/EffetActif.hpp"
#include "combat/EffetsActifs.hpp"
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
    // Résout le coup (`resoudreCoup`, qui tire dans `generateur`, contre les défenses que les effets actifs ont modifiées :
    // `appliquerLesEffets`) puis retire les points de vie. Si l'entité en meurt, ses effets actifs disparaissent ; sinon, et si le coup
    // n'est pas esquivé, elle reçoit ceux de l'attaque (`Attaque::effets`), après le coup : ils ne jouent qu'au suivant. Le
    // résultat porte les dégâts après armure, non plafonnés par les points de vie restants ; rien (0, sans
    // tirage) pour un mort. Un blocage réussi lance la récupération du blocage (le temps pendant lequel le
    // bouclier ne bloque plus), qui est un état de la vitalité : elle s'écoule par `mettreAJour`.
    ResultatDuCoup recevoirDegats(const Attaque& attaque, Generateur& generateur);
    // Fait couler le temps : la récupération du blocage diminue de `dt`, sans descendre sous 0, et les effets actifs s'écoulent. Les ticks
    // des effets dans le temps qui ont lieu sont appliqués tout de suite, dans l'ordre : les dégâts par `resoudreTick` contre les
    // résistances d'à présent, puis retirés des points de vie ; le soin est rendu (plafonné au maximum). Une entité qui en meurt perd ses
    // effets actifs, et les ticks suivants n'ont pas lieu. Sans effet sur un mort.
    void mettreAJour(float dt);
    // Ce que les ticks ont fait depuis le dernier appel, dans l'ordre, puis les oublie : des dégâts subis (après la résistance, non
    // plafonnés par les points de vie restants) ou des points de vie rendus (réellement, jamais 0). Les ticks sans effet (montant nul)
    // n'y figurent pas. `Partie` en fait des événements, une fois par image.
    [[nodiscard]] std::vector<Tick> prendreLesTicks();
    // Pose un effet actif (`EffetsActifs::poser` : le même skill rafraîchit, deux skills se cumulent). Sans effet sur un mort.
    // Un contrôle (étape 113) passe d'abord par la cible : son application est comptée pour le rendement décroissant (100 %, 50 %,
    // 25 %, puis l'immunité : rien n'est posé), et sa durée devient `dureePosee` avec la résistance de la cible à cet état, lue à la
    // pose. Chaque application est notée (`prendreLesControles`).
    void poserEffet(const EffetActif& effet);
    // Les contrôles que les poses ont donnés depuis le dernier appel, dans l'ordre, puis les oublie. `Partie` en fait des événements.
    [[nodiscard]] std::vector<ControleApplique> prendreLesControles();
    // Retire tous les effets actifs (un monstre qui rentre à son poste, invulnérable, n'en garde aucun).
    void viderLesEffets();
    [[nodiscard]] const EffetsActifs& getEffets() const;
    // L'armure qui joue contre le prochain coup : celle des défenses, effets actifs compris (`appliquerLesEffets`).
    [[nodiscard]] int getArmure() const;
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
    Defenses m_defenses;  // sans les effets actifs : ce que donnent les caractéristiques et l'équipement (ou la définition)
    EffetsActifs m_effets;
    std::vector<Tick> m_ticksAppliques;  // ce que les ticks ont fait depuis `prendreLesTicks`
    std::vector<ControleApplique> m_controlesAppliques;  // ce que les poses de contrôle ont donné depuis `prendreLesControles`
    float m_recuperationDuBlocage = 0.0f;  // en secondes ; 0 : le blocage est prêt
};
