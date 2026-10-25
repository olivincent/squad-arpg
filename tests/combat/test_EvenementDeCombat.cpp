// ./tests/combat/test_EvenementDeCombat.cpp
#include <catch2/catch_test_macros.hpp>
#include <variant>
#include <vector>

#include "combat/Degats.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/ResolutionDuCoup.hpp"

TEST_CASE("detailsDuCoup : un coup esquive donne Esquive seul") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 0, .critique = false, .esquive = true}, false);

    REQUIRE(details.size() == 1);
    REQUIRE(std::holds_alternative<Esquive>(details[0]));
}

TEST_CASE("detailsDuCoup : des degats donnent DegatsSubis, avec le coup gratuit et le critique") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 70, .critique = true}, true);

    REQUIRE(details.size() == 1);
    const DegatsSubis& degats = std::get<DegatsSubis>(details[0]);
    REQUIRE(degats.montant == 70);
    REQUIRE(degats.opportunite);
    REQUIRE(degats.critique);
}

TEST_CASE("detailsDuCoup : un blocage partiel donne les degats puis Bloque, au-dessus d'eux") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 60, .critique = false, .esquive = false, .bloque = 40}, false);

    REQUIRE(details.size() == 2);
    REQUIRE(std::get<DegatsSubis>(details[0]).montant == 60);
    const Bloque& bloque = std::get<Bloque>(details[1]);
    REQUIRE(bloque.montant == 40);
    REQUIRE_FALSE(bloque.total);
}

TEST_CASE("detailsDuCoup : un blocage total donne Bloque seul, sans DegatsSubis de 0") {
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

TEST_CASE("detailsDuCoup : l'element des degats est celui de la plus grosse portion apres les reductions") {
    Degats portions = degatsDe(Element::Physique, 20);
    portions[Element::Feu] = 35;
    portions[Element::Sacre] = 5;

    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 60, .critique = false, .esquive = false, .bloque = 0, .portions = portions}, false);

    REQUIRE(details.size() == 1);
    REQUIRE(std::get<DegatsSubis>(details[0]).element == Element::Feu);
    REQUIRE(std::get<DegatsSubis>(details[0]).montant == 60);  // un seul nombre : le total
}

TEST_CASE("detailsDuCoup : a egalite des portions, l'element est le premier de l'enumeration") {
    Degats portions = degatsDe(Element::Sacre, 10);
    portions[Element::Feu] = 10;

    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 20, .critique = false, .esquive = false, .bloque = 0, .portions = portions}, false);

    REQUIRE(std::get<DegatsSubis>(details[0]).element == Element::Feu);
}

TEST_CASE("detailsDuCoup : des degats purement physiques gardent l'element physique") {
    const std::vector<DetailEvenement> details = detailsDuCoup({.montant = 20, .critique = false, .esquive = false, .bloque = 0, .portions = degatsPhysiques(20)}, false);

    REQUIRE(std::get<DegatsSubis>(details[0]).element == Element::Physique);
}
