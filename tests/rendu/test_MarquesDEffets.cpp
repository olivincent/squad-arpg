// ./tests/rendu/test_MarquesDEffets.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <vector>

#include "rendu/MarquesDEffets.hpp"

TEST_CASE("positionMarquesDEffets place la rangee juste sous le carre, alignee sur son bord gauche") {
    // Carré de 20 x 20 centré en (100, 200) : bord gauche en x = 90, bas en y = 210.
    const sf::Vector2f position = positionMarquesDEffets({100.0f, 200.0f}, {20.0f, 20.0f});

    REQUIRE(position.x == 90.0f);
    REQUIRE(position.y > 210.0f);  // ne recouvre pas le carré
    REQUIRE(position.y < 210.0f + 2.0f * s_coteDUneMarque);
}

TEST_CASE("pointsDuChiffre : les points allumes d'un chiffre dans sa grille de 3 sur 5") {
    // Le 8 allume 13 des 15 points (tout sauf le milieu des lignes 1 et 3), le 1 en allume 8, le 7 en allume 7.
    REQUIRE(pointsDuChiffre(8).size() == 13);
    REQUIRE(pointsDuChiffre(1).size() == 8);
    REQUIRE(pointsDuChiffre(7).size() == 7);
    for (int chiffre = 0; chiffre <= 9; ++chiffre) {
        for (const sf::Vector2i point : pointsDuChiffre(chiffre)) {
            REQUIRE(point.x >= 0);
            REQUIRE(point.x <= 2);
            REQUIRE(point.y >= 0);
            REQUIRE(point.y <= 4);
        }
    }
}

TEST_CASE("pointsDuChiffre : le 1 est une barre (colonne du milieu) sur un pied, le 3 est ferme a droite") {
    const std::vector<sf::Vector2i> un = pointsDuChiffre(1);
    REQUIRE(un.front() == sf::Vector2i{1, 0});  // lecture de haut en bas, de gauche à droite : le haut de la barre
    REQUIRE(un.back() == sf::Vector2i{2, 4});   // le pied

    const std::vector<sf::Vector2i> trois = pointsDuChiffre(3);
    for (int ligne = 0; ligne < 5; ++ligne) {
        REQUIRE(std::find(trois.begin(), trois.end(), sf::Vector2i{2, ligne}) != trois.end());  // la colonne de droite est pleine
        REQUIRE(std::find(trois.begin(), trois.end(), sf::Vector2i{0, 1}) == trois.end());
    }
}
