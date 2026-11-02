// ./tests/combat/test_Energie.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Caracteristiques.hpp"
#include "combat/Energie.hpp"

namespace {
const Caracteristiques s_neutres({s_valeurDeReference, s_valeurDeReference, s_valeurDeReference, s_valeurDeReference, s_valeurDeReference});

Caracteristiques avecVolonte(int volonte) {
    return Caracteristiques({s_valeurDeReference, s_valeurDeReference, s_valeurDeReference, s_valeurDeReference, volonte});
}
}  // namespace

TEST_CASE("energieMax : 100 au niveau 1, 5 de plus par niveau") {
    REQUIRE(energieMax(s_neutres, 1, s_energieParNiveau) == 100);
    REQUIRE(energieMax(s_neutres, 2, s_energieParNiveau) == 105);
    REQUIRE(energieMax(s_neutres, 10, s_energieParNiveau) == 145);
}

TEST_CASE("regenerationDEnergie : 2 points par seconde de base, majores du pourcentage des affixes") {
    REQUIRE_THAT(regenerationDEnergie(0), Catch::Matchers::WithinAbs(2.0, 1e-6));
    REQUIRE_THAT(regenerationDEnergie(15), Catch::Matchers::WithinAbs(2.3, 1e-5));
    REQUIRE_THAT(regenerationDEnergie(100), Catch::Matchers::WithinAbs(4.0, 1e-6));
}

TEST_CASE("energieMax : le gain par niveau de classe est un parametre") {
    REQUIRE(energieMax(s_neutres, 1, 20) == 100);
    REQUIRE(energieMax(s_neutres, 4, 20) == 160);
    REQUIRE(energieMax(s_neutres, 4, 0) == 100);
}

TEST_CASE("energieMax : 3 points par point de Volonte au-dessus de la reference, un malus en dessous") {
    REQUIRE(energieMax(avecVolonte(s_valeurDeReference + 1), 1, s_energieParNiveau) == 103);
    REQUIRE(energieMax(avecVolonte(s_valeurDeReference + 10), 1, s_energieParNiveau) == 130);
    REQUIRE(energieMax(avecVolonte(s_valeurDeReference - 2), 1, s_energieParNiveau) == 94);
    REQUIRE(energieMax(avecVolonte(s_valeurDeReference + 4), 3, s_energieParNiveau) == 100 + 12 + 10);
}

TEST_CASE("energieMax : seule la Volonte compte parmi les caracteristiques") {
    REQUIRE(energieMax(Caracteristiques({30, 25, 40, 40, s_valeurDeReference}), 1, s_energieParNiveau) == 100);
}
