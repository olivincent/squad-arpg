// ./src/monde/Deplacement.hpp
#pragma once
#include "base/Cercle.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class TileMap;

[[nodiscard]] bool deplacementBloque(Cercle avant, Cercle apres, const std::vector<Cercle>& obstacles);

// Essai axe par axe (X puis Y), puis, si rien n'a passé et qu'un cercle a refusé, un essai de même
// longueur le long de la tangente du cercle bloquant le plus proche (refusé s'il chevauche un mur
// ou si deplacementBloque est vrai).
[[nodiscard]] sf::Vector2f deplacerAvecGlissement(const TileMap& carte, const sf::FloatRect& rect, sf::Vector2f deplacement,
                                    const std::vector<Cercle>& obstacles);
