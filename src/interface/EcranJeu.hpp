// ./src/interface/EcranJeu.hpp
#pragma once

#include <SFML/Graphics/View.hpp>
#include <random>
#include <vector>

#include "interface/BarreDeRaccourcis.hpp"
#include "interface/Controleur.hpp"
#include "interface/Ecran.hpp"
#include "jeu/Partie.hpp"
#include "escouade/Specialite.hpp"
#include "interface/TextesFlottants.hpp"

// Bascules de débogage (état d'interface de l'écran de jeu, remises à leur valeur de départ
// à chaque nouvelle partie) : F3 les points visés des personnages, F4 le carré de l'ancre,
// F6 le brouillard de guerre (visuel seulement).
struct OptionsDeDebogage {
    bool pointsVises = false;
    bool ancre = true;
    bool brouillard = true;
};

// L'écran de la partie : ce que faisait `main()` pour le jeu (partie,
// contrôleur, caméra, interface, nouvelle carte).
class EcranJeu : public Ecran {
public:
    // `specialites` : les classes de l'escouade, dans l'ordre des places ; elles servent à
    // chaque nouvelle carte (F5).
    explicit EcranJeu(std::vector<IdSpecialite> specialites);

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;

private:
    // La caméra montre toujours `s_hauteurDeVue` pixels de monde en hauteur :
    // sa taille suit le format de la fenêtre, lue à chaque image.
    void ajusterCamera(const Contexte& contexte);

    // Remplace la partie par une neuve (nouvelle graine) et remet l'état
    // d'interface à zéro.
    void nouvelleCarte();

    std::random_device m_tirageDeGraine;
    std::vector<IdSpecialite> m_specialites;  // déclarée avant m_partie : la construction la lit
    Partie m_partie;
    Controleur m_controleur;
    // La barre de raccourcis est de l'état d'interface (comme la caméra) : ni la Partie ni
    // l'Escouade ne savent qu'elle existe. Reconstruite à chaque nouvelle partie.
    BarreDeRaccourcis m_barre;
    // Les dégâts et les soins qui montent au-dessus des cibles : nourris par les événements de la
    // Partie, remis à zéro à chaque nouvelle partie.
    TextesFlottants m_textes;
    sf::View m_camera;
    OptionsDeDebogage m_debogage;
};
