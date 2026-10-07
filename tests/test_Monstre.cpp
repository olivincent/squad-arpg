// ./tests/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "Monstre.hpp"

// Monstre centré en (100, 100), taille 30x30 : bords en x: [85, 115),
// y: [85, 115).

TEST_CASE("contient renvoie vrai pour un point au centre") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.contient({100.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point à l'extérieur") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE_FALSE(monstre.contient({200.0f, 200.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord gauche") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.contient({85.0f, 100.0f}));
}

TEST_CASE("contient renvoie vrai pour un point sur le bord haut") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.contient({100.0f, 85.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord droit") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE_FALSE(monstre.contient({115.0f, 100.0f}));
}

TEST_CASE("contient renvoie faux pour un point sur le bord bas") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE_FALSE(monstre.contient({100.0f, 115.0f}));
}

TEST_CASE("getCentre renvoie le centre du monstre") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("getId renvoie l'identifiant de chaque monstre") {
    const Monstre premier({100.0f, 100.0f}, 1);
    const Monstre second({200.0f, 200.0f}, 2);

    REQUIRE(premier.getId() == 1);
    REQUIRE(second.getId() == 2);
}

// Le monstre a 100 points de vie à la création.

TEST_CASE("Le monstre est vivant à la création") {
    const Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est encore vivant après 80 points de dégâts") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(80);

    REQUIRE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort après 100 points de dégâts") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(100);

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre est mort après un coup qui dépasse les points de vie restants") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(80); // reste 20 points de vie
    monstre.recevoirDegats(50); // dépasse largement le reste

    REQUIRE_FALSE(monstre.estVivant());
}

TEST_CASE("Le monstre reste mort après un coup supplémentaire") {
    Monstre monstre({100.0f, 100.0f}, 1);

    monstre.recevoirDegats(100);
    monstre.recevoirDegats(1);

    REQUIRE_FALSE(monstre.estVivant());
}
