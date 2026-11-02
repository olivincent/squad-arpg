// ./tests/combat/test_RendementsDeControle.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "combat/RendementsDeControle.hpp"

using Catch::Approx;

TEST_CASE("RendementsDeControle : 100 %, 50 %, 25 %, puis immunite") {
    RendementsDeControle rendements;

    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(1.0f));
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(0.5f));
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(0.25f));
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(0.0f));
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(0.0f));
}

TEST_CASE("RendementsDeControle : facteurSuivant ne compte pas") {
    RendementsDeControle rendements;

    REQUIRE(rendements.facteurSuivant(EtatInflige::Disruption) == Approx(1.0f));
    REQUIRE(rendements.facteurSuivant(EtatInflige::Disruption) == Approx(1.0f));
    REQUIRE(rendements.applications(EtatInflige::Disruption) == 0);
}

TEST_CASE("RendementsDeControle : chaque etat a son compteur") {
    RendementsDeControle rendements;
    static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));
    static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));

    REQUIRE(rendements.appliquer(EtatInflige::Ralentissement) == Approx(1.0f));
    REQUIRE(rendements.appliquer(EtatInflige::Entrave) == Approx(1.0f));
    REQUIRE(rendements.appliquer(EtatInflige::Disruption) == Approx(1.0f));
    REQUIRE(rendements.applications(EtatInflige::Etourdissement) == 2);
}

TEST_CASE("RendementsDeControle : le compteur revient a zero 15 s apres la derniere application") {
    RendementsDeControle rendements;
    static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));
    static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));

    rendements.mettreAJour(14.0f);
    REQUIRE(rendements.applications(EtatInflige::Etourdissement) == 2);
    rendements.mettreAJour(1.0f);

    REQUIRE(rendements.applications(EtatInflige::Etourdissement) == 0);
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(1.0f));
}

TEST_CASE("RendementsDeControle : une application qui joue repousse la remise a zero") {
    RendementsDeControle rendements;
    static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));
    rendements.mettreAJour(10.0f);
    static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));

    rendements.mettreAJour(10.0f);  // 20 s depuis la première, 10 s depuis la dernière

    REQUIRE(rendements.applications(EtatInflige::Etourdissement) == 2);
}

TEST_CASE("RendementsDeControle : une application bloquee par l'immunite ne repousse pas la remise a zero") {
    RendementsDeControle rendements;
    for (int i = 0; i < 3; ++i) {
        static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));
    }
    rendements.mettreAJour(10.0f);
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(0.0f));  // bloquée à 10 s

    rendements.mettreAJour(5.0f);  // 15 s depuis la dernière application qui a joué

    REQUIRE(rendements.applications(EtatInflige::Etourdissement) == 0);
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(1.0f));
}

TEST_CASE("RendementsDeControle : vider remet tout a zero") {
    RendementsDeControle rendements;
    for (int i = 0; i < 3; ++i) {
        static_cast<void>(rendements.appliquer(EtatInflige::Etourdissement));
    }
    static_cast<void>(rendements.appliquer(EtatInflige::Ralentissement));

    rendements.vider();

    REQUIRE(rendements.applications(EtatInflige::Etourdissement) == 0);
    REQUIRE(rendements.applications(EtatInflige::Ralentissement) == 0);
    REQUIRE(rendements.appliquer(EtatInflige::Etourdissement) == Approx(1.0f));
}
