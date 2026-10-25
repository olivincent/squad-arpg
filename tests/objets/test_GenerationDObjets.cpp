// ./tests/objets/test_GenerationDObjets.cpp
#include <catch2/catch_test_macros.hpp>

#include "objets/CatalogueObjets.hpp"
#include "objets/GenerationDObjets.hpp"

// La rareté des objets tirés (étape 96) : les poids et la règle « ce modèle peut-il porter cette rareté ».

TEST_CASE("poidsDeRarete : commun 70, magique 25, rare 5") {
    REQUIRE(poidsDeRarete(Rarete::Commun) == 70);
    REQUIRE(poidsDeRarete(Rarete::Magique) == 25);
    REQUIRE(poidsDeRarete(Rarete::Rare) == 5);
}

TEST_CASE("peutPorterLaRarete : tous les modeles peuvent etre communs") {
    for (const DefinitionObjet& modele : catalogueObjets()) {
        INFO(modele.nom);
        REQUIRE(peutPorterLaRarete(modele.id, Rarete::Commun));
    }
}

TEST_CASE("peutPorterLaRarete : un bijou peut etre magique (un suffixe) mais jamais rare (aucun prefixe)") {
    for (const IdObjet bijou : {IdObjet::AnneauDeProtection, IdObjet::AnneauDePuissance, IdObjet::CollierDeProtection,
                                IdObjet::CollierDEndurance}) {
        INFO(definitionObjet(bijou).nom);
        REQUIRE(peutPorterLaRarete(bijou, Rarete::Magique));
        REQUIRE_FALSE(peutPorterLaRarete(bijou, Rarete::Rare));
    }
}

TEST_CASE("peutPorterLaRarete : les gants et les bottes peuvent etre rares (Reinforced), comme les armes et les armures") {
    for (const IdObjet modele : {IdObjet::GantsDeCuir, IdObjet::BottesDeCuir, IdObjet::CasqueDeFer, IdObjet::CuirasseDeMailles,
                                 IdObjet::BouclierRond, IdObjet::Arc, IdObjet::Masse}) {
        INFO(definitionObjet(modele).nom);
        REQUIRE(peutPorterLaRarete(modele, Rarete::Magique));
        REQUIRE(peutPorterLaRarete(modele, Rarete::Rare));
    }
}
