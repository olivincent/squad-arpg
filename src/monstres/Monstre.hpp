// ./src/monstres/Monstre.hpp
#pragma once
#include "base/Aleatoire.hpp"
#include "combat/Skill.hpp"
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/Mobile.hpp"
#include "combat/Vitalite.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

class TileMap;

class Monstre {
public:
    // Distance maximale au poste (en pixels) au-delà de laquelle un monstre abandonne.
    static constexpr float s_rayonTerritoire = 600.0f;
    // Zone de contrôle (en pixels, bord à bord) : tout monstre engage ce qui est à cette distance.
    static constexpr float s_zoneDeControle = 20.0f;

    // `groupe` est un identifiant opaque : des monstres de même groupe « vont
    // ensemble ». Le monstre ne sait pas ce qu'il désigne.
    Monstre(sf::Vector2f centre, int id, int armure, int groupe);
    // `marquerLeRetour` (débogage F3) : le carré d'un monstre qui rentre à son poste est semi-transparent.
    void dessiner(sf::RenderTarget& rendu, bool estCible, bool marquerLeRetour) const;
    // Débogage (F3) : trait fin du centre du monstre à travers le chemin restant de son Mobile.
    void dessinerChemin(sf::RenderTarget& rendu) const;
    // `proie` est le personnage que le monstre poursuit (sans dépendre de la vue) ;
    // `candidats` sont ceux qu'il peut frapper (identifiant = place du personnage).
    // Il s'arrête dès qu'il a une cible de frappe (voir `engagerAttaque`).
    // `obstacles` le bloquent (collision) ; `aContourner` est ce que son chemin évite (voir `Mobile::allerVers`) :
    // les autres monstres, pas les personnages (un personnage sur le chemin est quelqu'un à frapper).
    void mettreAJour(float dt, std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats,
                     const TileMap& carte, const std::vector<Cercle>& obstacles,
                     const std::vector<Cercle>& aContourner);
    // Attaque en trois temps, comme celle d'un personnage : engager (cible à
    // portée et en vue, monstre immobile, attaque prête) ; avancer l'exécution, qui rend
    // les dégâts au départ du coup ; annuler, sans recharge consommée.
    // Une cible est atteignable à portée ET en vue (`ligneDeVue`, en cases).
    // Même règle que `Escouade::peutAtteindre`.
    [[nodiscard]] bool peutAtteindre(Cercle cible, const TileMap& carte) const;
    // Engage sur la cible de frappe : la proie si elle est atteignable, sinon le
    // candidat atteignable le plus proche. Le monstre retient l'identifiant (la
    // place du personnage) jusqu'au départ du coup.
    [[nodiscard]] bool engagerAttaque(std::optional<EntiteVue> proie, const std::vector<EntiteVue>& candidats, const TileMap& carte);
    // Identifiant (place) du personnage visé par le coup en exécution ; vide hors exécution.
    [[nodiscard]] std::optional<int> cibleEngagee() const;
    [[nodiscard]] int avancerAttaque(float dt, Generateur& generateur);
    void annulerAttaque();
    [[nodiscard]] sf::Vector2f getCentre() const;
    [[nodiscard]] Cercle getCercle() const;
    [[nodiscard]] int getId() const;
    [[nodiscard]] int getGroupe() const;
    [[nodiscard]] bool contient(sf::Vector2f point) const;
    // Sans aucun effet (ni dégâts, ni alerte) pendant le retour au poste : le monstre est invulnérable.
    // Renvoie les dégâts réellement appliqués (après armure) ; 0 en retour ou si le monstre est mort.
    int recevoirDegats(int quantite);
    // Demande au monstre d'entrer en poursuite de sa proie habituelle, même
    // hors de la détection, s'il est dans son territoire. Prise en compte au
    // prochain `mettreAJour` ; sans effet sur un monstre déjà en poursuite
    // ou en retour au poste.
    void alerter();
    // Vrai quand le monstre est en poursuite de sa proie (le statut « attaque »).
    [[nodiscard]] bool estEnPoursuite() const;
    // Faux pendant le retour au poste : le monstre n'est alors la cible de personne
    // (il reste un obstacle physique).
    [[nodiscard]] bool estCiblable() const;
    [[nodiscard]] bool estVivant() const;
    [[nodiscard]] int getPointsDeVie() const;
    // Zone de contrôle du monstre, en pixels, bord à bord.
    [[nodiscard]] float getZoneDeControle() const;
    // Dégâts de son attaque, avant l'armure de la cible.
    [[nodiscard]] int getPuissanceAttaque() const;

private:
    enum class Etat { EnGarde, Poursuite, Retour };

    [[nodiscard]] bool proieDetectee(const std::optional<EntiteVue>& proie, const TileMap& carte) const;
    // Règle propre au monstre : `choisirCibleAFrapper` avec `peutAtteindre`, la proie préférée.
    [[nodiscard]] std::optional<EntiteVue> cibleDeFrappe(const std::optional<EntiteVue>& proie,
                                           const std::vector<EntiteVue>& candidats, const TileMap& carte) const;

    Mobile m_mobile;
    Vitalite m_vitalite; // points de vie et armure
    int m_id;
    int m_groupe;
    Skill m_attaque;
    float m_rayonDetection;
    float m_rayonTerritoire;
    Etat m_etat;
    sf::Vector2f m_poste;
    bool m_alerte; // posée par `alerter`, consommée par `mettreAJour`
    float m_soinAccumule; // points de vie rendus pas encore entiers, pendant le retour ; 0 ailleurs
    std::optional<int> m_idCibleEngagee; // lu seulement pendant l'exécution
};
