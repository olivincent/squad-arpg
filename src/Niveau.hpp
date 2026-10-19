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

// Une porte du niveau : les cases du plan (du sol) qu'elle ferme, alignées. Le plan
// reste de la géométrie pure : la Partie ferme ces cases à la construction.
struct PorteDuNiveau {
    std::vector<sf::Vector2i> cases;

    bool operator==(const PorteDuNiveau&) const = default;
};

// Les données d'un niveau, sans comportement : de quoi construire une Partie.
struct Niveau {
    std::vector<std::string> plan;       // '#' = mur, tout autre caractère = sol
    float tailleCase;                    // en pixels
    sf::Vector2f departEscouade;         // centre de l'ancre, en pixels du monde
    std::vector<MonstreDuNiveau> monstres;
    int armureMonstres = 0;              // armure de chaque monstre (le jeu donne la sienne)
    std::vector<PorteDuNiveau> portes = {};
};

// Le niveau du jeu.
[[nodiscard]] Niveau creerNiveau();
