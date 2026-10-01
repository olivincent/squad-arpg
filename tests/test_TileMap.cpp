// ./tests/test_TileMap.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
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

TEST_CASE("chevaucheUnMur renvoie faux pour un rectangle entièrement sur du sol") {
    // Mur unique en (2, 2), donc en pixels x dans [80, 120), y dans [80, 120)
    // pour une taille de case de 40.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {40.0f, 40.0f});
    REQUIRE_FALSE(carte.chevaucheUnMur(rect));
}

TEST_CASE("chevaucheUnMur renvoie vrai pour un rectangle qui chevauche un mur") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({90.0f, 90.0f}, {20.0f, 20.0f});
    REQUIRE(carte.chevaucheUnMur(rect));
}

TEST_CASE("Un rectangle qui touche exactement le bord d'un mur ne le chevauche pas") {
    // Bord droit du rectangle en x = 80, exactement sur le bord gauche du
    // mur (qui commence à x = 80) : convention ceil - 1, un contact sans
    // chevauchement ne doit pas compter comme une collision.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({40.0f, 80.0f}, {40.0f, 40.0f});
    REQUIRE_FALSE(carte.chevaucheUnMur(rect));
}

TEST_CASE("Le même rectangle décalé de 1 px vers le mur le chevauche") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({41.0f, 80.0f}, {40.0f, 40.0f});
    REQUIRE(carte.chevaucheUnMur(rect));
}

TEST_CASE("Un rectangle hors carte en coordonnées négatives chevauche toujours un mur") {
    // Bord en sol : seule la sortie de carte doit déclencher la collision,
    // pas un mur du plan.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({-10.0f, -10.0f}, {20.0f, 20.0f});
    REQUIRE(carte.chevaucheUnMur(rect));
}
