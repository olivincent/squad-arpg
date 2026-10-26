// ./src/monde/TileMap.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

enum class Tile {
    Floor,
    Wall,
    PorteFermee,
    PorteOuverte,
    Escalier  // du sol qu'on peut prendre pour descendre d'un étage ; ne bloque ni le déplacement ni la vue
};

// Les caractères d'un plan (`TileMap(plan, ...)`) : le mur et l'escalier ; tout autre caractère est du sol.
inline constexpr char s_caractereDeMur = '#';
inline constexpr char s_caractereDEscalier = '>';

// Une case bloquante arrête le déplacement et la vue (mur ou porte fermée).
[[nodiscard]] bool estBloquante(Tile tile);

class TileMap {
public:
    // `plan` : une chaîne par ligne, toutes de la même longueur ; `s_caractereDeMur` = mur, `s_caractereDEscalier` = escalier
    // (au plus un), tout autre caractère = sol.
    TileMap(const std::vector<std::string>& plan, float tailleCase);
    [[nodiscard]] Tile getTile(int x, int y) const;
    [[nodiscard]] int getLargeur() const;
    [[nodiscard]] int getHauteur() const;
    [[nodiscard]] float getTailleCase() const;
    // La case de l'escalier, s'il y en a un.
    [[nodiscard]] std::optional<sf::Vector2i> caseDeLEscalier() const;
    [[nodiscard]] sf::Vector2i caseContenant(sf::Vector2f point) const;
    [[nodiscard]] sf::Vector2f centreDeCase(sf::Vector2i coordCase) const;
    void dessiner(sf::RenderTarget& rendu) const;
    void placerPorte(sf::Vector2i coordCase);   // Floor -> PorteFermee
    void ouvrirPorte(sf::Vector2i coordCase);   // PorteFermee -> PorteOuverte
    [[nodiscard]] bool chevaucheUnMur(const sf::FloatRect& rect) const;
    [[nodiscard]] bool trajetLibre(const sf::FloatRect& rect, sf::Vector2f deplacement) const;

private:
    [[nodiscard]] int indice(int x, int y) const;
    int m_largeur;          // nombre de cases en X
    int m_hauteur;          // nombre de cases en Y
    float m_tailleCase;     // taille d'une case en pixels
    std::vector<Tile> m_cases;  // largeur * hauteur cases, indice = y * largeur + x
    std::optional<sf::Vector2i> m_escalier;
};