// ./tests/interface/test_EcranMenu.cpp
#include <catch2/catch_test_macros.hpp>

#include <variant>

#include "interface/EcranMenu.hpp"
#include "interface/ListeDeBoutons.hpp"

namespace {

const sf::Vector2f s_centre{600.0f, 400.0f};

// Centre du bouton `indice` du menu (même liste de libellés que EcranMenu).
sf::Vector2f centreDuBouton(std::size_t indice) {
    const ListeDeBoutons boutons({"Commencer partie", "Options", "Quitter"});
    const sf::FloatRect rect = boutons.rectangleDuBouton(indice, s_centre);
    return rect.position + rect.size / 2.0f;
}

}  // namespace

TEST_CASE("menu : Commencer partie empile un ecran") {
    const EcranMenu menu;

    const Transition transition = menu.cliquer(centreDuBouton(0), s_centre, Reglages{});

    REQUIRE(std::holds_alternative<Empiler>(transition));
    REQUIRE(std::get<Empiler>(transition).ecran != nullptr);
}

TEST_CASE("menu : Options empile l'ecran des options") {
    const EcranMenu menu;

    const Transition transition = menu.cliquer(centreDuBouton(1), s_centre, Reglages{});

    REQUIRE(std::holds_alternative<Empiler>(transition));
    REQUIRE(std::get<Empiler>(transition).ecran != nullptr);
}

TEST_CASE("menu : Quitter quitte") {
    const EcranMenu menu;

    REQUIRE(std::holds_alternative<Quitter>(menu.cliquer(centreDuBouton(2), s_centre, Reglages{})));
}

TEST_CASE("menu : un clic hors des boutons ne fait rien") {
    const EcranMenu menu;

    REQUIRE(std::holds_alternative<Aucune>(menu.cliquer({5.0f, 5.0f}, s_centre, Reglages{})));
}
