// ./tests/test_Cercle.cpp
#include <catch2/catch_test_macros.hpp>

#include "Cercle.hpp"

TEST_CASE("distanceBordABord renvoie l'ecart entre deux cercles separes") {
    Cercle a{{0.0f, 0.0f}, 10.0f};
    Cercle b{{50.0f, 0.0f}, 10.0f};

    REQUIRE(distanceBordABord(a, b) == 30.0f);
}

TEST_CASE("distanceBordABord renvoie 0 pour deux cercles qui se touchent") {
    Cercle a{{0.0f, 0.0f}, 10.0f};
    Cercle b{{20.0f, 0.0f}, 10.0f};

    REQUIRE(distanceBordABord(a, b) == 0.0f);
}

TEST_CASE("distanceBordABord renvoie 0 pour deux cercles qui se chevauchent") {
    Cercle a{{0.0f, 0.0f}, 10.0f};
    Cercle b{{5.0f, 0.0f}, 10.0f};

    REQUIRE(distanceBordABord(a, b) == 0.0f);
}

TEST_CASE("distanceBordABord tient compte de rayons differents") {
    Cercle a{{0.0f, 0.0f}, 5.0f};
    Cercle b{{100.0f, 0.0f}, 20.0f};

    REQUIRE(distanceBordABord(a, b) == 75.0f);
}
