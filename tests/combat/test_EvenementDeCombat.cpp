// ./tests/combat/test_EvenementDeCombat.cpp
#include <catch2/catch_test_macros.hpp>
#include <variant>
#include <vector>

#include "combat/EvenementDeCombat.hpp"
#include "combat/ResolutionDuCoup.hpp"

TEST_CASE("detailsDuCoup : un coup esquive donne Esquive seul") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 0, .critique = false, .esquive = true}, false);

    REQUIRE(details.size() == 1);
    REQUIRE(std::holds_alternative<Esquive>(details[0]));
}

TEST_CASE("detailsDuCoup : des degats donnent Degats, avec le coup gratuit et le critique") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 70, .critique = true}, true);

    REQUIRE(details.size() == 1);
    const Degats& degats = std::get<Degats>(details[0]);
    REQUIRE(degats.montant == 70);
    REQUIRE(degats.opportunite);
    REQUIRE(degats.critique);
}

TEST_CASE("detailsDuCoup : un blocage partiel donne les degats puis Bloque, au-dessus d'eux") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 60, .critique = false, .esquive = false, .bloque = 40}, false);

    REQUIRE(details.size() == 2);
    REQUIRE(std::get<Degats>(details[0]).montant == 60);
    const Bloque& bloque = std::get<Bloque>(details[1]);
    REQUIRE(bloque.montant == 40);
    REQUIRE_FALSE(bloque.total);
}

TEST_CASE("detailsDuCoup : un blocage total donne Bloque seul, sans Degats de 0") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 0, .critique = false, .esquive = false, .bloque = 25}, false);

    REQUIRE(details.size() == 1);
    const Bloque& bloque = std::get<Bloque>(details[0]);
    REQUIRE(bloque.montant == 25);
    REQUIRE(bloque.total);
}

TEST_CASE("detailsDuCoup : un resultat rien (mort, invulnerable, montant nul) ne donne aucun evenement") {
    REQUIRE(detailsDuCoup({}, false).empty());
    REQUIRE(detailsDuCoup({}, true).empty());
}
