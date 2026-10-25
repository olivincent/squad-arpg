// ./tests/objets/test_GenerationDObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <set>
#include <vector>

#include "base/Aleatoire.hpp"
#include "objets/CatalogueAffixes.hpp"
#include "objets/CatalogueObjets.hpp"
#include "objets/GenerationDObjets.hpp"

// Les objets tirés (étape 96) : les poids, la règle « ce modèle peut-il porter cette rareté » et `genererObjet`. Les tirages
// utilisent une graine fixe : un test qui échoue échoue toujours de la même façon.

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

namespace {
constexpr unsigned int s_graine = 96;
constexpr int s_tirages = 10000;

// Les objets tirés d'un générateur de graine fixe.
std::vector<Objet> tirer(int nombre, unsigned int graine = s_graine) {
    Generateur generateur(graine);
    std::vector<Objet> objets;
    for (int i = 0; i < nombre; ++i) {
        objets.push_back(genererObjet(generateur));
    }
    return objets;
}

bool estUnBijou(IdObjet modele) {
    const CategorieObjet categorie = definitionObjet(modele).categorie;
    return categorie == CategorieObjet::Anneau || categorie == CategorieObjet::Collier;
}

bool estDesGantsOuDesBottes(IdObjet modele) {
    const CategorieObjet categorie = definitionObjet(modele).categorie;
    return categorie == CategorieObjet::Gants || categorie == CategorieObjet::Bottes;
}

// La valeur de cet affixe est-elle dans la plage de son catalogue ?
bool dansLaPlage(const Affixe& affixe) {
    const PlageDeValeurs& plage = definitionAffixe(affixe.id).valeurs;
    return affixe.valeur >= plage.min && affixe.valeur <= plage.max;
}
}  // namespace

TEST_CASE("genererObjet : 10000 tirages, tous valides") {
    for (const Objet& objet : tirer(s_tirages)) {
        INFO(nomDeLObjet(objet));
        REQUIRE(objetValide(objet));
    }
}

TEST_CASE("genererObjet : chaque rarete, chaque modele et chaque affixe apparaissent, chaque valeur reste dans sa plage") {
    std::set<Rarete> raretes;
    std::set<IdObjet> modeles;
    std::set<IdAffixe> affixes;
    for (const Objet& objet : tirer(s_tirages)) {
        raretes.insert(objet.rarete);
        modeles.insert(objet.modele);
        for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
            if (affixe.has_value()) {
                affixes.insert(affixe->id);
                INFO(nomDeLObjet(objet));
                REQUIRE(dansLaPlage(*affixe));
            }
        }
    }
    REQUIRE(raretes.size() == s_raretes.size());
    REQUIRE(modeles.size() == catalogueObjets().size());
    REQUIRE(affixes.size() == catalogueAffixes().size());
}

TEST_CASE("genererObjet : les bornes de chaque plage sortent") {
    std::set<int> valeursDeFeu;
    for (const Objet& objet : tirer(s_tirages)) {
        for (const std::optional<Affixe>& affixe : {objet.prefixe, objet.suffixe}) {
            if (affixe.has_value() && affixe->id == IdAffixe::Flamboyant) {
                valeursDeFeu.insert(affixe->valeur);
            }
        }
    }
    const PlageDeValeurs& plage = definitionAffixe(IdAffixe::Flamboyant).valeurs;
    REQUIRE(valeursDeFeu.size() == static_cast<std::size_t>(plage.max - plage.min + 1));
    REQUIRE(*valeursDeFeu.begin() == plage.min);
    REQUIRE(*valeursDeFeu.rbegin() == plage.max);
}

TEST_CASE("genererObjet : un anneau ou un collier n'est jamais rare, des gants et des bottes peuvent l'etre") {
    bool gantsOuBottesRares = false;
    for (const Objet& objet : tirer(s_tirages)) {
        if (estUnBijou(objet.modele)) {
            REQUIRE(objet.rarete != Rarete::Rare);
        }
        if (estDesGantsOuDesBottes(objet.modele) && objet.rarete == Rarete::Rare) {
            gantsOuBottesRares = true;
            REQUIRE(objet.prefixe->id == IdAffixe::Renforce);  // leur seul prefixe
        }
    }
    REQUIRE(gantsOuBottesRares);
}

TEST_CASE("genererObjet : les proportions de raretes sont celles des poids, a deux points pres") {
    // Sur 10 000 tirages, l'écart type d'une proportion est d'au plus 0,5 point (à 70 %) : deux points, c'est plus de quatre
    // écarts types. La graine est fixe, donc la tolérance n'est pas là pour absorber le hasard : elle détecte un algorithme
    // modifié (poids changés, rareté retirée après coup parce que le modèle ne convient pas…), qui déplacerait une
    // proportion de plus de deux points.
    constexpr double tolerance = 0.02;
    int poidsTotal = 0;
    for (const Rarete rarete : s_raretes) {
        poidsTotal += poidsDeRarete(rarete);
    }
    std::array<int, s_raretes.size()> compte = {};  // indexé par la valeur de Rarete
    for (const Objet& objet : tirer(s_tirages)) {
        ++compte[static_cast<std::size_t>(objet.rarete)];
    }
    for (const Rarete rarete : s_raretes) {
        const double attendu = static_cast<double>(poidsDeRarete(rarete)) / poidsTotal;
        const double observe = static_cast<double>(compte[static_cast<std::size_t>(rarete)]) / s_tirages;
        INFO("rarete " << static_cast<int>(rarete) << " : attendu " << attendu << ", observe " << observe);
        REQUIRE(std::abs(observe - attendu) <= tolerance);
    }
}

TEST_CASE("genererObjet : la meme graine donne la meme suite d'objets, une autre graine une autre suite") {
    REQUIRE(tirer(200) == tirer(200));
    REQUIRE_FALSE(tirer(200) == tirer(200, s_graine + 1));
}
