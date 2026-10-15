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

TEST_CASE("distancePointSegment mesure la perpendiculaire au milieu du segment") {
    REQUIRE(distancePointSegment({5.0f, 3.0f}, {0.0f, 0.0f}, {10.0f, 0.0f}) == 3.0f);
}

TEST_CASE("distancePointSegment mesure la distance a l'extremite la plus proche hors du segment") {
    REQUIRE(distancePointSegment({13.0f, 4.0f}, {0.0f, 0.0f}, {10.0f, 0.0f}) == 5.0f);
    REQUIRE(distancePointSegment({-3.0f, 4.0f}, {0.0f, 0.0f}, {10.0f, 0.0f}) == 5.0f);
}

TEST_CASE("distancePointSegment avec un segment reduit a un point") {
    REQUIRE(distancePointSegment({3.0f, 4.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}) == 5.0f);
}
