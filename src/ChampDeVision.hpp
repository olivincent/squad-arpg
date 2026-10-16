// ./src/ChampDeVision.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

class TileMap;

// Cases qu'un observateur placé dans la case `origine` voit, jusqu'à `rayon` cases
// (distance entre les centres des cases <= rayon). Shadowcasting symétrique : pour deux
// cases non bloquantes A et B, B est dans le champ de A si et seulement si A est dans
// celui de B. Une case bloquante (mur, porte fermée) qui reçoit la lumière est révélée et
// l'arrête : on voit sa face, jamais ce qui est derrière. Une case hors de la carte est un
// mur, mais n'est jamais renvoyée. L'origine est révélée (si elle est dans la carte). Les
// doublons sont permis : les quadrants partagent leurs axes.
[[nodiscard]] std::vector<sf::Vector2i> champDeVision(const TileMap& carte, sf::Vector2i origine, int rayon);
