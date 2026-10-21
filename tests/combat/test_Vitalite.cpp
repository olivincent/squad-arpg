// ./tests/combat/test_Vitalite.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Vitalite.hpp"

TEST_CASE("Vitalite commence pleine") {
    const Vitalite vitalite(100, 0);

    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.getMaximum() == 100);
    REQUIRE(vitalite.getRatio() == 1.0f);
    REQUIRE(vitalite.estVivant());
}

TEST_CASE("Vitalite sans armure retire les degats entiers") {
    Vitalite vitalite(100, 0);

    REQUIRE(vitalite.recevoirDegats(30) == 30);
    REQUIRE(vitalite.getValeur() == 70);
}

TEST_CASE("Vitalite definirArmure remplace l'armure sans toucher aux points de vie") {
    Vitalite vitalite(100, 0);
    REQUIRE(vitalite.recevoirDegats(30) == 30);

    vitalite.definirArmure(100);  // armure 100 : la moitie des degats

    REQUIRE(vitalite.getValeur() == 70);
    REQUIRE(vitalite.recevoirDegats(30) == 15);

    vitalite.definirArmure(0);
    REQUIRE(vitalite.recevoirDegats(30) == 30);
}

TEST_CASE("Vitalite applique l'armure avant de retirer les points de vie") {
    Vitalite vitalite(100, 100);  // armure 100 : la moitie des degats

    REQUIRE(vitalite.recevoirDegats(30) == 15);
    REQUIRE(vitalite.getValeur() == 85);
}

TEST_CASE("Vitalite renvoie les degats apres armure sans les plafonner a la vie restante") {
    Vitalite vitalite(100, 0);

    REQUIRE(vitalite.recevoirDegats(90) == 90);
    REQUIRE(vitalite.recevoirDegats(50) == 50);
    REQUIRE(vitalite.getValeur() == 0);
    REQUIRE_FALSE(vitalite.estVivant());
}

TEST_CASE("Vitalite : un mort ne recoit plus de degats") {
    Vitalite vitalite(100, 0);
    REQUIRE(vitalite.recevoirDegats(100) == 100);

    REQUIRE(vitalite.recevoirDegats(10) == 0);
    REQUIRE(vitalite.getValeur() == 0);
}

TEST_CASE("Vitalite : un coup de 0 ne fait rien") {
    Vitalite vitalite(100, 50);

    REQUIRE(vitalite.recevoirDegats(0) == 0);
    REQUIRE(vitalite.getValeur() == 100);
}

TEST_CASE("Vitalite soigne jusqu'au maximum et renvoie les points rendus") {
    Vitalite vitalite(100, 0);
    static_cast<void>(vitalite.recevoirDegats(30));

    REQUIRE(vitalite.soigner(10) == 10);
    REQUIRE(vitalite.soigner(50) == 20);
    REQUIRE(vitalite.getValeur() == 100);
}

TEST_CASE("Vitalite : un mort ne se soigne pas") {
    Vitalite vitalite(100, 0);
    static_cast<void>(vitalite.recevoirDegats(100));

    REQUIRE(vitalite.soigner(50) == 0);
    REQUIRE_FALSE(vitalite.estVivant());
}

TEST_CASE("Vitalite getRatio suit les points de vie") {
    Vitalite vitalite(100, 0);
    static_cast<void>(vitalite.recevoirDegats(25));

    REQUIRE(vitalite.getRatio() == 0.75f);
}
