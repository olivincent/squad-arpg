// ./src/rendu/BarreDEnergie.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

// Hauteur de la barre d'énergie, en pixels : plus fine que la barre de vie qu'elle suit.
constexpr float s_hauteurBarreDEnergie = 3.0f;

// Coin haut-gauche de la barre d'énergie d'une entité de ce centre et de cette taille (pixels du monde) : juste sous sa barre de
// vie (`positionBarreDeVie`), entre elle et le carré, alignée sur son bord gauche.
[[nodiscard]] sf::Vector2f positionBarreDEnergie(sf::Vector2f centre, sf::Vector2f taille);

// Dessine une barre d'énergie de cette largeur, dont le coin haut-gauche est à `position` : fond sombre, puis partie bleue
// proportionnelle à `ratio` (de 0, vide, à 1, pleine).
void dessinerBarreDEnergie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio);
