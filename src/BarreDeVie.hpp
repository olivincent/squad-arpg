// ./src/BarreDeVie.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

// Hauteur de la barre de vie, en pixels.
constexpr float s_hauteurBarreDeVie = 5.0f;

// Coin haut-gauche de la barre de vie d'une entité de ce centre et de cette
// taille (pixels du monde) : juste au-dessus de son carré, avec un écart.
[[nodiscard]] sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille);

// Dessine une barre de vie de cette largeur, dont le coin haut-gauche est à
// `position` : fond gris, puis partie verte proportionnelle aux points de
// vie. Ne dessine rien si `pointsDeVieMax` est nul ou négatif.
void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur,
                        int pointsDeVie, int pointsDeVieMax);
