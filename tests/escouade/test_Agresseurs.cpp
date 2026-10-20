// ./tests/escouade/test_Agresseurs.cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "aides.hpp"
#include "escouade/Agresseurs.hpp"
#include "combat/EntiteVue.hpp"

TEST_CASE("Agresseurs vide : aucun agresseur") {
    const Agresseurs agresseurs;
    REQUIRE(agresseurs.liste().empty());
    REQUIRE(agresseurs.vusParmi({EntiteVue{7, cibleMonstre({300.0f, 60.0f})}}).empty());
}

TEST_CASE("Agresseurs.enregistrer retient l'identifiant du monstre et 3 s") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7);

    REQUIRE(agresseurs.liste().size() == 1);
    REQUIRE(agresseurs.liste()[0].id == 7);
    REQUIRE(agresseurs.liste()[0].tempsRestant == 3.0f);
}

TEST_CASE("Agresseurs.enregistrer d'un monstre connu ne le duplique pas et remet 3 s") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7);
    agresseurs.enregistrer(8);
    agresseurs.mettreAJour(2.0f);
    agresseurs.enregistrer(7);

    REQUIRE(agresseurs.liste().size() == 2);
    REQUIRE(agresseurs.liste()[0].id == 7);
    REQUIRE(agresseurs.liste()[0].tempsRestant == 3.0f);
    REQUIRE(agresseurs.liste()[1].id == 8);
    REQUIRE(agresseurs.liste()[1].tempsRestant < 1.5f);
}

TEST_CASE("Agresseurs.mettreAJour fait ecouler le temps et oublie l'agresseur apres 3 s") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7);

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

TEST_CASE("Agresseurs.retirerMonstresAbsents oublie les absents et garde les autres") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7);
    agresseurs.enregistrer(8);

    agresseurs.retirerMonstresAbsents({EntiteVue{8, cibleMonstre({320.0f, 70.0f})}});

    REQUIRE(agresseurs.liste().size() == 1);
    REQUIRE(agresseurs.liste()[0].id == 8);
}

TEST_CASE("Agresseurs.retirerMonstresAbsents avec une liste vide oublie tout") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7);

    agresseurs.retirerMonstresAbsents({});

    REQUIRE(agresseurs.liste().empty());
}

TEST_CASE("Agresseurs.vusParmi donne le cercle de la liste, dans l'ordre d'enregistrement") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(8);
    agresseurs.enregistrer(7);

    const std::vector<EntiteVue> vus = agresseurs.vusParmi({EntiteVue{7, cibleMonstre({300.0f, 60.0f})},
                                                            EntiteVue{8, cibleMonstre({310.0f, 60.0f})},
                                                            EntiteVue{9, cibleMonstre({320.0f, 60.0f})}});

    REQUIRE(vus.size() == 2);
    REQUIRE(vus[0].id == 8);
    verifierPointProche(vus[0].cercle.centre, {310.0f, 60.0f});
    REQUIRE(vus[0].cercle.rayon == 15.0f);
    REQUIRE(vus[1].id == 7);
    verifierPointProche(vus[1].cercle.centre, {300.0f, 60.0f});
}

TEST_CASE("Agresseurs.vusParmi ignore l'agresseur absent de la liste sans l'oublier") {
    Agresseurs agresseurs;
    agresseurs.enregistrer(7);
    agresseurs.enregistrer(8);

    const std::vector<EntiteVue> vus = agresseurs.vusParmi({EntiteVue{8, cibleMonstre({310.0f, 60.0f})}});

    REQUIRE(vus.size() == 1);
    REQUIRE(vus[0].id == 8);
    REQUIRE(agresseurs.liste().size() == 2);
}
