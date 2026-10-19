// ./tests/test_Reglages.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <sstream>
#include <vector>

#include "Reglages.hpp"

namespace {

Reglages lire(const std::string& texte) {
    std::istringstream flux(texte);
    return lireReglages(flux);
}

}  // namespace

TEST_CASE("reglages : ecriture puis lecture redonne les memes valeurs") {
    const Reglages ecrits{1600, 900, ModeAffichage::PleinEcran};
    std::ostringstream sortie;
    ecrireReglages(sortie, ecrits);

    const Reglages lus = lire(sortie.str());

    REQUIRE(lus.largeur == 1600);
    REQUIRE(lus.hauteur == 900);
    REQUIRE(lus.mode == ModeAffichage::PleinEcran);
}

TEST_CASE("reglages : le format est cle=valeur, une cle par ligne") {
    std::ostringstream sortie;
    ecrireReglages(sortie, Reglages{});

    REQUIRE(sortie.str() == "largeur=1200\nhauteur=800\nmode=fenetre\n");
}

TEST_CASE("reglages : un flux vide donne les valeurs par defaut") {
    const Reglages lus = lire("");

    REQUIRE(lus.largeur == 1200);
    REQUIRE(lus.hauteur == 800);
    REQUIRE(lus.mode == ModeAffichage::Fenetre);
}

TEST_CASE("reglages : cle inconnue et ligne mal formee sont ignorees") {
    const Reglages lus = lire("volume=11\nn'importe quoi\n\n=3\nlargeur=1280\nhauteur=720\n");

    REQUIRE(lus.largeur == 1280);
    REQUIRE(lus.hauteur == 720);
}

TEST_CASE("reglages : espaces et fins de ligne Windows toleres") {
    const Reglages lus = lire("largeur = 1600 \r\nhauteur=900\r\nmode=plein_ecran\r\n");

    REQUIRE(lus.largeur == 1600);
    REQUIRE(lus.hauteur == 900);
    REQUIRE(lus.mode == ModeAffichage::PleinEcran);
}

TEST_CASE("reglages : valeur invalide donne la valeur par defaut") {
    const Reglages lus = lire("largeur=abc\nhauteur=-5\nmode=cinema\n");

    REQUIRE(lus.largeur == 1200);
    REQUIRE(lus.hauteur == 800);
    REQUIRE(lus.mode == ModeAffichage::Fenetre);
}

TEST_CASE("reglages : un nombre suivi de texte est invalide") {
    const Reglages lus = lire("largeur=1600px\nhauteur=900\n");

    // largeur invalide (1200) avec hauteur 900 : pas dans la liste.
    REQUIRE(lus.largeur == 1200);
    REQUIRE(lus.hauteur == 800);
}

TEST_CASE("reglages : resolution hors liste donne 1200 x 800") {
    const Reglages lus = lire("largeur=1000\nhauteur=1000\nmode=plein_ecran\n");

    REQUIRE(lus.largeur == 1200);
    REQUIRE(lus.hauteur == 800);
    REQUIRE(lus.mode == ModeAffichage::PleinEcran);
}

TEST_CASE("reglages : largeur et hauteur de deux resolutions differentes donnent le defaut") {
    const Reglages lus = lire("largeur=1920\nhauteur=720\n");

    REQUIRE(lus.largeur == 1200);
    REQUIRE(lus.hauteur == 800);
}

TEST_CASE("reglages : les resolutions proposees commencent par 1200 x 800") {
    REQUIRE(resolutionsProposees().front() == Resolution{1200, 800});
}

TEST_CASE("reglages : le filtrage garde les resolutions qui tiennent dans le bureau") {
    const std::vector<Resolution> disponibles = resolutionsDisponibles({1600, 900});

    REQUIRE(disponibles == std::vector<Resolution>{{1200, 800}, {1280, 720}, {1600, 900}});
}

TEST_CASE("reglages : le filtrage compare la largeur ET la hauteur") {
    const std::vector<Resolution> disponibles = resolutionsDisponibles({1920, 760});

    REQUIRE(disponibles == std::vector<Resolution>{{1280, 720}});
}

TEST_CASE("reglages : un grand bureau garde toutes les resolutions") {
    REQUIRE(resolutionsDisponibles({3840, 2160}) == resolutionsProposees());
}

TEST_CASE("reglages : les touches par defaut sont la grille Q W E R / A S D F / Z X C V") {
    const Reglages reglages;

    REQUIRE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::Q) == 0);
    REQUIRE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::R) == 3);
    REQUIRE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::A) == 4);
    REQUIRE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::Z) == 8);
    REQUIRE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::V) == 11);
}

TEST_CASE("reglages : une touche hors de la table n'a pas d'emplacement") {
    const Reglages reglages;

    REQUIRE_FALSE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::Num1).has_value());
    REQUIRE_FALSE(emplacementPourTouche(reglages.touchesDesRaccourcis, sf::Keyboard::Scan::Escape).has_value());
}
