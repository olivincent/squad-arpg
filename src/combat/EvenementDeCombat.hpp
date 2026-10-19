// ./src/combat/EvenementDeCombat.hpp
#pragma once
#include <SFML/System/Vector2.hpp>

// Ce qui s'est passé pendant une image, en données : le noyau (Partie) les produit, l'affichage
// les lit. Une file d'événements, pas de callback : Partie ne connaît pas ceux qui la lisent.
enum class TypeEvenement { Degats, Soin };

// Le camp de la cible : l'escouade (idCible = place, de 1 à N) ou les monstres (idCible = identifiant).
enum class Camp { Escouade, Monstres };

// `montant` : ce qui a réellement été appliqué (armure comprise, soin plafonné), jamais 0.
// `position` : COPIE du centre de la cible à ce moment ; aucune référence vers l'entité, qui peut
// disparaître dans la même image (monstre tué par ce coup). `opportunite` : coup gratuit.
struct EvenementDeCombat {
    TypeEvenement type;
    Camp campCible;
    int idCible;
    int montant;
    sf::Vector2f position;
    bool opportunite;
};
