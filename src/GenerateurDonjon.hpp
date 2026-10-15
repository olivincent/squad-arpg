// ./src/GenerateurDonjon.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include "Aleatoire.hpp"
#include "Niveau.hpp"

// L'emprise d'une salle : un rectangle en cases. Elle sert au placement, au
// test de chevauchement et au centre ; la forme réellement creusée dans cette
// emprise est l'affaire de `creuserSalle`.
struct Salle {
    int x;       // colonne de la case en haut à gauche
    int y;       // ligne de la case en haut à gauche
    int largeur; // en cases
    int hauteur; // en cases

    // Case au centre de l'emprise (coordonnées de grille, arrondie vers le haut-gauche).
    [[nodiscard]] sf::Vector2i centre() const;

    // Les deux emprises sont-elles à moins de `marge` cases l'une de l'autre
    // (ou se recouvrent-elles) ? Avec une marge de 0, deux salles qui se
    // touchent par un bord ne se chevauchent pas.
    [[nodiscard]] bool chevauche(const Salle& autre, int marge) const;
};

struct ParametresDonjon {
    int largeurCarte = 59;  // en cases
    int hauteurCarte = 40;  // en cases
    float tailleCase = 40.0f; // en pixels
    int tentatives = 30;
    int coteMin = 6;        // côté minimal d'une salle, en cases
    int coteMax = 12;       // côté maximal d'une salle, en cases
    int marge = 1;          // cases de mur exigées entre deux salles
    int largeurCouloir = 2; // en cases
};

// Une liaison entre deux salles, désignées par leur indice dans la liste des
// salles : `de` est une salle déjà reliée, `vers` la salle qu'on y rattache.
struct Liaison {
    std::size_t de;
    std::size_t vers;
};

// Par où commence un couloir en L : par le tronçon horizontal ou par le vertical.
enum class SensCouloir { HorizontalPuisVertical, VerticalPuisHorizontal };

// Tire des salles au hasard et rejette celles qui chevauchent une salle déjà
// gardée. Les emprises ne touchent jamais le bord de la carte. La première
// tentative réussit toujours : le résultat contient au moins une salle.
[[nodiscard]] std::vector<Salle> placerSalles(Generateur& generateur, const ParametresDonjon& parametres);

// Creuse la salle dans le plan : ici, toute l'emprise devient du sol.
void creuserSalle(std::vector<std::string>& plan, const Salle& salle);

// Choisit quelles salles relier : la salle 0 est la racine, et chaque salle
// i >= 1 est reliée à la plus proche (distance entre centres) parmi les salles
// 0 à i - 1 ; égalité : le plus petit indice. Renvoie n - 1 liaisons, sans
// aléatoire, dans l'ordre de `vers` croissant.
[[nodiscard]] std::vector<Liaison> choisirLiaisons(const std::vector<Salle>& salles);

// Creuse dans le plan un couloir en L de `largeur` cases entre deux cases :
// le tronçon de départ longe la ligne (ou la colonne) de `depart`, l'autre
// celle d'`arrivee`, et la largeur s'étend vers le bas et vers la droite.
// Le coude est plein ; `depart` et `arrivee` deviennent du sol.
void creuserCouloir(std::vector<std::string>& plan, sf::Vector2i depart, sf::Vector2i arrivee, SensCouloir sens,
                    int largeur);

// Niveau fait de salles reliées par des couloirs en L, sans monstre ;
// l'escouade part au centre de la première salle placée.
[[nodiscard]] Niveau genererNiveau(Generateur& generateur, const ParametresDonjon& parametres);
