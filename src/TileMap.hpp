// ./src/TileMap.hpp
#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>

enum class Tile {
    Floor,
    Wall
};

class TileMap {
public:
    TileMap(const std::vector<std::string>& plan, float tailleCase);
    Tile getTile(int x, int y) const;
    int getLargeur() const;
    int getHauteur() const;
    sf::Vector2i caseContenant(sf::Vector2f point) const;
    sf::Vector2f centreDeCase(sf::Vector2i coordCase) const;
    void dessiner(sf::RenderTarget& rendu) const;
    bool chevaucheUnMur(const sf::FloatRect& rect) const;

private:
    int indice(int x, int y) const;
    int m_largeur;          // nombre de cases en X
    int m_hauteur;          // nombre de cases en Y
    float m_tailleCase;     // taille d'une case en pixels
    std::vector<Tile> m_cases;  // largeur * hauteur cases, indice = y * largeur + x
};