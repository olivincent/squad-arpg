// ./tests/test_Eclaircir.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>

#include "Eclaircir.hpp"

TEST_CASE("eclaircir melange la couleur au blanc, sans toucher l'opacite") {
    REQUIRE(eclaircir(sf::Color(255, 0, 0, 200), 0.5f) == sf::Color(255, 128, 128, 200));
}

TEST_CASE("eclaircir : 0 laisse la couleur, 1 donne du blanc") {
    REQUIRE(eclaircir(sf::Color(10, 20, 30), 0.0f) == sf::Color(10, 20, 30));
    REQUIRE(eclaircir(sf::Color(10, 20, 30), 1.0f) == sf::Color::White);
}

TEST_CASE("assombrir melange la couleur au noir, sans toucher l'opacite") {
    REQUIRE(assombrir(sf::Color(200, 100, 50, 200), 0.5f) == sf::Color(100, 50, 25, 200));
}

TEST_CASE("assombrir : 0 laisse la couleur, 1 donne du noir") {
    REQUIRE(assombrir(sf::Color(10, 20, 30), 0.0f) == sf::Color(10, 20, 30));
    REQUIRE(assombrir(sf::Color(10, 20, 30), 1.0f) == sf::Color::Black);
}
