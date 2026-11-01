// ./tests/combat/test_Energie.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Energie.hpp"

TEST_CASE("energieMax : 100 au niveau 1, 5 de plus par niveau") {
    REQUIRE(energieMax(1, s_energieParNiveau) == 100);
    REQUIRE(energieMax(2, s_energieParNiveau) == 105);
    REQUIRE(energieMax(10, s_energieParNiveau) == 145);
}

TEST_CASE("regenerationDEnergie : 2 points par seconde de base, majores du pourcentage des affixes") {
    REQUIRE_THAT(regenerationDEnergie(0), Catch::Matchers::WithinAbs(2.0, 1e-6));
    REQUIRE_THAT(regenerationDEnergie(15), Catch::Matchers::WithinAbs(2.3, 1e-5));
    REQUIRE_THAT(regenerationDEnergie(100), Catch::Matchers::WithinAbs(4.0, 1e-6));
}

TEST_CASE("energieMax : le gain par niveau de classe est un parametre") {
    REQUIRE(energieMax(1, 20) == 100);
    REQUIRE(energieMax(4, 20) == 160);
    REQUIRE(energieMax(4, 0) == 100);
}
