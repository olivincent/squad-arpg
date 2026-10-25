// ./tests/combat/test_Blocage.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Blocage.hpp"
#include "combat/NatureDAttaque.hpp"
#include "combat/StatistiquesDeCombat.hpp"

using Catch::Matchers::WithinAbs;

namespace {
constexpr double s_tolerance = 1e-6;

// Un défenseur au bouclier rond d'aujourd'hui : 30 % de chance, 40 points, 3 s de récupération.
const StatistiquesDeCombat s_bouclier{.chanceDeBlocage = 30, .montantBloque = 40};
}  // namespace

TEST_CASE("chanceDeBlocage : le bouclier bloque le contact et les projectiles de la meme facon") {
    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Contact, s_bouclier, true), WithinAbs(0.30, s_tolerance));
    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Projectile, s_bouclier, true), WithinAbs(0.30, s_tolerance));
}

TEST_CASE("chanceDeBlocage : une zone ne se bloque jamais") {
    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Zone, s_bouclier, true), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDeBlocage : nulle pendant la recuperation") {
    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Contact, s_bouclier, false), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDeBlocage : nulle sans bouclier, meme avec une chance (rien a absorber)") {
    const StatistiquesDeCombat sansBouclier;
    const StatistiquesDeCombat chanceSansMontant{.chanceDeBlocage = 30, .montantBloque = 0};

    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Contact, sansBouclier, true), WithinAbs(0.0, s_tolerance));
    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Contact, chanceSansMontant, true), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("chanceDeBlocage : une chance hors de 0 a 100 % est ramenee dans cet intervalle") {
    const StatistiquesDeCombat trop{.chanceDeBlocage = 250, .montantBloque = 10};
    const StatistiquesDeCombat negative{.chanceDeBlocage = -20, .montantBloque = 10};

    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Contact, trop, true), WithinAbs(1.0, s_tolerance));
    REQUIRE_THAT(chanceDeBlocage(NatureDAttaque::Contact, negative, true), WithinAbs(0.0, s_tolerance));
}

TEST_CASE("appliquerBlocage : un tirage sous la chance retire le montant bloque du coup") {
    const ResultatDuBlocage resultat = appliquerBlocage(100, 40, 0.30f, 0.10f);

    REQUIRE(resultat.bloque == 40);
    REQUIRE(resultat.montant == 60);
}

TEST_CASE("appliquerBlocage : un coup plus faible que le bouclier est absorbe en entier") {
    const ResultatDuBlocage resultat = appliquerBlocage(25, 40, 0.30f, 0.10f);

    REQUIRE(resultat.bloque == 25);
    REQUIRE(resultat.montant == 0);
}

TEST_CASE("appliquerBlocage : un tirage egal ou au-dessus de la chance ne bloque rien") {
    const ResultatDuBlocage pile = appliquerBlocage(100, 40, 0.30f, 0.30f);
    const ResultatDuBlocage haut = appliquerBlocage(100, 40, 0.30f, 0.90f);

    REQUIRE(pile.bloque == 0);
    REQUIRE(pile.montant == 100);
    REQUIRE(haut.bloque == 0);
    REQUIRE(haut.montant == 100);
}

TEST_CASE("appliquerBlocage : sans chance, jamais, meme au tirage 0") {
    const ResultatDuBlocage resultat = appliquerBlocage(100, 40, 0.0f, 0.0f);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(resultat.montant == 100);
}

TEST_CASE("appliquerBlocage : la portion est un flottant, le bouclier retire des points entiers") {
    const ResultatDuBlocage resultat = appliquerBlocage(52.5f, 40, 1.0f, 0.0f);

    REQUIRE_THAT(resultat.bloque, WithinAbs(40.0, s_tolerance));
    REQUIRE_THAT(resultat.montant, WithinAbs(12.5, s_tolerance));
}

TEST_CASE("appliquerBlocage : ne retire jamais plus que la portion") {
    const ResultatDuBlocage resultat = appliquerBlocage(12.5f, 40, 1.0f, 0.0f);

    REQUIRE_THAT(resultat.bloque, WithinAbs(12.5, s_tolerance));
    REQUIRE_THAT(resultat.montant, WithinAbs(0.0, s_tolerance));
}
