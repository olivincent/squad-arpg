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
