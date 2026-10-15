// ./tests/test_Agresseurs.cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "aides.hpp"
#include "Agresseurs.hpp"
#include "EntiteVue.hpp"

TEST_CASE("Agresseurs vide : aucun agresseur") {
    const Agresseurs agresseurs;
    REQUIRE(agresseurs.liste().empty());
    REQUIRE(agresseurs.monstresVus().empty());
}

TEST_CASE("Agresseurs.enregistrer retient le monstre, son origine (rayon nul) et 3 s") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7, {300.0f, 60.0f});

    REQUIRE(agresseurs.liste().size() == 1);
    REQUIRE(agresseurs.liste()[0].monstre.id == 7);
    verifierPointProche(agresseurs.liste()[0].monstre.cercle.centre, {300.0f, 60.0f});
    REQUIRE(agresseurs.liste()[0].monstre.cercle.rayon == 0.0f);
    REQUIRE(agresseurs.liste()[0].tempsRestant == 3.0f);
}

TEST_CASE("Agresseurs.enregistrer d'un monstre connu ne le duplique pas, deplace son centre et remet 3 s") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7, {300.0f, 60.0f});
    agresseurs.enregistrer(8, {310.0f, 60.0f});
    agresseurs.mettreAJour(2.0f);
    agresseurs.enregistrer(7, {320.0f, 60.0f});

    REQUIRE(agresseurs.liste().size() == 2);
    REQUIRE(agresseurs.liste()[0].monstre.id == 7);
    verifierPointProche(agresseurs.liste()[0].monstre.cercle.centre, {320.0f, 60.0f});
    REQUIRE(agresseurs.liste()[0].tempsRestant == 3.0f);
    REQUIRE(agresseurs.liste()[1].monstre.id == 8);
    REQUIRE(agresseurs.liste()[1].tempsRestant < 1.5f);
}

TEST_CASE("Agresseurs.mettreAJour fait ecouler le temps et oublie l'agresseur apres 3 s") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7, {300.0f, 60.0f});

    agresseurs.mettreAJour(2.9f);
    REQUIRE(agresseurs.liste().size() == 1);
    agresseurs.mettreAJour(0.2f);
    REQUIRE(agresseurs.liste().empty());
}

TEST_CASE("Agresseurs.mettreAJour sans agresseur ne fait rien") {
    Agresseurs agresseurs;
    agresseurs.mettreAJour(1.0f);
    REQUIRE(agresseurs.liste().empty());
}

TEST_CASE("Agresseurs.actualiser oublie les absents et met a jour le cercle des autres") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7, {300.0f, 60.0f});
    agresseurs.enregistrer(8, {310.0f, 60.0f});

    agresseurs.actualiser({EntiteVue{8, cibleMonstre({320.0f, 70.0f})}});

    REQUIRE(agresseurs.liste().size() == 1);
    REQUIRE(agresseurs.liste()[0].monstre.id == 8);
    verifierPointProche(agresseurs.liste()[0].monstre.cercle.centre, {320.0f, 70.0f});
    REQUIRE(agresseurs.liste()[0].monstre.cercle.rayon == 15.0f);
}

TEST_CASE("Agresseurs.actualiser avec une liste vide oublie tout") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7, {300.0f, 60.0f});

    agresseurs.actualiser({});

    REQUIRE(agresseurs.liste().empty());
}

TEST_CASE("Agresseurs.monstresVus renvoie les agresseurs dans l'ordre d'enregistrement") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7, {300.0f, 60.0f});
    agresseurs.enregistrer(8, {310.0f, 60.0f});

    const std::vector<EntiteVue> vus = agresseurs.monstresVus();

    REQUIRE(vus.size() == 2);
    REQUIRE(vus[0].id == 7);
    verifierPointProche(vus[0].cercle.centre, {300.0f, 60.0f});
    REQUIRE(vus[1].id == 8);
}
