// ./src/EcranDeFin.hpp
#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <string>

// Dessine l'écran de fin de partie : un voile sombre sur toute la fenêtre, puis
// le titre (« Victoire » ou « Défaite ») et la consigne pour revenir au choix de l'escouade,
// au centre. En coordonnées de la vue active : à appeler avec la vue de
// l'interface (`vueDInterface`), après la barre de skills.
void dessinerEcranDeFin(sf::RenderTarget& rendu, const sf::Font& police, const std::string& titre);
