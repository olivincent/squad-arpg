// ./src/Chemin.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

class TileMap;

std::vector<sf::Vector2i> trouverChemin(const TileMap& carte, sf::Vector2i depart, sf::Vector2i arrivee);

std::vector<sf::Vector2f> lisserChemin(const TileMap& carte, sf::Vector2f depart,
                                        const std::vector<sf::Vector2f>& points, sf::Vector2f taille);

sf::Vector2f replierVersOrigine(const TileMap& carte, sf::Vector2f origine, sf::Vector2f cible,
                                sf::Vector2f taille);
