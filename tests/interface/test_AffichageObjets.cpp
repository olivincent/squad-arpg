// ./tests/interface/test_AffichageObjets.cpp
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "interface/AffichageObjets.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"

TEST_CASE("affichage : chaque emplacement a un nom, tous differents") {
    std::set<std::string> noms;
    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        const std::string nom = nomDeLEmplacement(static_cast<Emplacement>(i));
        REQUIRE_FALSE(nom.empty());
        noms.insert(nom);
    }
    REQUIRE(noms.size() == s_nombreEmplacements);
}

TEST_CASE("affichage : chaque categorie a un nom") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE_FALSE(nomDeLaCategorie(definition.categorie).empty());
    }
}

TEST_CASE("affichage : chaque objet du catalogue a un sigle de deux lettres, tous differents") {
    std::set<std::string> sigles;
    for (const DefinitionObjet& definition : catalogueObjets()) {
        const std::string sigle = sigleDeLObjet(definition.id);
        REQUIRE(sigle.size() == 2);
        sigles.insert(sigle);
    }
    REQUIRE(sigles.size() == catalogueObjets().size());
}

TEST_CASE("affichage : l'infobulle donne le nom, la categorie et l'armure") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::CasqueDeFer);

    REQUIRE(lignes.size() == 3);
    REQUIRE(lignes[0] == definitionObjet(IdObjet::CasqueDeFer).nom);
    REQUIRE(lignes[1] == "Casque");
    REQUIRE(lignes[2] == "Armure : +" + std::to_string(definitionObjet(IdObjet::CasqueDeFer).armure));
}
