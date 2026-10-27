// ./src/escouade/Provocations.hpp
#pragma once
#include <optional>
#include <vector>
#include "combat/EntiteVue.hpp"
#include "combat/IdMonstre.hpp"
#include "combat/Place.hpp"

// Les monstres provoqués : chacun prend le personnage d'une place pour proie
// jusqu'à ce que son temps restant tombe à zéro. Ne connaît ni Personnage,
// ni Monstre, ni Escouade : la mort du provocateur lui est signalée.
class Provocations {
public:
    struct Provocation {
        IdMonstre id;        // du monstre provoqué
        Place place;         // du provocateur
        float tempsRestant;  // en secondes
    };

    // Ajoute la provocation, ou remet la durée au maximum (et change le
    // provocateur) si ce monstre est déjà provoqué.
    void provoquer(IdMonstre id, Place place, float duree);
    // Fait écouler le temps ; retire les provocations échues.
    void mettreAJour(float dt);
    // Retire les monstres absents de la liste (morts).
    void retirerMonstresAbsents(const std::vector<MonstreVu>& monstres);
    // Retire toutes les provocations de ce provocateur (mort).
    void retirerProvocateur(Place place);
    // Place du provocateur de ce monstre, vide s'il n'est pas provoqué.
    [[nodiscard]] std::optional<Place> provocateurDe(IdMonstre id) const;
    [[nodiscard]] const std::vector<Provocation>& liste() const;

private:
    std::vector<Provocation> m_provocations;
};
