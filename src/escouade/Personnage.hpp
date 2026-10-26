// ./src/escouade/Personnage.hpp
#pragma once
#include "combat/ActionEngagee.hpp"
#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/EtatSkill.hpp"
#include "combat/IdSkill.hpp"
#include "monde/Mobile.hpp"
#include "combat/Skill.hpp"
#include "combat/SkillConnu.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "combat/Vitalite.hpp"
#include "escouade/Composition.hpp"
#include "objets/Equipement.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

class TileMap;

class Personnage {
public:
    // Construit depuis sa définition (`DefinitionPersonnage`) : `rayonVision` est jusqu'où il voit, en pixels
    // (l'union des champs des vivants est la vision de l'escouade) ; `skills` ceux qu'il connaît (assert :
    // pas deux fois le même identifiant) ; `equipement` ce qu'il porte à sa création : son armure,
    // son attaque de base (celle de son arme, les mains nues sans arme) et la zone qu'il engage autour de lui
    // (celle d'une attaque de contact, aucune pour un projectile) en sont calculées. Son `rang` n'est lu que par
    // la formation.
    // `niveau` (>= 1) est celui de l'escouade : sa base est la classe plus la croissance de ce niveau, et ses points de vie en
    // dépendent (`pointsDeVieMax`).
    Personnage(sf::Vector2f centre, DefinitionPersonnage definition, Equipement equipement, int niveau);
    void dessiner(sf::RenderTarget& rendu) const;
    // `obstacles` le bloquent ; `aContourner` est ce que son chemin évite : la ligne droite n'est
    // prise que si elle passe loin d'eux, sinon A* avec coût (voir Mobile::allerVers).
    void mettreAJour(float dt, sf::Vector2f cible, const TileMap& carte, const std::vector<Cercle>& obstacles,
                     const std::vector<Cercle>& aContourner);
    // Pousse le personnage vivant de `decalage` (murs et obstacles respectés), sans
    // changer son chemin ; sans effet s'il est mort.
    void decaler(sf::Vector2f decalage, const TileMap& carte, const std::vector<Cercle>& obstacles);
    [[nodiscard]] bool estAPortee(Cercle cible) const;
    // Portée de l'attaque de base, bord à bord.
    [[nodiscard]] float getPorteeAttaqueDeBase() const;
    // Son attaque de base : les dégâts (caractéristiques comprises, `montantEffectif`), avant l'armure de la
    // cible, et sa capacité offensive.
    [[nodiscard]] Attaque getAttaqueDeBase() const;
    // Zone de contrôle, en pixels, bord à bord ; 0 : il n'engage personne. Celle de son attaque de base, donc de son arme.
    [[nodiscard]] float getZoneDeControle() const;
    // Attaque de base en trois temps : engager (vivant, immobile, attaque
    // prête) ; avancer l'exécution, qui rend l'attaque au moment où le coup
    // part (vide avant) ; annuler, sans recharge consommée.
    // Refuse tant qu'une arme changée n'a pas relancé la recharge (`relancerSiLArmeAChange`).
    [[nodiscard]] bool engagerAttaque(int idCible);
    // Quand l'arme a changé (`poserEquipement` / `retirerEquipement`), l'attaque de base repart pour une recharge
    // complète, tirée dans l'intervalle de la nouvelle arme : sans cela, retirer et remettre une arme lente (partie
    // figée) donnerait une attaque prête tout de suite. Le tirage attend un générateur, que l'équipement n'a pas :
    // `Escouade::attaquer` l'appelle au début de chaque image. Sans effet si l'arme n'a pas changé.
    void relancerSiLArmeAChange(Generateur& generateur);
    [[nodiscard]] std::optional<int> cibleEngagee() const;
    [[nodiscard]] std::optional<Attaque> avancerAttaque(float dt, Generateur& generateur);
    // Sans effet sur une activation de skill : seul annulerSkill l'annule.
    void annulerAttaque();
    // Voir Mobile::estEnMouvement.
    [[nodiscard]] bool estEnMouvement() const;
    // Façade de Mobile::cheminRestant, pour le débogage.
    [[nodiscard]] std::vector<sf::Vector2f> cheminRestant() const;
    // Un coup de base en exécution ou un skill en activation : le personnage reste sur place.
    [[nodiscard]] bool estEnExecution() const;
    // Les questions sur un skill se posent par identifiant. Un identifiant que ce
    // personnage ne connaît pas se comporte comme « pas de skill » : vide, faux ou rayon 0.
    [[nodiscard]] std::optional<TypeSkill> typeSkill(IdSkill id) const;
    // Identifiants des skills qu'il connaît, dans l'ordre de sa liste.
    [[nodiscard]] std::vector<IdSkill> idsDesSkills() const;
    // L'arme portée permet-elle de lancer ce skill ? Faux pour un skill que le personnage ne connaît pas.
    [[nodiscard]] bool armeConvientAuSkill(IdSkill id) const;
    // Prêt : le lanceur est vivant, connaît le skill, qui n'est pas en recharge, et porte l'arme qu'il exige.
    [[nodiscard]] bool skillEstPret(IdSkill id) const;
    [[nodiscard]] std::optional<EtatSkill> etatDuSkill(IdSkill id) const;
    [[nodiscard]] bool skillEstAPortee(IdSkill id, Cercle cible) const;
    [[nodiscard]] Cercle zoneDuSkill(IdSkill id) const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill(IdSkill id) const;
    // Skill en trois temps, comme l'attaque de base : engager (vivant, skill
    // prêt, aucun autre skill en activation ; annule le coup de base en
    // exécution, mais ne demande pas d'immobilité) ; avancer l'activation, qui
    // rend l'attaque (dégâts, soin ou durée de provocation selon le type du skill, et capacité offensive) au moment
    // où l'effet part, vide avant (et relance la recharge de l'attaque de base) ; annuler, sans recharge consommée. Une seule action
    // engagée à la fois ; les recharges des skills sont indépendantes. La portée
    // et la vue sont vérifiées par Escouade ; la cible est stockée sans être interprétée.
    [[nodiscard]] bool engagerSkill(IdSkill id, std::optional<Cible> cible);
    // Les conditions d'engagerSkill : vivant, skill connu et prêt, arme exigée portée, aucun skill en
    // activation (le lanceur est libre).
    [[nodiscard]] bool peutEngagerSkill(IdSkill id) const;
    [[nodiscard]] std::optional<ActionEngagee> skillEngage() const;
    [[nodiscard]] std::optional<Attaque> avancerSkill(float dt, Generateur& generateur);
    void annulerSkill();
    // `recevoirDegats` rend le résultat du coup (`resoudreCoup`, qui tire dans `generateur`) : dégâts après
    // armure, rien pour un mort. `recevoirSoin` rend les points de vie réellement rendus (plafonnés au
    // maximum, 0 pour un mort).
    ResultatDuCoup recevoirDegats(const Attaque& attaque, Generateur& generateur);
    int recevoirSoin(int quantite);
    [[nodiscard]] bool estVivant() const;
    [[nodiscard]] int getPointsDeVie() const;
    [[nodiscard]] int getPointsDeVieMax() const;
    [[nodiscard]] sf::Vector2f getCentre() const;
    [[nodiscard]] Cercle getCercle() const;
    // Rayon de vision, en pixels.
    [[nodiscard]] float getRayonVision() const;
    [[nodiscard]] sf::Color getCouleur() const;
    [[nodiscard]] const Equipement& getEquipement() const;
    // Ce qui a un effet (étape 92.2) : l'équipement sans ses objets inactifs, dont le seuil n'est plus atteint. Son armure
    // et ses bonus sont ceux du personnage.
    [[nodiscard]] const Equipement& getEquipementActif() const;
    // Ses caractéristiques effectives : sa base plus les bonus de son équipement.
    [[nodiscard]] const Caracteristiques& getCaracteristiques() const;
    // Celles de sa classe au niveau 1 plus la croissance de son niveau, sans l'équipement : la base des seuils d'équipement.
    [[nodiscard]] const Caracteristiques& getCaracteristiquesDeBase() const;
    [[nodiscard]] int getNiveau() const;
    // Passe à ce niveau (`assert` : au moins le sien, tous les niveaux gagnés d'un coup s'appliquent). Sa base, ses
    // caractéristiques, ses statistiques, ses objets actifs et son maximum de points de vie sont recalculés par le chemin de
    // l'équipement (`recalculerDepuisLEquipement`) ; un vivant gagne autant de points de vie que son maximum, un mort reste mort.
    void monterAuNiveau(int niveau);
    // Ses statistiques de combat, tirées de ses caractéristiques effectives (`statistiquesDuPersonnage`).
    [[nodiscard]] const StatistiquesDeCombat& getStatistiques() const;
    // Porte `objet` à cet emplacement (`assert` : `peutAller`) et rend celui qui y était. L'armure est
    // recalculée en entier depuis l'équipement. Permis pour un mort.
    [[nodiscard]] std::optional<Objet> poserEquipement(Emplacement emplacement, Objet objet);
    // Enlève l'objet de cet emplacement et le rend ; l'armure est recalculée.
    [[nodiscard]] std::optional<Objet> retirerEquipement(Emplacement emplacement);

private:
    Mobile m_mobile;        // position (centre en pixels du monde) et déplacement
    sf::Color m_couleur;
    Caracteristiques m_caracteristiquesDeClasse;  // celles de sa classe au niveau 1, jamais modifiées
    PaireDeCroissance m_croissance;              // ce qui grandit avec le niveau
    int m_niveau;                                // celui de l'escouade, >= 1
    Caracteristiques m_caracteristiquesDeBase;   // sa classe plus la croissance de son niveau (`baseAuNiveau`), sans l'équipement
    Equipement m_equipement;  // ce qu'il porte, objets inactifs compris ; déclaré avant ce qui en est tiré
    Equipement m_equipementActif;  // ce qui a un effet (seuils atteints, `actifPour`) : seul `recalculerDepuisLEquipement` l'écrit
    DescriptionDAttaque m_descriptionDeBase;    // celle de l'équipement, que `m_attaqueDeBase` met en œuvre
    Skill m_attaqueDeBase;  // la machine à états de l'attaque de base : refaite quand l'équipement la change
    bool m_rechargeARelancer = false;  // l'arme a changé : la recharge complète reste à tirer
    std::vector<SkillConnu> m_skills; // sans doublon d'identifiant
    std::optional<sf::Vector2f> m_cibleChemin; // cible du dernier A* ; vide = pas de chemin en cours
    Caracteristiques m_caracteristiques;        // effectives : seul `recalculerDepuisLEquipement` les écrit
    StatistiquesDeCombat m_statistiques;        // tirées des caractéristiques effectives, écrites au même endroit
    Vitalite m_vitalite;    // points de vie et défenses
    float m_rayonVision;    // en pixels
    // Ce qu'il oppose à un coup : l'armure de son équipement et ses statistiques de combat.
    [[nodiscard]] Defenses defenses() const;
    // Sa classe plus la croissance de ce niveau (`croissanceAuNiveau`) : l'unique calcul de sa base.
    [[nodiscard]] Caracteristiques baseAuNiveau(int niveau) const;
    // Seul endroit où les caractéristiques effectives et tout ce qui en vient (statistiques de combat, points
    // de vie maximum, défenses de la vitalité, attaque de base) changent : toujours la base plus l'équipement,
    // jamais ajustées. Une arme qui change annule le coup de base en cours, sans recharge consommée, puis
    // l'attaque repart pour une recharge complète (`relancerSiLArmeAChange`).
    void recalculerDepuisLEquipement();
    // L'attaque de base : les portions de l'arme portée (ou des mains nues), la Puissance sur chaque portion
    // (`montantEffectif`, un arrondi par portion), la nature et la pénétration de l'arme, la capacité offensive du personnage.
    [[nodiscard]] Attaque attaqueDeLArme() const;
    // L'attaque de ce skill de ce personnage : son montant, caractéristiques comprises (`montantEffectif`), rangé
    // dans le champ que dit le type du skill (dégâts, soin ou durée de provocation), la
    // capacité offensive du personnage ; aucune pénétration.
    [[nodiscard]] Attaque attaqueDe(const Skill& skill) const;
    // Celle d'un skill de frappe : toutes les portions, la nature et la pénétration de l'arme portée (affixes compris), plus
    // le bonus du skill ; la Puissance joue sur chaque portion (`montantEffectif`, avec les coefficients du skill).
    [[nodiscard]] Attaque attaqueDeFrappe(const Skill& skill) const;
    // Son arme permet-elle de lancer ce skill (`Skill::convientA`) ?
    [[nodiscard]] bool armeConvientA(const Skill& skill) const;
    [[nodiscard]] const Skill* trouverSkill(IdSkill id) const;  // nullptr si le personnage ne connaît pas ce skill
    [[nodiscard]] Skill* trouverSkill(IdSkill id);
    std::optional<ActionEngagee> m_actionEngagee; // action en exécution et sa cible ; lue seulement pendant l'exécution
};
