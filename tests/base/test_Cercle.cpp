// ./tests/base/test_Cercle.cpp
#include <catch2/catch_test_macros.hpp>

#include "base/Cercle.hpp"

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

TEST_CASE("distanceCercleRectangle : cercle a cote du rectangle") {
    const sf::FloatRect rectangle({0.0f, 0.0f}, {40.0f, 20.0f});
    REQUIRE(distanceCercleRectangle({{60.0f, 10.0f}, 5.0f}, rectangle) == 15.0f);
    REQUIRE(distanceCercleRectangle({{20.0f, -30.0f}, 10.0f}, rectangle) == 20.0f);
}

TEST_CASE("distanceCercleRectangle : contact, chevauchement et centre dedans valent 0") {
    const sf::FloatRect rectangle({0.0f, 0.0f}, {40.0f, 20.0f});
    REQUIRE(distanceCercleRectangle({{50.0f, 10.0f}, 10.0f}, rectangle) == 0.0f);
    REQUIRE(distanceCercleRectangle({{45.0f, 10.0f}, 10.0f}, rectangle) == 0.0f);
    REQUIRE(distanceCercleRectangle({{20.0f, 10.0f}, 1.0f}, rectangle) == 0.0f);
}

TEST_CASE("distanceCercleRectangle : en diagonale, mesure depuis le coin") {
    const sf::FloatRect rectangle({0.0f, 0.0f}, {40.0f, 20.0f});
    // Coin (40, 20) ; centre en (43, 24) : distance 5, rayon 2.
    REQUIRE(distanceCercleRectangle({{43.0f, 24.0f}, 2.0f}, rectangle) == 3.0f);
}
