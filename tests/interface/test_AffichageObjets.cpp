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

TEST_CASE("affichage : l'infobulle d'un objet sans armure n'a pas de ligne d'armure, mais une ligne par bonus") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::AnneauDePuissance);

    REQUIRE(lignes.size() == 3);
    REQUIRE(lignes[0] == definitionObjet(IdObjet::AnneauDePuissance).nom);
    REQUIRE(lignes[1] == "Anneau");
    REQUIRE(lignes[2] == "Puissance : +" + std::to_string(definitionObjet(IdObjet::AnneauDePuissance).bonus.valeur(Caracteristique::Puissance)));
}

TEST_CASE("affichage : l'infobulle du collier d'endurance donne l'Endurance") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::CollierDEndurance);

    REQUIRE(lignes.size() == 3);
    REQUIRE(lignes[2] == "Endurance : +" + std::to_string(definitionObjet(IdObjet::CollierDEndurance).bonus.valeur(Caracteristique::Endurance)));
}

TEST_CASE("affichage : aucun objet du catalogue n'affiche une valeur nulle") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        for (const std::string& ligne : lignesDeLInfobulle(definition.id)) {
            REQUIRE(ligne.find("+0") == std::string::npos);
        }
    }
}

TEST_CASE("affichage : l'infobulle des bottes de cuir donne l'esquive contre la melee") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::BottesDeCuir);

    REQUIRE(lignes.back() == "Esquive contre la mêlée : +5 %");
}

TEST_CASE("affichage : l'infobulle du casque de cuir donne l'esquive contre les projectiles") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::CasqueDeCuir);

    REQUIRE(lignes.back() == "Esquive contre les projectiles : +5 %");
}

TEST_CASE("affichage : l'infobulle du bouclier rond donne son blocage") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::BouclierRond);

    const StatistiquesDeCombat& bouclier = definitionObjet(IdObjet::BouclierRond).bonusDeCombat;
    REQUIRE(lignes.back() == "Blocage : " + std::to_string(bouclier.chanceDeBlocage) + " %, " +
                                 std::to_string(bouclier.montantBloque) + " points, 3 s");
}
