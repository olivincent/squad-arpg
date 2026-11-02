// ./src/rendu/MarquesDEffets.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

// Côté d'une marque d'effet actif et écart entre deux marques, en pixels. Provisoire : des icônes dessinées les remplaceront.
constexpr float s_coteDUneMarque = 4.0f;
constexpr float s_ecartEntreMarques = 2.0f;

// Coin haut-gauche de la rangée de marques d'une entité de ce centre et de cette taille (pixels du monde) : juste sous son carré,
// alignée sur son bord gauche.
[[nodiscard]] sf::Vector2f positionMarquesDEffets(sf::Vector2f centre, sf::Vector2f taille);

// Dessine une marque par effet actif à partir de `position`, de gauche à droite : d'abord `bonus` marques vertes, puis `malus` marques
// rouges. Rien si les deux sont nuls.
void dessinerMarquesDEffets(sf::RenderTarget& rendu, sf::Vector2f position, int bonus, int malus);
