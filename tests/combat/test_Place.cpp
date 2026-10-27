// ./tests/combat/test_Place.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <type_traits>

#include "combat/IdMonstre.hpp"
#include "combat/Place.hpp"

// Les deux types forts : une place (de 1 à N) et un identifiant de monstre ne se confondent pas entre eux ni avec un entier.
static_assert(!std::is_convertible_v<std::size_t, Place>);
static_assert(!std::is_convertible_v<int, IdMonstre>);
static_assert(!std::is_convertible_v<Place, IdMonstre>);
static_assert(!std::is_convertible_v<IdMonstre, Place>);

TEST_CASE("Place : le numero de 1 a N donne l'indice de 0 a N-1") {
    REQUIRE(Place{1}.indice() == 0);
    REQUIRE(Place{2}.indice() == 1);
    REQUIRE(Place{3}.indice() == 2);
}

TEST_CASE("Place::depuisIndice est l'inverse de indice") {
    for (std::size_t indice = 0; indice < 5; ++indice) {
        REQUIRE(Place::depuisIndice(indice).numero == indice + 1);
        REQUIRE(Place::depuisIndice(indice).indice() == indice);
    }
    REQUIRE(Place::depuisIndice(0) == Place{1});
}

TEST_CASE("Place : egalite et ordre par numero") {
    REQUIRE(Place{2} == Place{2});
    REQUIRE(Place{2} != Place{3});
    REQUIRE(Place{1} < Place{2});
    REQUIRE(Place{3} > Place{2});
}

TEST_CASE("IdMonstre : egalite et ordre par valeur") {
    REQUIRE(IdMonstre{7} == IdMonstre{7});
    REQUIRE(IdMonstre{7} != IdMonstre{8});
    REQUIRE(IdMonstre{7} < IdMonstre{8});
}
