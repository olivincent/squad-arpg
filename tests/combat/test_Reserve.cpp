// ./tests/combat/test_Reserve.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Reserve.hpp"

namespace {
constexpr float s_tic = 1.0f / 60.0f;  // une image à 60 par seconde

// Fait couler `secondes` secondes en tics de 1/60 s.
void faireCouler(Reserve& reserve, int secondes) {
    for (int i = 0; i < secondes * 60; ++i) {
        reserve.regenerer(s_tic);
    }
}
}  // namespace

TEST_CASE("Reserve commence pleine") {
    const Reserve reserve(100, 2.0f);

    REQUIRE(reserve.getValeur() == 100);
    REQUIRE(reserve.getMaximum() == 100);
    REQUIRE_THAT(reserve.getRegeneration(), Catch::Matchers::WithinAbs(2.0, 1e-6));
}

TEST_CASE("Reserve payer retire le cout") {
    Reserve reserve(100, 2.0f);

    reserve.payer(43);

    REQUIRE(reserve.getValeur() == 57);
}

TEST_CASE("Reserve suffit dit si le cout est couvert, au point pres") {
    Reserve reserve(100, 2.0f);
    reserve.payer(57);

    REQUIRE(reserve.suffit(43));
    REQUIRE_FALSE(reserve.suffit(44));
    REQUIRE(reserve.suffit(0));
}

TEST_CASE("Reserve regenerer ajoute la partie entiere et garde le reste") {
    Reserve reserve(100, 2.0f);
    reserve.payer(50);

    // 0,6 s à 2 points par seconde : 1,2 point, un seul est ajouté, 0,2 est gardé.
    reserve.regenerer(0.6f);
    REQUIRE(reserve.getValeur() == 51);
    // 0,2 + 1,0 = 1,2 de plus : un point de nouveau.
    reserve.regenerer(0.5f);
    REQUIRE(reserve.getValeur() == 52);
}

TEST_CASE("Reserve regenerer a petits pas ne perd rien") {
    Reserve reserve(100, 2.0f);
    reserve.payer(100);

    faireCouler(reserve, 10);

    // 20 points en 10 s ; l'erreur d'arrondi des flottants peut en retarder un d'un tic.
    REQUIRE(reserve.getValeur() >= 19);
    REQUIRE(reserve.getValeur() <= 20);
}

TEST_CASE("Reserve regenerer s'arrete au maximum et ne met rien de cote") {
    Reserve reserve(100, 2.0f);
    reserve.payer(5);

    reserve.regenerer(100.0f);
    REQUIRE(reserve.getValeur() == 100);

    // Pleine, elle n'a accumulé aucun reste : après un paiement, il faut bien une seconde pour 2 points.
    reserve.payer(10);
    reserve.regenerer(0.4f);
    REQUIRE(reserve.getValeur() == 90);
}

TEST_CASE("Reserve regenerer a debit nul ne change rien") {
    Reserve reserve(100, 0.0f);
    reserve.payer(40);

    reserve.regenerer(30.0f);

    REQUIRE(reserve.getValeur() == 60);
}

TEST_CASE("Reserve definirRegeneration change le debit") {
    Reserve reserve(100, 2.0f);
    reserve.payer(100);

    reserve.definirRegeneration(10.0f);
    reserve.regenerer(1.0f);

    REQUIRE(reserve.getValeur() == 10);
}

TEST_CASE("Reserve changerMaximum monte la valeur de la meme difference") {
    Reserve reserve(100, 2.0f);
    reserve.payer(30);

    reserve.changerMaximum(105);

    REQUIRE(reserve.getMaximum() == 105);
    REQUIRE(reserve.getValeur() == 75);
}

TEST_CASE("Reserve definirMaximum laisse la valeur") {
    Reserve reserve(100, 2.0f);
    reserve.payer(30);

    reserve.definirMaximum(105);

    REQUIRE(reserve.getMaximum() == 105);
    REQUIRE(reserve.getValeur() == 70);
}

TEST_CASE("Reserve getRatio") {
    Reserve reserve(100, 2.0f);
    reserve.payer(75);

    REQUIRE_THAT(reserve.getRatio(), Catch::Matchers::WithinAbs(0.25, 1e-6));
}
