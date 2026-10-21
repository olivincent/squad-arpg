// ./src/interface/Ecran.hpp
#pragma once

#include "interface/Transition.hpp"

namespace sf {
class Event;
}  // namespace sf

struct Contexte;

// Un écran de l'application (menu, options, jeu) : patron State. L'Application
// en garde une pile de std::unique_ptr<Ecran> ; seul l'écran du dessus reçoit
// les événements et est mis à jour (ceux du dessous sont donc figés) ; il est dessiné avec,
// dessous, les écrans qu'il laisse voir (`laisseVoirDessous`). L'Application vide la fenêtre avant de
// dessiner : un écran ne le fait jamais.
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

    // Vrai si l'écran n'est qu'une couche : l'`Application` dessine alors aussi l'écran en dessous
    // (puis le sien par-dessus). Faux : il remplit la fenêtre et cache ce qui est dessous.
    [[nodiscard]] virtual bool laisseVoirDessous() const { return false; }
};
