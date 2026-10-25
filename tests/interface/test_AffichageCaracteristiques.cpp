// ./tests/interface/test_AffichageCaracteristiques.cpp
#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>

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
