// ./tests/monstres/test_RoleDeMonstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <variant>

#include "monde/CategorieDeMonstre.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/RoleDeMonstre.hpp"

// Le rôle d'un monstre (étape 107.1) : une donnée de la définition, un état de même forme dans le monstre.

TEST_CASE("role : les quatre categories d'avant les roles sont des melees") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        REQUIRE(std::holds_alternative<RoleMelee>(definitionDeMonstre(categorie).role));
    }
}

TEST_CASE("role : l'etat initial est l'alternative de meme rang que le role") {
    REQUIRE(std::holds_alternative<EtatMelee>(etatInitialDuRole(RoleMelee{})));
    REQUIRE(std::holds_alternative<EtatTireur>(etatInitialDuRole(RoleTireur{})));
    REQUIRE(std::holds_alternative<EtatSoigneur>(etatInitialDuRole(RoleSoigneur{})));
}

TEST_CASE("role : renforcer garde le role de la definition") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(categorie);
        REQUIRE(renforcer(definition, 4).role.index() == definition.role.index());
    }
}
