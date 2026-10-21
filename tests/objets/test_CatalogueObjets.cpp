// ./tests/objets/test_CatalogueObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

#include "objets/CatalogueObjets.hpp"
#include "objets/Emplacement.hpp"

namespace {
bool vaQuelquePart(CategorieObjet categorie) {
    for (std::size_t i = 0; i < s_nombreEmplacements; ++i) {
        if (peutAller(categorie, static_cast<Emplacement>(i))) {
            return true;
        }
    }
    return false;
}
}  // namespace

TEST_CASE("catalogueObjets : suit l'ordre de IdObjet") {
    const std::vector<DefinitionObjet>& catalogue = catalogueObjets();

    REQUIRE_FALSE(catalogue.empty());
    for (std::size_t i = 0; i < catalogue.size(); ++i) {
        REQUIRE(static_cast<std::size_t>(catalogue[i].id) == i);
    }
    REQUIRE(static_cast<std::size_t>(IdObjet::AnneauDeProtection) == catalogue.size() - 1);
}

TEST_CASE("catalogueObjets : chaque objet a un nom, une armure positive et une categorie qui va quelque part") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE_FALSE(definition.nom.empty());
        REQUIRE(definition.armure > 0);
        REQUIRE(vaQuelquePart(definition.categorie));
    }
}

TEST_CASE("definitionObjet : rend l'entree du catalogue de cet identifiant") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        REQUIRE(&definitionObjet(definition.id) == &definition);
    }
    REQUIRE(definitionObjet(IdObjet::BouclierRond).categorie == CategorieObjet::Bouclier);
    REQUIRE(definitionObjet(IdObjet::AnneauDeProtection).categorie == CategorieObjet::Anneau);
}

TEST_CASE("Objet : deux objets de meme modele sont egaux") {
    REQUIRE(Objet{IdObjet::GantsDeCuir} == Objet{IdObjet::GantsDeCuir});
    REQUIRE(Objet{IdObjet::GantsDeCuir} != Objet{IdObjet::BottesDeCuir});
}

TEST_CASE("sacDeDepart : des objets du catalogue, dont deux anneaux") {
    const std::vector<Objet> sac = sacDeDepart();

    REQUIRE_FALSE(sac.empty());
    std::size_t anneaux = 0;
    for (const Objet& objet : sac) {
        if (definitionObjet(objet.id).categorie == CategorieObjet::Anneau) {
            ++anneaux;
        }
    }
    REQUIRE(anneaux == 2);
}
