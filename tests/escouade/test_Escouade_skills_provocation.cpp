// ./tests/escouade/test_Escouade_skills_provocation.cpp
#include <catch2/catch_test_macros.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "aides.hpp"
#include "aides_escouade.hpp"
#include "base/Cercle.hpp"
#include "escouade/Escouade.hpp"
#include "combat/EntiteVue.hpp"
#include "monde/TileMap.hpp"

// Provocation (touche 2) : lanceur (guerrier) en (235, 210), rayon 150 px bord à bord.

TEST_CASE("Provocation : provoque tous les monstres dans le rayon, sans degats") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {
        MonstreVu{IdMonstre{1}, cibleMonstre({335.0f, 210.0f})},
        MonstreVu{IdMonstre{2}, cibleMonstre({235.0f, 310.0f})},
        MonstreVu{IdMonstre{3}, cibleMonstre({800.0f, 210.0f})},
    };

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());

    const auto& provocations = escouade.getProvocations();
    REQUIRE(provocations.size() == 2);
    REQUIRE(provocations[0].id == IdMonstre{1});
    REQUIRE(provocations[1].id == IdMonstre{2});
    REQUIRE(provocations[0].place == Place{1});
    REQUIRE(provocations[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : la zone se mesure depuis le centre du lanceur, bord du monstre compris") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    // Centre du lanceur (235, 210), rayon de zone 150, rayon du monstre 15 :
    // le monstre est touché jusqu'à 165 px de distance entre centres.
    const std::vector<MonstreVu> monstres = {
        MonstreVu{IdMonstre{1}, cibleMonstre({400.0f, 210.0f})},
        MonstreVu{IdMonstre{2}, cibleMonstre({401.0f, 210.0f})},
    };

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().size() == 1);
    REQUIRE(escouade.getProvocations()[0].id == IdMonstre{1});
}

TEST_CASE("Provocation : sans monstre dans le rayon, elle part quand meme et consomme sa recharge") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<MonstreVu> loin = {MonstreVu{IdMonstre{1}, cibleMonstre({800.0f, 210.0f})}};
    REQUIRE(escouade.skillEstPret(s_adresseProvocation));

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, loin, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
    REQUIRE_FALSE(escouade.skillEstPret(s_adresseProvocation));

    // Recharge en cours : un monstre qui arrive n'est pas provoqué.
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, {MonstreVu{IdMonstre{1}, cibleMonstre({335.0f, 210.0f})}}, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations().empty());
}


TEST_CASE("Provocation : un nouvel appui pendant la recharge est ignore, puis accepte apres 10 s") {
    Generateur generateur = generateurDeTest();
    const TileMap carte = carteDegageeEntouree();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    const std::vector<MonstreVu> monstres = {MonstreVu{IdMonstre{1}, cibleMonstre({335.0f, 210.0f})}};

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    escouade.mettreAJour(3.0f, carte, {});
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations()[0].tempsRestant < 1.5f);  // pas remis à 4

    escouade.mettreAJour(7.1f, carte, {});
    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());
    REQUIRE(escouade.getProvocations()[0].tempsRestant == 4.0f);
}

TEST_CASE("Provocation : refusee si le lanceur est mort") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    infliger(escouade, IdMonstre{1}, {235.0f, 210.0f}, s_coupMortel);  // tue la place 1

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, {MonstreVu{IdMonstre{1}, cibleMonstre({335.0f, 210.0f})}}, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
}

TEST_CASE("Provocation : refusee si un autre skill est en attente") {
    Generateur generateur = generateurDeTest();
    Escouade escouade = uneEscouade({200.0f, 200.0f});
    escouade.verrouiller(IdMonstre{1});
    const std::vector<MonstreVu> monstres = {MonstreVu{IdMonstre{1}, cibleMonstre({335.0f, 210.0f})}};
    REQUIRE(declencherEtAttendre(escouade, s_adresseFrappe, monstres, carteOuverte(), generateur).empty());  // dégâts hors de portée de mêlée : mis en attente

    REQUIRE(declencherEtAttendre(escouade, s_adresseProvocation, monstres, carteOuverte(), generateur).empty());

    REQUIRE(escouade.getProvocations().empty());
}
