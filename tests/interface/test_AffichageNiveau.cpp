// ./tests/interface/test_AffichageNiveau.cpp
#include <catch2/catch_test_macros.hpp>

#include "interface/AffichageNiveau.hpp"

TEST_CASE("ligneDeNiveau : le mot Level et le niveau, en anglais") {
    REQUIRE(ligneDeNiveau(1) == "Level 1");
    REQUIRE(ligneDeNiveau(12) == "Level 12");
}

TEST_CASE("ligneDePointsADepenser : le signal en anglais") {
    REQUIRE(ligneDePointsADepenser() == "Points to spend");
}
