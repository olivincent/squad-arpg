// ./tests/aides.hpp
#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "Cercle.hpp"
#include "Cible.hpp"
#include "Coup.hpp"
#include "Escouade.hpp"
#include "MonstreVu.hpp"
#include "TileMap.hpp"
#include "TypeSkill.hpp"

// Aides communes à plusieurs fichiers de tests. Chaque fonction est
// `inline` (définie dans un en-tête). Une aide utilisée par un seul
// fichier reste dans ce fichier, dans un `namespace {}`.

// Compare un point à un point attendu avec une tolérance : les positions
// manipulées par les entités sont des flottants accumulés sur plusieurs tics
// (vitesse * dt, racines carrées de sf::Vector2f::length...), donc deux
// calculs mathématiquement égaux peuvent différer de quelques poussières
// d'erreur d'arrondi. Comparer avec `==` serait fragile.
inline void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

// Cible ponctuelle (rayon nul), pour les tests qui ne portent pas sur
// la forme de la cible elle-même.
inline Cercle pointCible(sf::Vector2f centre) {
    return {centre, 0.0f};
}

// Cible de la taille d'un monstre (rayon 15 px), pour les tests de combat
// où la taille de la cible décide si les mêlées peuvent l'atteindre.
inline Cercle cibleMonstre(sf::Vector2f centre) {
    return {centre, 15.0f};
}

// Carte dégagée de 10 × 10 cases de 40 px, sans mur.
inline TileMap carteDegagee10x10() {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    return TileMap(plan, 40.0f);
}

// Carte dégagée de 800 × 600 px (cases de 40 px), entourée de murs.
inline TileMap carteDegageeEntouree() {
    std::vector<std::string> plan;
    plan.push_back(std::string(20, '#'));
    for (int i = 0; i < 13; ++i) {
        plan.push_back("#" + std::string(18, '.') + "#");
    }
    plan.push_back(std::string(20, '#'));
    return TileMap(plan, 40.0f);
}

// Grande carte sans mur (4000 × 4000 px) pour les tests qui ne portent pas sur la vue.
inline const TileMap& carteOuverte() {
    static const TileMap carte(std::vector<std::string>(100, std::string(100, '.')), 40.0f);
    return carte;
}

// Simule le temps qui passe par tics de 1/60 s (comme setFramerateLimit(60)
// dans main.cpp), pendant la durée demandée. Les tests passent une durée
// généreuse par rapport au trajet attendu et ne vérifient que l'état final.
inline void simuler(Escouade& escouade, const TileMap& carte, float dureeSecondes) {
    constexpr float dt = 1.0f / 60.0f;
    const int nombreTics = static_cast<int>(dureeSecondes / dt);
    for (int i = 0; i < nombreTics; ++i) {
        escouade.mettreAJour(dt, carte, {});
    }
}

// Appelle attaquer() avec un seul monstre (identifiant 1) et renvoie le
// total des dégâts de la liste de coups.
inline int attaquerCible(Escouade& escouade, const TileMap& carte, Cercle cible) {
    int total = 0;
    for (const Coup& coup : escouade.attaquer({MonstreVu{1, cible}}, carte)) {
        total += coup.degats;
    }
    return total;
}

// Place de l'allié désigné par choisirCible, vide si aucune cible.
inline std::optional<std::size_t> placeAlliePourSoin(const Escouade& escouade) {
    const std::optional<Cible> cible = escouade.choisirCible(TypeSkill::Soin, 3, {}, carteOuverte());
    if (!cible.has_value()) {
        return std::nullopt;
    }
    return std::get<CibleAllie>(*cible).place;
}
