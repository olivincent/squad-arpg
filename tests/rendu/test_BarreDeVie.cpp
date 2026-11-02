// ./tests/rendu/test_BarreDeVie.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "rendu/BarreDeVie.hpp"

TEST_CASE("positionBarreDeVie place la barre au-dessus du carre, alignee sur son bord gauche") {
    // Carré de 20 x 20 centré en (100, 200) : haut en y = 190, gauche en x = 90.
    // Barre : 6 px d'écart et 5 px de haut, donc haut de la barre en y = 190 - 6 - 5.
    const sf::Vector2f position = positionBarreDeVie({100.0f, 200.0f}, {20.0f, 20.0f});

    REQUIRE(position.x == 90.0f);
    REQUIRE(position.y == 179.0f);
}

// Les segments d'absorption (étape 114.5).

TEST_CASE("segmentsDAbsorption : sans bouclier, deux segments vides") {
    const auto [aegis, ward] = segmentsDAbsorption(0.6f, 0.0f, 0.0f);

    REQUIRE(aegis.fin == aegis.debut);
    REQUIRE(ward.fin == ward.debut);
}

TEST_CASE("segmentsDAbsorption : les segments prolongent la partie pleine, l'Aegis puis le Ward") {
    const auto [aegis, ward] = segmentsDAbsorption(0.5f, 0.2f, 0.1f);

    REQUIRE(aegis.debut == 0.5f);
    REQUIRE(aegis.fin == 0.7f);
    REQUIRE(ward.debut == 0.7f);
    REQUIRE(ward.fin == 0.8f);
}

TEST_CASE("segmentsDAbsorption : a pleine vie les segments se calent contre le bord droit") {
    const auto [aegis, ward] = segmentsDAbsorption(1.0f, 0.25f, 0.0f);

    REQUIRE(aegis.debut == 0.75f);
    REQUIRE(aegis.fin == 1.0f);
}

TEST_CASE("segmentsDAbsorption : un bouclier plus grand que la barre la remplit sans la depasser") {
    const auto [aegis, ward] = segmentsDAbsorption(0.4f, 3.0f, 2.0f);

    REQUIRE(aegis.debut == 0.0f);
    REQUIRE(aegis.fin == 1.0f);
    REQUIRE(ward.fin <= 1.0f);
    REQUIRE(ward.fin == ward.debut);  // l'Aegis a tout pris
}
