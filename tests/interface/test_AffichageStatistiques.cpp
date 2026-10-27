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

    REQUIRE(lignes == std::vector<std::string>{"Offensive Ability 120", "Defensive Ability 95", "Block 30 % (40)",
                                               "Melee Dodge 5 %", "Projectile Dodge 12 %"});
}

TEST_CASE("affichage : sans bouclier, le blocage s'affiche a 0 %") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.capaciteOffensive = 100, .capaciteDefensive = 100});

    REQUIRE(lignes[2] == "Block 0 %");
    REQUIRE(lignes[3] == "Melee Dodge 0 %");
}

TEST_CASE("affichage : une chance de blocage sans rien a absorber s'affiche a 0 %") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.chanceDeBlocage = 30, .montantBloque = 0});

    REQUIRE(lignes[2] == "Block 0 %");
}

TEST_CASE("affichage : l'esquive s'affiche plafonnee, comme elle joue") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.esquiveContact = 80, .esquiveProjectile = 200});

    REQUIRE(lignes[3] == "Melee Dodge 50 %");
    REQUIRE(lignes[4] == "Projectile Dodge 50 %");
}

TEST_CASE("affichage : le bonus de combat d'un objet donne une ligne par bonus non nul") {
    const std::vector<std::string> lignes = lignesDeBonusDeCombat(
        {.capaciteOffensive = 10, .capaciteDefensive = -5, .esquiveContact = 5, .esquiveProjectile = 7});

    REQUIRE(lignes == std::vector<std::string>{"Offensive Ability: +10", "Defensive Ability: −5",
                                               "Dodge vs Melee: +5 %", "Dodge vs Projectiles: +7 %"});
}

TEST_CASE("affichage : un objet sans bonus de combat n'a aucune ligne") {
    REQUIRE(lignesDeBonusDeCombat({}).empty());
}

TEST_CASE("affichage : l'esquive negative d'une armure lourde s'affiche 0, jamais sous 0") {
    const std::vector<std::string> lignes = lignesDeStatistiques({.esquiveContact = -6, .esquiveProjectile = -2});

    REQUIRE(lignes[3] == "Melee Dodge 0 %");
    REQUIRE(lignes[4] == "Projectile Dodge 0 %");
}

TEST_CASE("affichage : deux esquives egales et non nulles tiennent sur une ligne, des esquives differentes sur deux") {
    REQUIRE(lignesDeBonusDeCombat({.esquiveContact = -3, .esquiveProjectile = -3}) == std::vector<std::string>{"Dodge: −3 %"});
    REQUIRE(lignesDeBonusDeCombat({.esquiveContact = 4, .esquiveProjectile = 4}) == std::vector<std::string>{"Dodge: +4 %"});
    REQUIRE(lignesDeBonusDeCombat({.esquiveContact = -3, .esquiveProjectile = -1}) ==
            std::vector<std::string>{"Dodge vs Melee: −3 %", "Dodge vs Projectiles: −1 %"});
}

TEST_CASE("affichage : les lignes de malus sont celles des seules valeurs negatives") {
    REQUIRE(lignesDeMalusDeCombat({.capaciteOffensive = 10, .capaciteDefensive = -5, .esquiveContact = -3, .esquiveProjectile = -3}) ==
            std::vector<std::string>{"Defensive Ability: −5", "Dodge: −3 %"});
    REQUIRE(lignesDeMalusDeCombat({.capaciteOffensive = 10, .esquiveContact = 5}).empty());
    REQUIRE(lignesDeMalusDeCombat({}).empty());
}
