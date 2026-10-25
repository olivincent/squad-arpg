// ./tests/objets/test_CatalogueAffixes.cpp
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "objets/CatalogueAffixes.hpp"

// Les données du catalogue d'affixes (étape 95.3). Les règles qui les lient aux objets (position, catégorie, rareté)
// sont dans `test_CatalogueObjets.cpp` (`objetValide`).

TEST_CASE("catalogueAffixes : suit l'ordre de IdAffixe") {
    const std::vector<DefinitionDAffixe>& catalogue = catalogueAffixes();

    REQUIRE_FALSE(catalogue.empty());
    for (std::size_t i = 0; i < catalogue.size(); ++i) {
        REQUIRE(static_cast<std::size_t>(catalogue[i].id) == i);
    }
    REQUIRE(static_cast<std::size_t>(IdAffixe::DeBlocage) == catalogue.size() - 1);
}

TEST_CASE("catalogueAffixes : chaque affixe a un nom distinct et au moins une categorie permise") {
    std::set<std::string> noms;
    for (const DefinitionDAffixe& definition : catalogueAffixes()) {
        REQUIRE_FALSE(definition.nom.empty());
        REQUIRE_FALSE(definition.categories.empty());
        noms.insert(definition.nom);
    }
    REQUIRE(noms.size() == catalogueAffixes().size());
}

TEST_CASE("catalogueAffixes : quatre prefixes puis huit suffixes, comme decide a l'etape 95") {
    std::size_t prefixes = 0;
    std::size_t suffixes = 0;
    for (const DefinitionDAffixe& definition : catalogueAffixes()) {
        (definition.position == PositionDAffixe::Prefixe ? prefixes : suffixes) += 1;
    }
    REQUIRE(prefixes == 4);
    REQUIRE(suffixes == 8);
}

TEST_CASE("definitionAffixe : rend l'entree du catalogue de cet identifiant") {
    for (const DefinitionDAffixe& definition : catalogueAffixes()) {
        REQUIRE(&definitionAffixe(definition.id) == &definition);
    }
    REQUIRE(definitionAffixe(IdAffixe::Flamboyant).nom == "Blazing");
    REQUIRE(definitionAffixe(IdAffixe::DuRenard).nom == "of the Fox");
}

TEST_CASE("affixePermis : les armes pour le feu, le bouclier pour le blocage, toutes les categories pour une caracteristique") {
    REQUIRE(affixePermis(IdAffixe::Flamboyant, CategorieObjet::Arme));
    REQUIRE_FALSE(affixePermis(IdAffixe::Flamboyant, CategorieObjet::Casque));
    REQUIRE(affixePermis(IdAffixe::DeBlocage, CategorieObjet::Bouclier));
    REQUIRE_FALSE(affixePermis(IdAffixe::DeBlocage, CategorieObjet::Arme));
    REQUIRE(affixePermis(IdAffixe::Renforce, CategorieObjet::Jambieres));
    REQUIRE_FALSE(affixePermis(IdAffixe::Renforce, CategorieObjet::Anneau));
    REQUIRE(affixePermis(IdAffixe::DEsquive, CategorieObjet::Bottes));
    REQUIRE(affixePermis(IdAffixe::DeDeviation, CategorieObjet::Casque));
    for (const CategorieObjet categorie :
         {CategorieObjet::Casque, CategorieObjet::Armure, CategorieObjet::Jambieres, CategorieObjet::Gants,
          CategorieObjet::Bottes, CategorieObjet::Bouclier, CategorieObjet::Collier, CategorieObjet::Anneau,
          CategorieObjet::Arme}) {
        REQUIRE(affixePermis(IdAffixe::DuLion, categorie));
    }
}
