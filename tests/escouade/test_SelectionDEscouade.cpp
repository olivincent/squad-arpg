// ./tests/escouade/test_SelectionDEscouade.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "escouade/SelectionDEscouade.hpp"
#include "escouade/TailleDeLEscouade.hpp"

namespace {

using Liste = std::vector<IdSpecialite>;

const sf::Vector2f s_centre{600.0f, 400.0f};

sf::Vector2f centreDeLaCase(std::size_t indice) {
    const sf::FloatRect rect = rectangleDeLaCase(indice, s_centre);
    return rect.position + rect.size / 2.0f;
}

}  // namespace

TEST_CASE("selection : elle commence vide et incomplete") {
    const SelectionDEscouade selection;

    REQUIRE(selection.liste().empty());
    REQUIRE_FALSE(selection.estComplete());
}

TEST_CASE("selection : la taille de l'escouade est de quatre personnages") {
    REQUIRE(s_tailleDeLEscouade == 4);
}

TEST_CASE("selection : ajouter remplit les cases dans l'ordre jusqu'a quatre") {
    SelectionDEscouade selection;

    selection.ajouter(IdSpecialite::Mage);
    selection.ajouter(IdSpecialite::Guerrier);
    REQUIRE(selection.liste() == Liste{IdSpecialite::Mage, IdSpecialite::Guerrier});
    REQUIRE_FALSE(selection.estComplete());

    selection.ajouter(IdSpecialite::Rodeur);
    REQUIRE(selection.liste() == Liste{IdSpecialite::Mage, IdSpecialite::Guerrier, IdSpecialite::Rodeur});
    REQUIRE_FALSE(selection.estComplete());  // trois ne suffisent plus

    selection.ajouter(IdSpecialite::Pretre);
    REQUIRE(selection.liste() == Liste{IdSpecialite::Mage, IdSpecialite::Guerrier, IdSpecialite::Rodeur, IdSpecialite::Pretre});
    REQUIRE(selection.estComplete());
}

TEST_CASE("selection : ajouter au-dela de quatre est sans effet") {
    SelectionDEscouade selection;
    for (int i = 0; i < 4; ++i) {
        selection.ajouter(IdSpecialite::Mage);
    }

    selection.ajouter(IdSpecialite::Paladin);

    REQUIRE(selection.liste() == Liste{IdSpecialite::Mage, IdSpecialite::Mage, IdSpecialite::Mage, IdSpecialite::Mage});
    REQUIRE(selection.estComplete());
}

TEST_CASE("selection : quatre fois la meme classe est permis") {
    SelectionDEscouade selection;

    for (int i = 0; i < 4; ++i) {
        selection.ajouter(IdSpecialite::Guerrier);
    }

    REQUIRE(selection.liste() == Liste(4, IdSpecialite::Guerrier));
    REQUIRE(selection.estComplete());
}

TEST_CASE("selection : une meme classe peut etre prise plusieurs fois") {
    SelectionDEscouade selection;

    selection.ajouter(IdSpecialite::Guerrier);
    selection.ajouter(IdSpecialite::Guerrier);

    REQUIRE(selection.liste() == Liste{IdSpecialite::Guerrier, IdSpecialite::Guerrier});
}

TEST_CASE("selection : retirer une case du milieu fait remonter les suivantes") {
    SelectionDEscouade selection;
    selection.ajouter(IdSpecialite::Guerrier);
    selection.ajouter(IdSpecialite::Pretre);
    selection.ajouter(IdSpecialite::Mage);

    selection.retirer(1);

    REQUIRE(selection.liste() == Liste{IdSpecialite::Guerrier, IdSpecialite::Mage});
    REQUIRE_FALSE(selection.estComplete());
}

TEST_CASE("selection : retirer la premiere case, puis ajouter, remet la selection complete") {
    SelectionDEscouade selection;
    selection.ajouter(IdSpecialite::Guerrier);
    selection.ajouter(IdSpecialite::Pretre);
    selection.ajouter(IdSpecialite::Mage);
    selection.ajouter(IdSpecialite::Rodeur);

    selection.retirer(0);
    REQUIRE_FALSE(selection.estComplete());
    selection.ajouter(IdSpecialite::Paladin);

    REQUIRE(selection.liste() == Liste{IdSpecialite::Pretre, IdSpecialite::Mage, IdSpecialite::Rodeur, IdSpecialite::Paladin});
    REQUIRE(selection.estComplete());
}

TEST_CASE("caseSous : un point au centre de chaque case donne cette case") {
    for (std::size_t i = 0; i < s_tailleDeLEscouade; ++i) {
        REQUIRE(caseSous(centreDeLaCase(i), s_centre) == i);
    }
}

TEST_CASE("caseSous : hors des cases ou dans un ecart, rien") {
    const sf::FloatRect premiere = rectangleDeLaCase(0, s_centre);
    const sf::FloatRect deuxieme = rectangleDeLaCase(1, s_centre);
    const float milieuDeLEcart = (premiere.position.y + premiere.size.y + deuxieme.position.y) / 2.0f;

    REQUIRE_FALSE(caseSous({s_centre.x, milieuDeLEcart}, s_centre).has_value());
    REQUIRE_FALSE(caseSous({5.0f, 5.0f}, s_centre).has_value());
    REQUIRE_FALSE(caseSous({premiere.position.x - 1.0f, premiere.position.y + 1.0f}, s_centre).has_value());
}

TEST_CASE("rectangleDeLaCase : les cases sont empilees, centrees sur le centre donne") {
    const sf::FloatRect premiere = rectangleDeLaCase(0, s_centre);
    const sf::FloatRect derniere = rectangleDeLaCase(s_tailleDeLEscouade - 1, s_centre);

    REQUIRE(premiere.position.x == derniere.position.x);
    REQUIRE(premiere.position.y < derniere.position.y);
    REQUIRE(premiere.position.x + premiere.size.x / 2.0f == s_centre.x);
    REQUIRE(((premiere.position.y + derniere.position.y + derniere.size.y) / 2.0f) == s_centre.y);
}
