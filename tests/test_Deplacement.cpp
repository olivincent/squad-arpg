// ./tests/test_Deplacement.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Cercle.hpp"
#include "Deplacement.hpp"
#include "TileMap.hpp"

TEST_CASE("deplacementBloque : aucun obstacle ne bloque rien") {
    const Cercle avant{{0.0f, 0.0f}, 10.0f};
    const Cercle apres{{50.0f, 0.0f}, 10.0f};

    REQUIRE_FALSE(deplacementBloque(avant, apres, {}));
}

TEST_CASE("deplacementBloque : un deplacement qui entre dans un obstacle est bloque") {
    // Obstacle à 40 px, rayons 10 + 10 = 20. Après le déplacement, la
    // distance est 10 < 20 (chevauchement) et elle a diminué (40 -> 10).
    const Cercle avant{{0.0f, 0.0f}, 10.0f};
    const Cercle apres{{30.0f, 0.0f}, 10.0f};
    const std::vector<Cercle> obstacles = {{{40.0f, 0.0f}, 10.0f}};

    REQUIRE(deplacementBloque(avant, apres, obstacles));
}

TEST_CASE("deplacementBloque : arriver exactement au contact n'est pas bloque") {
    // Distance après = 20 = 10 + 10 : les cercles se touchent sans se chevaucher.
    const Cercle avant{{0.0f, 0.0f}, 10.0f};
    const Cercle apres{{20.0f, 0.0f}, 10.0f};
    const std::vector<Cercle> obstacles = {{{40.0f, 0.0f}, 10.0f}};

    REQUIRE_FALSE(deplacementBloque(avant, apres, obstacles));
}

TEST_CASE("deplacementBloque : deja en chevauchement, s'eloigner n'est pas bloque") {
    // Distance avant 10, après 15 : toujours en chevauchement (< 20) mais on s'éloigne.
    const Cercle avant{{0.0f, 0.0f}, 10.0f};
    const Cercle apres{{-5.0f, 0.0f}, 10.0f};
    const std::vector<Cercle> obstacles = {{{10.0f, 0.0f}, 10.0f}};

    REQUIRE_FALSE(deplacementBloque(avant, apres, obstacles));
}

TEST_CASE("deplacementBloque : deja en chevauchement, se rapprocher est bloque") {
    // Distance avant 15, après 10 : en chevauchement et plus près.
    const Cercle avant{{0.0f, 0.0f}, 10.0f};
    const Cercle apres{{5.0f, 0.0f}, 10.0f};
    const std::vector<Cercle> obstacles = {{{15.0f, 0.0f}, 10.0f}};

    REQUIRE(deplacementBloque(avant, apres, obstacles));
}

TEST_CASE("deplacementBloque : un seul obstacle sur plusieurs suffit a bloquer") {
    const Cercle avant{{0.0f, 0.0f}, 10.0f};
    const Cercle apres{{30.0f, 0.0f}, 10.0f};
    const std::vector<Cercle> obstacles = {
        {{0.0f, 200.0f}, 10.0f},   // loin
        {{40.0f, 0.0f}, 10.0f},    // bloque
        {{-100.0f, 0.0f}, 10.0f},  // loin
    };

    REQUIRE(deplacementBloque(avant, apres, obstacles));
}

TEST_CASE("deplacerAvecGlissement avance normalement en terrain degage") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {50.0f, 30.0f}, {});

    REQUIRE(position == sf::Vector2f(50.0f, 30.0f));
}

TEST_CASE("Un mur bloque completement un deplacement sur un seul axe") {
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
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {60.0f, 0.0f}, {});

    REQUIRE(position == sf::Vector2f(50.0f, 90.0f));
}

TEST_CASE("Un deplacement oblique contre un mur ne garde que la composante parallele") {
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
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {60.0f, 30.0f}, {});

    REQUIRE(position == sf::Vector2f(50.0f, 120.0f));
}

TEST_CASE("Un deplacement nul laisse la position inchangee") {
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({50.0f, 50.0f}, {20.0f, 20.0f});
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {0.0f, 0.0f}, {});

    REQUIRE(position == sf::Vector2f(50.0f, 50.0f));
}

TEST_CASE("Un obstacle droit devant bloque le deplacement sur cet axe") {
    // Rectangle 20x20 en (0, 0), centre (10, 10), rayon 10. Obstacle de rayon 10
    // centré en (50, 10). Déplacement de +30 en X : le centre irait en
    // (40, 10), à 10 < 10 + 10 de l'obstacle et plus près qu'avant (40) :
    // bloqué. Aucun déplacement en Y : la position ne change pas.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    const std::vector<Cercle> obstacles = {{{50.0f, 10.0f}, 10.0f}};
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {30.0f, 0.0f}, obstacles);

    REQUIRE(position == sf::Vector2f(0.0f, 0.0f));
}

TEST_CASE("Un deplacement oblique contre un obstacle laisse passer l'axe libre") {
    // Même rectangle et même obstacle. Déplacement (30, 30) : l'essai en X est
    // bloqué (voir ci-dessus), l'essai en Y part de (0, 0) vers centre (10, 40) :
    // distance à (50, 10) = 50 > 20, libre. Seule la composante Y passe.
    const std::vector<std::string> plan = {
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    };
    TileMap carte(plan, 40.0f);

    const sf::FloatRect rect({0.0f, 0.0f}, {20.0f, 20.0f});
    const std::vector<Cercle> obstacles = {{{50.0f, 10.0f}, 10.0f}};
    const sf::Vector2f position = deplacerAvecGlissement(carte, rect, {30.0f, 30.0f}, obstacles);

    REQUIRE(position == sf::Vector2f(0.0f, 30.0f));
}
