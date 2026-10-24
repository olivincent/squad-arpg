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
    // pas deux fois le même identifiant) ; `equipementDeDepart` ce qu'il porte à sa création, et son armure
    // est celle de cet équipement ; `zoneDeControle` la zone qu'il engage autour de lui, en pixels, bord à
    // bord (0 : il n'engage personne). Son `rang` n'est lu que par la formation.
    Personnage(sf::Vector2f centre, DefinitionPersonnage definition);
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
    // Dégâts de l'attaque de base, avant l'armure de la cible.
    // Le montant de son attaque de base, caractéristiques comprises (`montantEffectif`).
    [[nodiscard]] int getPuissanceAttaqueDeBase() const;
    // Zone de contrôle, en pixels, bord à bord ; 0 : il n'engage personne.
    [[nodiscard]] float getZoneDeControle() const;
    // Attaque de base en trois temps : engager (vivant, immobile, attaque
    // prête) ; avancer l'exécution, qui rend les dégâts au moment où le coup
    // part ; annuler, sans recharge consommée.
    [[nodiscard]] bool engagerAttaque(int idCible);
    [[nodiscard]] std::optional<int> cibleEngagee() const;
    [[nodiscard]] int avancerAttaque(float dt, Generateur& generateur);
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
    [[nodiscard]] bool skillEstPret(IdSkill id) const;
    [[nodiscard]] std::optional<EtatSkill> etatDuSkill(IdSkill id) const;
    [[nodiscard]] bool skillEstAPortee(IdSkill id, Cercle cible) const;
    [[nodiscard]] Cercle zoneDuSkill(IdSkill id) const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill(IdSkill id) const;
    // Skill en trois temps, comme l'attaque de base : engager (vivant, skill
    // prêt, aucun autre skill en activation ; annule le coup de base en
    // exécution, mais ne demande pas d'immobilité) ; avancer l'activation, qui
    // rend la puissance au moment où l'effet part (et relance la recharge de
    // l'attaque de base) ; annuler, sans recharge consommée. Une seule action
    // engagée à la fois ; les recharges des skills sont indépendantes. La portée
    // et la vue sont vérifiées par Escouade ; la cible est stockée sans être interprétée.
    [[nodiscard]] bool engagerSkill(IdSkill id, std::optional<Cible> cible);
    // Les conditions d'engagerSkill : vivant, skill connu et prêt, aucun skill en
    // activation (le lanceur est libre).
    [[nodiscard]] bool peutEngagerSkill(IdSkill id) const;
    [[nodiscard]] std::optional<ActionEngagee> skillEngage() const;
    [[nodiscard]] int avancerSkill(float dt, Generateur& generateur);
    void annulerSkill();
    // Renvoient le montant réellement appliqué : dégâts après armure (0 pour un mort), points
    // de vie réellement rendus (plafonnés au maximum, 0 pour un mort).
    int recevoirDegats(int quantite);
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
    // Ses caractéristiques effectives : la base de sa classe plus les bonus de son équipement.
    [[nodiscard]] const Caracteristiques& getCaracteristiques() const;
    // Porte `objet` à cet emplacement (`assert` : `peutAller`) et rend celui qui y était. L'armure est
    // recalculée en entier depuis l'équipement. Permis pour un mort.
    [[nodiscard]] std::optional<Objet> poserEquipement(Emplacement emplacement, Objet objet);
    // Enlève l'objet de cet emplacement et le rend ; l'armure est recalculée.
    [[nodiscard]] std::optional<Objet> retirerEquipement(Emplacement emplacement);

private:
    Mobile m_mobile;        // position (centre en pixels du monde) et déplacement
    sf::Color m_couleur;
    Skill m_attaqueDeBase;
    std::vector<SkillConnu> m_skills; // sans doublon d'identifiant
    std::optional<sf::Vector2f> m_cibleChemin; // cible du dernier A* ; vide = pas de chemin en cours
    Equipement m_equipement;  // déclaré avant m_vitalite : l'armure en est tirée à la construction
    Caracteristiques m_caracteristiquesDeBase;  // celles de sa classe, jamais modifiées
    Caracteristiques m_caracteristiques;        // effectives : seul `recalculerCaracteristiques` les écrit
    int m_pointsDeVieDeBase;                    // sans l'Endurance
    Vitalite m_vitalite;    // points de vie et armure
    float m_rayonVision;    // en pixels
    float m_zoneDeControle; // en pixels, bord à bord
    // Seul endroit où l'armure de la vitalité change : toujours la somme de l'équipement, jamais ajustée.
    void recalculerArmure();
    // Seul endroit où les caractéristiques effectives (et le maximum de points de vie qui en vient)
    // changent : toujours la base plus l'équipement, jamais ajustées.
    void recalculerCaracteristiques();
    // Le montant de ce skill de ce personnage, caractéristiques comprises (`montantEffectif`).
    [[nodiscard]] int montantDe(const Skill& skill) const;
    [[nodiscard]] const Skill* trouverSkill(IdSkill id) const;  // nullptr si le personnage ne connaît pas ce skill
    [[nodiscard]] Skill* trouverSkill(IdSkill id);
    std::optional<ActionEngagee> m_actionEngagee; // action en exécution et sa cible ; lue seulement pendant l'exécution
};
