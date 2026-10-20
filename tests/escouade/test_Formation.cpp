// ./tests/escouade/test_Formation.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>
#include <vector>

#include "aides.hpp"
#include "base/Cercle.hpp"
#include "escouade/EcartEntrePersonnages.hpp"
#include "escouade/Formation.hpp"
#include "monde/TileMap.hpp"

namespace {

// Valeurs de test, indépendantes de celles du jeu.
constexpr float s_vitesseRotationDeTest = 90.0f;  // degrés par seconde : 45° en une demi-seconde
constexpr float s_rayonDeTest = 10.0f;
constexpr float s_demiSeconde = 0.5f;
constexpr float s_longtemps = 10.0f;               // assez pour finir n'importe quelle rotation
constexpr float s_tolerance = 0.01f;
constexpr float s_diagonale = 21.2132f;            // 30 × cos(45°)

const sf::Vector2f s_centre(200.0f, 200.0f);
const sf::Vector2f s_devant(30.0f, 0.0f);

// Une formation d'un seul personnage, placé à 30 px devant le centre (vers +x).
Formation formationAUnPersonnage() {
    return Formation({s_devant}, s_centre, s_vitesseRotationDeTest);
}

void avancer(Formation& formation, float dt, sf::Vector2f centre = s_centre) {
    formation.mettreAJour(dt, centre, std::vector<float>(formation.getPlaces().size(), s_rayonDeTest),
                          carteDegagee10x10(), {});
}

// Carte de 10 × 10 cases de 40 px, avec un mur sur toute la colonne 5 (x de 200 à 240 px).
TileMap carteAvecColonneDeMur() {
    return TileMap(std::vector<std::string>(10, ".....#...."), 40.0f);
}

bool chevaucheUnMur(const TileMap& carte, sf::Vector2f place) {
    const sf::Vector2f demiCote(s_rayonDeTest, s_rayonDeTest);
    return carte.chevaucheUnMur(sf::FloatRect(place - demiCote, 2.0f * demiCote));
}

}  // namespace

TEST_CASE("Formation : places de depart theoriques, orientees vers +x") {
    const Formation formation({{30.0f, 0.0f}, {-30.0f, -30.0f}, {-30.0f, 30.0f}}, s_centre, s_vitesseRotationDeTest);

    const auto& places = formation.getPlaces();
    REQUIRE(places.size() == 3);
    verifierPointProche(places[0], {230.0f, 200.0f}, s_tolerance);
    verifierPointProche(places[1], {170.0f, 170.0f}, s_tolerance);
    verifierPointProche(places[2], {170.0f, 230.0f}, s_tolerance);
}

TEST_CASE("Formation : en terrain degage, les places suivent le centre") {
    Formation formation = formationAUnPersonnage();

    avancer(formation, s_demiSeconde, {100.0f, 150.0f});

    verifierPointProche(formation.getPlaces()[0], {130.0f, 150.0f}, s_tolerance);
}

TEST_CASE("Formation : pivote progressivement vers la direction de marche") {
    Formation formation = formationAUnPersonnage();
    formation.choisirOrientation(sf::Vector2f(0.0f, 1.0f), s_centre);  // vers +y : 90°

    avancer(formation, s_demiSeconde);
    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(s_diagonale, s_diagonale), s_tolerance);

    avancer(formation, s_demiSeconde);
    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(0.0f, 30.0f), s_tolerance);

    // Arrivée à l'orientation voulue : elle n'en bouge plus.
    avancer(formation, s_demiSeconde);
    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(0.0f, 30.0f), s_tolerance);
}

TEST_CASE("Formation : tourne par le plus court cote") {
    Formation formation = formationAUnPersonnage();
    formation.choisirOrientation(sf::Vector2f(-1.0f, 1.0f), s_centre);  // 135°
    avancer(formation, s_longtemps);

    // De 135° à -135° : +90° en passant par 180°, pas -270°. En une demi-seconde, +45° : 180°.
    formation.choisirOrientation(sf::Vector2f(-1.0f, -1.0f), s_centre);
    avancer(formation, s_demiSeconde);

    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(-30.0f, 0.0f), s_tolerance);
}

TEST_CASE("Formation : a l'arret sans point a regarder, l'orientation voulue est gardee") {
    Formation formation = formationAUnPersonnage();
    formation.choisirOrientation(sf::Vector2f(0.0f, 1.0f), s_centre);
    avancer(formation, s_demiSeconde);

    // L'ancre s'arrête en pleine rotation : la formation finit de pivoter sur place.
    formation.choisirOrientation(std::nullopt, s_centre);
    avancer(formation, s_demiSeconde);

    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(0.0f, 30.0f), s_tolerance);
}

TEST_CASE("Formation : a l'arret, se tourne vers le point a regarder") {
    Formation formation = formationAUnPersonnage();
    formation.regarderVers(s_centre + sf::Vector2f(0.0f, 100.0f));

    formation.choisirOrientation(std::nullopt, s_centre);
    avancer(formation, s_longtemps);

    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(0.0f, 30.0f), s_tolerance);
}

TEST_CASE("Formation : en marche, la direction de marche gagne sur le point a regarder") {
    Formation formation = formationAUnPersonnage();
    formation.regarderVers(s_centre + sf::Vector2f(0.0f, 100.0f));

    formation.choisirOrientation(sf::Vector2f(-1.0f, 0.0f), s_centre);
    avancer(formation, s_longtemps);

    verifierPointProche(formation.getPlaces()[0], s_centre + sf::Vector2f(-30.0f, 0.0f), s_tolerance);
}

TEST_CASE("Formation : un point a regarder oublie ne tourne plus la formation") {
    Formation formation = formationAUnPersonnage();
    formation.regarderVers(s_centre + sf::Vector2f(0.0f, 100.0f));
    formation.oublierPointARegarder();

    formation.choisirOrientation(std::nullopt, s_centre);
    avancer(formation, s_longtemps);

    verifierPointProche(formation.getPlaces()[0], s_centre + s_devant, s_tolerance);
}

TEST_CASE("Formation : un point a regarder sur le centre ne change pas l'orientation") {
    Formation formation = formationAUnPersonnage();
    formation.regarderVers(s_centre);

    formation.choisirOrientation(std::nullopt, s_centre);
    avancer(formation, s_longtemps);

    verifierPointProche(formation.getPlaces()[0], s_centre + s_devant, s_tolerance);
}

TEST_CASE("Formation : une place dans un mur est ramenee vers le centre") {
    // Centre à x = 150, place théorique à x = 210, dans le mur (200 à 240) : le repli la ramène
    // sur le segment, juste avant que le carré du personnage (20 px) ne touche le mur.
    const TileMap carte = carteAvecColonneDeMur();
    const sf::Vector2f centre(150.0f, 200.0f);
    Formation formation({{60.0f, 0.0f}}, centre, s_vitesseRotationDeTest);

    formation.mettreAJour(s_demiSeconde, centre, {s_rayonDeTest}, carte, {});

    const sf::Vector2f place = formation.getPlaces()[0];
    REQUIRE(place.x > 180.0f);
    REQUIRE(place.x < 191.0f);
    REQUIRE(place.y == 200.0f);
    REQUIRE_FALSE(chevaucheUnMur(carte, place));
}

TEST_CASE("Formation : deux places repliees au meme endroit sont ecartees") {
    // Les deux places théoriques sont dans le mur, à 10 px l'une de l'autre : le repli les ramène
    // presque au même point, et la seconde est remplacée par une place libre autour du centre.
    const TileMap carte = carteAvecColonneDeMur();
    const sf::Vector2f centre(150.0f, 200.0f);
    Formation formation({{60.0f, -5.0f}, {60.0f, 5.0f}}, centre, s_vitesseRotationDeTest);

    formation.mettreAJour(s_demiSeconde, centre, {s_rayonDeTest, s_rayonDeTest}, carte, {});

    const auto& places = formation.getPlaces();
    REQUIRE((places[1] - places[0]).length() >= s_ecartEntrePersonnages);
    REQUIRE_FALSE(chevaucheUnMur(carte, places[0]));
    REQUIRE_FALSE(chevaucheUnMur(carte, places[1]));
}

TEST_CASE("Formation : une place dans un monstre en est sortie") {
    Formation formation = formationAUnPersonnage();
    const Cercle monstre{s_centre + s_devant, 15.0f};

    formation.mettreAJour(s_demiSeconde, s_centre, {s_rayonDeTest}, carteDegagee10x10(), {monstre});

    const sf::Vector2f place = formation.getPlaces()[0];
    REQUIRE((place - monstre.centre).length() >= monstre.rayon + s_rayonDeTest);
}

TEST_CASE("Formation : un monstre eloigne ne change pas les places") {
    Formation formation = formationAUnPersonnage();
    const Cercle monstre{{50.0f, 50.0f}, 15.0f};

    formation.mettreAJour(s_demiSeconde, s_centre, {s_rayonDeTest}, carteDegagee10x10(), {monstre});

    verifierPointProche(formation.getPlaces()[0], s_centre + s_devant, s_tolerance);
}
