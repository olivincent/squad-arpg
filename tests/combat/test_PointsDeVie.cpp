// ./tests/combat/test_PointsDeVie.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/PointsDeVie.hpp"

TEST_CASE("PointsDeVie commence plein") {
    const PointsDeVie vie(100);

    REQUIRE(vie.getValeur() == 100);
    REQUIRE(vie.getMaximum() == 100);
    REQUIRE(vie.estVivant());
}

TEST_CASE("PointsDeVie subirDegats retire des points") {
    PointsDeVie vie(100);

    vie.subirDegats(30);

    REQUIRE(vie.getValeur() == 70);
    REQUIRE(vie.estVivant());
}

TEST_CASE("PointsDeVie subirDegats est borne a zero") {
    PointsDeVie vie(100);

    vie.subirDegats(250);

    REQUIRE(vie.getValeur() == 0);
    REQUIRE_FALSE(vie.estVivant());
}

TEST_CASE("PointsDeVie subirDegats de zero est sans effet") {
    PointsDeVie vie(100);

    vie.subirDegats(0);

    REQUIRE(vie.getValeur() == 100);
}

TEST_CASE("PointsDeVie soigner rend des points") {
    PointsDeVie vie(100);
    vie.subirDegats(50);

    vie.soigner(20);

    REQUIRE(vie.getValeur() == 70);
}

TEST_CASE("PointsDeVie soigner est borne au maximum") {
    PointsDeVie vie(100);
    vie.subirDegats(10);

    vie.soigner(50);

    REQUIRE(vie.getValeur() == 100);
}

TEST_CASE("PointsDeVie soigner un mort est sans effet") {
    PointsDeVie vie(100);
    vie.subirDegats(100);

    vie.soigner(50);

    REQUIRE(vie.getValeur() == 0);
    REQUIRE_FALSE(vie.estVivant());
}

TEST_CASE("PointsDeVie getRatio plein, moitie et mort") {
    PointsDeVie vie(100);
    REQUIRE_THAT(vie.getRatio(), Catch::Matchers::WithinAbs(1.0, 1e-6));

    vie.subirDegats(50);
    REQUIRE_THAT(vie.getRatio(), Catch::Matchers::WithinAbs(0.5, 1e-6));

    vie.subirDegats(50);
    REQUIRE_THAT(vie.getRatio(), Catch::Matchers::WithinAbs(0.0, 1e-6));
}

TEST_CASE("PointsDeVie soigner renvoie les points reellement rendus") {
    PointsDeVie vie(100);
    vie.subirDegats(50);

    REQUIRE(vie.soigner(20) == 20);
    REQUIRE(vie.getValeur() == 70);
}

TEST_CASE("PointsDeVie soigner renvoie seulement ce qui tient sous le maximum") {
    PointsDeVie vie(100);
    vie.subirDegats(10);

    REQUIRE(vie.soigner(50) == 10);
    REQUIRE(vie.soigner(50) == 0);  // deja plein
}

TEST_CASE("PointsDeVie soigner un mort renvoie zero") {
    PointsDeVie vie(100);
    vie.subirDegats(100);

    REQUIRE(vie.soigner(50) == 0);
}
