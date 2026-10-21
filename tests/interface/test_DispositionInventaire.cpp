// ./tests/interface/test_DispositionInventaire.cpp
#include <catch2/catch_test_macros.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "interface/DispositionInventaire.hpp"

namespace {

const sf::Vector2f s_vueMinimale{1200.0f, 800.0f};
const sf::Vector2f s_vueLarge{1422.0f, 800.0f};  // 16:9
// Marge gardée libre de chaque côté pour le panneau de l'écran (voir EcranInventaire).
constexpr float s_margeDuPanneau = 40.0f;

// Tous les rectangles de l'écran, pour les vérifications globales.
std::vector<sf::FloatRect> tousLesRectangles(sf::Vector2f vue) {
    std::vector<sf::FloatRect> rectangles;
    for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
        rectangles.push_back(rectangleDeLEnTete(place, vue));
        rectangles.push_back(rectangleDeLArmure(place, vue));
        for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
            rectangles.push_back(rectangleDeLEmplacement(place, static_cast<Emplacement>(i), vue));
        }
    }
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        rectangles.push_back(rectangleDeLaCaseDuSac(indice, vue));
    }
    return rectangles;
}

sf::Vector2f centre(sf::FloatRect rectangle) {
    return rectangle.position + rectangle.size / 2.0f;
}

}  // namespace

TEST_CASE("disposition : tout tient dans la vue, dans le panneau") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        const sf::Vector2f coinHautGauche{s_margeDuPanneau, s_margeDuPanneau};
        const sf::Vector2f coinBasDroit = vue - coinHautGauche;
        for (const sf::FloatRect& rectangle : tousLesRectangles(vue)) {
            REQUIRE(rectangle.position.x >= coinHautGauche.x);
            REQUIRE(rectangle.position.y >= coinHautGauche.y);
            REQUIRE(rectangle.position.x + rectangle.size.x <= coinBasDroit.x);
            REQUIRE(rectangle.position.y + rectangle.size.y <= coinBasDroit.y);
        }
    }
}

TEST_CASE("disposition : aucun chevauchement") {
    const std::vector<sf::FloatRect> rectangles = tousLesRectangles(s_vueMinimale);

    for (std::size_t i = 0; i < rectangles.size(); ++i) {
        for (std::size_t j = i + 1; j < rectangles.size(); ++j) {
            REQUIRE_FALSE(rectangles[i].findIntersection(rectangles[j]).has_value());
        }
    }
}

TEST_CASE("disposition : les colonnes sont dans l'ordre des places, centrees") {
    const sf::FloatRect premiere = rectangleDeLEnTete(1, s_vueMinimale);
    const sf::FloatRect derniere = rectangleDeLEnTete(s_nombreDePlaces, s_vueMinimale);

    REQUIRE(premiere.position.x < rectangleDeLEnTete(2, s_vueMinimale).position.x);
    REQUIRE(rectangleDeLEnTete(2, s_vueMinimale).position.x < derniere.position.x);
    REQUIRE(premiere.position.x == s_vueMinimale.x - (derniere.position.x + derniere.size.x));
}

TEST_CASE("disposition : les emplacements suivent l'ordre de l'enum, en deux colonnes de cinq") {
    const sf::FloatRect tete = rectangleDeLEmplacement(1, Emplacement::Tete, s_vueMinimale);
    const sf::FloatRect bottes = rectangleDeLEmplacement(1, Emplacement::Bottes, s_vueMinimale);
    const sf::FloatRect mainDroite = rectangleDeLEmplacement(1, Emplacement::MainDroite, s_vueMinimale);
    const sf::FloatRect anneau2 = rectangleDeLEmplacement(1, Emplacement::Anneau2, s_vueMinimale);

    REQUIRE(tete.position.x == bottes.position.x);
    REQUIRE(tete.position.y < bottes.position.y);
    REQUIRE(mainDroite.position.x > tete.position.x);
    REQUIRE(mainDroite.position.y == tete.position.y);
    REQUIRE(anneau2.position.y == bottes.position.y);
}

TEST_CASE("disposition : le sac a 4 rangees de 10 cases carrees dans l'ordre des indices") {
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, s_vueMinimale);
    const sf::FloatRect dixieme = rectangleDeLaCaseDuSac(9, s_vueMinimale);
    const sf::FloatRect onzieme = rectangleDeLaCaseDuSac(10, s_vueMinimale);

    REQUIRE(premiere.size.x == premiere.size.y);
    REQUIRE(dixieme.position.y == premiere.position.y);
    REQUIRE(dixieme.position.x > premiere.position.x);
    REQUIRE(onzieme.position.x == premiere.position.x);
    REQUIRE(onzieme.position.y > premiere.position.y);
    REQUIRE(rectangleDeLaCaseDuSac(39, s_vueMinimale).position.y == rectangleDeLaCaseDuSac(30, s_vueMinimale).position.y);
}

TEST_CASE("disposition : le sac est sous les colonnes des personnages") {
    const sf::FloatRect dernierEmplacement = rectangleDeLEmplacement(1, Emplacement::Bottes, s_vueMinimale);

    REQUIRE(rectangleDeLaCaseDuSac(0, s_vueMinimale).position.y > dernierEmplacement.position.y + dernierEmplacement.size.y);
}

TEST_CASE("elementSous : le centre de chaque emplacement le designe") {
    for (const sf::Vector2f vue : {s_vueMinimale, s_vueLarge}) {
        for (std::size_t place = 1; place <= s_nombreDePlaces; ++place) {
            for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
                const Emplacement emplacement = static_cast<Emplacement>(i);

                const std::optional<ElementInventaire> element = elementSous(centre(rectangleDeLEmplacement(place, emplacement, vue)), vue);

                REQUIRE(element == ElementInventaire{ElementEmplacement{place, emplacement}});
            }
        }
    }
}

TEST_CASE("elementSous : le centre de chaque case du sac la designe") {
    for (std::size_t indice = 0; indice < s_casesDuSac; ++indice) {
        const std::optional<ElementInventaire> element = elementSous(centre(rectangleDeLaCaseDuSac(indice, s_vueMinimale)), s_vueMinimale);

        REQUIRE(element == ElementInventaire{ElementCase{indice}});
    }
}

TEST_CASE("elementSous : rien dans un ecart, sur un en-tete ou hors des elements") {
    REQUIRE_FALSE(elementSous({5.0f, 5.0f}, s_vueMinimale).has_value());
    REQUIRE_FALSE(elementSous(centre(rectangleDeLEnTete(1, s_vueMinimale)), s_vueMinimale).has_value());
    REQUIRE_FALSE(elementSous(centre(rectangleDeLArmure(2, s_vueMinimale)), s_vueMinimale).has_value());
    // L'écart entre deux cases du sac.
    const sf::FloatRect premiere = rectangleDeLaCaseDuSac(0, s_vueMinimale);
    REQUIRE_FALSE(elementSous({premiere.position.x + premiere.size.x + 1.0f, premiere.position.y + 1.0f}, s_vueMinimale).has_value());
}
