// ./tests/combat/test_Jauge.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Jauge.hpp"

TEST_CASE("Jauge commence pleine") {
    const Jauge jauge(100);

    REQUIRE(jauge.getValeur() == 100);
    REQUIRE(jauge.getMaximum() == 100);
}

TEST_CASE("Jauge retirer renvoie ce qui a ete retire et s'arrete a zero") {
    Jauge jauge(100);

    REQUIRE(jauge.retirer(30) == 30);
    REQUIRE(jauge.getValeur() == 70);
    REQUIRE(jauge.retirer(250) == 70);
    REQUIRE(jauge.getValeur() == 0);
    REQUIRE(jauge.retirer(10) == 0);
}

TEST_CASE("Jauge ajouter renvoie ce qui a ete ajoute et s'arrete au maximum") {
    Jauge jauge(100);
    static_cast<void>(jauge.retirer(40));

    REQUIRE(jauge.ajouter(15) == 15);
    REQUIRE(jauge.getValeur() == 75);
    REQUIRE(jauge.ajouter(50) == 25);
    REQUIRE(jauge.getValeur() == 100);
    REQUIRE(jauge.ajouter(1) == 0);
}

TEST_CASE("Jauge ajouter et retirer zero sont sans effet") {
    Jauge jauge(100);
    static_cast<void>(jauge.retirer(40));

    REQUIRE(jauge.ajouter(0) == 0);
    REQUIRE(jauge.retirer(0) == 0);
    REQUIRE(jauge.getValeur() == 60);
}

TEST_CASE("Jauge getRatio plein, moitie et vide") {
    Jauge jauge(100);
    REQUIRE_THAT(jauge.getRatio(), Catch::Matchers::WithinAbs(1.0, 1e-6));

    static_cast<void>(jauge.retirer(50));
    REQUIRE_THAT(jauge.getRatio(), Catch::Matchers::WithinAbs(0.5, 1e-6));

    static_cast<void>(jauge.retirer(50));
    REQUIRE_THAT(jauge.getRatio(), Catch::Matchers::WithinAbs(0.0, 1e-6));
}

TEST_CASE("Jauge changerMaximum decale la valeur de la meme difference") {
    Jauge jauge(100);
    static_cast<void>(jauge.retirer(30));

    jauge.changerMaximum(160);
    REQUIRE(jauge.getMaximum() == 160);
    REQUIRE(jauge.getValeur() == 130);

    jauge.changerMaximum(60);
    REQUIRE(jauge.getMaximum() == 60);
    REQUIRE(jauge.getValeur() == 30);
}

TEST_CASE("Jauge changerMaximum ne descend pas sous zero ni sous le plancher") {
    Jauge jauge(100);
    static_cast<void>(jauge.retirer(90));

    jauge.changerMaximum(20);
    REQUIRE(jauge.getValeur() == 0);

    Jauge protegee(100);
    static_cast<void>(protegee.retirer(90));
    protegee.changerMaximum(20, 1);
    REQUIRE(protegee.getValeur() == 1);
}

TEST_CASE("Jauge definirMaximum ne change la valeur que pour la plafonner") {
    Jauge jauge(100);
    static_cast<void>(jauge.retirer(70));

    jauge.definirMaximum(300);
    REQUIRE(jauge.getMaximum() == 300);
    REQUIRE(jauge.getValeur() == 30);

    jauge.definirMaximum(20);
    REQUIRE(jauge.getMaximum() == 20);
    REQUIRE(jauge.getValeur() == 20);
}
