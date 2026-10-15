// ./src/Chemin.hpp
#pragma once
#include "Cercle.hpp"
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <vector>

class TileMap;

// Coût ajouté pour entrer dans une case (en plus du coût normal de 1 par pas).
// Une fonction vide veut dire : aucun coût ajouté.
using FonctionDeCout = std::function<int(sf::Vector2i)>;

// Les cases de départ et d'arrivée ne sont jamais pénalisées (`cout` n'est pas appelée pour elles).
[[nodiscard]] std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee,
                                                      const FonctionDeCout& cout = {});

// Construit la fonction de coût : `penalite` pour une case dont le rectangle touche (chevauche)
// l'un des `obstacles` agrandi de `rayonMobile`, 0 ailleurs (et hors de la carte). Les cases
// occupées sont calculées ici, une fois ; la fonction renvoyée en garde sa propre copie.
// Fonction vide si `obstacles` est vide.
[[nodiscard]] FonctionDeCout coutDesObstacles(const TileMap& carte, const std::vector<Cercle>& obstacles,
                                              float rayonMobile, int penalite);

// Le segment [a, b] passe-t-il à moins de (rayon de l'obstacle + rayonMobile) du centre d'un des
// obstacles ? Le test des raccourcis du lissage, et de la ligne droite d'un personnage.
[[nodiscard]] bool traverseUnObstacle(sf::Vector2f a, sf::Vector2f b, const std::vector<Cercle>& obstacles,
                                      float rayonMobile);

// Un raccourci est refusé s'il passe à moins de (rayon de l'obstacle + rayon du mobile,
// moitié de `taille`) du centre d'un des `aContourner`.
[[nodiscard]] std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart,
                                        const std::vector<sf::Vector2f>& points, sf::Vector2f taille,
                                        const std::vector<Cercle>& aContourner = {});

[[nodiscard]] sf::Vector2f replierVersOrigine(const TileMap& carte, sf::Vector2f origine, sf::Vector2f cible,
                                sf::Vector2f taille);
