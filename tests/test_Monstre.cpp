// ./tests/test_Monstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "Monstre.hpp"
#include "TileMap.hpp"

namespace {

TileMap carteDegagee() {
    const std::vector<std::string> plan = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    return TileMap(plan, 40.0f);
}

// Simule le temps qui passe par tics de 1/60 s, comme dans main.cpp.
void ecoulerTemps(Monstre& monstre, float duree, std::optional<sf::Vector2f> cible, const TileMap& carte) {
    const float dt = 1.0f / 60.0f;
    float tempsRestant = duree;
    while (tempsRestant > 0.0f) {
        const float pas = (tempsRestant < dt) ? tempsRestant : dt;
        monstre.mettreAJour(pas, cible, carte);
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
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(150.0f, 100.0f);

    REQUIRE(monstre.frapper(cible) == 10);
    ecoulerTemps(monstre, 1.2f, std::nullopt, carte);
    REQUIRE(monstre.frapper(cible) == 10);
}

// Monstre centré en (100, 100), rayon de détection 200 px.

TEST_CASE("Monstre.mettreAJour approche une cible dans le rayon de détection") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(300.0f, 100.0f);
    const float distanceInitiale = (cible - monstre.getCentre()).length();

    ecoulerTemps(monstre, 0.5f, cible, carte);

    const float distanceApres = (cible - monstre.getCentre()).length();
    REQUIRE(distanceApres < distanceInitiale);
}

TEST_CASE("Monstre.mettreAJour ignore une cible hors du rayon de détection") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f cible(400.0f, 100.0f); // 300 px, hors des 200 px de détection
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, cible, carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}

TEST_CASE("Monstre.mettreAJour reste immobile sans cible") {
    const TileMap carte = carteDegagee();
    Monstre monstre({100.0f, 100.0f}, 1);
    const sf::Vector2f centreInitial = monstre.getCentre();

    ecoulerTemps(monstre, 1.0f, std::nullopt, carte);

    REQUIRE(monstre.getCentre() == centreInitial);
}
