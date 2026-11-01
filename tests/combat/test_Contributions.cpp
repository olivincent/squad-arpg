// ./tests/combat/test_Contributions.cpp
#include <algorithm>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "combat/Caracteristiques.hpp"
#include "combat/Contributions.hpp"
#include "combat/StatistiquesDeCombat.hpp"

using Catch::Approx;

TEST_CASE("apportDesCaracteristiques : zero a la reference, pour chaque statistique") {
    const Caracteristiques neutres({10, 10, 10, 10, 10});

    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::PointsDeVieMax, neutres) == Approx(0.0f));
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::CapaciteOffensive, neutres) == Approx(0.0f));
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::CapaciteDefensive, neutres) == Approx(0.0f));
}

TEST_CASE("apportDesCaracteristiques : poids fois l'ecart a la reference, negatif en dessous") {
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::PointsDeVieMax, Caracteristiques({10, 14, 10, 10, 10})) ==
            Approx(static_cast<float>(4 * s_pvParPointDEndurance)));
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::PointsDeVieMax, Caracteristiques({10, 0, 10, 10, 10})) ==
            Approx(static_cast<float>(-10 * s_pvParPointDEndurance)));
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::CapaciteOffensive, Caracteristiques({10, 10, 13, 10, 10})) ==
            Approx(static_cast<float>(3 * s_capaciteParPointDAdresse)));
}

TEST_CASE("apportDesCaracteristiques : chaque statistique ne lit que sa caracteristique") {
    const Caracteristiques seulementLEndurance({10, 20, 10, 10, 10});
    const Caracteristiques seulementLAdresse({10, 10, 20, 10, 10});

    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::PointsDeVieMax, seulementLAdresse) == Approx(0.0f));
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::CapaciteOffensive, seulementLEndurance) == Approx(0.0f));
    REQUIRE(apportDesCaracteristiques(StatistiqueAlimentee::CapaciteDefensive, seulementLEndurance) == Approx(0.0f));
}

TEST_CASE("la table redonne les formules d'avant : 240 + 6 x Endurance et 50 + 5 x Adresse") {
    for (int valeur = 0; valeur <= 40; ++valeur) {
        const Caracteristiques caracteristiques({10, valeur, valeur, 10, 10});

        REQUIRE(pointsDeVieMax(caracteristiques, 1, s_pvParNiveau) == 240 + 6 * valeur);
        const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(caracteristiques, StatistiquesDeCombat{});
        REQUIRE(statistiques.capaciteOffensive == std::max(0, 50 + 5 * valeur));
        REQUIRE(statistiques.capaciteDefensive == std::max(1, 50 + 5 * valeur));
    }
}
