// ./src/interface/CommandeDeTouche.hpp
#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <variant>

#include "interface/Reglages.hpp"

// Ce qu'une touche demande à l'écran de jeu. `EcranJeu` ne fait qu'exécuter la commande ; quelle
// touche donne quelle commande est décidé ici, par une fonction pure.
namespace commande {
struct Rien {};
struct Annuler {};  // Échap : sortie de la visée
struct OuvrirInventaire {};
struct RetourAuChoix {};
struct NouvelleCarte {};
struct BasculerPointsVises {};
struct BasculerAncre {};
struct BasculerBrouillard {};
struct ActiverEmplacement {
    std::size_t emplacement;
};
}  // namespace commande

using CommandeDeTouche =
    std::variant<commande::Rien, commande::Annuler, commande::OuvrirInventaire, commande::RetourAuChoix, commande::NouvelleCarte,
                 commande::BasculerPointsVises, commande::BasculerAncre, commande::BasculerBrouillard,
                 commande::ActiverEmplacement>;

// La commande d'un appui sur `touche` (physique : scancode). Échap, F3 à F6 valent en tout temps ;
// R (retour au choix) seulement sur l'écran de fin, I (inventaire) seulement hors de l'écran de fin ; les touches des raccourcis de `touches`
// seulement hors de l'écran de fin. Toute autre touche : `Rien`.
[[nodiscard]] CommandeDeTouche commandePourTouche(sf::Keyboard::Scancode touche, bool finDePartie,
                                                  const TouchesDesRaccourcis& touches);
