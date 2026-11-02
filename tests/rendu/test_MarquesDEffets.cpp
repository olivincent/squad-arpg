// ./tests/rendu/test_MarquesDEffets.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>

#include "rendu/MarquesDEffets.hpp"

TEST_CASE("positionMarquesDEffets place la rangee juste sous le carre, alignee sur son bord gauche") {
    // Carré de 20 x 20 centré en (100, 200) : bord gauche en x = 90, bas en y = 210.
    const sf::Vector2f position = positionMarquesDEffets({100.0f, 200.0f}, {20.0f, 20.0f});

    REQUIRE(position.x == 90.0f);
    REQUIRE(position.y > 210.0f);  // ne recouvre pas le carré
    REQUIRE(position.y < 210.0f + 2.0f * s_coteDUneMarque);
}
