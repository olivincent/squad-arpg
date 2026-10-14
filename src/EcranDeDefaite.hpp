// ./src/EcranDeDefaite.hpp
#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

// Dessine l'écran de défaite : un voile sombre sur toute la fenêtre, puis
// « Défaite » et la consigne pour recommencer, au centre. En coordonnées de
// la fenêtre : à appeler avec la vue par défaut, après la barre de skills.
void dessinerEcranDeDefaite(sf::RenderTarget& rendu, const sf::Font& police);
