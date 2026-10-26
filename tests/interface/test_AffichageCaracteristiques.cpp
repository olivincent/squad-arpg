// ./tests/interface/test_AffichageCaracteristiques.cpp
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <set>
#include <string>

#include "combat/Caracteristiques.hpp"
#include "combat/StatistiquesDeCombat.hpp"
#include "interface/AffichageCaracteristiques.hpp"

TEST_CASE("affichage : chaque caracteristique a un nom, tous differents") {
    std::set<std::string> noms;
    for (const Caracteristique caracteristique : s_caracteristiques) {
        const std::string nom = nomDeLaCaracteristique(caracteristique);
        REQUIRE_FALSE(nom.empty());
        noms.insert(nom);
    }
    REQUIRE(noms.size() == s_nombreDeCaracteristiques);
}

TEST_CASE("affichage : la ligne d'une caracteristique donne sa valeur effective") {
    REQUIRE(ligneDeCaracteristique(Caracteristique::Endurance, 14, 0) == "Constitution 14");
}

TEST_CASE("affichage : un bonus de l'equipement s'ajoute entre parentheses") {
    REQUIRE(ligneDeCaracteristique(Caracteristique::Puissance, 15, 3) == "Might 15 (+3)");
    REQUIRE(ligneDeCaracteristique(Caracteristique::Savoir, 4, -2) == "Intellect 4 (-2)");
}

// Les points libres (étape 100.2).
TEST_CASE("affichage : les points libres s ecrivent Points: N") {
    REQUIRE(ligneDePointsLibres(3) == "Points: 3");
    REQUIRE(ligneDePointsLibres(0) == "Points: 0");
}

TEST_CASE("affichage : les points en attente s ecrivent (+N), rien s il n y en a pas") {
    REQUIRE(textePointsEnAttente(2) == "(+2)");
    REQUIRE(textePointsEnAttente(0).empty());
}

TEST_CASE("affichage : l'effet d'une caracteristique lit les constantes du jeu, sans les recopier") {
    REQUIRE(ligneDEffetDeLaCaracteristique(Caracteristique::Puissance) ==
            "Might: +" + std::to_string(std::lround(s_bonusParPoint * 100.0f)) + "% damage and healing per point");
    REQUIRE(ligneDEffetDeLaCaracteristique(Caracteristique::Endurance) ==
            "Constitution: +" + std::to_string(s_pvParPointDEndurance) + " health per point");
    REQUIRE(ligneDEffetDeLaCaracteristique(Caracteristique::Adresse) ==
            "Dexterity: +" + std::to_string(s_capaciteParPointDAdresse) + " offensive and defensive capability per point");
    REQUIRE(ligneDEffetDeLaCaracteristique(Caracteristique::Savoir) == "Intellect: no effect yet");
    REQUIRE(ligneDEffetDeLaCaracteristique(Caracteristique::Volonte) == "Resolve: no effect yet");
}
