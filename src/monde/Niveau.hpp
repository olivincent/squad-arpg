// ./src/monde/Niveau.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include "monde/CategorieDeMonstre.hpp"

// Un monstre du niveau : où il commence, à quel groupe il appartient et sa catégorie (ses statistiques sont
// celles du catalogue de monstres). Sans catégorie précisée, c'est l'infanterie, le monstre de base.
struct MonstreDuNiveau {  // NOLINT(cppcoreguidelines-pro-type-member-init): position et groupe sans valeur par défaut exprès, un oubli est signalé par le compilateur
    sf::Vector2f position; // centre, en pixels du monde
    int groupe;            // identifiant opaque : même valeur = « vont ensemble »
    CategorieDeMonstre categorie = CategorieDeMonstre::Infanterie;

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
    std::vector<std::string> plan;       // '#' = mur, '>' = escalier (au plus un), tout autre caractère = sol
    float tailleCase;                    // en pixels
    sf::Vector2f departEscouade;         // centre de l'ancre, en pixels du monde
    std::vector<MonstreDuNiveau> monstres;
    std::vector<PorteDuNiveau> portes = {};
};

// Le niveau du jeu.
[[nodiscard]] Niveau creerNiveau();
