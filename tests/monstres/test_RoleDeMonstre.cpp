// ./tests/monstres/test_RoleDeMonstre.cpp
#include <catch2/catch_test_macros.hpp>
#include <variant>

#include "combat/EffetDeSkill.hpp"
#include "combat/NatureDAttaque.hpp"
#include "monde/CategorieDeMonstre.hpp"
#include "monstres/DefinitionDeMonstre.hpp"
#include "monstres/RoleDeMonstre.hpp"

// Le rôle d'un monstre (étape 107.1) : une donnée de la définition, un état de même forme dans le monstre.

TEST_CASE("role : les quatre categories d'avant les roles sont des melees") {
    for (const CategorieDeMonstre categorie : {CategorieDeMonstre::Nuee, CategorieDeMonstre::Infanterie,
                                               CategorieDeMonstre::InfanterieLourde, CategorieDeMonstre::CreatureMonstrueuse}) {
        REQUIRE(std::holds_alternative<RoleMelee>(definitionDeMonstre(categorie).role));
    }
}

TEST_CASE("role : le tireur a un recul qui a un sens") {
    const DefinitionDeMonstre& definition = definitionDeMonstre(CategorieDeMonstre::Tireur);
    const RoleTireur* tireur = std::get_if<RoleTireur>(&definition.role);

    REQUIRE(tireur != nullptr);
    REQUIRE(tireur->distanceDeContact > 0.0f);
    REQUIRE(tireur->distanceDeRecul > 0.0f);
    REQUIRE(tireur->delaiEntreReculs > 0.0f);
    // Il frappe de loin : sa portée dépasse son contact, et il n'engage personne.
    REQUIRE(definition.attaque.portee > tireur->distanceDeContact);
    REQUIRE(definition.zoneDeControle == 0.0f);
    REQUIRE(std::get<EffetDegats>(definition.attaque.effet).nature == NatureDAttaque::Projectile);
}

TEST_CASE("role : l'etat initial est l'alternative de meme rang que le role") {
    const RoleTireur tireur{.distanceDeContact = 20.0f, .distanceDeRecul = 80.0f, .delaiEntreReculs = 8.0f};
    const EtatDuRole etat = etatInitialDuRole(tireur);

    REQUIRE(std::holds_alternative<EtatMelee>(etatInitialDuRole(RoleMelee{})));
    REQUIRE(std::holds_alternative<EtatTireur>(etat));
    REQUIRE(std::holds_alternative<EtatSoigneur>(etatInitialDuRole(RoleSoigneur{})));
    // Le tireur part avec ses données, un recul disponible tout de suite et aucun recul en cours.
    const EtatTireur& tireurInitial = std::get<EtatTireur>(etat);
    REQUIRE(tireurInitial.role.distanceDeRecul == 80.0f);
    REQUIRE(tireurInitial.delaiRestant == 0.0f);
    REQUIRE_FALSE(tireurInitial.enRecul);
}

TEST_CASE("role : renforcer garde le role de la definition") {
    for (const CategorieDeMonstre categorie : s_categoriesDeMonstre) {
        const DefinitionDeMonstre& definition = definitionDeMonstre(categorie);
        REQUIRE(renforcer(definition, 4).role.index() == definition.role.index());
    }
}
