// ./tests/interface/test_AffichageStatistiques.cpp
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "combat/StatistiquesDeCombat.hpp"
#include "interface/AffichageStatistiques.hpp"

TEST_CASE("affichage : les capacites offensive et defensive, dans cet ordre") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.capaciteOffensive = 120, .capaciteDefensive = 95});

    REQUIRE(lignes == std::vector<std::string>{"Capacité offensive 120", "Capacité défensive 95"});
}
