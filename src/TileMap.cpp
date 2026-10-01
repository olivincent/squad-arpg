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

int TileMap::indice(int x, int y) const {
    return y * m_largeur + x;
}

Tile TileMap::getTile(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_largeur || y >= m_hauteur) {
        return Tile::Wall;
    }
    return m_cases[indice(x, y)];
}

void TileMap::dessiner(sf::RenderTarget& rendu) const {
    sf::RectangleShape forme({m_tailleCase, m_tailleCase});
    forme.setOutlineThickness(-1.0f);
    forme.setOutlineColor(sf::Color(20, 20, 20));

    for (int y = 0; y < m_hauteur; ++y) {
        for (int x = 0; x < m_largeur; ++x) {
            forme.setPosition({x * m_tailleCase, y * m_tailleCase});
            if (m_cases[indice(x, y)] == Tile::Wall) {
                forme.setFillColor(sf::Color(128, 128, 128));
            } else {
                forme.setFillColor(sf::Color(60, 45, 35));
            }
            rendu.draw(forme);
        }
    }
}