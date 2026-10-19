// ./src/escouade/Provocations.hpp
#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "combat/EntiteVue.hpp"

// Les monstres provoqués : chacun prend le personnage d'une place pour proie
// jusqu'à ce que son temps restant tombe à zéro. Ne connaît ni Personnage,
// ni Monstre, ni Escouade : la mort du provocateur lui est signalée.
class Provocations {
public:
    struct Provocation {
        int id;              // du monstre provoqué
        std::size_t place;   // du provocateur, de 1 à N
        float tempsRestant;  // en secondes
    };

    // Ajoute la provocation, ou remet la durée au maximum (et change le
    // provocateur) si ce monstre est déjà provoqué.
    void provoquer(int id, std::size_t place, float duree);
    // Fait écouler le temps ; retire les provocations échues.
    void mettreAJour(float dt);
    // Retire les monstres absents de la liste (morts).
    void retirerMonstresAbsents(const std::vector<EntiteVue>& monstres);
    // Retire toutes les provocations de ce provocateur (mort).
    void retirerProvocateur(std::size_t place);
    // Place du provocateur de ce monstre, vide s'il n'est pas provoqué.
    [[nodiscard]] std::optional<std::size_t> provocateurDe(int id) const;
    [[nodiscard]] const std::vector<Provocation>& liste() const;

private:
    std::vector<Provocation> m_provocations;
};
