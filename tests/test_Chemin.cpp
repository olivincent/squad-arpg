// ./tests/test_Chemin.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Chemin.hpp"
#include "TileMap.hpp"

TEST_CASE("Le chemin entre une case de départ et d'arrivée identiques est cette seule case") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    const std::vector<sf::Vector2i> chemin = trouverChemin(carte, {1, 1}, {1, 1});

    REQUIRE(chemin.size() == 1);
    REQUIRE(chemin[0] == sf::Vector2i(1, 1));
}

TEST_CASE("Le chemin est vide si le départ est un mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {1, 1}, {0, 0}).empty());
}

TEST_CASE("Le chemin est vide si l'arrivée est un mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {1, 1}).empty());
}

TEST_CASE("Le chemin est vide si le départ est hors carte") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {-1, 0}, {0, 0}).empty());
}

TEST_CASE("Le chemin est vide si l'arrivée est hors carte") {
    const std::vector<std::string> plan = {
        "...",
        "...",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {0, 0}, {3, 0}).empty());
}

TEST_CASE("Le chemin est vide quand départ et arrivée sont la même case murée") {
    // Ce cas doit l'emporter sur le raccourci « départ == arrivée » : être
    // sur la même case ne suffit pas si cette case est un mur.
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(trouverChemin(carte, {1, 1}, {1, 1}).empty());
}
