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
    REQUIRE(affixePermis(IdAffixe::Renforce, CategorieObjet::Gants));  // des pieces d'armure (etape 96)
    REQUIRE(affixePermis(IdAffixe::Renforce, CategorieObjet::Bottes));
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

TEST_CASE("affixesPermisPour : les affixes de la position demandee, dans l'ordre du catalogue") {
    const std::vector<IdAffixe> prefixesDArme = affixesPermisPour(CategorieObjet::Arme, PositionDAffixe::Prefixe);
    REQUIRE(prefixesDArme == std::vector<IdAffixe>{IdAffixe::Flamboyant, IdAffixe::Sanctifie, IdAffixe::Acere});

    const std::vector<IdAffixe> suffixesDeBottes = affixesPermisPour(CategorieObjet::Bottes, PositionDAffixe::Suffixe);
    REQUIRE(suffixesDeBottes == std::vector<IdAffixe>{IdAffixe::DuBoeuf, IdAffixe::DeLOurs, IdAffixe::DuRenard,
                                                      IdAffixe::DeLaChouette, IdAffixe::DuLion, IdAffixe::DEsquive});

    REQUIRE(affixesPermisPour(CategorieObjet::Anneau, PositionDAffixe::Prefixe).empty());
    REQUIRE(affixesPermisPour(CategorieObjet::Gants, PositionDAffixe::Prefixe) == std::vector<IdAffixe>{IdAffixe::Renforce});
}

TEST_CASE("plageAuNiveau : au niveau 1, la plage du catalogue, pour chaque affixe") {
    for (const DefinitionDAffixe& affixe : catalogueAffixes()) {
        INFO(affixe.nom);
        const PlageDeValeurs plage = plageAuNiveau(affixe.id, 1);
        REQUIRE(plage.min == affixe.valeurs.min);
        REQUIRE(plage.max == affixe.valeurs.max);
    }
}

TEST_CASE("plageAuNiveau : au niveau 5, les bornes doublent (Blazing 3 a 8 devient 6 a 16, un suffixe de caracteristique 1 a 3 devient 2 a 6)") {
    const PlageDeValeurs feu = plageAuNiveau(IdAffixe::Flamboyant, 5);
    REQUIRE(feu.min == 6);
    REQUIRE(feu.max == 16);

    const PlageDeValeurs caracteristique = plageAuNiveau(IdAffixe::DuRenard, 5);
    REQUIRE(caracteristique.min == 2);
    REQUIRE(caracteristique.max == 6);
}

TEST_CASE("plageAuNiveau : les bornes sont arrondies au plus proche") {
    // Niveau 2 : facteur 1,25. Blazing 3 a 8 : 3,75 et 10 ; Keen 10 a 20 : 12,5 (arrondi vers le haut) et 25.
    const PlageDeValeurs feu = plageAuNiveau(IdAffixe::Flamboyant, 2);
    REQUIRE(feu.min == 4);
    REQUIRE(feu.max == 10);
    const PlageDeValeurs penetration = plageAuNiveau(IdAffixe::Acere, 2);
    REQUIRE(penetration.min == 13);
    REQUIRE(penetration.max == 25);
    // Niveau 3 : facteur 1,5. Blazing : 4,5 (vers le haut) et 12.
    const PlageDeValeurs feuAuTrois = plageAuNiveau(IdAffixe::Flamboyant, 3);
    REQUIRE(feuAuTrois.min == 5);
    REQUIRE(feuAuTrois.max == 12);
}

TEST_CASE("plageAuNiveau : une plage ne rapetisse jamais d'un niveau a l'autre, reste positive et ordonnee, jusqu'au niveau 20") {
    for (const DefinitionDAffixe& affixe : catalogueAffixes()) {
        INFO(affixe.nom);
        PlageDeValeurs precedente = plageAuNiveau(affixe.id, 1);
        for (int niveau = 2; niveau <= 20; ++niveau) {
            INFO("niveau " << niveau);
            const PlageDeValeurs plage = plageAuNiveau(affixe.id, niveau);
            REQUIRE(plage.min >= precedente.min);
            REQUIRE(plage.max >= precedente.max);
            REQUIRE(plage.min >= 1);
            REQUIRE(plage.min <= plage.max);
            precedente = plage;
        }
    }
}
