// ./src/TileMap.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

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
    [[nodiscard]] float getTailleCase() const;
    [[nodiscard]] sf::Vector2i caseContenant(sf::Vector2f point) const;
    [[nodiscard]] sf::Vector2f centreDeCase(sf::Vector2i coordCase) const;
    void dessiner(sf::RenderTarget& rendu) const;
    [[nodiscard]] bool chevaucheUnMur(const sf::FloatRect& rect) const;
    [[nodiscard]] bool trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const;
    [[nodiscard]] bool ligneDeVue(sf::Vector2f a, sf::Vector2f b) const;

private:
    int indice(int x, int y) const;
    int m_largeur;          // nombre de cases en X
    int m_hauteur;          // nombre de cases en Y
    float m_tailleCase;     // taille d'une case en pixels
    std::vector<Tile> m_cases;  // largeur * hauteur cases, indice = y * largeur + x
};