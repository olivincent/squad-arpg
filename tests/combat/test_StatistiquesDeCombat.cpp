// ./tests/combat/test_StatistiquesDeCombat.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"

using Catch::Approx;

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

TEST_CASE("statistiquesDuPersonnage : seule l'Adresse compte parmi les caracteristiques pour les capacites") {
    const StatistiquesDeCombat sansAutre = statistiquesDuPersonnage(Caracteristiques({0, 0, 8, 0, 0}), s_sansBonus);
    const StatistiquesDeCombat tout = statistiquesDuPersonnage(Caracteristiques({30, 25, 8, 40, 12}), s_sansBonus);

    REQUIRE(sansAutre.capaciteOffensive == tout.capaciteOffensive);
    REQUIRE(sansAutre.capaciteDefensive == tout.capaciteDefensive);
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

TEST_CASE("les resistances sont a zero par defaut et s'additionnent avec les bonus") {
    StatistiquesDeCombat statistiques;
    REQUIRE(statistiques.resistances.total() == 0);

    StatistiquesDeCombat bonus;
    bonus.resistances[Element::Feu] = 10;
    StatistiquesDeCombat autreBonus;
    autreBonus.resistances[Element::Feu] = 15;
    autreBonus.resistances[Element::Sacre] = -20;

    const StatistiquesDeCombat somme = statistiques + bonus + autreBonus;

    REQUIRE(somme.resistances[Element::Feu] == 25);
    REQUIRE(somme.resistances[Element::Sacre] == -20);
    REQUIRE(somme.resistances[Element::Physique] == 0);
}

TEST_CASE("statistiquesDuPersonnage garde les resistances du bonus d'equipement") {
    StatistiquesDeCombat bonus;
    bonus.resistances[Element::Feu] = 25;

    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 10, 10, 10, 10}), bonus);

    REQUIRE(statistiques.resistances[Element::Feu] == 25);
    REQUIRE(statistiques.resistances[Element::Sacre] == 0);
}

TEST_CASE("statistiquesDuPersonnage : les resistances au controle viennent de la Constitution et de la Resolve") {
    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 20, 10, 10, 14}), s_sansBonus);

    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Etourdissement] == Approx(3.0f));
    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Ralentissement] == Approx(3.0f));
    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Charme] == Approx(1.2f));
    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Peur] == Approx(1.2f));
    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Disruption] == Approx(1.2f));
}

TEST_CASE("statistiquesDuPersonnage : sous la reference, un leger malus de resistance au controle") {
    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 0, 10, 10, 0}), s_sansBonus);

    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Etourdissement] == Approx(-3.0f));
    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Peur] == Approx(-3.0f));
}

TEST_CASE("statistiquesDuPersonnage : le bonus d'equipement de resistance au controle s'ajoute a celui des caracteristiques") {
    StatistiquesDeCombat bonus;
    bonus.resistancesAuControle[EtatDeControle::Charme] = 5.0f;

    const StatistiquesDeCombat statistiques = statistiquesDuPersonnage(Caracteristiques({10, 10, 10, 10, 20}), bonus);

    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Charme] == Approx(8.0f));
    REQUIRE(statistiques.resistancesAuControle[EtatDeControle::Peur] == Approx(3.0f));
}

TEST_CASE("StatistiquesDeCombat : les resistances au controle sont a zero par defaut et s'additionnent") {
    StatistiquesDeCombat a;
    a.resistancesAuControle[EtatDeControle::Peur] = 2.0f;
    StatistiquesDeCombat b;
    b.resistancesAuControle[EtatDeControle::Peur] = 1.5f;
    b.resistancesAuControle[EtatDeControle::Disruption] = 4.0f;

    REQUIRE(StatistiquesDeCombat().resistancesAuControle == ResistancesAuControle());
    REQUIRE((a + b).resistancesAuControle[EtatDeControle::Peur] == Approx(3.5f));
    REQUIRE((a + b).resistancesAuControle[EtatDeControle::Disruption] == Approx(4.0f));
    REQUIRE((a + b).resistancesAuControle[EtatDeControle::Etourdissement] == Approx(0.0f));
}
