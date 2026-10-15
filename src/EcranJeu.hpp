// ./src/EcranJeu.hpp
#pragma once

#include <SFML/Graphics/View.hpp>
#include <random>

#include "Controleur.hpp"
#include "Ecran.hpp"
#include "Partie.hpp"

// L'écran de la partie : ce que faisait `main()` pour le jeu (partie,
// contrôleur, caméra, interface, nouvelle carte).
class EcranJeu : public Ecran {
public:
    explicit EcranJeu(const Contexte& contexte);

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;

private:
    // Remplace la partie par une neuve (nouvelle graine) et remet l'état
    // d'interface à zéro.
    void nouvelleCarte();

    std::random_device m_tirageDeGraine;
    Partie m_partie;
    Controleur m_controleur;
    sf::View m_camera;
};
