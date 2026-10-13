// ./src/Agresseurs.hpp
#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "MonstreVu.hpp"

// Les monstres qui viennent de frapper l'escouade, retenus jusqu'à ce que
// leur temps restant tombe à zéro. Ne connaît ni Personnage, ni Monstre, ni
// Escouade.
class Agresseurs {
public:
    struct Agresseur {
        MonstreVu monstre;   // cercle à jour à chaque actualiser() ; centre seul (rayon nul) à l'enregistrement
        float tempsRestant;  // en secondes, remis au maximum à chaque coup reçu
    };

    // Ajoute ce monstre, ou remet son temps au maximum et son centre à
    // l'origine du coup s'il est déjà connu.
    void enregistrer(int id, sf::Vector2f origine);
    // Fait écouler le temps ; oublie les agresseurs échus.
    void mettreAJour(float dt);
    // Oublie les agresseurs absents de la liste (morts) et met à jour le cercle des autres.
    void actualiser(const std::vector<MonstreVu>& monstres);
    // Les agresseurs sous la forme où l'escouade voit un monstre.
    [[nodiscard]] std::vector<MonstreVu> monstresVus() const;
    [[nodiscard]] const std::vector<Agresseur>& liste() const;

private:
    std::vector<Agresseur> m_agresseurs;
};
