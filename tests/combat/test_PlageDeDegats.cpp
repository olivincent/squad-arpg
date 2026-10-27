// ./tests/combat/test_PlageDeDegats.cpp
#include <catch2/catch_test_macros.hpp>
#include <set>

#include "aides.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Degats.hpp"
#include "combat/Element.hpp"
#include "combat/PlageDeDegats.hpp"

// Les dégâts d'une arme avant le coup (étape 102.3) : une plage par élément, dont chaque coup tire ses dégâts.
namespace {
PlageDeDegats physique(int min, int max) {
    return {degatsPhysiques(min), degatsPhysiques(max)};
}
}  // namespace

TEST_CASE("plageFixe : le minimum est le maximum") {
    const PlageDeDegats plage = plageFixe(degatsPhysiques(12));

    REQUIRE(plage.min == degatsPhysiques(12));
    REQUIRE(plage.max == degatsPhysiques(12));
}

TEST_CASE("tirerLesDegats : une plage fixe donne ses degats et ne tire rien") {
    Generateur generateur = generateurDeTest();
    const Generateur avant = generateur;

    REQUIRE(tirerLesDegats(generateur, plageFixe(degatsPhysiques(12))) == degatsPhysiques(12));
    REQUIRE(tirerLesDegats(generateur, plageFixe(Degats{})) == Degats{});

    REQUIRE(generateur == avant);
}

TEST_CASE("tirerLesDegats : les degats restent dans la plage et les deux bornes sortent") {
    Generateur generateur = generateurDeTest();
    std::set<int> vus;

    for (int i = 0; i < 2000; ++i) {
        const Degats degats = tirerLesDegats(generateur, physique(13, 17));
        REQUIRE(degats[Element::Physique] >= 13);
        REQUIRE(degats[Element::Physique] <= 17);
        REQUIRE(degats[Element::Feu] == 0);
        vus.insert(degats[Element::Physique]);
    }

    REQUIRE(vus == std::set<int>{13, 14, 15, 16, 17});
}

TEST_CASE("tirerLesDegats : un tirage par element qui a un ecart, dans l'ordre des elements, aucun pour les autres") {
    PlageDeDegats plage;
    plage.min[Element::Physique] = 5;
    plage.max[Element::Physique] = 9;
    plage.min[Element::Feu] = 4;  // une portion fixe de feu, comme celle d'un affixe : aucun tirage
    plage.max[Element::Feu] = 4;
    plage.min[Element::Sacre] = 2;
    plage.max[Element::Sacre] = 3;
    Generateur generateur = generateurDeTest();
    Generateur temoin = generateurDeTest();

    const Degats degats = tirerLesDegats(generateur, plage);

    REQUIRE(degats[Element::Physique] == tirerEntre(temoin, 5, 9));
    REQUIRE(degats[Element::Feu] == 4);
    REQUIRE(degats[Element::Sacre] == tirerEntre(temoin, 2, 3));
    REQUIRE(generateur == temoin);  // exactement deux tirages
}

TEST_CASE("tirerLesDegats : la meme graine donne les memes degats") {
    Generateur a = generateurDeTest();
    Generateur b = generateurDeTest();

    for (int i = 0; i < 50; ++i) {
        REQUIRE(tirerLesDegats(a, physique(1, 100)) == tirerLesDegats(b, physique(1, 100)));
    }
}
