// ./tests/combat/test_EntiteVue.cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "combat/EntiteVue.hpp"

TEST_CASE("trouverEntite renvoie le monstre portant cet identifiant") {
    const std::vector<MonstreVu> monstres = {{IdMonstre{1}, {{10.0f, 20.0f}, 15.0f}}, {IdMonstre{7}, {{30.0f, 40.0f}, 12.0f}}};

    const MonstreVu* trouve = trouverEntite(monstres, IdMonstre{7});

    REQUIRE(trouve != nullptr);
    REQUIRE(trouve->id == IdMonstre{7});
    REQUIRE(trouve->cercle.centre.x == 30.0f);
    REQUIRE(trouve->cercle.rayon == 12.0f);
    REQUIRE(trouve == &monstres[1]);
}

TEST_CASE("trouverEntite renvoie le premier monstre de la liste s'il porte l'identifiant") {
    const std::vector<MonstreVu> monstres = {{IdMonstre{1}, {{10.0f, 20.0f}, 15.0f}}, {IdMonstre{7}, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverEntite(monstres, IdMonstre{1}) == &monstres[0]);
}

TEST_CASE("trouverEntite renvoie nullptr si l'identifiant est absent") {
    const std::vector<MonstreVu> monstres = {{IdMonstre{1}, {{10.0f, 20.0f}, 15.0f}}, {IdMonstre{7}, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverEntite(monstres, IdMonstre{2}) == nullptr);
}

TEST_CASE("trouverEntite renvoie nullptr sur une liste vide") {
    const std::vector<MonstreVu> monstres;

    REQUIRE(trouverEntite(monstres, IdMonstre{1}) == nullptr);
}

TEST_CASE("trouverEntite cherche aussi un personnage par sa place") {
    const std::vector<PersonnageVu> personnages = {{Place{1}, {{10.0f, 20.0f}, 15.0f}}, {Place{3}, {{30.0f, 40.0f}, 12.0f}}};

    REQUIRE(trouverEntite(personnages, Place{3}) == &personnages[1]);
    REQUIRE(trouverEntite(personnages, Place{2}) == nullptr);
}
