// ./tests/test_EntiteVue.cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "EntiteVue.hpp"

TEST_CASE("trouverEntite renvoie le monstre portant cet identifiant") {
    const std::vector<EntiteVue> monstres = {{1, {{10.0f, 20.0f}, 15.0f}}, {7, {{30.0f, 40.0f}, 12.0f}}};

    const EntiteVue* trouve = trouverEntite(monstres, 7);

    REQUIRE(trouve != nullptr);
    REQUIRE(trouve->id == 7);
    REQUIRE(trouve->cercle.centre.x == 30.0f);
    REQUIRE(trouve->cercle.rayon == 12.0f);
    REQUIRE(trouve == &monstres[1]);
}

TEST_CASE("trouverEntite renvoie le premier monstre de la liste s'il porte l'identifiant") {
    const std::vector<EntiteVue> monstres = {{1, {{10.0f, 20.0f}, 15.0f}}, {7, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverEntite(monstres, 1) == &monstres[0]);
}

TEST_CASE("trouverEntite renvoie nullptr si l'identifiant est absent") {
    const std::vector<EntiteVue> monstres = {{1, {{10.0f, 20.0f}, 15.0f}}, {7, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverEntite(monstres, 2) == nullptr);
}

TEST_CASE("trouverEntite renvoie nullptr sur une liste vide") {
    const std::vector<EntiteVue> monstres;

    REQUIRE(trouverEntite(monstres, 1) == nullptr);
}
