// ./src/Eclaircir.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>

// Indice visuel d'un coup en préparation : la couleur mélangée au blanc, à
// raison de `fraction` (0 = inchangée, 1 = blanc). L'opacité est conservée.
[[nodiscard]] sf::Color eclaircir(sf::Color couleur, float fraction);

// Nuance d'une couleur : la couleur mélangée au noir, à raison de `fraction` (0 = inchangée,
// 1 = noir). L'opacité est conservée. Ne s'emploie pas pour l'indice d'exécution, qui éclaircit.
[[nodiscard]] sf::Color assombrir(sf::Color couleur, float fraction);
