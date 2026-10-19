// ./src/monde/ObjetInteractif.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <variant>
#include <vector>

class TileMap;

// Une porte : les cases de la carte qu'elle ferme (alignées, sur une même ligne ou une même colonne).
struct Porte {
    std::vector<sf::Vector2i> cases;
};

// Un objet avec lequel le joueur interagit : une partie commune et une partie
// propre à sa nature (une seule alternative pour l'instant). Que des données :
// l'effet de l'interaction dépend de la Partie, qui visite `nature`.
struct ObjetInteractif {
    // `tailleCase` sert à calculer la zone, rectangle (en pixels du monde) qui couvre les cases de la porte.
    ObjetInteractif(int identifiant, Porte porte, float tailleCase);

    int id;
    sf::FloatRect zone;
    std::variant<Porte> nature;
};

// Point (pixels du monde) où l'escouade se place devant la porte : le milieu des centres des cases
// d'un des deux côtés, le côté libre le plus proche de `depuis`. Vide si aucun côté n'est libre.
// Porte horizontale (cases de même y) : côtés y - 1 et y + 1 ; verticale : x - 1 et x + 1.
[[nodiscard]] std::optional<sf::Vector2f> pointDApproche(const Porte& porte, const TileMap& carte, sf::Vector2f depuis);
