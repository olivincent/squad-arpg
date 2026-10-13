// ./src/Controleur.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>

class Partie;

// L'état d'interface : le mode visée du skill de zone et le déplacement
// continu. Traduit les actions du joueur (déjà dépouillées des événements
// SFML par main.cpp) en demandes à la Partie. Tous les points sont en
// pixels du monde.
class Controleur {
public:
    explicit Controleur(Partie& partie);

    // À appeler au début de chaque image : sort de la visée si le lanceur
    // ne peut plus lancer (mort, défaite).
    void debutImage();
    // Clic gauche : en visée, lance le sort au point ; sinon engage un
    // combat (monstre) ou arme le déplacement continu (sol).
    void appuiGauche(sf::Vector2f point);
    // Clic droit : annule la visée.
    void appuiDroit();
    // Touches 1 à 3 : skill de cette place. Touche 4 : entre en visée
    // (skill prêt) ou en sort. Toute autre place est ignorée.
    void appuiTouche(std::size_t place);
    void appuiEchap();
    // `boutonTenu` : bouton gauche enfoncé ET fenêtre avec le focus.
    void mettreAJourDeplacement(bool boutonTenu, sf::Vector2f curseur);

    // Cercle de zone sous le curseur et cercle de portée du lanceur ; rien hors visée.
    void dessinerApercu(sf::RenderTarget& rendu, sf::Vector2f curseur) const;
    std::optional<std::size_t> getPlaceEnVisee() const;

private:
    Partie& m_partie;
    bool m_deplacementContinu;
    // Vide = mode normal, sinon la place du lanceur en cours de visée.
    std::optional<std::size_t> m_placeEnVisee;
};
