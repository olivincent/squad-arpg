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
