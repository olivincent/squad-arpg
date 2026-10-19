// ./tests/interface/test_ClicInterface.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include "interface/BarreDeRaccourcis.hpp"
#include "interface/ClicInterface.hpp"

namespace {
constexpr sf::Vector2f s_vue{1422.0f, 800.0f};
}

TEST_CASE("clicSurInterface : un clic sur une icone designe la barre et l'emplacement") {
    const sf::FloatRect rect = rectangleDeLEmplacement(5, s_vue);

    const std::optional<ClicInterface> clic = clicSurInterface(rect.position + rect.size / 2.0f, s_vue);

    REQUIRE(clic == ClicInterface{Barre::Raccourcis, 5});
}

TEST_CASE("clicSurInterface : un clic sur le monde n'est pas un clic d'interface") {
    REQUIRE_FALSE(clicSurInterface({100.0f, 100.0f}, s_vue).has_value());
    REQUIRE_FALSE(clicSurInterface({s_vue.x / 2.0f, 300.0f}, s_vue).has_value());
}

TEST_CASE("clicSurInterface : un clic dans l'ecart entre deux icones tombe sur le monde") {
    const sf::FloatRect premier = rectangleDeLEmplacement(0, s_vue);

    REQUIRE_FALSE(clicSurInterface({premier.position.x + premier.size.x + 2.0f, premier.position.y + 20.0f}, s_vue).has_value());
}
