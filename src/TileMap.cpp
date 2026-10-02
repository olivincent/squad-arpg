// ./src/TileMap.cpp
#include "TileMap.hpp"
#include <cmath>
#include <stdexcept>

TileMap::TileMap(const std::vector<std::string>& plan, float tailleCase)
    : m_largeur(static_cast<int>(plan.at(0).size())),
      m_hauteur(static_cast<int>(plan.size())),
      m_tailleCase(tailleCase),
      m_cases(m_largeur * m_hauteur, Tile::Floor)
{
    for (int y = 0; y < m_hauteur; ++y) {
        const std::string& ligne = plan[y];
        if (static_cast<int>(ligne.size()) != m_largeur) {
            throw std::invalid_argument("TileMap : la ligne " + std::to_string(y) + " n'a pas la bonne longueur");
        }
        for (int x = 0; x < m_largeur; ++x) {
            if (ligne[x] == '#') {
                m_cases[indice(x, y)] = Tile::Wall;
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

int TileMap::getLargeur() const {
    return m_largeur;
}

int TileMap::getHauteur() const {
    return m_hauteur;
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

bool TileMap::chevaucheUnMur(const sf::FloatRect& rect) const {
    const int xMin = static_cast<int>(std::floor(rect.position.x / m_tailleCase));
    const int yMin = static_cast<int>(std::floor(rect.position.y / m_tailleCase));
    const int xMax = static_cast<int>(std::ceil((rect.position.x + rect.size.x) / m_tailleCase)) - 1;
    const int yMax = static_cast<int>(std::ceil((rect.position.y + rect.size.y) / m_tailleCase)) - 1;

    for (int y = yMin; y <= yMax; ++y) {
        for (int x = xMin; x <= xMax; ++x) {
            if (getTile(x, y) == Tile::Wall) {
                return true;
            }
        }
    }
    return false;
}