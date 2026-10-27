// ./src/escouade/Agresseurs.hpp
#pragma once
#include <vector>
#include "combat/EntiteVue.hpp"
#include "combat/IdMonstre.hpp"

// Les monstres qui viennent de frapper l'escouade, retenus par leur identifiant jusqu'à ce que
// leur temps restant tombe à zéro. Ne garde aucune position : celle d'un agresseur est lue, au
// moment de s'en servir, dans la liste de monstres que l'appelant tient à jour. Ne connaît ni
// Personnage, ni Monstre, ni Escouade.
class Agresseurs {
public:
    struct Agresseur {
        IdMonstre id;
        float tempsRestant;  // en secondes, remis au maximum à chaque coup reçu
    };

    // Ajoute ce monstre, ou remet son temps au maximum s'il est déjà connu.
    void enregistrer(IdMonstre id);
    // Fait écouler le temps ; oublie les agresseurs échus.
    void mettreAJour(float dt);
    // Oublie les agresseurs absents de la liste (morts, ou plus ciblables).
    void retirerMonstresAbsents(const std::vector<MonstreVu>& monstres);
    // Les agresseurs sous la forme où l'escouade voit un monstre, avec le cercle qu'ils ont dans
    // cette liste ; un agresseur absent de la liste est ignoré. Dans l'ordre d'enregistrement.
    [[nodiscard]] std::vector<MonstreVu> vusParmi(const std::vector<MonstreVu>& monstres) const;
    [[nodiscard]] const std::vector<Agresseur>& liste() const;

private:
    std::vector<Agresseur> m_agresseurs;
};
