// ./tests/test_ObjetInteractif.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "ObjetInteractif.hpp"
#include "TileMap.hpp"

// Plan de 7 x 5 cases de 40 px : une porte horizontale de 2 cases en (2..3, 2),
// entre un côté nord (rangée 1) et un côté sud (rangée 3).
namespace {
const std::vector<std::string> s_plan = {
    "#######",
    ".......",
    ".......",
    ".......",
    "#######",
};
const Porte s_porte{{{2, 2}, {3, 2}}};
}  // namespace

TEST_CASE("ObjetInteractif : la zone couvre toutes les cases de la porte") {
    const ObjetInteractif objet(1, s_porte, 40.0f);
    REQUIRE(objet.id == 1);
    REQUIRE(objet.zone.position == sf::Vector2f(80.0f, 80.0f));
    REQUIRE(objet.zone.size == sf::Vector2f(80.0f, 40.0f));
}

TEST_CASE("pointDApproche : le cote le plus proche de depuis") {
    const TileMap carte(s_plan, 40.0f);
    // Milieu des centres des cases (2, 1) et (3, 1), ou (2, 3) et (3, 3).
    verifierPointProche(*pointDApproche(s_porte, carte, {100.0f, 20.0f}), {120.0f, 60.0f});
    verifierPointProche(*pointDApproche(s_porte, carte, {100.0f, 180.0f}), {120.0f, 140.0f});
}

TEST_CASE("pointDApproche : une porte verticale a ses cotes a gauche et a droite") {
    const TileMap carte(s_plan, 40.0f);
    const Porte verticale{{{3, 1}, {3, 2}}};
    verifierPointProche(*pointDApproche(verticale, carte, {0.0f, 80.0f}), {100.0f, 80.0f});
    verifierPointProche(*pointDApproche(verticale, carte, {280.0f, 80.0f}), {180.0f, 80.0f});
}

TEST_CASE("pointDApproche : un cote bloque est ecarte") {
    // Un mur en (2, 1) bloque le côté nord, même si depuis en est tout près.
    const std::vector<std::string> plan = {
        "#######",
        "..#....",
        ".......",
        ".......",
        "#######",
    };
    const TileMap carte(plan, 40.0f);
    verifierPointProche(*pointDApproche(s_porte, carte, {100.0f, 20.0f}), {120.0f, 140.0f});
}

TEST_CASE("pointDApproche : vide quand aucun cote n'est libre") {
    const std::vector<std::string> plan = {
        "#######",
        "..#....",
        ".......",
        "...#...",
        "#######",
    };
    const TileMap carte(plan, 40.0f);
    REQUIRE_FALSE(pointDApproche(s_porte, carte, {100.0f, 100.0f}).has_value());
}
