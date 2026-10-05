// ./tests/test_Deplacement.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Deplacement.hpp"
#include "TileMap.hpp"

TEST_CASE("deplacerAvecGlissement avance normalement en terrain dégagé") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {50.0f, 30.0f});

    REQUIRE(position == sf::Vector2f(50.0f, 30.0f));
}

TEST_CASE("Un mur bloque complètement un déplacement sur un seul axe") {
    // Mur en (2, 2) : x et y dans [80, 120). Rectangle en (50, 90), taille
    // 20x20 (y : [90, 110), dans la bande du mur). Déplacement purement
    // horizontal de 60 px : l'essai en X couvrirait x : [110, 130), qui
    // chevauche la case du mur. Aucun déplacement en Y n'est demandé :
    // la position ne doit donc pas changer du tout.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({50.0f, 90.0f}, {20.0f, 20.0f});
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {60.0f, 0.0f});

    REQUIRE(position == sf::Vector2f(50.0f, 90.0f));
}

TEST_CASE("Un déplacement oblique contre un mur ne garde que la composante parallèle") {
    // Même mur et même rectangle de départ que le test précédent.
    // Déplacement oblique (60, 30) : l'essai en X échoue pour la même
    // raison (chevauche le mur), mais l'essai en Y part de la position
    // inchangée et vise (50, 120), qui ne chevauche pas le mur (x reste
    // dans [50, 70), hors de la bande [80, 120) du mur) : il réussit. Seule
    // la composante verticale (parallèle à la face du mur) est conservée.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        "..#..",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({50.0f, 90.0f}, {20.0f, 20.0f});
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {60.0f, 30.0f});

    REQUIRE(position == sf::Vector2f(50.0f, 120.0f));
}

TEST_CASE("Un déplacement nul laisse la position inchangée") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({50.0f, 50.0f}, {20.0f, 20.0f});
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {0.0f, 0.0f});

    REQUIRE(position == sf::Vector2f(50.0f, 50.0f));
}
