// ./src/EcranMenu.hpp
#pragma once

#include <SFML/System/Vector2.hpp>

#include "Ecran.hpp"
#include "ListeDeBoutons.hpp"
#include "Reglages.hpp"

// Le menu de départ : « Commencer partie », « Options » et « Quitter ».
class EcranMenu : public Ecran {
public:
    EcranMenu();

    // Un clic gauche en `point` (coordonnées de la vue de l'interface), les
    // boutons étant centrés sur `centre`. Sépare la logique de la fenêtre :
    // `traiterEvenement` convertit l'événement puis appelle cette fonction.
    // Commencer partie : empile l'écran de choix de l'escouade ; Options : empile l'écran des
    // options, qui part de `reglages` ; Quitter : quitte ; un clic hors des
    // boutons : rien.
    [[nodiscard]] Transition cliquer(sf::Vector2f point, sf::Vector2f centre, const Reglages& reglages) const;

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;

private:
    ListeDeBoutons m_boutons;
};
