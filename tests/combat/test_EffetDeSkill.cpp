// ./tests/combat/test_EffetDeSkill.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/Degats.hpp"
#include "combat/EffetDeSkill.hpp"
#include "combat/Element.hpp"
#include "combat/NatureDAttaque.hpp"

TEST_CASE("campTouche : les degats, la frappe et la provocation touchent les monstres, le soin les allies") {
    REQUIRE(campTouche(EffetDegats{.montant = 45, .nature = NatureDAttaque::Zone, .element = Element::Feu}) == CampTouche::Monstres);
    REQUIRE(campTouche(EffetFrappe{.bonus = degatsDe(Element::Sacre, 45)}) == CampTouche::Monstres);
    REQUIRE(campTouche(EffetProvocation{.duree = 4}) == CampTouche::Monstres);
    REQUIRE(campTouche(EffetSoin{.montant = 90}) == CampTouche::Allies);
}

TEST_CASE("EffetDegats : l'element est physique et les coefficients nuls par defaut") {
    const EffetDegats degats{.montant = 75, .nature = NatureDAttaque::Contact};

    REQUIRE(degats.element == Element::Physique);
    REQUIRE(degats.coefficients == Coefficients());
}
