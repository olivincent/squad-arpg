// ./tests/test_Mobile.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "Cercle.hpp"
#include "Mobile.hpp"
#include "TileMap.hpp"

namespace {

void simuler(Mobile& mobile, const TileMap& carte, float dureeSecondes,
             const std::vector<Cercle>& obstacles = {}) {
    constexpr float dt = s_dureeTic;
    const int nombreTics = nombreDeTics(dureeSecondes);
    for (int i = 0; i < nombreTics; ++i) {
        mobile.mettreAJour(dt, carte, obstacles);
    }
}

}  // namespace

TEST_CASE("Mobile.getCentre renvoie le centre donne au constructeur") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    REQUIRE(mobile.getCentre() == sf::Vector2f(60.0f, 60.0f));
}

TEST_CASE("Mobile.getTaille renvoie la taille donnee au constructeur") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    REQUIRE(mobile.getTaille() == sf::Vector2f(20.0f, 20.0f));
}

TEST_CASE("Mobile.mettreAJour sans ordre ne bouge pas") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    simuler(mobile, carte, 1.0f);

    verifierPointProche(mobile.getCentre(), {60.0f, 60.0f});
}

TEST_CASE("Mobile.mettreAJour avance de vitesse * dt sur un seul tic") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    mobile.mettreAJour(0.1f, carte, {});

    const float distanceParcourue = (mobile.getCentre() - sf::Vector2f(60.0f, 60.0f)).length();
    REQUIRE_THAT(distanceParcourue, Catch::Matchers::WithinAbs(18.0f, 0.5f));
}

TEST_CASE("Mobile.allerVers atteint le point exact en terrain degage") {
    const TileMap carte = carteDegagee10x10();
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

TEST_CASE("Mobile.allerVers vers un mur est ignore") {
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
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    simuler(mobile, carte, 0.3f); // en route, loin d'avoir fini

    mobile.arreter();
    const sf::Vector2f centreApresArret = mobile.getCentre();
    simuler(mobile, carte, 1.0f);

    verifierPointProche(mobile.getCentre(), centreApresArret);
}

TEST_CASE("Mobile.directionVoulue est vide a la creation") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    REQUIRE_FALSE(mobile.directionVoulue().has_value());
}

TEST_CASE("Mobile.directionVoulue pointe vers la destination apres allerVers") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    const std::optional<sf::Vector2f> direction = mobile.directionVoulue();

    REQUIRE(direction.has_value());
    REQUIRE(direction->x > 0.0f);
    REQUIRE(direction->y > 0.0f);
}

TEST_CASE("Mobile.directionVoulue est vide une fois arrive") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({137.0f, 251.0f}, carte);
    simuler(mobile, carte, 10.0f);

    REQUIRE_FALSE(mobile.directionVoulue().has_value());
}

TEST_CASE("Mobile.mettreAJour s'arrete avant un obstacle place sur son trajet") {
    // Mobile de rayon 10 en (60, 60), destination (300, 60) en ligne droite.
    // Obstacle de rayon 10 centré en (180, 60) : le contact exact est à
    // x = 160, le mobile ne doit jamais dépasser ce centre.
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    const std::vector<Cercle> obstacles = {{{180.0f, 60.0f}, 10.0f}};

    mobile.allerVers({300.0f, 60.0f}, carte);
    simuler(mobile, carte, 5.0f, obstacles);

    REQUIRE(mobile.getCentre().x > 100.0f);
    REQUIRE(mobile.getCentre().x <= 160.0f);
    REQUIRE_THAT(mobile.getCentre().y, Catch::Matchers::WithinAbs(60.0f, 0.5f));
}

TEST_CASE("allerToutDroit mene le mobile exactement au point, sans A*") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 100.0f);

    mobile.allerToutDroit({160.0f, 100.0f});
    simuler(mobile, carte, 2.0f);

    verifierPointProche(mobile.getCentre(), {160.0f, 100.0f}, 0.01f);
}

TEST_CASE("allerToutDroit vers un point dans un mur ne fait pas contourner le mur") {
    const TileMap carte({"..#..",
                         "..#..",
                         "..#.."},
                        40.0f);
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 100.0f);

    mobile.allerToutDroit({140.0f, 60.0f});
    simuler(mobile, carte, 5.0f);

    // Bloqué contre le mur (x = 80) : il glisse, mais ne passe pas de l'autre côté.
    REQUIRE(mobile.getCentre().x <= 70.0f);
}

TEST_CASE("Mobile.estEnMouvement est faux a la creation et une fois arrive") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    REQUIRE_FALSE(mobile.estEnMouvement());

    mobile.allerVers({137.0f, 251.0f}, carte);
    simuler(mobile, carte, 10.0f);

    REQUIRE_FALSE(mobile.estEnMouvement());
}

TEST_CASE("Mobile.estEnMouvement est vrai en marche et faux apres arreter") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 300.0f}, carte);
    simuler(mobile, carte, 0.1f);
    REQUIRE(mobile.estEnMouvement());

    mobile.arreter();
    REQUIRE_FALSE(mobile.estEnMouvement());
}

TEST_CASE("Mobile.estEnMouvement reste vrai quand le mobile est bloque en essayant d'avancer") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    const std::vector<Cercle> obstacles = {{{180.0f, 60.0f}, 10.0f}};

    mobile.allerToutDroit({300.0f, 60.0f});
    simuler(mobile, carte, 5.0f, obstacles);
    const sf::Vector2f centreBloque = mobile.getCentre();
    simuler(mobile, carte, 0.5f, obstacles);

    REQUIRE(mobile.getCentre() == centreBloque);
    REQUIRE(mobile.estEnMouvement());
}
