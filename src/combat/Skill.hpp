// ./src/combat/Skill.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/ModeDeCible.hpp"
#include "combat/TypeDArme.hpp"
#include <optional>
#include <vector>

// Un skill se décrit en données : COMMENT il vise (un mode de cible), OÙ il s'applique (la cible, ou une zone autour d'elle) et
// CE QU'IL FAIT (un effet), plus sa portée, sa recharge (un intervalle [rechargeMin, rechargeMax] en secondes, tiré au hasard à
// chaque déclenchement ; fixe si les deux bornes sont égales, sans aucun tirage) et son temps d'exécution.
//
// La zone est une propriété du skill, séparée de l'effet : un rayon optionnel. Sans rayon, l'effet touche la cible ; avec un
// rayon, il touche chaque entité du camp concerné (`campTouche` : les monstres pour les dégâts et la provocation, les alliés pour
// un soin) dans le cercle de ce rayon centré sur la cible, sur le point visé (mode `Point`) ou sur le lanceur (mode `Soi`).
//
// Machine à états (PhaseSkill) d'un coup à temps d'exécution :
//   Pret --commencer--> EnExecution --avancer (fin de l'exécution)--> EnRecharge
//   EnRecharge --mettreAJour/avancer (recharge écoulée)--> Pret
//   EnExecution --annuler--> Pret   (aucune recharge consommée)
// Le coup part à la fin de l'exécution : c'est à ce moment seulement que la
// recharge est tirée. Avec un temps d'exécution nul, le coup part dès le
// premier `avancer` (Pret -> EnExecution -> EnRecharge).
// Le skill ne sait pas sur qui porte le coup engagé : c'est son propriétaire
// qui le retient.
enum class PhaseSkill { Pret, EnExecution, EnRecharge };

// Les données d'un skill, nommées à la construction (initialisation désignée) : des valeurs de types voisins qu'on ne
// confond plus. Voir `Skill` pour leur sens.
struct DefinitionDeSkill {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    // Comment le skill vise (`ModeDeCible`) : ce qui décide du choix de la cible, du déclenchement, de la mise en attente et de la
    // visée. Il n'est lu que pour un skill de personnage ; l'attaque de base et celle d'un monstre écrivent `Monstre`.
    ModeDeCible mode;
    float portee;        // en pixels, écart bord à bord
    float rechargeMin;   // en secondes
    float rechargeMax;   // en secondes
    // Ce que le skill fait : une liste d'effets, jamais vide. Le premier est l'effet principal (`Skill::getEffet`) : il décide du camp
    // que le skill touche et de l'attaque qui en part. La part de la définition de chaque effet est la sienne : montant, élément,
    // nature, coefficients… n'existent que dans l'alternative qui les lit. Pour un skill de frappe (`EffetFrappe`), les dégâts
    // physiques, la nature et la pénétration sont ceux de l'arme portée.
    std::vector<EffetDeSkill> effets;
    // Vide : l'effet touche la cible. Présent : il touche tout le camp concerné dans ce rayon (en pixels) autour de la cible, du point
    // visé ou du lanceur. Un skill de mode `Point` en a toujours un (sinon il n'aurait rien à toucher).
    std::optional<float> rayonZone = std::nullopt;
    // Combien le rayon de la zone profite du bonus d'aire de l'Intellect du lanceur (`rayonEffectif`) : 0 = pas du tout, 1 = à plein. Sans effet
    // sans rayon de zone. Un sort de zone en a un ; une zone qui n'en est pas un (la pluie de flèches, la provocation) garde 0.
    float coefficientDAire = 0.0f;
    float tempsExecution = 0.0f;  // en secondes, entre l'engagement du coup et son départ
    // Les types d'arme qui permettent de lancer le skill (modèle de *Titan Quest* : pas de restriction par classe) ;
    // vide : aucune exigence. Sans l'une de ces armes en main droite (les mains nues n'en sont pas une), le skill est
    // refusé comme celui d'un lanceur mort.
    std::vector<TypeDArme> armesExigees = {};
    // L'énergie que le skill coûte à son lanceur (étape 108) : vérifiée à l'engagement, payée au départ de l'effet, jamais pour un skill
    // annulé. 0 : gratuit (l'attaque de base et les skills des monstres, qui n'ont pas d'énergie).
    int cout = 0;
};

class Skill {
public:
    explicit Skill(const DefinitionDeSkill& definition);
    void mettreAJour(float dt);
    // Pret -> EnExecution ; faux (sans rien changer) dans toute autre phase.
    [[nodiscard]] bool commencer();
    // Fait avancer l'exécution (EnExecution) ou la recharge (autres phases).
    // Vrai une seule fois, au moment où le coup part : la recharge est alors
    // tirée et la phase passe à EnRecharge.
    [[nodiscard]] bool avancer(float dt, Generateur& generateur);
    // EnExecution -> Pret, sans recharge ; sans effet dans toute autre phase.
    void annuler();
    [[nodiscard]] PhaseSkill getPhase() const;
    [[nodiscard]] bool estAPortee(Cercle origine, Cercle cible) const;
    void relancerRecharge(Generateur& generateur);
    [[nodiscard]] float getProgressionRecharge() const;
    [[nodiscard]] float getRechargeRestante() const;
    [[nodiscard]] ModeDeCible getMode() const;
    // L'effet principal : le premier de la liste.
    [[nodiscard]] const EffetDeSkill& getEffet() const;
    // Tous les effets, le principal d'abord. Un effet secondaire est un modificateur (`EffetModificateur`) qui suit des dégâts ou une
    // frappe et touche les monstres que le coup atteint.
    [[nodiscard]] const std::vector<EffetDeSkill>& getEffets() const;
    // Le rayon de la zone touchée ; vide si l'effet touche la cible seule.
    [[nodiscard]] std::optional<float> getRayonZone() const;
    // Sa part du bonus d'aire du lanceur, de 0 à 1.
    [[nodiscard]] float getCoefficientDAire() const;
    [[nodiscard]] float getPortee() const;
    [[nodiscard]] const std::vector<TypeDArme>& getArmesExigees() const;
    [[nodiscard]] int getCout() const;
    // Cette arme (vide : les mains nues) permet-elle de lancer le skill ? Toujours vrai sans exigence.
    [[nodiscard]] bool convientA(std::optional<TypeDArme> arme) const;

private:
    ModeDeCible m_mode;
    float m_portee;         // en pixels, écart bord à bord
    float m_rechargeMin;    // en secondes
    float m_rechargeMax;    // en secondes
    float m_tempsRecharge;  // en secondes, durée de la recharge en cours (dernier tirage)
    float m_recharge;       // en secondes, avant le prochain déclenchement possible
    std::vector<EffetDeSkill> m_effets;  // jamais vide ; le premier est l'effet principal
    std::optional<float> m_rayonZone;
    float m_coefficientDAire;
    float m_tempsExecution; // en secondes, entre l'engagement du coup et son départ
    std::vector<TypeDArme> m_armesExigees;
    int m_cout;             // en points d'énergie
    float m_executionRestante; // en secondes, lue seulement en phase EnExecution
    PhaseSkill m_phase;

    float tirerRecharge(Generateur& generateur) const;
};
