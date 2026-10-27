// ./tests/escouade/test_Provocations.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>
#include <vector>

#include "combat/EntiteVue.hpp"
#include "escouade/Provocations.hpp"

TEST_CASE("Provocations vide : aucune provocation, aucun provocateur") {
    const Provocations provocations;
    REQUIRE(provocations.liste().empty());
    REQUIRE_FALSE(provocations.provocateurDe(IdMonstre{1}).has_value());
}

TEST_CASE("Provocations.provoquer enregistre le monstre, le provocateur et la duree") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);

    REQUIRE(provocations.liste().size() == 1);
    REQUIRE(provocations.liste()[0].id == IdMonstre{7});
    REQUIRE(provocations.liste()[0].place == Place{2});
    REQUIRE(provocations.liste()[0].tempsRestant == 4.0f);
    REQUIRE(provocations.provocateurDe(IdMonstre{7}) == std::optional<Place>(Place{2}));
    REQUIRE_FALSE(provocations.provocateurDe(IdMonstre{8}).has_value());
}

TEST_CASE("Provocations.provoquer sur un monstre deja provoque ne duplique pas, remet la duree et change de provocateur") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);
    provocations.provoquer(IdMonstre{8}, Place{2}, 4.0f);
    provocations.mettreAJour(3.0f);
    provocations.provoquer(IdMonstre{7}, Place{1}, 4.0f);

    REQUIRE(provocations.liste().size() == 2);
    REQUIRE(provocations.liste()[0].id == IdMonstre{7});
    REQUIRE(provocations.liste()[0].place == Place{1});
    REQUIRE(provocations.liste()[0].tempsRestant == 4.0f);
    REQUIRE(provocations.liste()[1].tempsRestant < 1.5f);
}

TEST_CASE("Provocations.mettreAJour fait ecouler le temps et retire les provocations echues") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);

    provocations.mettreAJour(3.9f);
    REQUIRE(provocations.liste().size() == 1);
    REQUIRE(provocations.liste()[0].tempsRestant < 0.2f);
    provocations.mettreAJour(0.2f);
    REQUIRE(provocations.liste().empty());
}

TEST_CASE("Provocations.mettreAJour sans provocation ne fait rien") {
    Provocations provocations;
    provocations.mettreAJour(1.0f);
    REQUIRE(provocations.liste().empty());
}

TEST_CASE("Provocations.retirerMonstresAbsents ne garde que les monstres presents") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);
    provocations.provoquer(IdMonstre{8}, Place{2}, 4.0f);

    provocations.retirerMonstresAbsents({MonstreVu{IdMonstre{8}, {{0.0f, 0.0f}, 15.0f}}});

    REQUIRE(provocations.liste().size() == 1);
    REQUIRE(provocations.liste()[0].id == IdMonstre{8});
}

TEST_CASE("Provocations.retirerMonstresAbsents avec une liste vide retire tout") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);

    provocations.retirerMonstresAbsents({});

    REQUIRE(provocations.liste().empty());
}

TEST_CASE("Provocations.retirerProvocateur retire seulement les provocations de cette place") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);
    provocations.provoquer(IdMonstre{8}, Place{1}, 4.0f);
    provocations.provoquer(IdMonstre{9}, Place{2}, 4.0f);

    provocations.retirerProvocateur(Place{2});

    REQUIRE(provocations.liste().size() == 1);
    REQUIRE(provocations.liste()[0].id == IdMonstre{8});
}

TEST_CASE("Provocations.retirerProvocateur d'une place sans provocation ne fait rien") {
    Provocations provocations;
    provocations.provoquer(IdMonstre{7}, Place{2}, 4.0f);

    provocations.retirerProvocateur(Place{3});

    REQUIRE(provocations.liste().size() == 1);
}
