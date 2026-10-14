// ./src/Niveau.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

// Les données d'un niveau, sans comportement : de quoi construire une Partie.
struct Niveau {
    std::vector<std::string> plan;               // '#' = mur, tout autre caractère = sol
    float tailleCase;                            // en pixels
    sf::Vector2f departEscouade;                 // centre de l'ancre, en pixels du monde
    std::vector<sf::Vector2f> positionsMonstres; // centre de chaque monstre, en pixels du monde
    int armureMonstres;                          // armure de chaque monstre
};

// Le niveau du jeu.
Niveau creerNiveau();
