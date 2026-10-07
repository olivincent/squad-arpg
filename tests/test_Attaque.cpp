// ./tests/test_Attaque.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "Attaque.hpp"

namespace {

void ecoulerTemps(Attaque& attaque, float duree) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        attaque.mettreAJour(pas);
        tempsRestant -= pas;
    }
}

}  // namespace

// Origine en (100, 100), portée 80 px.

TEST_CASE("Attaque.estAPortee renvoie vrai pour un point proche") {
    Attaque attaque(80.0f, 5, 0.5f);

    REQUIRE(attaque.estAPortee({100.0f, 100.0f}, {150.0f, 100.0f}));
}

TEST_CASE("Attaque.estAPortee renvoie faux pour un point lointain") {
    Attaque attaque(80.0f, 5, 0.5f);

    REQUIRE_FALSE(attaque.estAPortee({100.0f, 100.0f}, {300.0f, 100.0f}));
}

TEST_CASE("Attaque.estAPortee renvoie vrai pour une distance exactement égale à la portée") {
    Attaque attaque(80.0f, 5, 0.5f);

    REQUIRE(attaque.estAPortee({100.0f, 100.0f}, {180.0f, 100.0f}));
}

TEST_CASE("Attaque.frapper renvoie les dégâts pour une cible à portée") {
    Attaque attaque(80.0f, 5, 0.5f);

    REQUIRE(attaque.frapper({100.0f, 100.0f}, {150.0f, 100.0f}) == 5);
}

TEST_CASE("Attaque.frapper renvoie 0 pour une cible hors de portée") {
    Attaque attaque(80.0f, 5, 0.5f);

    REQUIRE(attaque.frapper({100.0f, 100.0f}, {300.0f, 100.0f}) == 0);
}

TEST_CASE("Attaque.frapper renvoie 0 au second appel immédiat (recharge en cours)") {
    Attaque attaque(80.0f, 5, 0.5f);
    const sf::Vector2f origine(100.0f, 100.0f);
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(attaque.frapper(origine, cible) == 5);
    REQUIRE(attaque.frapper(origine, cible) == 0);
}

TEST_CASE("Attaque.frapper renvoie 0 après une durée simulée trop courte") {
    Attaque attaque(80.0f, 5, 0.5f);
    const sf::Vector2f origine(100.0f, 100.0f);
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(attaque.frapper(origine, cible) == 5);
    ecoulerTemps(attaque, 0.3f);
    REQUIRE(attaque.frapper(origine, cible) == 0);
}

TEST_CASE("Attaque.frapper renvoie de nouveau les dégâts après une durée simulée généreuse") {
    Attaque attaque(80.0f, 5, 0.5f);
    const sf::Vector2f origine(100.0f, 100.0f);
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(attaque.frapper(origine, cible) == 5);
    ecoulerTemps(attaque, 0.6f);
    REQUIRE(attaque.frapper(origine, cible) == 5);
}

TEST_CASE("Un coup d'Attaque tenté hors de portée ne déclenche pas la recharge") {
    Attaque attaque(80.0f, 5, 0.5f);
    const sf::Vector2f origine(100.0f, 100.0f);
    const sf::Vector2f loin(300.0f, 100.0f);
    const sf::Vector2f proche(150.0f, 100.0f);

    REQUIRE(attaque.frapper(origine, loin) == 0);
    REQUIRE(attaque.frapper(origine, proche) == 5);
}
