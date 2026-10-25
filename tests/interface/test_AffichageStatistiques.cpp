// ./tests/interface/test_AffichageStatistiques.cpp
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "combat/StatistiquesDeCombat.hpp"
#include "interface/AffichageStatistiques.hpp"

TEST_CASE("affichage : les capacites, le blocage et les esquives, dans l'ordre des cellules") {
    const std::vector<std::string> lignes = lignesDeStatistiques(
        {.capaciteOffensive = 120, .capaciteDefensive = 95, .esquiveContact = 5, .esquiveProjectile = 12,
         .chanceDeBlocage = 30, .montantBloque = 40});

    REQUIRE(lignes == std::vector<std::string>{"Capacité offensive 120", "Capacité défensive 95", "Blocage 30 % (40)",
                                               "Esquive mêlée 5 %", "Esquive projectile 12 %"});
}

TEST_CASE("affichage : sans bouclier, le blocage s'affiche a 0 %") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.capaciteOffensive = 100, .capaciteDefensive = 100});

    REQUIRE(lignes[2] == "Blocage 0 %");
    REQUIRE(lignes[3] == "Esquive mêlée 0 %");
}

TEST_CASE("affichage : une chance de blocage sans rien a absorber s'affiche a 0 %") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.chanceDeBlocage = 30, .montantBloque = 0});

    REQUIRE(lignes[2] == "Blocage 0 %");
}

TEST_CASE("affichage : l'esquive s'affiche plafonnee, comme elle joue") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.esquiveContact = 80, .esquiveProjectile = 200});

    REQUIRE(lignes[3] == "Esquive mêlée 50 %");
    REQUIRE(lignes[4] == "Esquive projectile 50 %");
}

TEST_CASE("affichage : le bonus de combat d'un objet donne une ligne par bonus non nul") {
    const std::vector<std::string> lignes = lignesDeBonusDeCombat(
        {.capaciteOffensive = 10, .capaciteDefensive = -5, .esquiveContact = 5, .esquiveProjectile = 7});

    REQUIRE(lignes == std::vector<std::string>{"Capacité offensive : +10", "Capacité défensive : -5",
                                               "Esquive contre la mêlée : +5 %", "Esquive contre les projectiles : +7 %"});
}

TEST_CASE("affichage : un objet sans bonus de combat n'a aucune ligne") {
    REQUIRE(lignesDeBonusDeCombat({}).empty());
}
