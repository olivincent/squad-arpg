// ./src/TileMap.hpp
#pragma once
#include <vector>

enum class Tile {
    Floor,
    Wall
};

class TileMap {
private:
    int m_largeur;          // nombre de cases en X
    int m_hauteur;          // nombre de cases en Y
    float m_tailleCase;     // taille d'une case en pixels
    std::vector<Tile> m_cases;  // largeur * hauteur cases, indice = y * largeur + x
};