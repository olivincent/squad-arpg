// ./tests/combat/test_Element.cpp
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <type_traits>

#include "combat/Degats.hpp"
#include "combat/Element.hpp"

TEST_CASE("ParElement : toutes les valeurs sont a zero par defaut") {
    const ParElement<int> valeurs;

    for (const Element element : s_elements) {
        REQUIRE(valeurs[element] == 0);
    }
    REQUIRE(valeurs.total() == 0);
}

TEST_CASE("ParElement : l'acces par element lit et ecrit la valeur de cet element seul") {
    ParElement<int> valeurs;

    valeurs[Element::Feu] = 40;

    REQUIRE(valeurs[Element::Feu] == 40);
    REQUIRE(valeurs[Element::Physique] == 0);
    REQUIRE(valeurs[Element::Sacre] == 0);
}

TEST_CASE("ParElement : le total est la somme des portions") {
    ParElement<int> valeurs;
    valeurs[Element::Physique] = 15;
    valeurs[Element::Feu] = 20;
    valeurs[Element::Sacre] = 45;

    REQUIRE(valeurs.total() == 80);
}

TEST_CASE("ParElement : le total d'une valeur negative se soustrait") {
    ParElement<int> resistances;
    resistances[Element::Feu] = -50;
    resistances[Element::Sacre] = 30;

    REQUIRE(resistances.total() == -20);
}

TEST_CASE("ParElement : la copie est une valeur, la modifier ne touche pas l'original") {
    ParElement<int> original;
    original[Element::Feu] = 10;

    ParElement<int> copie = original;
    copie[Element::Feu] = 99;

    REQUIRE(original[Element::Feu] == 10);
    REQUIRE(copie[Element::Feu] == 99);
    REQUIRE(original != copie);
    copie[Element::Feu] = 10;
    REQUIRE(original == copie);
}

TEST_CASE("ParElement : sa taille suit l'enumeration, un element de plus l'agrandit") {
    // La sentinelle `Element::Nombre` donne la taille : rien d'autre à mettre à jour en ajoutant un élément.
    STATIC_REQUIRE(s_nombreDElements == static_cast<std::size_t>(Element::Nombre));
    STATIC_REQUIRE(ParElement<int>{}.valeurs.size() == s_nombreDElements);
    STATIC_REQUIRE(s_elements.size() == s_nombreDElements);
    STATIC_REQUIRE(s_nombreDElements >= 3);  // physique, feu, sacré
}

TEST_CASE("s_elements liste chaque element une fois, dans l'ordre de l'enumeration") {
    for (std::size_t i = 0; i < s_nombreDElements; ++i) {
        REQUIRE(static_cast<std::size_t>(s_elements[i]) == i);
    }
    REQUIRE(s_elements.front() == Element::Physique);
    REQUIRE(s_elements.back() != Element::Nombre);  // la sentinelle n'est pas un élément
}

TEST_CASE("ParElement est un type valeur sans reference ni pointeur") {
    STATIC_REQUIRE(std::is_trivially_copyable_v<ParElement<int>>);
    STATIC_REQUIRE(std::is_standard_layout_v<ParElement<int>>);
}

TEST_CASE("ParElement fonctionne avec un autre type que int") {
    ParElement<float> parts;
    parts[Element::Sacre] = 0.5f;

    REQUIRE(parts[Element::Sacre] == 0.5f);
    REQUIRE(parts[Element::Feu] == 0.0f);
    REQUIRE(parts.total() == 0.5f);
}

TEST_CASE("degatsDe : des degats d'un seul element") {
    const Degats degats = degatsDe(Element::Feu, 45);

    REQUIRE(degats[Element::Feu] == 45);
    REQUIRE(degats[Element::Physique] == 0);
    REQUIRE(degats[Element::Sacre] == 0);
    REQUIRE(degats.total() == 45);
}

TEST_CASE("Degats : des portions de plusieurs elements s'additionnent dans le total") {
    Degats degats = degatsDe(Element::Physique, 15);
    degats[Element::Sacre] = 45;

    REQUIRE(degats.total() == 60);
}
