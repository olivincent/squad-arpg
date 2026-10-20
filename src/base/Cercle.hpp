// ./src/base/Cercle.hpp
#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

struct Cercle {  // NOLINT(cppcoreguidelines-pro-type-member-init): champs sans valeur par défaut exprès, un oubli est signalé par le compilateur
    sf::Vector2f centre;
    float rayon;
    bool operator==(const Cercle&) const = default;
};

// Cercle de collision d'une entité rectangulaire de coin haut-gauche `coin` : centré sur le
// rectangle, de rayon la demi-largeur (les entités sont des carrés). Le seul endroit où cette
// règle est écrite : Mobile::getCercle et le glissement de Deplacement l'appellent.
[[nodiscard]] Cercle cercleDuRectangle(sf::Vector2f coin, sf::Vector2f taille);

[[nodiscard]] float distanceBordABord(Cercle a, Cercle b);

// Distance du point `p` au segment [a, b] (au point le plus proche du segment, extrémités comprises).
[[nodiscard]] float distancePointSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b);

// Écart bord à bord entre un cercle et un rectangle plein : 0 en cas de contact ou de chevauchement, jamais négatif.
[[nodiscard]] float distanceCercleRectangle(Cercle cercle, sf::FloatRect rectangle);
