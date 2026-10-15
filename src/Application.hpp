// ./src/Application.hpp
#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <vector>

#include "Contexte.hpp"
#include "Ecran.hpp"
#include "Reglages.hpp"

// Possède la fenêtre, la police, les réglages et la pile d'écrans ; fait
// tourner la boucle SFML. Seul l'écran du dessus reçoit les événements, est
// mis à jour et est dessiné. Les transitions demandées par les écrans sont
// appliquées ici, une fois leur appel terminé.
class Application {
public:
    // Ouvre la fenêtre et charge la police ; std::runtime_error si la police
    // manque.
    Application();

    // La boucle principale ; rend le code de sortie du programme.
    int executer();

private:
    // Crée ou recrée la fenêtre à partir des réglages. `create` oublie la
    // limite d'images et la répétition des touches : elles sont réappliquées.
    void ouvrirFenetre();

    Contexte contexte();
    void appliquer(Transition transition);

    sf::RenderWindow m_fenetre;
    sf::Font m_police;
    Reglages m_reglages;
    std::vector<std::unique_ptr<Ecran>> m_pile;
};
