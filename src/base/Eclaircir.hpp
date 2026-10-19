// ./src/base/Eclaircir.hpp
#pragma once
#include <SFML/Graphics/Color.hpp>

// Indice visuel d'un coup en préparation : la couleur mélangée au blanc, à
// raison de `fraction` (0 = inchangée, 1 = blanc). L'opacité est conservée.
[[nodiscard]] sf::Color eclaircir(sf::Color couleur, float fraction);

// Part de blanc d'une entité (personnage ou monstre) pendant l'exécution d'un coup ou l'activation
// d'un skill : la même règle visuelle dans les deux camps.
inline constexpr float s_eclaircissementExecution = 0.5f;

// Nuance d'une couleur : la couleur mélangée au noir, à raison de `fraction` (0 = inchangée,
// 1 = noir). L'opacité est conservée. Ne s'emploie pas pour l'indice d'exécution, qui éclaircit.
[[nodiscard]] sf::Color assombrir(sf::Color couleur, float fraction);
