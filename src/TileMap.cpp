// ./src/TileMap.cpp
#include "TileMap.hpp"

TileMap::TileMap(int largeur, int hauteur, float tailleCase)
    : m_largeur(largeur),
      m_hauteur(hauteur),
      m_tailleCase(tailleCase),
      m_cases(largeur * hauteur, Tile::Floor)
{
    for (int y = 0; y < m_hauteur; ++y) {
        for (int x = 0; x < m_largeur; ++x) {
            if (x == 0 || y == 0 || x == m_largeur - 1 || y == m_hauteur - 1) {
                m_cases[y * m_largeur + x] = Tile::Wall;
            }
        }
    }
}