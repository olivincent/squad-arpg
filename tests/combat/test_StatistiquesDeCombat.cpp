// ./tests/combat/test_StatistiquesDeCombat.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"

TEST_CASE("statistiquesDuPersonnage : Adresse 10 donne 100 aux deux capacites") {
    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 10, 10, 10, 10}));

    REQUIRE(statistiques.capaciteOffensive == 100);
    REQUIRE(statistiques.capaciteDefensive == 100);
}

TEST_CASE("statistiquesDuPersonnage : 5 par point d'Adresse au-dessus de la base de 50") {
    REQUIRE(statistiquesDuPersonnage(Caracteristiques({10, 10, 0, 10, 10})) == StatistiquesDeCombat{50, 50});
    REQUIRE(statistiquesDuPersonnage(Caracteristiques({10, 10, 14, 10, 10})) == StatistiquesDeCombat{120, 120});
}

TEST_CASE("statistiquesDuPersonnage : seule l'Adresse compte") {
    const StatistiquesDeCombat sansAutre = statistiquesDuPersonnage(Caracteristiques({0, 0, 8, 0, 0}));
    const StatistiquesDeCombat tout = statistiquesDuPersonnage(Caracteristiques({30, 25, 8, 40, 12}));

    REQUIRE(sansAutre == tout);
}

TEST_CASE("statistiquesDuPersonnage : bornees, l'offensive a 0 et la defensive a 1") {
    const StatistiquesDeCombat tresNegative = statistiquesDuPersonnage(Caracteristiques({10, 10, -20, 10, 10}));

    REQUIRE(tresNegative.capaciteOffensive == 0);
    REQUIRE(tresNegative.capaciteDefensive == 1);
}
