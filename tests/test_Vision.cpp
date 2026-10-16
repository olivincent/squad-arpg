// ./tests/test_Vision.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Cercle.hpp"
#include "TileMap.hpp"
#include "Vision.hpp"

namespace {

constexpr float s_tailleCase = 40.0f;

// Salle dégagée de 11 × 11 cases de 40 px.
TileMap salle() {
    return TileMap(std::vector<std::string>(11, std::string(11, '.')), s_tailleCase);
}

// Centre en pixels de la case (x, y).
sf::Vector2f centre(int x, int y) {
    return {(static_cast<float>(x) + 0.5f) * s_tailleCase, (static_cast<float>(y) + 0.5f) * s_tailleCase};
}

}  // namespace

TEST_CASE("Vision : tout est inconnu a la construction") {
    const TileMap carte = salle();
    const Vision vision(carte);

    for (int y = 0; y < 11; ++y) {
        for (int x = 0; x < 11; ++x) {
            REQUIRE(vision.etat({x, y}) == EtatCase::Inconnue);
        }
    }
    REQUIRE_FALSE(vision.estVisible(centre(5, 5)));
}

TEST_CASE("Vision : les cases du champ d'une source deviennent visibles") {
    const TileMap carte = salle();
    Vision vision(carte);

    vision.mettreAJour(carte, {Cercle{centre(5, 5), 2.0f * s_tailleCase}});

    REQUIRE(vision.etat({5, 5}) == EtatCase::Visible);
    REQUIRE(vision.etat({7, 5}) == EtatCase::Visible);
    REQUIRE(vision.etat({5, 3}) == EtatCase::Visible);
    REQUIRE(vision.etat({8, 5}) == EtatCase::Inconnue);
    REQUIRE(vision.etat({0, 0}) == EtatCase::Inconnue);
}

TEST_CASE("Vision : le rayon en cases est arrondi vers le bas") {
    const TileMap carte = salle();
    Vision vision(carte);

    vision.mettreAJour(carte, {Cercle{centre(5, 5), 2.9f * s_tailleCase}});

    REQUIRE(vision.etat({7, 5}) == EtatCase::Visible);
    REQUIRE(vision.etat({8, 5}) == EtatCase::Inconnue);
}

TEST_CASE("Vision : une case quittee devient exploree et ne redevient jamais inconnue") {
    const TileMap carte = salle();
    Vision vision(carte);
    vision.mettreAJour(carte, {Cercle{centre(2, 5), 2.0f * s_tailleCase}});
    REQUIRE(vision.etat({2, 5}) == EtatCase::Visible);

    vision.mettreAJour(carte, {Cercle{centre(8, 5), 2.0f * s_tailleCase}});

    REQUIRE(vision.etat({2, 5}) == EtatCase::Exploree);
    REQUIRE(vision.etat({0, 5}) == EtatCase::Exploree);
    REQUIRE(vision.etat({8, 5}) == EtatCase::Visible);
    REQUIRE(vision.etat({5, 5}) == EtatCase::Inconnue);  // jamais vue

    // Même sans aucune source, le visible devient exploré, puis le reste.
    vision.mettreAJour(carte, {});
    REQUIRE(vision.etat({8, 5}) == EtatCase::Exploree);
    vision.mettreAJour(carte, {});
    REQUIRE(vision.etat({8, 5}) == EtatCase::Exploree);
    REQUIRE(vision.etat({2, 5}) == EtatCase::Exploree);
}

TEST_CASE("Vision : plusieurs sources donnent l'union de leurs champs") {
    const TileMap carte = salle();
    Vision vision(carte);

    vision.mettreAJour(carte, {Cercle{centre(1, 1), 1.0f * s_tailleCase}, Cercle{centre(9, 9), 1.0f * s_tailleCase}});

    REQUIRE(vision.etat({1, 1}) == EtatCase::Visible);
    REQUIRE(vision.etat({2, 1}) == EtatCase::Visible);
    REQUIRE(vision.etat({9, 9}) == EtatCase::Visible);
    REQUIRE(vision.etat({9, 8}) == EtatCase::Visible);
    REQUIRE(vision.etat({5, 5}) == EtatCase::Inconnue);
}

TEST_CASE("Vision : hors de la carte, une case est inconnue") {
    const TileMap carte = salle();
    Vision vision(carte);
    vision.mettreAJour(carte, {Cercle{centre(0, 0), 3.0f * s_tailleCase}});

    REQUIRE(vision.etat({-1, 0}) == EtatCase::Inconnue);
    REQUIRE(vision.etat({0, -1}) == EtatCase::Inconnue);
    REQUIRE(vision.etat({11, 0}) == EtatCase::Inconnue);
    REQUIRE(vision.etat({0, 11}) == EtatCase::Inconnue);
}

TEST_CASE("Vision : estVisible prend un point en pixels du monde") {
    const TileMap carte = salle();
    Vision vision(carte);
    vision.mettreAJour(carte, {Cercle{centre(5, 5), 1.0f * s_tailleCase}});

    REQUIRE(vision.estVisible({200.0f, 200.0f}));           // coin haut gauche de la case (5, 5)
    REQUIRE(vision.estVisible({239.0f, 239.0f}));           // son coin bas droit (exclu à 240)
    REQUIRE(vision.estVisible({199.0f, 220.0f}));           // case (4, 5), visible aussi
    REQUIRE_FALSE(vision.estVisible({320.0f, 220.0f}));     // case (8, 5), inconnue
    REQUIRE_FALSE(vision.estVisible({-5.0f, -5.0f}));       // hors de la carte
    REQUIRE_FALSE(vision.estVisible({10000.0f, 220.0f}));
}

TEST_CASE("Vision : un mur arrete la vue de la source") {
    std::vector<std::string> plan(5, std::string(9, '.'));
    for (std::string& ligne : plan) {
        ligne[4] = '#';
    }
    const TileMap carte(plan, s_tailleCase);
    Vision vision(carte);

    vision.mettreAJour(carte, {Cercle{centre(1, 2), 6.0f * s_tailleCase}});

    REQUIRE(vision.etat({4, 2}) == EtatCase::Visible);  // la face du mur
    REQUIRE(vision.etat({6, 2}) == EtatCase::Inconnue);
}
