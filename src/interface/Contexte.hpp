// ./src/interface/Contexte.hpp
#pragma once

namespace sf {
class Font;
class RenderWindow;
}  // namespace sf

struct Reglages;

// Ce que l'Application prête à un écran pour la durée d'un appel : la fenêtre,
// la police et les réglages (en lecture seule). Construit à chaque appel et
// jamais stocké par les écrans : la fenêtre peut être recréée entre deux
// appels (même règle que pour la Partie dans le Controleur, C.12).
struct Contexte {
    sf::RenderWindow& fenetre;
    const sf::Font& police;
    const Reglages& reglages;
};
