// ./tests/test_BarreDeSkills.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <vector>

#include "BarreDeSkills.hpp"
#include "TypeSkill.hpp"

namespace {

// Aire du polygone formé par les sommets dans l'ordre (formule du lacet).
float aire(const std::vector<sf::Vector2f>& points) {
    float somme = 0.0f;
    for (std::size_t i = 0; i < points.size(); ++i) {
        const sf::Vector2f a = points[i];
        const sf::Vector2f b = points[(i + 1) % points.size()];
        somme += a.x * b.y - b.x * a.y;
    }
    return std::abs(somme) / 2.0f;
}

}  // namespace

TEST_CASE("lettreDuSkill donne une lettre par type") {
    REQUIRE(lettreDuSkill(TypeSkill::Degats) == 'D');
    REQUIRE(lettreDuSkill(TypeSkill::Provocation) == 'P');
    REQUIRE(lettreDuSkill(TypeSkill::Soin) == 'S');
    REQUIRE(lettreDuSkill(TypeSkill::Zone) == 'Z');
}

TEST_CASE("secondesAffichees arrondit vers le haut") {
    REQUIRE(secondesAffichees(4.2f) == 5);
    REQUIRE(secondesAffichees(1.0f) == 1);
    REQUIRE(secondesAffichees(0.01f) == 1);
    REQUIRE(secondesAffichees(0.0f) == 0);
}

TEST_CASE("pointsDuQuartier n'a aucun sommet pour une fraction nulle") {
    REQUIRE(pointsDuQuartier({100.0f, 100.0f}, 56.0f, 0.0f).empty());
}

TEST_CASE("pointsDuQuartier commence au centre et finit à midi") {
    const sf::Vector2f centre{100.0f, 200.0f};

    for (const float fraction : {0.1f, 0.3f, 0.5f, 0.75f, 1.0f}) {
        const std::vector<sf::Vector2f> points = pointsDuQuartier(centre, 56.0f, fraction);
        REQUIRE(points.size() >= 3);
        REQUIRE(points.front() == centre);
        REQUIRE_THAT(points.back().x, Catch::Matchers::WithinAbs(centre.x, 0.001));
        REQUIRE_THAT(points.back().y, Catch::Matchers::WithinAbs(centre.y - 28.0f, 0.001));
    }
}

TEST_CASE("pointsDuQuartier couvre la fraction demandée de l'aire du carré") {
    const sf::Vector2f centre{100.0f, 200.0f};
    const float cote = 56.0f;

    for (int huitiemes = 1; huitiemes <= 8; ++huitiemes) {
        const float fraction = static_cast<float>(huitiemes) / 8.0f;
        const std::vector<sf::Vector2f> points = pointsDuQuartier(centre, cote, fraction);
        REQUIRE_THAT(aire(points), Catch::Matchers::WithinAbs(fraction * cote * cote, 0.5));
    }
}

TEST_CASE("pointsDuQuartier reste dans le carré") {
    const sf::Vector2f centre{100.0f, 200.0f};

    for (const sf::Vector2f point : pointsDuQuartier(centre, 56.0f, 0.6f)) {
        REQUIRE(std::abs(point.x - centre.x) <= 28.001f);
        REQUIRE(std::abs(point.y - centre.y) <= 28.001f);
    }
}
