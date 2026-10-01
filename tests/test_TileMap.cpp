// ./tests/test_TileMap.cpp
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "TileMap.hpp"

TEST_CASE("getTile distingue une case de sol d'une case de mur") {
    const std::vector<std::string> plan = {
        "...",
        ".#.",
        "...",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.getTile(0, 0) == Tile::Floor);
    REQUIRE(carte.getTile(1, 1) == Tile::Wall);
}

TEST_CASE("Les dimensions de la carte sont déduites du plan") {
    // Bord en sol (pas en mur) : sinon une case de bord et une case hors
    // carte renverraient toutes les deux Wall, et ce test ne prouverait
    // rien sur les dimensions réelles de la carte.
    const std::vector<std::string> plan = {
        "....",
        "....",
        "....",
        "....",
    };
    TileMap carte(plan, 40.0f);

    // Dernière case valide : (largeur - 1, hauteur - 1) = (3, 3)
    REQUIRE(carte.getTile(3, 3) == Tile::Floor);
    // Juste au-delà en X et en Y : hors carte, donc mur
    REQUIRE(carte.getTile(4, 3) == Tile::Wall);
    REQUIRE(carte.getTile(3, 4) == Tile::Wall);
}

TEST_CASE("Une coordonnée négative est hors carte, donc un mur") {
    const std::vector<std::string> plan = {
        "....",
        "....",
        "....",
        "....",
    };
    TileMap carte(plan, 40.0f);

    REQUIRE(carte.getTile(-1, -1) == Tile::Wall);
}

TEST_CASE("Un plan vide lève std::out_of_range") {
    const std::vector<std::string> plan = {};

    REQUIRE_THROWS_AS(TileMap(plan, 40.0f), std::out_of_range);
}

TEST_CASE("Des lignes de longueurs différentes lèvent std::invalid_argument") {
    const std::vector<std::string> plan = {
        "....",
        "...",
        "....",
    };

    REQUIRE_THROWS_AS(TileMap(plan, 40.0f), std::invalid_argument);
}
