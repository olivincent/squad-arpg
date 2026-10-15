// ./src/TailleDeVue.hpp
#pragma once

#include <SFML/System/Vector2.hpp>

// Taille d'une vue qui montre toujours `hauteur` pixels de monde en hauteur ;
// sa largeur suit le format de la fenêtre : hauteur × largeur / hauteur de la
// fenêtre. `tailleFenetre.y` doit être > 0.
[[nodiscard]] sf::Vector2f tailleDeVue(sf::Vector2u tailleFenetre, float hauteur);
