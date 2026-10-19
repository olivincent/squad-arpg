// ./tests/monde/test_Encerclement.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

#include "monde/Encerclement.hpp"

using Catch::Approx;

namespace {

float distanceEntre(sf::Vector2f a, sf::Vector2f b) {
    return (b - a).length();
}

}  // namespace

TEST_CASE("anglesCandidats : un anneau trop petit n'a qu'une place") {
    REQUIRE(anglesCandidats(sf::degrees(0.0f), 10.0f, 26.0f).size() == 1);
    REQUIRE(anglesCandidats(sf::degrees(0.0f), 13.0f - 0.1f, 26.0f).size() == 1);
}

TEST_CASE("anglesCandidats : le nombre de places croit avec le rayon") {
    // Corde 26 sur un rayon de 26 : un sixieme de tour (60 degres) => 6 places.
    REQUIRE(anglesCandidats(sf::degrees(0.0f), 26.0f, 26.0f).size() == 6);
    // Sur un rayon de 13, deux places opposees (corde = diametre = 26).
    REQUIRE(anglesCandidats(sf::degrees(0.0f), 13.0f, 26.0f).size() == 2);
    REQUIRE(anglesCandidats(sf::degrees(0.0f), 52.0f, 26.0f).size() > 6);
}

TEST_CASE("anglesCandidats : la premiere candidate est la direction de depart") {
    const auto angles = anglesCandidats(sf::degrees(37.0f), 60.0f, 26.0f);

    REQUIRE(angles.front().asDegrees() == Approx(37.0f));
}

TEST_CASE("anglesCandidats : alternance gauche droite") {
    // Rayon 26 : 6 places, pas de 60 degres.
    const auto angles = anglesCandidats(sf::degrees(0.0f), 26.0f, 26.0f);

    REQUIRE(angles.size() == 6);
    REQUIRE(angles[0].wrapSigned().asDegrees() == Approx(0.0f));
    REQUIRE(angles[1].wrapSigned().asDegrees() == Approx(60.0f));
    REQUIRE(angles[2].wrapSigned().asDegrees() == Approx(-60.0f));
    REQUIRE(angles[3].wrapSigned().asDegrees() == Approx(120.0f));
    REQUIRE(angles[4].wrapSigned().asDegrees() == Approx(-120.0f));
    REQUIRE(std::abs(angles[5].wrapSigned().asDegrees()) == Approx(180.0f));
}

TEST_CASE("anglesCandidats : deux places voisines sont a l'ecart minimal ou plus") {
    for (const float rayon : {13.0f, 20.0f, 33.0f, 47.5f, 80.0f, 140.0f}) {
        const auto angles = anglesCandidats(sf::degrees(10.0f), rayon, 26.0f);

        // Les places voisines sur l'anneau : tri par angle.
        std::vector<float> degres;
        for (const sf::Angle a : angles) {
            degres.push_back(a.wrapUnsigned().asDegrees());
        }
        std::sort(degres.begin(), degres.end());
        for (std::size_t i = 0; i < degres.size() && degres.size() > 1; ++i) {
            const float suivant = degres[(i + 1) % degres.size()];
            const float ecart = std::fmod(suivant - degres[i] + 360.0f, 360.0f);
            const float corde = 2.0f * rayon * std::sin(sf::degrees(ecart).asRadians() / 2.0f);
            REQUIRE(corde >= 26.0f - 0.001f);
        }
    }
}

TEST_CASE("positionDePlace : un point de l'anneau") {
    const sf::Vector2f centre{100.0f, 50.0f};

    const sf::Vector2f droite = positionDePlace(centre, 30.0f, sf::degrees(0.0f));
    REQUIRE(droite.x == Approx(130.0f));
    REQUIRE(droite.y == Approx(50.0f));

    const sf::Vector2f bas = positionDePlace(centre, 30.0f, sf::degrees(90.0f));
    REQUIRE(bas.x == Approx(100.0f).margin(0.001));
    REQUIRE(bas.y == Approx(80.0f));

    REQUIRE(distanceEntre(centre, positionDePlace(centre, 30.0f, sf::degrees(123.0f))) == Approx(30.0f));
}

TEST_CASE("choisirAngleLibre : l'angle libre le plus proche de l'angle actuel") {
    const std::vector<sf::Angle> candidats{sf::degrees(0.0f), sf::degrees(60.0f), sf::degrees(-60.0f)};

    const auto choix = choisirAngleLibre(candidats, sf::degrees(50.0f), [](sf::Angle) { return true; });

    REQUIRE(choix.has_value());
    REQUIRE(choix->asDegrees() == Approx(60.0f));
}

TEST_CASE("choisirAngleLibre : saute les angles refuses") {
    const std::vector<sf::Angle> candidats{sf::degrees(0.0f), sf::degrees(60.0f), sf::degrees(-60.0f)};

    const auto choix = choisirAngleLibre(candidats, sf::degrees(50.0f), [](sf::Angle a) {
        return a.asDegrees() != 60.0f;
    });

    REQUIRE(choix.has_value());
    REQUIRE(choix->asDegrees() == Approx(0.0f));
}

TEST_CASE("choisirAngleLibre : l'ecart se mesure par le plus court cote") {
    const std::vector<sf::Angle> candidats{sf::degrees(90.0f), sf::degrees(-170.0f)};

    // 175 degres est a 15 degres de -170 (par le plus court cote), 85 de 90.
    const auto choix = choisirAngleLibre(candidats, sf::degrees(175.0f), [](sf::Angle) { return true; });

    REQUIRE(choix.has_value());
    REQUIRE(choix->asDegrees() == Approx(-170.0f));
}

TEST_CASE("choisirAngleLibre : egalite departagee par l'ordre des candidats") {
    const std::vector<sf::Angle> candidats{sf::degrees(-30.0f), sf::degrees(30.0f)};

    const auto choix = choisirAngleLibre(candidats, sf::degrees(0.0f), [](sf::Angle) { return true; });

    REQUIRE(choix.has_value());
    REQUIRE(choix->asDegrees() == Approx(-30.0f));
}

TEST_CASE("choisirAngleLibre : aucun angle libre, aucun choix") {
    const std::vector<sf::Angle> candidats{sf::degrees(0.0f), sf::degrees(60.0f)};

    REQUIRE_FALSE(choisirAngleLibre(candidats, sf::degrees(0.0f), [](sf::Angle) { return false; }).has_value());
    REQUIRE_FALSE(choisirAngleLibre({}, sf::degrees(0.0f), [](sf::Angle) { return true; }).has_value());
}
