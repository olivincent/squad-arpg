// ./src/Niveau.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

// Armure des monstres, commune à la carte faite à la main et aux cartes générées.
inline constexpr int s_armureMonstresParDefaut = 20;

// Un monstre du niveau : où il commence et à quel groupe il appartient.
struct MonstreDuNiveau {
    sf::Vector2f position; // centre, en pixels du monde
    int groupe;            // identifiant opaque : même valeur = « vont ensemble »

    bool operator==(const MonstreDuNiveau&) const = default;
};

// Les données d'un niveau, sans comportement : de quoi construire une Partie.
struct Niveau {
    std::vector<std::string> plan;       // '#' = mur, tout autre caractère = sol
    float tailleCase;                    // en pixels
    sf::Vector2f departEscouade;         // centre de l'ancre, en pixels du monde
    std::vector<MonstreDuNiveau> monstres;
    int armureMonstres;                  // armure de chaque monstre
};

// Le niveau du jeu.
Niveau creerNiveau();
