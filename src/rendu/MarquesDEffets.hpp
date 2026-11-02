// ./src/rendu/MarquesDEffets.hpp
#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

// Côté d'une marque d'effet actif et écart entre deux marques, en pixels. Provisoire : des icônes dessinées les remplaceront.
constexpr float s_coteDUneMarque = 4.0f;
constexpr float s_ecartEntreMarques = 2.0f;

// Coin haut-gauche de la rangée de marques d'une entité de ce centre et de cette taille (pixels du monde) : juste sous son carré,
// alignée sur son bord gauche.
[[nodiscard]] sf::Vector2f positionMarquesDEffets(sf::Vector2f centre, sf::Vector2f taille);

// Côté d'un point des chiffres du nombre de piles (un chiffre fait 3 × 5 points, collés les uns aux autres), en pixels.
constexpr float s_cotePointDeChiffre = 1.5f;

// Les points allumés d'un chiffre de 0 à 9 dans sa grille de 3 colonnes sur 5 lignes (x de 0 à 2, y de 0 à 4, de haut en bas), dans
// l'ordre de lecture. Pas de police à l'écran pour ce nombre : les entités se dessinent sans elle, avec des rectangles. Précondition :
// 0 <= chiffre <= 9.
[[nodiscard]] std::vector<sf::Vector2i> pointsDuChiffre(int chiffre);

// Dessine une marque par effet actif à partir de `position`, de gauche à droite : d'abord une marque verte par élément de `pilesDesBonus`,
// puis une rouge par élément de `pilesDesMalus`. Le nombre de piles de l'effet (l'élément) suit sa marque, en chiffres de la même couleur,
// quand il dépasse 1. Rien si les deux sont vides.
void dessinerMarquesDEffets(sf::RenderTarget& rendu, sf::Vector2f position, const std::vector<int>& pilesDesBonus,
                            const std::vector<int>& pilesDesMalus);
