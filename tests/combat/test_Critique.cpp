// ./tests/combat/test_Critique.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Critique.hpp"

using Catch::Matchers::WithinAbs;

namespace {
constexpr double s_tolerance = 1e-6;
}

TEST_CASE("rapportDeCapacites : l'offensive divisee par la defensive") {
    REQUIRE_THAT(rapportDeCapacites(100, 100), WithinAbs(1.0, s_tolerance));
    REQUIRE_THAT(rapportDeCapacites(50, 100), WithinAbs(0.5, s_tolerance));
    REQUIRE_THAT(rapportDeCapacites(200, 100), WithinAbs(2.0, s_tolerance));
    REQUIRE_THAT(rapportDeCapacites(0, 100), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDeCritique : 5 % du carre du rapport") {
    REQUIRE_THAT(chanceDeCritique(0.0f), WithinAbs(0.0, s_tolerance));
    REQUIRE_THAT(chanceDeCritique(0.5f), WithinAbs(0.0125, s_tolerance));
    REQUIRE_THAT(chanceDeCritique(1.0f), WithinAbs(0.05, s_tolerance));
    REQUIRE_THAT(chanceDeCritique(2.0f), WithinAbs(0.20, s_tolerance));
    REQUIRE_THAT(chanceDeCritique(3.0f), WithinAbs(0.45, s_tolerance));
}

TEST_CASE("chanceDeCritique : plafonnee a 50 %") {
    REQUIRE_THAT(chanceDeCritique(4.0f), WithinAbs(0.50, s_tolerance));
    REQUIRE_THAT(chanceDeCritique(100.0f), WithinAbs(0.50, s_tolerance));
}

TEST_CASE("multiplicateurDeCritique : 1,5 au rapport 1, plus 0,25 par point de rapport") {
    REQUIRE_THAT(multiplicateurDeCritique(1.0f), WithinAbs(1.5, s_tolerance));
    REQUIRE_THAT(multiplicateurDeCritique(2.0f), WithinAbs(1.75, s_tolerance));
    REQUIRE_THAT(multiplicateurDeCritique(3.0f), WithinAbs(2.0, s_tolerance));
}

TEST_CASE("multiplicateurDeCritique : borne a [1,5 ; 2,0]") {
    REQUIRE_THAT(multiplicateurDeCritique(0.0f), WithinAbs(1.5, s_tolerance));
    REQUIRE_THAT(multiplicateurDeCritique(0.5f), WithinAbs(1.5, s_tolerance));
    REQUIRE_THAT(multiplicateurDeCritique(4.0f), WithinAbs(2.0, s_tolerance));
    REQUIRE_THAT(multiplicateurDeCritique(100.0f), WithinAbs(2.0, s_tolerance));
}

TEST_CASE("appliquerCritique : un tirage sous la chance donne un critique multiplie") {
    // Rapport 2 : 20 % de chance, ×1,75.
    const ResultatCritique critique = appliquerCritique(100, 2.0f, 0.19f);

    REQUIRE(critique.critique);
    REQUIRE(critique.montant == 175);
}

TEST_CASE("appliquerCritique : un tirage egal ou au-dessus de la chance ne donne rien") {
    const ResultatCritique pile = appliquerCritique(100, 2.0f, 0.20f);
    const ResultatCritique haut = appliquerCritique(100, 2.0f, 0.99f);

    REQUIRE_FALSE(pile.critique);
    REQUIRE(pile.montant == 100);
    REQUIRE_FALSE(haut.critique);
    REQUIRE(haut.montant == 100);
}

TEST_CASE("appliquerCritique : arrondi au plus proche") {
    REQUIRE(appliquerCritique(3, 1.0f, 0.0f).montant == 5);   // 4,5
    REQUIRE(appliquerCritique(7, 1.0f, 0.0f).montant == 11);  // 10,5
    REQUIRE(appliquerCritique(5, 1.0f, 0.0f).montant == 8);   // 7,5
}

TEST_CASE("appliquerCritique : un rapport nul n'est jamais critique, meme au tirage 0") {
    const ResultatCritique critique = appliquerCritique(100, 0.0f, 0.0f);

    REQUIRE_FALSE(critique.critique);
    REQUIRE(critique.montant == 100);
}

TEST_CASE("appliquerCritique : un montant de 0 reste 0") {
    REQUIRE(appliquerCritique(0, 3.0f, 0.0f).montant == 0);
}
