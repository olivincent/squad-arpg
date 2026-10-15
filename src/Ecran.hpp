// ./src/Ecran.hpp
#pragma once

#include "Transition.hpp"

namespace sf {
class Event;
}  // namespace sf

struct Contexte;

// Un écran de l'application (menu, options, jeu) : patron State. L'Application
// en garde une pile de std::unique_ptr<Ecran> ; seul l'écran du dessus reçoit
// les événements, est mis à jour et est dessiné.
class Ecran {
public:
    Ecran() = default;
    Ecran(const Ecran&) = delete;
    Ecran& operator=(const Ecran&) = delete;
    virtual ~Ecran() = default;

    // Un événement SFML (la fermeture de la fenêtre est traitée par
    // l'Application, quel que soit l'écran). Renvoie ce que l'écran demande
    // à l'Application.
    virtual Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) = 0;

    virtual void mettreAJour(float dt, const Contexte& contexte) = 0;

    virtual void dessiner(const Contexte& contexte) const = 0;
};
