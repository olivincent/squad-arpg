// ./tests/rendu/test_BarreDEnergie.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "rendu/BarreDEnergie.hpp"
#include "rendu/BarreDeVie.hpp"

TEST_CASE("positionBarreDEnergie place la barre juste sous la barre de vie, alignee sur son bord gauche") {
    // Carré de 20 x 20 centré en (100, 200) : haut en y = 190, barre de vie de y = 179 à 184.
    const sf::Vector2f vie = positionBarreDeVie({100.0f, 200.0f}, {20.0f, 20.0f});
    const sf::Vector2f energie = positionBarreDEnergie({100.0f, 200.0f}, {20.0f, 20.0f});

    REQUIRE(energie.x == vie.x);
    REQUIRE(energie.y > vie.y + s_hauteurBarreDeVie);  // ne recouvre pas la barre de vie
}

TEST_CASE("la barre d'energie tient entre la barre de vie et le carre") {
    const sf::Vector2f energie = positionBarreDEnergie({100.0f, 200.0f}, {20.0f, 20.0f});

    REQUIRE(energie.y + s_hauteurBarreDEnergie <= 190.0f);  // le haut du carré
    REQUIRE(s_hauteurBarreDEnergie < s_hauteurBarreDeVie);
}
