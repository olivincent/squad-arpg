// ./tests/test_Personnage.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Personnage.hpp"
#include "TileMap.hpp"

namespace {

// Carte dégagée dédiée aux tests de recharge : le personnage ne bouge
// pas (cible = son propre centre), seul le temps doit s'écouler.
TileMap carteDegagee() {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    return TileMap(plan, 40.0f);
}

void ecoulerTemps(Personnage& personnage, const TileMap& carte, sf::Vector2f centre, float duree) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        personnage.mettreAJour(pas, centre, carte);
        tempsRestant -= pas;
    }
}

}  // namespace

// Personnage centré en (100, 100), portée 80 px.

TEST_CASE("estAPortee renvoie vrai pour un point proche") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.estAPortee({150.0f, 100.0f}));
}

TEST_CASE("estAPortee renvoie faux pour un point lointain") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE_FALSE(personnage.estAPortee({300.0f, 100.0f}));
}

TEST_CASE("estAPortee renvoie vrai pour une distance exactement égale à la portée") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.estAPortee({180.0f, 100.0f}));
}

TEST_CASE("frapper renvoie les dégâts pour une cible à portée") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.frapper({150.0f, 100.0f}) == 5);
}

TEST_CASE("frapper renvoie 0 pour une cible hors de portée") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.frapper({300.0f, 100.0f}) == 0);
}

TEST_CASE("frapper renvoie 0 au second appel immédiat (recharge en cours)") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.frapper({150.0f, 100.0f}) == 5);
    REQUIRE(personnage.frapper({150.0f, 100.0f}) == 0);
}

TEST_CASE("frapper renvoie 0 après une durée simulée trop courte") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);
    const TileMap carte = carteDegagee();
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(personnage.frapper(cible) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.3f);
    REQUIRE(personnage.frapper(cible) == 0);
}

TEST_CASE("frapper renvoie de nouveau les dégâts après une durée simulée généreuse") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);
    const TileMap carte = carteDegagee();
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(personnage.frapper(cible) == 5);
    ecoulerTemps(personnage, carte, {100.0f, 100.0f}, 0.6f);
    REQUIRE(personnage.frapper(cible) == 5);
}

TEST_CASE("Un coup tenté hors de portée ne déclenche pas la recharge") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);
    const sf::Vector2f loin(300.0f, 100.0f);
    const sf::Vector2f proche(150.0f, 100.0f);

    REQUIRE(personnage.frapper(loin) == 0);
    REQUIRE(personnage.frapper(proche) == 5);
}

TEST_CASE("Un personnage est vivant a la creation") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est encore vivant apres 80 degats") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    personnage.recevoirDegats(80);

    REQUIRE(personnage.estVivant());
}

TEST_CASE("Un personnage est mort apres 100 degats") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    personnage.recevoirDegats(100);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("getCentre renvoie le centre donne au constructeur") {
    const Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    REQUIRE(personnage.getCentre() == sf::Vector2f(100.0f, 100.0f));
}

TEST_CASE("Un personnage est mort apres un coup qui depasse le reste de ses points de vie") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);

    personnage.recevoirDegats(80);
    personnage.recevoirDegats(50);

    REQUIRE_FALSE(personnage.estVivant());
}

TEST_CASE("Un personnage mort ne frappe plus") {
    Personnage personnage({100.0f, 100.0f}, sf::Color::White);
    const sf::Vector2f cible(150.0f, 100.0f);

    personnage.recevoirDegats(100);

    REQUIRE(personnage.frapper(cible) == 0);
}
