// ./tests/combat/test_StatistiquesDeCombat.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"

namespace {
const StatistiquesDeCombat s_sansBonus;
}

TEST_CASE("statistiquesDuPersonnage : Adresse 10 donne 100 aux deux capacites") {
    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 10, 10, 10, 10}), s_sansBonus);

    REQUIRE(statistiques.capaciteOffensive == 100);
    REQUIRE(statistiques.capaciteDefensive == 100);
}

TEST_CASE("statistiquesDuPersonnage : 5 par point d'Adresse au-dessus de la base de 50") {
    REQUIRE(statistiquesDuPersonnage(Caracteristiques({10, 10, 0, 10, 10}), s_sansBonus) == StatistiquesDeCombat{50, 50, 0, 0});
    REQUIRE(statistiquesDuPersonnage(Caracteristiques({10, 10, 14, 10, 10}), s_sansBonus) == StatistiquesDeCombat{120, 120, 0, 0});
}

TEST_CASE("statistiquesDuPersonnage : seule l'Adresse compte parmi les caracteristiques") {
    const StatistiquesDeCombat sansAutre = statistiquesDuPersonnage(Caracteristiques({0, 0, 8, 0, 0}), s_sansBonus);
    const StatistiquesDeCombat tout = statistiquesDuPersonnage(Caracteristiques({30, 25, 8, 40, 12}), s_sansBonus);

    REQUIRE(sansAutre == tout);
}

TEST_CASE("statistiquesDuPersonnage : bornees, l'offensive a 0 et la defensive a 1") {
    const StatistiquesDeCombat tresNegative = statistiquesDuPersonnage(Caracteristiques({10, 10, -20, 10, 10}), s_sansBonus);

    REQUIRE(tresNegative.capaciteOffensive == 0);
    REQUIRE(tresNegative.capaciteDefensive == 1);
}

TEST_CASE("statistiquesDuPersonnage : sans bonus d'equipement, aucune esquive") {
    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 10, 10, 10, 10}), s_sansBonus);

    REQUIRE(statistiques.esquiveContact == 0);
    REQUIRE(statistiques.esquiveProjectile == 0);
}

TEST_CASE("statistiquesDuPersonnage : les bonus de l'equipement s'ajoutent, esquives et capacites") {
    const StatistiquesDeCombat bonus{.capaciteOffensive = 10, .capaciteDefensive = -20, .esquiveContact = 5, .esquiveProjectile = 7};

    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 10, 10, 10, 10}), bonus);

    REQUIRE(statistiques == StatistiquesDeCombat{110, 80, 5, 7});
}

TEST_CASE("StatistiquesDeCombat : l'addition se fait champ par champ") {
    const StatistiquesDeCombat a{1, 2, 3, 4};
    const StatistiquesDeCombat b{10, 20, 30, 40};

    REQUIRE(a + b == StatistiquesDeCombat{11, 22, 33, 44});
    REQUIRE(a == StatistiquesDeCombat{1, 2, 3, 4});  // les opérandes ne changent pas
}
