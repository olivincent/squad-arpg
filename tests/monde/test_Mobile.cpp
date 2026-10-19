// ./tests/monde/test_Mobile.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "monde/Mobile.hpp"
#include "monde/TileMap.hpp"

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

TEST_CASE("Mobile.getCercle est centre sur le mobile, de rayon la moitie du cote") {
    const Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    const Cercle cercle = mobile.getCercle();

    REQUIRE(cercle.centre == sf::Vector2f(60.0f, 60.0f));
    REQUIRE(cercle.rayon == 10.0f);
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

    // À quelques centièmes de pixel près : poussé droit contre le cercle, le mobile ne fait plus
    // que des corrections minuscules.
    verifierPointProche(mobile.getCentre(), centreBloque, 0.5f);
    REQUIRE(mobile.estEnMouvement());
}

TEST_CASE("Mobile.allerVers contourne un cercle a contourner et atteint la destination") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    const std::vector<Cercle> obstacle = {{{180.0f, 60.0f}, 10.0f}};

    mobile.allerVers({300.0f, 60.0f}, carte, obstacle);

    // Aucun segment du chemin ne passe à moins de 10 + 10 px du centre de l'obstacle.
    sf::Vector2f precedent = mobile.getCentre();
    for (const sf::Vector2f& point : mobile.cheminRestant()) {
        REQUIRE(distancePointSegment(obstacle[0].centre, precedent, point) >= 20.0f);
        precedent = point;
    }

    // Le même cercle bloque aussi : sans contournement, le mobile resterait collé contre lui.
    simuler(mobile, carte, 10.0f, obstacle);
    verifierPointProche(mobile.getCentre(), {300.0f, 60.0f});
}

TEST_CASE("Mobile.allerVers sans cercle a contourner garde la ligne droite") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({300.0f, 60.0f}, carte);

    REQUIRE(mobile.cheminRestant() == std::vector<sf::Vector2f>{{300.0f, 60.0f}});
}

TEST_CASE("Mobile.cheminRestant se vide au fil du trajet") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);
    REQUIRE(mobile.cheminRestant().size() == 1); // le centre de départ, point initial

    mobile.allerVers({137.0f, 251.0f}, carte);
    REQUIRE_FALSE(mobile.cheminRestant().empty());
    REQUIRE(mobile.cheminRestant().back() == sf::Vector2f(137.0f, 251.0f));

    simuler(mobile, carte, 10.0f);
    REQUIRE(mobile.cheminRestant().empty());
}

namespace {
// Mobile lancé en ligne droite vers un cercle qui le bloque (contact en x = 160 vers t = 0,55 s).
struct MobileContreObstacle {
    TileMap carte = carteDegagee10x10();
    Mobile mobile{{60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f};
    std::vector<Cercle> obstacles = {{{180.0f, 60.0f}, 10.0f}};

    MobileContreObstacle() { mobile.allerToutDroit({300.0f, 60.0f}); }
};
} // namespace

TEST_CASE("Mobile.estBloque devient vrai apres 0,3 s contre un obstacle, pas avant") {
    MobileContreObstacle m;

    simuler(m.mobile, m.carte, 0.4f, m.obstacles);
    REQUIRE_FALSE(m.mobile.estBloque()); // encore en marche

    simuler(m.mobile, m.carte, 0.3f, m.obstacles); // contact vers 0,55 s : bloqué depuis ~0,15 s
    REQUIRE_FALSE(m.mobile.estBloque());

    simuler(m.mobile, m.carte, 0.3f, m.obstacles); // bloqué depuis ~0,45 s
    REQUIRE(m.mobile.estBloque());
}

TEST_CASE("Mobile.estBloque est faux pour un mobile arrete") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    simuler(mobile, carte, 1.0f);
    REQUIRE_FALSE(mobile.estBloque());

    MobileContreObstacle m;
    simuler(m.mobile, m.carte, 1.5f, m.obstacles);
    REQUIRE(m.mobile.estBloque());
    m.mobile.arreter();
    REQUIRE_FALSE(m.mobile.estBloque());
    simuler(m.mobile, m.carte, 1.0f, m.obstacles);
    REQUIRE_FALSE(m.mobile.estBloque());
}

TEST_CASE("Mobile.estBloque reste faux pour un mobile qui avance normalement jusqu'a l'arrivee") {
    const TileMap carte = carteDegagee10x10();
    Mobile mobile({60.0f, 60.0f}, {20.0f, 20.0f}, 180.0f);

    mobile.allerVers({137.0f, 251.0f}, carte);
    for (int i = 0; i < nombreDeTics(5.0f); ++i) {
        mobile.mettreAJour(s_dureeTic, carte, {});
        REQUIRE_FALSE(mobile.estBloque());
    }
}

TEST_CASE("Mobile.allerVers et allerToutDroit remettent le blocage a zero") {
    MobileContreObstacle m;
    simuler(m.mobile, m.carte, 1.5f, m.obstacles);
    REQUIRE(m.mobile.estBloque());

    m.mobile.allerVers({300.0f, 60.0f}, m.carte);
    REQUIRE_FALSE(m.mobile.estBloque());

    simuler(m.mobile, m.carte, 1.0f, m.obstacles);
    REQUIRE(m.mobile.estBloque());

    m.mobile.allerToutDroit({300.0f, 60.0f});
    REQUIRE_FALSE(m.mobile.estBloque());
}
