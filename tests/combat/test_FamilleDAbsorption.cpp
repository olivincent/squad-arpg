// ./tests/combat/test_FamilleDAbsorption.cpp
#include <catch2/catch_test_macros.hpp>

#include "combat/FamilleDAbsorption.hpp"

TEST_CASE("familleQuiAbsorbe : le physique va aux Aegis, tout le reste aux Ward") {
    REQUIRE(familleQuiAbsorbe(Element::Physique) == FamilleDAbsorption::Aegis);
    REQUIRE(familleQuiAbsorbe(Element::Feu) == FamilleDAbsorption::Ward);
    REQUIRE(familleQuiAbsorbe(Element::Sacre) == FamilleDAbsorption::Ward);
}

TEST_CASE("familleQuiImmunise : Aegis pour le controle physique, Ward pour le controle magique") {
    REQUIRE(familleQuiImmunise(EtatDeControle::Etourdissement) == FamilleDAbsorption::Aegis);
    REQUIRE(familleQuiImmunise(EtatDeControle::Ralentissement) == FamilleDAbsorption::Aegis);
    REQUIRE(familleQuiImmunise(EtatDeControle::Charme) == FamilleDAbsorption::Ward);
    REQUIRE(familleQuiImmunise(EtatDeControle::Peur) == FamilleDAbsorption::Ward);
    REQUIRE(familleQuiImmunise(EtatDeControle::Disruption) == FamilleDAbsorption::Ward);
}

TEST_CASE("PointsDAbsorption : les points se lisent par famille") {
    const PointsDAbsorption points{.aegis = 30, .ward = 12};

    REQUIRE(points.deLaFamille(FamilleDAbsorption::Aegis) == 30);
    REQUIRE(points.deLaFamille(FamilleDAbsorption::Ward) == 12);
}

TEST_CASE("portionsAbsorbees : la portion physique va aux Aegis, les autres aux Ward") {
    Degats portions;
    portions[Element::Physique] = 40;
    portions[Element::Feu] = 25;
    portions[Element::Sacre] = 10;

    const Degats absorbees = portionsAbsorbees(portions, {.aegis = 15, .ward = 100});

    REQUIRE(absorbees[Element::Physique] == 15);  // l'Aegis ne couvre pas tout le physique
    REQUIRE(absorbees[Element::Feu] == 25);
    REQUIRE(absorbees[Element::Sacre] == 10);
}

TEST_CASE("portionsAbsorbees : un Aegis n'absorbe jamais le feu, un Ward jamais le physique") {
    const Degats physique = degatsPhysiques(30);
    const Degats feu = degatsDe(Element::Feu, 30);

    REQUIRE(portionsAbsorbees(physique, {.aegis = 0, .ward = 100}).total() == 0);
    REQUIRE(portionsAbsorbees(feu, {.aegis = 100, .ward = 0}).total() == 0);
}

TEST_CASE("portionsAbsorbees : un Ward trop petit couvre les elements dans l'ordre de l'enumeration") {
    Degats portions;
    portions[Element::Feu] = 25;
    portions[Element::Sacre] = 10;

    const Degats absorbees = portionsAbsorbees(portions, {.aegis = 0, .ward = 30});

    REQUIRE(absorbees[Element::Feu] == 25);
    REQUIRE(absorbees[Element::Sacre] == 5);  // il ne reste que 5 au Ward
}

TEST_CASE("portionsAbsorbees : sans bouclier, rien n'est absorbe") {
    REQUIRE(portionsAbsorbees(degatsPhysiques(50), {}).total() == 0);
}
