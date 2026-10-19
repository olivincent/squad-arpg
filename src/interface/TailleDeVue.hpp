// ./src/interface/TailleDeVue.hpp
#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

// Hauteur de monde visible, en pixels : la même pour la caméra du jeu et pour
// la vue de l'interface, quelle que soit la résolution de la fenêtre.
inline constexpr float s_hauteurDeVue = 800.0f;

// Taille d'une vue qui montre toujours `hauteur` pixels de monde en hauteur ;
// sa largeur suit le format de la fenêtre : hauteur × largeur / hauteur de la
// fenêtre. `tailleFenetre.y` doit être > 0.
[[nodiscard]] sf::Vector2f tailleDeVue(sf::Vector2u tailleFenetre, float hauteur);

// La vue de l'interface (barre de skills, compteur, écran de fin, menus) :
// `s_hauteurDeVue` de haut, coin haut gauche en (0, 0). Tout y grandit avec la
// résolution ; en 1200 × 800 elle est la vue par défaut de la fenêtre.
[[nodiscard]] sf::View vueDInterface(sf::Vector2u tailleFenetre);
