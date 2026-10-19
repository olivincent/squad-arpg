// ./tests/test_Composition.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "aides.hpp"
#include "Composition.hpp"

namespace {

// Les décalages sont calculés par des flottants exacts (multiples de 12,5) : on compare directement.
using Decalages = std::vector<sf::Vector2f>;

}  // namespace

TEST_CASE("decalagesDeFormation : une composition vide donne aucun decalage") {
    REQUIRE(decalagesDeFormation({}).empty());
}

TEST_CASE("decalagesDeFormation : un seul personnage par rang est sur l'axe de l'escouade") {
    REQUIRE(decalagesDeFormation({Rang::Avant}) == Decalages{{25.0f, 0.0f}});
    REQUIRE(decalagesDeFormation({Rang::Arriere}) == Decalages{{-25.0f, 0.0f}});
}

TEST_CASE("decalagesDeFormation : deux par rang, espaces de 50 px et centres") {
    REQUIRE(decalagesDeFormation({Rang::Avant, Rang::Avant}) == Decalages{{25.0f, -25.0f}, {25.0f, 25.0f}});
    REQUIRE(decalagesDeFormation({Rang::Arriere, Rang::Arriere}) == Decalages{{-25.0f, -25.0f}, {-25.0f, 25.0f}});
}

TEST_CASE("decalagesDeFormation : trois par rang") {
    REQUIRE(decalagesDeFormation({Rang::Avant, Rang::Avant, Rang::Avant}) ==
            Decalages{{25.0f, -50.0f}, {25.0f, 0.0f}, {25.0f, 50.0f}});
}

TEST_CASE("decalagesDeFormation : 2 devant et 2 derriere donnent les decalages du carre historique") {
    const std::vector<Rang> rangs = {Rang::Avant, Rang::Avant, Rang::Arriere, Rang::Arriere};

    REQUIRE(decalagesDeFormation(rangs) ==
            Decalages{{25.0f, -25.0f}, {25.0f, 25.0f}, {-25.0f, -25.0f}, {-25.0f, 25.0f}});
}

TEST_CASE("decalagesDeFormation : 1 devant et 2 derriere forment un triangle") {
    const std::vector<Rang> rangs = {Rang::Avant, Rang::Arriere, Rang::Arriere};

    REQUIRE(decalagesDeFormation(rangs) == Decalages{{25.0f, 0.0f}, {-25.0f, -25.0f}, {-25.0f, 25.0f}});
}

TEST_CASE("decalagesDeFormation : l'ordre de la composition est celui des decalages, rangs entremeles") {
    const std::vector<Rang> rangs = {Rang::Arriere, Rang::Avant, Rang::Arriere};

    REQUIRE(decalagesDeFormation(rangs) == Decalages{{-25.0f, -25.0f}, {25.0f, 0.0f}, {-25.0f, 25.0f}});
}

TEST_CASE("compositionParDefaut : chaque personnage a un rang, de la vision et des skills sans doublon") {
    const std::vector<DefinitionPersonnage> composition = compositionParDefaut();

    REQUIRE_FALSE(composition.empty());
    for (const DefinitionPersonnage& definition : composition) {
        REQUIRE(definition.rayonVision == s_rayonVisionDeTest);
        for (std::size_t i = 0; i < definition.skills.size(); ++i) {
            for (std::size_t j = i + 1; j < definition.skills.size(); ++j) {
                REQUIRE(definition.skills[i].id != definition.skills[j].id);
            }
        }
    }
}
