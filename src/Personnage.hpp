// ./src/Personnage.hpp
#pragma once
#include "ActionEngagee.hpp"
#include "Aleatoire.hpp"
#include "Cercle.hpp"
#include "EtatSkill.hpp"
#include "Mobile.hpp"
#include "PointsDeVie.hpp"
#include "Skill.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

class TileMap;

class Personnage {
public:
    // `rayonVision` : jusqu'où il voit, en pixels (l'union des champs des vivants est la vision de l'escouade).
    Personnage(sf::Vector2f centre, sf::Color couleur, Skill attaqueDeBase, std::optional<Skill> skill, int armure, float rayonVision);
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
    // Attaque de base en trois temps : engager (vivant, immobile, attaque
    // prête) ; avancer l'exécution, qui rend les dégâts au moment où le coup
    // part ; annuler, sans recharge consommée.
    bool engagerAttaque(int idCible);
    std::optional<int> cibleEngagee() const;
    [[nodiscard]] int avancerAttaque(float dt, Generateur& generateur);
    // Sans effet sur une activation de skill : seul annulerSkill l'annule.
    void annulerAttaque();
    // Voir Mobile::estEnMouvement.
    [[nodiscard]] bool estEnMouvement() const;
    // Façade de Mobile::cheminRestant, pour le débogage.
    [[nodiscard]] std::vector<sf::Vector2f> cheminRestant() const;
    // Un coup de base en exécution ou un skill en activation : le personnage reste sur place.
    bool estEnExecution() const;
    std::optional<TypeSkill> typeSkill() const;
    bool skillEstPret() const;
    std::optional<EtatSkill> etatDuSkill() const;
    [[nodiscard]] bool skillEstAPortee(Cercle cible) const;
    [[nodiscard]] Cercle zoneDuSkill() const;
    [[nodiscard]] Cercle cercleDePorteeDuSkill() const;
    // Skill en trois temps, comme l'attaque de base : engager (vivant, skill
    // prêt ; annule le coup de base en exécution, mais ne demande pas
    // d'immobilité) ; avancer l'activation, qui rend la puissance au moment
    // où l'effet part (et relance la recharge de l'attaque de base) ;
    // annuler, sans recharge consommée. La portée et la vue sont vérifiées
    // par Escouade ; la cible est stockée sans être interprétée.
    bool engagerSkill(std::optional<Cible> cible);
    std::optional<ActionEngagee> skillEngage() const;
    [[nodiscard]] int avancerSkill(float dt, Generateur& generateur);
    void annulerSkill();
    void recevoirDegats(int quantite);
    void recevoirSoin(int quantite);
    bool estVivant() const;
    int getPointsDeVie() const;
    int getPointsDeVieMax() const;
    sf::Vector2f getCentre() const;
    Cercle getCercle() const;
    // Rayon de vision, en pixels.
    [[nodiscard]] float getRayonVision() const;

private:
    Mobile m_mobile;        // position (centre en pixels du monde) et déplacement
    sf::Color m_couleur;
    Skill m_attaqueDeBase;
    std::optional<Skill> m_skill; // vide = pas de skill
    std::optional<sf::Vector2f> m_cibleChemin; // cible du dernier A* ; vide = pas de chemin en cours
    PointsDeVie m_pointsDeVie;
    int m_armure;
    float m_rayonVision;    // en pixels
    std::optional<ActionEngagee> m_actionEngagee; // action en exécution et sa cible ; lue seulement pendant l'exécution
};
