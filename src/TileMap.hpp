// ./src/TileMap.hpp
#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

enum class Tile {
    Floor,
    Wall
};

class TileMap {
public:
    TileMap(int largeur, int hauteur, float tailleCase);
    Tile getTile(int x, int y) const;
    void dessiner(sf::RenderTarget& rendu) const;


private:
    int indice(int x, int y) const;
    int m_largeur;          // nombre de cases en X
    int m_hauteur;          // nombre de cases en Y
    float m_tailleCase;     // taille d'une case en pixels
    std::vector<Tile> m_cases;  // largeur * hauteur cases, indice = y * largeur + x
};