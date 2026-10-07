// ./tests/test_Mobile.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "Cercle.hpp"
#include "Mobile.hpp"
#include "TileMap.hpp"

namespace {

// Même tolérance que test_Escouade.cpp, pour la même raison (flottants
// accumulés sur plusieurs tics).
void verifierPointProche(sf::Vector2f point, sf::Vector2f attendu, float tolerance = 0.5f) {
    REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(attendu.x, tolerance));
    REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(attendu.y, tolerance));
}

void simuler(Mobile& mobile, const TileMap& carte, float dureeSecondes,
             const std::vector<Cercle>& obstacles = {}) {
    constexpr float dt = 1.0f / 60.0f;
    const int nombreTics = static_cast<int>(dureeSecondes / dt);
    for (int i = 0; i < nombreTics; ++i) {
        mobile.mettreAJour(dt, carte, obstacles);
    }
}

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

}  // namespace

TEST_CASE("Mobile.getCentre renvoie le centre donné au constructeur") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    REQUIRE(mobile.getCentre() == sf::Vector2f(60.0f, 60.0f));
}

TEST_CASE("Mobile.getTaille renvoie la taille donnée au constructeur") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    REQUIRE(mobile.getTaille() == sf::Vector2f(20.0f, 20.0f));
}

TEST_CASE("Mobile.mettreAJour sans ordre ne bouge pas") {
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    simuler(mobile, carte, 1.0f);

    verifierPointProche(mobile.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("Mobile.mettreAJour avance de vitesse * dt sur un seul tic") {
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    mobile.mettreAJour(0.1f, carte, {});

    const float distanceParcourue = (mobile.getCentre() - sf::Vector2f(60.0f, 60.0f)).length();
    REQUIRE_THAT(distanceParcourue, Catch::Matchers::WithinAbs(18.0f, 0.5f));
}

TEST_CASE("Mobile.allerVers atteint le point exact en terrain dégagé") {
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    const sf::Vector2f destination{137.0f, 251.0f};
    mobile.allerVers(destination, carte);
    simuler(mobile, carte, 10.0f);

    verifierPointProche(mobile.getCentre(), destination);
}

TEST_CASE("Mobile.allerVers contourne un mur et atteint la destination") {
    const std::vector<std::string> plan = {
        ".......",
        ".......",
        ".......",
        ".###...",
        ".......",
        ".......",
    };
    TileMap carte(plan, 40.0f);
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    const sf::Vector2f destination = carte.centreDeCase({1, 5});
    mobile.allerVers(destination, carte);
    simuler(mobile, carte, 10.0f);

    verifierPointProche(mobile.getCentre(), destination);
}

TEST_CASE("Mobile.allerVers vers un mur est ignoré") {
    const std::vector<std::string> plan = {
        "....",
        "....",
        "..#.",
        "....",
    };
    TileMap carte(plan, 40.0f);
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    const sf::Vector2f centreInitial = mobile.getCentre();

    mobile.allerVers(carte.centreDeCase({2, 2}), carte);
    simuler(mobile, carte, 1.0f);

    verifierPointProche(mobile.getCentre(), centreInitial);
}

TEST_CASE("Mobile.arreter stoppe le mobile en cours de trajet") {
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    simuler(mobile, carte, 0.3f); // en route, loin d'avoir fini

    mobile.arreter();
    const sf::Vector2f centreApresArret = mobile.getCentre();
    simuler(mobile, carte, 1.0f);

    verifierPointProche(mobile.getCentre(), centreApresArret);
}

TEST_CASE("Mobile.directionVoulue est vide à la création") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    REQUIRE_FALSE(mobile.directionVoulue().has_value());
}

TEST_CASE("Mobile.directionVoulue pointe vers la destination après allerVers") {
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    const std::optional<sf::Vector2f> direction = mobile.directionVoulue();

    REQUIRE(direction.has_value());
    REQUIRE(direction->x > 0.0f);
    REQUIRE(direction->y > 0.0f);
}

TEST_CASE("Mobile.directionVoulue est vide une fois arrivé") {
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({137.0f, 251.0f}, carte);
    simuler(mobile, carte, 10.0f);

    REQUIRE_FALSE(mobile.directionVoulue().has_value());
}

TEST_CASE("Mobile.mettreAJour s'arrête avant un obstacle placé sur son trajet") {
    // Mobile de rayon 10 en (60, 60), destination (300, 60) en ligne droite.
    // Obstacle de rayon 10 centré en (180, 60) : le contact exact est à
    // x = 160, le mobile ne doit jamais dépasser ce centre.
    const TileMap carte = carteDegagee();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    const std::vector<Cercle> obstacles = {{{180.0f, 60.0f}, 10.0f}};

    mobile.allerVers({300.0f, 60.0f}, carte);
    simuler(mobile, carte, 5.0f, obstacles);

    REQUIRE(mobile.getCentre().x > 100.0f);
    REQUIRE(mobile.getCentre().x <= 160.0f);
    REQUIRE_THAT(mobile.getCentre().y, Catch::Matchers::WithinAbs(60.0f, 0.5f));
}
