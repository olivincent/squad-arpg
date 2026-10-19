// ./src/Controleur.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include "AdresseSkill.hpp"

class Partie;

// L'état d'interface : le mode visée d'un skill de zone et le déplacement
// continu. Traduit les actions du joueur (déjà dépouillées des événements
// SFML par main.cpp) en demandes à la Partie. Tous les points sont en
// pixels du monde. Type valeur : la Partie est reçue à chaque appel, sans
// référence membre, pour que le Controleur reste réaffectable.
class Controleur {
public:
    // À appeler au début de chaque image : sort de la visée si le lanceur
    // ne peut plus lancer (mort, défaite).
    void debutImage(const Partie& partie);
    // Clic gauche : en visée, lance le sort au point ; sinon engage un
    // combat (monstre) ou arme le déplacement continu (sol).
    void appuiGauche(Partie& partie, sf::Vector2f point);
    // Clic droit : annule la visée.
    void appuiDroit();
    // Touche provisoire de 1 à s_nombreDeTouches : le skill à son adresse (voir
    // adresseDeLaTouche). Un skill de type Zone entre en visée (skill prêt) ou en
    // sort ; tout autre type est demandé à la partie. Toute autre touche est ignorée.
    void appuiTouche(Partie& partie, std::size_t indiceTouche);
    static constexpr std::size_t s_nombreDeTouches = 4;
    // Adresse du skill de la touche `indiceTouche` (de 1 à s_nombreDeTouches) ; vide sinon.
    static std::optional<AdresseSkill> adresseDeLaTouche(std::size_t indiceTouche);
    void appuiEchap();
    // `boutonTenu` : bouton gauche enfoncé ET fenêtre avec le focus.
    void mettreAJourDeplacement(Partie& partie, bool boutonTenu, sf::Vector2f curseur);

    // Cercle de zone sous le curseur et cercle de portée du lanceur ; rien hors visée.
    void dessinerApercu(const Partie& partie, sf::RenderTarget& rendu, sf::Vector2f curseur) const;
    std::optional<AdresseSkill> getAdresseEnVisee() const;

private:
    bool m_deplacementContinu = false;
    // Vide = mode normal, sinon le skill de zone en cours de visée.
    std::optional<AdresseSkill> m_adresseEnVisee;
};
