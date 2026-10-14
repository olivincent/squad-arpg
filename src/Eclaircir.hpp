// ./src/Eclaircir.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>

// Indice visuel d'un coup en préparation : la couleur mélangée au blanc, à
// raison de `fraction` (0 = inchangée, 1 = blanc). L'opacité est conservée.
[[nodiscard]] sf::Color eclaircir(sf::Color couleur, float fraction);
