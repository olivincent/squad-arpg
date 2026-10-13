// ./tests/test_MonstreVu.cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "MonstreVu.hpp"

TEST_CASE("trouverMonstre renvoie le monstre portant cet identifiant") {
    const std::vector<MonstreVu> monstres = {{1, {{10.0f, 20.0f}, 15.0f}}, {7, {{30.0f, 40.0f}, 12.0f}}};

    const MonstreVu* trouve = trouverMonstre(monstres, 7);

    REQUIRE(trouve != nullptr);
    REQUIRE(trouve->id == 7);
    REQUIRE(trouve->cercle.centre.x == 30.0f);
    REQUIRE(trouve->cercle.rayon == 12.0f);
    REQUIRE(trouve == &monstres[1]);
}

TEST_CASE("trouverMonstre renvoie le premier monstre de la liste s'il porte l'identifiant") {
    const std::vector<MonstreVu> monstres = {{1, {{10.0f, 20.0f}, 15.0f}}, {7, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverMonstre(monstres, 1) == &monstres[0]);
}

TEST_CASE("trouverMonstre renvoie nullptr si l'identifiant est absent") {
    const std::vector<MonstreVu> monstres = {{1, {{10.0f, 20.0f}, 15.0f}}, {7, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverMonstre(monstres, 2) == nullptr);
}

TEST_CASE("trouverMonstre renvoie nullptr sur une liste vide") {
    const std::vector<MonstreVu> monstres;

    REQUIRE(trouverMonstre(monstres, 1) == nullptr);
}
