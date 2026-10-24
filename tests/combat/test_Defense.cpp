// ./tests/combat/test_Defense.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Defense.hpp"

TEST_CASE("Defense sans armure laisse le montant inchange") {
    REQUIRE(appliquerArmure(10, 0) == 10);
    REQUIRE(appliquerArmure(1, 0) == 1);
    REQUIRE(appliquerArmure(137, 0) == 137);
}

TEST_CASE("Defense au-dessus du plancher : l'armure retire un montant fixe") {
    REQUIRE(appliquerArmure(100, 30) == 70);
    REQUIRE(appliquerArmure(50, 10) == 40);
    REQUIRE(appliquerArmure(100, 79) == 21);
}

TEST_CASE("Defense au plancher : le coup garde 20 pour cent") {
    REQUIRE(appliquerArmure(100, 80) == 20);
    REQUIRE(appliquerArmure(100, 500) == 20);
    REQUIRE(appliquerArmure(50, 100000) == 10);
}

TEST_CASE("Defense : le plancher est arrondi au plus proche") {
    REQUIRE(appliquerArmure(12, 1000) == 2);   // 2,4
    REQUIRE(appliquerArmure(13, 1000) == 3);   // 2,6
}

TEST_CASE("Defense : un coup qui passe inflige au moins 1") {
    REQUIRE(appliquerArmure(1, 50) == 1);
    REQUIRE(appliquerArmure(1, 1000000) == 1);
    REQUIRE(appliquerArmure(2, 1000) == 1);  // 0,4 -> 1
}

TEST_CASE("Defense : un coup de 0 reste 0") {
    REQUIRE(appliquerArmure(0, 0) == 0);
    REQUIRE(appliquerArmure(0, 50) == 0);
}

TEST_CASE("Defense : jamais plus que le coup, jamais moins que le plancher") {
    for (int armure = 0; armure <= 300; armure += 7) {
        const int subis = appliquerArmure(100, armure);
        REQUIRE(subis <= 100);
        REQUIRE(subis >= 20);
    }
}
