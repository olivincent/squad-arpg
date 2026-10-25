// ./tests/interface/test_AffichageObjets.cpp
#include <catch2/catch_test_macros.hpp>
#include <algorithm>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "combat/DescriptionDAttaque.hpp"
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

TEST_CASE("affichage : l'infobulle donne le nom, la categorie, le seuil et l'armure") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::CasqueDeFer);

    REQUIRE(lignes.size() == 4);
    REQUIRE(lignes[0] == definitionObjet(IdObjet::CasqueDeFer).nom);
    REQUIRE(lignes[1] == "Casque");
    REQUIRE(lignes[2] == ligneDeSeuil(definitionObjet(IdObjet::CasqueDeFer).seuils.front()));
    REQUIRE(lignes[3] == "Armure : +" + std::to_string(definitionObjet(IdObjet::CasqueDeFer).armure));
}

TEST_CASE("affichage : une ligne de seuil donne la caracteristique et la valeur exigee") {
    REQUIRE(ligneDeSeuil({Caracteristique::Puissance, 11}) == "Exige Puissance 11");
    REQUIRE(ligneDeSeuil({Caracteristique::Endurance, 12}) == "Exige Endurance 12");
}

TEST_CASE("affichage : l'infobulle liste tous les seuils, un par ligne, apres la categorie") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::EpeeADeuxMains);

    REQUIRE(lignes[1] == "Arme");
    REQUIRE(lignes[2] == "Exige Puissance 13");
    REQUIRE(lignes[3] == "Exige Endurance 12");
}

TEST_CASE("affichage : un objet sans seuil n'a aucune ligne d'exigence") {
    for (const IdObjet libre : {IdObjet::GantsDeCuir, IdObjet::BottesDeCuir, IdObjet::AnneauDePuissance, IdObjet::CollierDEndurance}) {
        for (const std::string& ligne : lignesDeLInfobulle(libre)) {
            REQUIRE(ligne.find("Exige") == std::string::npos);
        }
    }
}

TEST_CASE("affichage : la ligne d'un objet inactif") {
    REQUIRE(ligneDObjetInactif() == "Inactif : seuil non atteint");
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

    const DonneesDeBouclier& bouclier = definitionObjet(IdObjet::BouclierRond).bouclier.value();
    REQUIRE(lignes.back() == "Blocage : " + std::to_string(bouclier.chanceDeBlocage) + " %, " +
                                 std::to_string(bouclier.montantBloque) + " points");
}

TEST_CASE("affichage : l'infobulle d'une arme a une main donne ses degats, sa recharge, sa portee et sa nature") {
    const std::vector<std::string> lignes = lignesDeLInfobulle(IdObjet::EpeeLongue);
    const DescriptionDAttaque& attaque = definitionObjet(IdObjet::EpeeLongue).arme.value().attaque;

    REQUIRE(lignes == std::vector<std::string>{"Épée longue", "Arme", "Exige Puissance 11", "Dégâts : " + std::to_string(attaque.degats),
                                               "Recharge : 0,3 à 0,7 s", "Portée : 20 px", "Contact"});
}

TEST_CASE("affichage : l'infobulle d'une arme a deux mains le dit, celle d'une arme de tir donne un projectile") {
    const std::vector<std::string> arc = lignesDeLInfobulle(IdObjet::Arc);
    const std::vector<std::string> sceptre = lignesDeLInfobulle(IdObjet::Sceptre);
    const std::vector<std::string> deuxMains = lignesDeLInfobulle(IdObjet::EpeeADeuxMains);

    REQUIRE(arc.back() == "Deux mains");
    REQUIRE(arc[arc.size() - 2] == "Projectile");
    REQUIRE(arc[4] == "Recharge : 0,4 à 0,6 s");
    REQUIRE(arc[5] == "Portée : 120 px");
    REQUIRE(sceptre.back() == "Projectile");  // une main : pas de « Deux mains »
    REQUIRE(deuxMains.back() == "Deux mains");
    REQUIRE(deuxMains[5] == "Recharge : 1,2 à 2,8 s");  // après deux lignes de seuil
}

TEST_CASE("affichage : seules les armes ont des lignes d'attaque, et les deux mains ne sont dites que des armes a deux mains") {
    for (const DefinitionObjet& definition : catalogueObjets()) {
        const std::vector<std::string> lignes = lignesDeLInfobulle(definition.id);
        const bool aDesDegats = std::ranges::any_of(lignes, [](const std::string& ligne) { return ligne.starts_with("Dégâts"); });
        const bool deuxMains = std::ranges::any_of(lignes, [](const std::string& ligne) { return ligne == "Deux mains"; });
        REQUIRE(aDesDegats == definition.arme.has_value());
        REQUIRE(deuxMains == estADeuxMains(definition.id));
    }
}
