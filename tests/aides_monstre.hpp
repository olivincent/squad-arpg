// ./tests/aides_monstre.hpp
#pragma once

#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "base/Aleatoire.hpp"
#include "base/Cercle.hpp"
#include "combat/EntiteVue.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/Monstre.hpp"
#include "monde/TileMap.hpp"

// Aides communes aux tests de Monstre (test_Monstre*.cpp, étape 80). Chaque fonction est
// `inline` (définie dans un en-tête).

// Dégagée, mais assez longue pour les tests de territoire (une proie à
// 1000 px du poste, par exemple).
inline TileMap carteLongue() {
    const std::vector<std::string> plan = {
        "..............................",
        "..............................",
        "..............................",
        "..............................",
        "..............................",
    };
    return TileMap(plan, 40.0f);
}

// 10 × 10 cases de 40 px, avec un mur de 4 cases (colonne 4, lignes 0 à 3) :
// un monstre en (100, 100) ne voit pas (250, 100), mais voit (200, 260).
inline TileMap carteAvecMur() {
    std::vector<std::string> plan(10, "..........");
    for (int ligne = 0; ligne < 4; ++ligne) {
        plan[static_cast<std::size_t>(ligne)][4] = '#';
    }
    return TileMap(plan, 40.0f);
}

// Simule le temps qui passe par tics de 1/60 s, comme dans main.cpp.
// La cible est à la fois la proie (place 1) et l'unique candidat de frappe.
inline void ecoulerTemps(Monstre& monstre, float duree, std::optional<Cercle> cible, const TileMap& carte,
                  const std::vector<Cercle>& obstacles = {}, const std::vector<Cercle>& aContourner = {}) {
    std::optional<EntiteVue> proie;
    std::vector<EntiteVue> candidats;
    if (cible) {
        proie = EntiteVue{1, *cible};
        candidats.push_back(*proie);
    }
    const int nombreTics = nombreDeTics(duree);
    for (int i = 0; i < nombreTics; ++i) {
        monstre.mettreAJour(s_dureeTic, proie, candidats, carte, obstacles, aContourner);
    }
}

// Engage un coup sur cette cible, à la fois proie et unique candidat.
inline bool engager(Monstre& monstre, Cercle cible, int id, const TileMap& carte) {
    const EntiteVue entite{id, cible};
    return monstre.engagerAttaque(entite, {entite}, carte);
}

// Frappe en un seul appel, comme l'ancien frapper() : engage le coup sur la
// cible (refusée hors de portée), puis avance l'exécution (0,3 s) jusqu'au
// départ. Renvoie les dégâts, 0 si rien ne part.
inline int frapperEnEntier(Monstre& monstre, Cercle cible, Generateur& generateur) {
    if (!engager(monstre, cible, 1, carteOuverte())) {
        return 0;
    }
    for (int i = 0; i < nombreDeTics(1.0f); ++i) {
        if (const int degats = degatsOuZero(monstre.avancerAttaque(s_dureeTic, generateur)); degats > 0) {
            return degats;
        }
    }
    return 0;
}

// Amène le monstre en retour, sorti du territoire (plus de `definitionDeMonstre(CategorieDeMonstre::Infanterie).rayonTerritoire`
// de son poste) : la proie lointaine à 800 px de son poste n'est jamais détectée.
// `degatsEnPoursuite` est infligé avant le retour (en retour, le monstre est invulnérable).
inline void amenerEnRetourHorsTerritoire(Monstre& monstre, const TileMap& carte, int degatsEnPoursuite = 0) {
    ecoulerTemps(monstre, 0.2f, pointCible({250.0f, 100.0f}), carte); // détection
    subir(monstre, degatsEnPoursuite);
    while ((monstre.getCentre() - sf::Vector2f(100.0f, 100.0f)).length() <= definitionDeMonstre(CategorieDeMonstre::Infanterie).rayonTerritoire) {
        ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte);
    }
    ecoulerTemps(monstre, 1.0f / 60.0f, pointCible({900.0f, 100.0f}), carte); // bascule en retour
}

