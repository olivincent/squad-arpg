// ./tests/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "Monstre.hpp"

namespace {

// Simule le temps qui passe par tics de 1/60 s, comme dans main.cpp.
// Prouve que Monstre::mettreAJour fait bien avancer la recharge de m_attaque.
void ecoulerTemps(Monstre& monstre, float duree) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        monstre.mettreAJour(pas);
        tempsRestant -= pas;
    }
}

}  // namespace

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

// Monstre centré en (100, 100), portée 80 px, dégâts 10, recharge 1 s.

TEST_CASE("Monstre.frapper renvoie les dégâts pour une cible à portée") {
    Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.frapper({150.0f, 100.0f}) == 10);
}

TEST_CASE("Monstre.frapper renvoie 0 pour une cible hors de portée") {
    Monstre monstre({100.0f, 100.0f}, 1);

    REQUIRE(monstre.frapper({300.0f, 100.0f}) == 0);
}

TEST_CASE("Monstre.frapper renvoie de nouveau les dégâts après une durée simulée généreuse") {
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(monstre.frapper(cible) == 10);
    ecoulerTemps(monstre, 1.2f);
    REQUIRE(monstre.frapper(cible) == 10);
}
