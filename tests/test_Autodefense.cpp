// ./tests/test_Autodefense.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <vector>

#include "Autodefense.hpp"
#include "Coup.hpp"
#include "Personnage.hpp"

namespace {

// Personnage en (0, 0), rayon 10, portée 20 px bord à bord : un cercle de
// rayon 15 est à portée si son centre est à 45 px ou moins.
Personnage personnageDePortee20() {
    return Personnage({0.0f, 0.0f}, sf::Color::Red, Skill(TypeSkill::Degats, 20.0f, 5, 0.5f, 0.0f), std::nullopt);
}

MonstreVu monstreEn(int id, float x) {
    return {id, {{x, 0.0f}, 15.0f}};
}

std::optional<int> choisir(const Personnage& personnage,
                           const std::vector<MonstreVu>& monstres,
                           std::optional<int> idVerrouille) {
    return choisirMonstreAFrapper(
        personnage.getCercle(), [&](Cercle c) { return personnage.estAPortee(c); }, monstres, idVerrouille);
}

}  // namespace

TEST_CASE("choisirMonstreAFrapper : aucun monstre a portee, aucun choix") {
    const Personnage personnage = personnageDePortee20();

    REQUIRE_FALSE(choisir(personnage, {}, std::nullopt).has_value());
    REQUIRE_FALSE(choisir(personnage, {monstreEn(1, 100.0f)}, std::nullopt).has_value());
}

TEST_CASE("choisirMonstreAFrapper : un seul monstre a portee est choisi, sans verrou") {
    const Personnage personnage = personnageDePortee20();

    REQUIRE(choisir(personnage, {monstreEn(7, 40.0f)}, std::nullopt) == 7);
}

TEST_CASE("choisirMonstreAFrapper : le verrou a portee passe avant un monstre plus proche") {
    const Personnage personnage = personnageDePortee20();
    const std::vector<MonstreVu> monstres = {monstreEn(1, 30.0f), monstreEn(2, 44.0f)};

    REQUIRE(choisir(personnage, monstres, 2) == 2);
}

TEST_CASE("choisirMonstreAFrapper : verrou hors de portee, le plus proche a portee est choisi") {
    const Personnage personnage = personnageDePortee20();
    const std::vector<MonstreVu> monstres = {monstreEn(1, 100.0f), monstreEn(2, 44.0f), monstreEn(3, 35.0f)};

    REQUIRE(choisir(personnage, monstres, 1) == 3);
}

TEST_CASE("choisirMonstreAFrapper : verrou absent de la liste, le plus proche a portee est choisi") {
    const Personnage personnage = personnageDePortee20();
    const std::vector<MonstreVu> monstres = {monstreEn(2, 44.0f), monstreEn(3, 35.0f)};

    REQUIRE(choisir(personnage, monstres, 99) == 3);
}

TEST_CASE("choisirMonstreAFrapper : plusieurs monstres a portee, le plus proche est choisi") {
    const Personnage personnage = personnageDePortee20();
    const std::vector<MonstreVu> monstres = {monstreEn(1, 44.0f), monstreEn(2, 30.0f), monstreEn(3, 40.0f)};

    REQUIRE(choisir(personnage, monstres, std::nullopt) == 2);
}

TEST_CASE("choisirMonstreAFrapper : a egalite, le premier de la liste est choisi") {
    const Personnage personnage = personnageDePortee20();
    const std::vector<MonstreVu> monstres = {
        {5, {{30.0f, 0.0f}, 15.0f}},
        {6, {{-30.0f, 0.0f}, 15.0f}},
    };

    REQUIRE(choisir(personnage, monstres, std::nullopt) == 5);
}

TEST_CASE("choisirAgresseurAViser : aucun agresseur, aucun choix") {
    REQUIRE_FALSE(choisirAgresseurAViser({{0.0f, 0.0f}, 10.0f}, {0.0f, 0.0f}, 100.0f, {}).has_value());
}

TEST_CASE("choisirAgresseurAViser : un agresseur hors de la laisse est ignore") {
    const std::vector<MonstreVu> agresseurs = {monstreEn(1, 101.0f)};

    REQUIRE_FALSE(choisirAgresseurAViser({{0.0f, 0.0f}, 10.0f}, {0.0f, 0.0f}, 100.0f, agresseurs).has_value());
}

TEST_CASE("choisirAgresseurAViser : la laisse se mesure depuis la place, pas depuis le personnage") {
    const std::vector<MonstreVu> agresseurs = {monstreEn(1, 150.0f)};
    const Cercle personnage{{120.0f, 0.0f}, 10.0f};  // loin de sa place, mais proche de l'agresseur

    REQUIRE_FALSE(choisirAgresseurAViser(personnage, {0.0f, 0.0f}, 100.0f, agresseurs).has_value());
    REQUIRE(choisirAgresseurAViser(personnage, {50.0f, 0.0f}, 100.0f, agresseurs).has_value());
}

TEST_CASE("choisirAgresseurAViser : un agresseur pile a la laisse compte") {
    const std::vector<MonstreVu> agresseurs = {monstreEn(1, 100.0f)};

    REQUIRE(choisirAgresseurAViser({{0.0f, 0.0f}, 10.0f}, {0.0f, 0.0f}, 100.0f, agresseurs).has_value());
}

TEST_CASE("choisirAgresseurAViser : le plus proche du personnage est choisi parmi ceux de la laisse") {
    const std::vector<MonstreVu> agresseurs = {monstreEn(1, 90.0f), monstreEn(2, 40.0f), monstreEn(3, 150.0f)};
    const auto choix = choisirAgresseurAViser({{0.0f, 0.0f}, 10.0f}, {0.0f, 0.0f}, 100.0f, agresseurs);

    REQUIRE(choix.has_value());
    REQUIRE(choix->centre.x == 40.0f);
}

TEST_CASE("choisirAgresseurAViser : a egalite, le premier de la liste est choisi") {
    const std::vector<MonstreVu> agresseurs = {
        {5, {{30.0f, 0.0f}, 15.0f}},
        {6, {{-30.0f, 0.0f}, 15.0f}},
    };
    const auto choix = choisirAgresseurAViser({{0.0f, 0.0f}, 10.0f}, {0.0f, 0.0f}, 100.0f, agresseurs);

    REQUIRE(choix.has_value());
    REQUIRE(choix->centre.x == 30.0f);
}
