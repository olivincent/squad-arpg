// ./tests/combat/test_Esquive.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Esquive.hpp"
#include "combat/NatureDAttaque.hpp"
#include "combat/StatistiquesDeCombat.hpp"

using Catch::Matchers::WithinAbs;

namespace {
constexpr double s_tolerance = 1e-6;
}

TEST_CASE("esquiveEffective : ramenee entre 0 et le plafond de 50 %") {
    REQUIRE(esquiveEffective(0) == 0);
    REQUIRE(esquiveEffective(5) == 5);
    REQUIRE(esquiveEffective(50) == 50);
    REQUIRE(esquiveEffective(51) == 50);
    REQUIRE(esquiveEffective(1000) == 50);
    REQUIRE(esquiveEffective(-10) == 0);
}

TEST_CASE("chanceDEsquive : l'esquive contre le contact ne joue que contre le contact") {
    const StatistiquesDeCombat defenseur{.esquiveContact = 20, .esquiveProjectile = 0};

    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Contact, defenseur), WithinAbs(0.20, s_tolerance));
    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Projectile, defenseur), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDEsquive : l'esquive contre les projectiles ne joue que contre les projectiles") {
    const StatistiquesDeCombat defenseur{.esquiveContact = 0, .esquiveProjectile = 5};

    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Projectile, defenseur), WithinAbs(0.05, s_tolerance));
    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Contact, defenseur), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDEsquive : une zone ne s'esquive jamais, quelle que soit l'esquive") {
    const StatistiquesDeCombat defenseur{.esquiveContact = 50, .esquiveProjectile = 50};

    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Zone, defenseur), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDEsquive : plafonnee a 50 %") {
    const StatistiquesDeCombat defenseur{.esquiveContact = 80, .esquiveProjectile = 500};

    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Contact, defenseur), WithinAbs(0.50, s_tolerance));
    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Projectile, defenseur), WithinAbs(0.50, s_tolerance));
}

TEST_CASE("chanceDEsquive : une esquive negative vaut zero") {
    const StatistiquesDeCombat defenseur{.esquiveContact = -10};

    REQUIRE_THAT(chanceDEsquive(NatureDAttaque::Contact, defenseur), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("coupEsquive : un tirage sous la chance esquive, egal ou au-dessus non") {
    REQUIRE(coupEsquive(0.05f, 0.04f));
    REQUIRE(coupEsquive(0.05f, 0.0f));
    REQUIRE_FALSE(coupEsquive(0.05f, 0.05f));
    REQUIRE_FALSE(coupEsquive(0.05f, 0.9f));
}

TEST_CASE("coupEsquive : sans chance, jamais, meme au tirage 0") {
    REQUIRE_FALSE(coupEsquive(0.0f, 0.0f));
}
