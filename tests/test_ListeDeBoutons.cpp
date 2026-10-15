// ./tests/test_ListeDeBoutons.cpp
#include "aides.hpp"

#include "ListeDeBoutons.hpp"

namespace {

const sf::Vector2f s_centre{600.0f, 400.0f};

ListeDeBoutons troisBoutons() {
    return ListeDeBoutons({"A", "B", "C"});
}

sf::Vector2f milieu(const sf::FloatRect& rect) {
    return rect.position + rect.size / 2.0f;
}

}  // namespace

TEST_CASE("boutons : le centre de chaque bouton le designe") {
    const ListeDeBoutons boutons = troisBoutons();

    for (std::size_t i = 0; i < 3; ++i) {
        REQUIRE(boutons.boutonSous(milieu(boutons.rectangleDuBouton(i, s_centre)), s_centre) == i);
    }
}

TEST_CASE("boutons : la colonne est centree sur le point donne") {
    const ListeDeBoutons boutons = troisBoutons();

    verifierPointProche(milieu(boutons.rectangleDuBouton(1, s_centre)), s_centre);
}

TEST_CASE("boutons : les boutons sont ranges de haut en bas, sans chevauchement") {
    const ListeDeBoutons boutons = troisBoutons();
    const sf::FloatRect premier = boutons.rectangleDuBouton(0, s_centre);
    const sf::FloatRect deuxieme = boutons.rectangleDuBouton(1, s_centre);

    REQUIRE(deuxieme.position.y > premier.position.y + premier.size.y);
}

TEST_CASE("boutons : l'espace entre deux boutons ne designe rien") {
    const ListeDeBoutons boutons = troisBoutons();
    const sf::FloatRect premier = boutons.rectangleDuBouton(0, s_centre);
    const sf::Vector2f dansLEspace{s_centre.x, premier.position.y + premier.size.y + 1.0f};

    REQUIRE_FALSE(boutons.boutonSous(dansLEspace, s_centre).has_value());
}

TEST_CASE("boutons : un point hors de la colonne ne designe rien") {
    const ListeDeBoutons boutons = troisBoutons();
    const sf::FloatRect premier = boutons.rectangleDuBouton(0, s_centre);

    REQUIRE_FALSE(boutons.boutonSous({premier.position.x - 1.0f, milieu(premier).y}, s_centre).has_value());
    REQUIRE_FALSE(boutons.boutonSous({s_centre.x, premier.position.y - 1.0f}, s_centre).has_value());
}

TEST_CASE("boutons : la disposition suit le centre recu") {
    const ListeDeBoutons boutons = troisBoutons();
    const sf::Vector2f autreCentre{900.0f, 400.0f};

    REQUIRE(boutons.boutonSous(milieu(boutons.rectangleDuBouton(0, autreCentre)), autreCentre) == 0);
    REQUIRE_FALSE(boutons.boutonSous(milieu(boutons.rectangleDuBouton(0, autreCentre)), s_centre).has_value());
}
