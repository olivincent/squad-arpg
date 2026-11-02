// ./src/rendu/BarreDeVie.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <utility>

// Hauteur de la barre de vie, en pixels.
constexpr float s_hauteurBarreDeVie = 5.0f;

// Coin haut-gauche de la barre de vie d'une entité de ce centre et de cette
// taille (pixels du monde) : juste au-dessus de son carré, avec un écart.
[[nodiscard]] sf::Vector2f positionBarreDeVie(sf::Vector2f centre, sf::Vector2f taille);

// Dessine une barre de vie de cette largeur, dont le coin haut-gauche est à
// `position` : fond gris, puis partie verte proportionnelle à `ratio`
// (de 0, vide, à 1, pleine).
void dessinerBarreDeVie(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratio);

// Un segment de la barre de vie, en fractions de sa largeur (de 0, le bord gauche, à 1, le bord droit).
struct SegmentDeBarre {
    float debut;
    float fin;
};

// Les segments des boucliers d'absorption (étape 114) : Aegis puis Ward, côte à côte. Ils prolongent la partie pleine de la barre
// (à partir de `ratioDeVie`) ; s'ils dépassent le bord droit, ils se calent contre lui et recouvrent la fin de la vie, pour rester
// visibles à pleine vie. Chaque ratio est en fraction des points de vie maximum ; l'ensemble est borné à toute la barre.
[[nodiscard]] std::pair<SegmentDeBarre, SegmentDeBarre> segmentsDAbsorption(float ratioDeVie, float ratioAegis, float ratioWard);

// Dessine ces deux segments sur la barre dont le coin haut-gauche est à `position` (argenté pour l'Aegis, bleu pour le Ward).
// À appeler juste après `dessinerBarreDeVie`.
void dessinerAbsorption(sf::RenderTarget& rendu, sf::Vector2f position, float largeur, float ratioDeVie, float ratioAegis, float ratioWard);
