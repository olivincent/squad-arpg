// ./tests/test_Escouade.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Escouade.hpp"
#include "TileMap.hpp"

namespace {

// Compare un point à un point attendu avec une tolérance : les positions
// manipulées par Escouade sont des flottants accumulés sur plusieurs tics
// (vitesse * dt, racines carrées de sf::Vector2f::length...), donc deux
// calculs mathématiquement égaux peuvent différer de quelques poussières
// d'erreur d'arrondi. Comparer avec `==` serait fragile.
void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

} // namespace

TEST_CASE("Le centre initial est au centre du carré de départ") {
    const Escouade escouade({50.0f, 50.0f});

    verifierPointProche(escouade.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("Sans ordre, l'escouade ne bouge pas") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);
    Escouade escouade({50.0f, 50.0f});

    for (int i = 0; i < 60; ++i) {
        escouade.mettreAJour(1.0f / 60.0f, carte);
    }

    verifierPointProche(escouade.getCentre(), {60.0f, 60.0f});
}
