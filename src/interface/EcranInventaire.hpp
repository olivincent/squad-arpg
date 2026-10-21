// ./src/interface/EcranInventaire.hpp
#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <string>
#include <vector>

#include "interface/Ecran.hpp"

class Escouade;

// L'inventaire, ouvert par-dessus la partie (touche I) : l'équipement des personnages et le sac
// de l'escouade, en lecture seule. Il laisse voir l'écran de jeu dessous, figé : seul l'écran du
// dessus est mis à jour. I ou Échap le ferment.
class EcranInventaire : public Ecran {
public:
    // `escouade` : celle de l'écran de jeu, qui reste sous celui-ci dans la pile et lui survit
    // donc (invariant de la pile, `docs/ecrans.md`). `nomsDesClasses` : le nom de la classe de
    // chaque place, dans l'ordre des places (`s_nombreDePlaces` noms).
    EcranInventaire(const Escouade& escouade, std::vector<std::string> nomsDesClasses);

    // Une touche pressée : I ou Échap dépilent l'écran, les autres ne font rien. Sépare la logique
    // de l'événement SFML, comme `cliquer` pour les menus.
    [[nodiscard]] Transition appuyer(sf::Keyboard::Scancode touche) const;

    Transition traiterEvenement(const sf::Event& evenement, const Contexte& contexte) override;
    void mettreAJour(float dt, const Contexte& contexte) override;
    void dessiner(const Contexte& contexte) const override;
    [[nodiscard]] bool laisseVoirDessous() const override;

private:
    const Escouade& m_escouade;
    std::vector<std::string> m_nomsDesClasses;
};
