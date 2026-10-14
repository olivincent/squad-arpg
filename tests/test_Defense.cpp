// ./tests/test_Defense.cpp
#include <catch2/catch_test_macros.hpp>

#include "Defense.hpp"

TEST_CASE("Defense sans armure laisse le montant inchange") {
    REQUIRE(appliquerArmure(10, 0) == 10);
    REQUIRE(appliquerArmure(1, 0) == 1);
    REQUIRE(appliquerArmure(137, 0) == 137);
}

TEST_CASE("Defense 100 d'armure divise les degats par deux") {
    REQUIRE(appliquerArmure(10, 100) == 5);
    REQUIRE(appliquerArmure(50, 100) == 25);
}

TEST_CASE("Defense arrondit au plus proche") {
    REQUIRE(appliquerArmure(10, 50) == 7);   // 6,67
    REQUIRE(appliquerArmure(5, 20) == 4);    // 4,17
    REQUIRE(appliquerArmure(15, 20) == 13);  // 12,5
    REQUIRE(appliquerArmure(25, 20) == 21);  // 20,83
}

TEST_CASE("Defense inflige toujours au moins 1 pour un coup positif") {
    REQUIRE(appliquerArmure(1, 50) == 1);
    REQUIRE(appliquerArmure(1, 1000000) == 1);
    REQUIRE(appliquerArmure(2, 1000) == 1);
}

TEST_CASE("Defense un montant nul reste nul") {
    REQUIRE(appliquerArmure(0, 0) == 0);
    REQUIRE(appliquerArmure(0, 50) == 0);
}

TEST_CASE("Defense la reduction n'atteint jamais 100 pour cent") {
    REQUIRE(appliquerArmure(1000, 100000) >= 1);
    REQUIRE(appliquerArmure(1000, 100000) < 1000);
    REQUIRE(appliquerArmure(1000000, 100000) >= 1);
}
