// ./src/EcranDeFin.hpp
#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <string>

// Dessine l'écran de fin de partie : un voile sombre sur toute la fenêtre, puis
// le titre (« Victoire » ou « Défaite ») et la consigne pour recommencer, au
// centre. En coordonnées de la fenêtre : à appeler avec la vue par défaut,
// après la barre de skills.
void dessinerEcranDeFin(sf::RenderTarget& rendu, const sf::Font& police, const std::string& titre);
