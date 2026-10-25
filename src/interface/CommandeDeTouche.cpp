// ./src/interface/CommandeDeTouche.cpp
#include "interface/CommandeDeTouche.hpp"

#include <optional>

CommandeDeTouche commandePourTouche(sf::Keyboard::Scancode touche, bool finDePartie, const TouchesDesRaccourcis& touches) {
    if (touche == sf::Keyboard::Scan::Escape) {
        return commande::Annuler{};
    }
    if (touche == sf::Keyboard::Scan::R && finDePartie) {
        return commande::RetourAuChoix{};
    }
    // Touches de débogage : à tout moment.
    if (touche == sf::Keyboard::Scan::F5) {
        return commande::NouvelleCarte{};
    }
    if (touche == sf::Keyboard::Scan::F3) {
        return commande::BasculerPointsVises{};
    }
    if (touche == sf::Keyboard::Scan::F4) {
        return commande::BasculerAncre{};
    }
    if (touche == sf::Keyboard::Scan::F6) {
        return commande::BasculerBrouillard{};
    }
    if (touche == sf::Keyboard::Scan::F7) {
        return commande::AjouterDesObjetsDeDemonstration{};
    }
    // Sur l'écran de fin, seul R compte : l'inventaire et les raccourcis sont ignorés.
    if (touche == sf::Keyboard::Scan::I && !finDePartie) {
        return commande::OuvrirInventaire{};
    }
    if (!finDePartie) {
        if (const std::optional<std::size_t> emplacement = emplacementPourTouche(touches, touche)) {
            return commande::ActiverEmplacement{*emplacement};
        }
    }
    return commande::Rien{};
}
