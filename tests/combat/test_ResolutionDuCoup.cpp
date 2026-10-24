// ./tests/combat/test_ResolutionDuCoup.cpp
#include <catch2/catch_test_macros.hpp>
#include "combat/Defense.hpp"
#include "combat/ResolutionDuCoup.hpp"

TEST_CASE("resoudreCoup applique l'armure du defenseur") {
    REQUIRE(resoudreCoup(100, {.armure = 30}) == appliquerArmure(100, 30));
    REQUIRE(resoudreCoup(50, {.armure = 0}) == 50);
}

TEST_CASE("resoudreCoup : un coup de 0 reste 0") {
    REQUIRE(resoudreCoup(0, {.armure = 30}) == 0);
}
