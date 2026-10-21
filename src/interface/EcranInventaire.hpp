// ./src/interface/EcranInventaire.hpp
#pragma once

#include <SFML/Window/Keyboard.hpp>

#include "interface/Ecran.hpp"

// L'inventaire, ouvert par-dessus la partie (touche I). Il laisse voir l'écran de jeu dessous,
// figé : seul l'écran du dessus est mis à jour. I ou Échap le ferment.
class EcranInventaire : public Ecran {
public:
    // Une touche pressée : I ou Échap dépilent l'écran, les autres ne font rien. Sépare la logique
    // de l'événement SFML, comme `cliquer` pour les menus.
    [[nodiscard]] Transition appuyer(sf::Keyboard::Scancode touche) const;

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;
    [[nodiscard]] bool laisseVoirDessous() const override;
};
