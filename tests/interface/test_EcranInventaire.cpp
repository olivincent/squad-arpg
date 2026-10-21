// ./tests/interface/test_EcranInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <variant>

#include "interface/EcranInventaire.hpp"

TEST_CASE("inventaire : I et Echap le ferment") {
    const EcranInventaire ecran;

    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::I)));
    REQUIRE(std::holds_alternative<Depiler>(ecran.appuyer(sf::Keyboard::Scan::Escape)));
}

TEST_CASE("inventaire : les autres touches sont ignorees") {
    const EcranInventaire ecran;

    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::Q)));
    REQUIRE(std::holds_alternative<Aucune>(ecran.appuyer(sf::Keyboard::Scan::F5)));
}

TEST_CASE("inventaire : laisse voir la partie dessous") {
    const EcranInventaire ecran;

    REQUIRE(ecran.laisseVoirDessous());
}
