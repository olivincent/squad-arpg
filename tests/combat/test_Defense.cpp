// ./tests/combat/test_Defense.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "combat/Defense.hpp"

TEST_CASE("Defense sans armure laisse le montant inchange") {
    REQUIRE(appliquerArmure(10, 0) == 10);
    REQUIRE(appliquerArmure(1, 0) == 1);
    REQUIRE(appliquerArmure(137, 0) == 137);
}

TEST_CASE("Defense au-dessus du plancher : l'armure retire un montant fixe") {
    REQUIRE(appliquerArmure(100, 30) == 70);
    REQUIRE(appliquerArmure(50, 10) == 40);
    REQUIRE(appliquerArmure(100, 79) == 21);
}

TEST_CASE("Defense au plancher : le coup garde 20 pour cent") {
    REQUIRE(appliquerArmure(100, 80) == 20);
    REQUIRE(appliquerArmure(100, 500) == 20);
    REQUIRE(appliquerArmure(50, 100000) == 10);
}

TEST_CASE("Defense : le plancher est arrondi au plus proche") {
    REQUIRE(appliquerArmure(12, 1000) == 2);   // 2,4
    REQUIRE(appliquerArmure(13, 1000) == 3);   // 2,6
}

TEST_CASE("Defense : un coup qui passe inflige au moins 1") {
    REQUIRE(appliquerArmure(1, 50) == 1);
    REQUIRE(appliquerArmure(1, 1000000) == 1);
    REQUIRE(appliquerArmure(2, 1000) == 1);  // 0,4 -> 1
}

TEST_CASE("Defense : un coup de 0 reste 0") {
    REQUIRE(appliquerArmure(0, 0) == 0);
    REQUIRE(appliquerArmure(0, 50) == 0);
}

TEST_CASE("Defense : jamais plus que le coup, jamais moins que le plancher") {
    for (int armure = 0; armure <= 300; armure += 7) {
        const int subis = appliquerArmure(100, armure);
        REQUIRE(subis <= 100);
        REQUIRE(subis >= 20);
    }
}

// La pénétration d'armure (étape 92.4) : elle réduit l'armure avant la soustraction M1.
TEST_CASE("Defense avec penetration : l'armure effective est armure x (1 - penetration)") {
    REQUIRE(appliquerArmure(100, 40, 25) == 70);    // armure 30
    REQUIRE(appliquerArmure(100, 40, 50) == 80);    // armure 20
    REQUIRE(appliquerArmure(100, 30, 0) == 70);     // sans pénétration : inchangé
    REQUIRE(appliquerArmure(100, 30, 30) == 79);    // 30 x 0,7 = 21
}

TEST_CASE("Defense avec penetration : pas d'arrondi intermediaire, l'arrondi est celui de M1") {
    // 10 x 0,7 = 7 ; 100 - 7 = 93. Avec 15 d'armure : 15 x 0,7 = 10,5 ; 100 - 10,5 = 89,5 -> 90 (arrondi au plus proche, a la fin).
    REQUIRE(appliquerArmure(100, 10, 30) == 93);
    REQUIRE(appliquerArmure(100, 15, 30) == 90);
    // Un arrondi de l'armure d'abord (10,5 -> 10 ou 11) donnerait 90 ou 89 : ici 89,5 arrondi donne 90.
}

TEST_CASE("Defense avec penetration : plafonnee a 75 %") {
    REQUIRE(appliquerArmure(100, 40, 75) == 90);    // armure 10
    REQUIRE(appliquerArmure(100, 40, 100) == 90);   // plafonnée
    REQUIRE(appliquerArmure(100, 40, 1000) == appliquerArmure(100, 40, s_penetrationMaximale));
}

TEST_CASE("Defense avec penetration : une penetration negative ne compte pas, le plancher de 20 % reste") {
    REQUIRE(appliquerArmure(100, 30, -50) == 70);
    REQUIRE(appliquerArmure(100, 500, 30) == 20);  // 500 x 0,7 = 350 : le plancher de 20 %
}

TEST_CASE("Defense avec penetration : elle ne fait jamais subir moins ni plus que sans pour une meme armure") {
    for (int armure = 0; armure <= 300; armure += 7) {
        for (int penetration = 0; penetration <= 75; penetration += 15) {
            const int sans = appliquerArmure(100, armure, 0);
            const int avec = appliquerArmure(100, armure, penetration);
            REQUIRE(avec >= sans);
            REQUIRE(avec <= 100);
        }
    }
}

// Les étapes flottantes du moteur du coup (étape 94) : aucun arrondi, un seul à la fin du coup.
TEST_CASE("armureAppliquee : l'armure M1 sans arrondi") {
    REQUIRE_THAT(armureAppliquee(100.0f, 30), Catch::Matchers::WithinAbs(70.0, 1e-4));
    REQUIRE_THAT(armureAppliquee(12.0f, 1000), Catch::Matchers::WithinAbs(2.4, 1e-4));     // le plancher, 2,4 : pas arrondi
    REQUIRE_THAT(armureAppliquee(100.0f, 15, 30), Catch::Matchers::WithinAbs(89.5, 1e-4));  // 15 × 0,7 = 10,5
    REQUIRE_THAT(armureAppliquee(0.0f, 50), Catch::Matchers::WithinAbs(0.0, 1e-4));         // 0 reste 0, sans minimum de 1
}

TEST_CASE("resistanceEffective : ramenee entre -100 et 80") {
    REQUIRE(resistanceEffective(0) == 0);
    REQUIRE(resistanceEffective(40) == 40);
    REQUIRE(resistanceEffective(80) == 80);
    REQUIRE(resistanceEffective(81) == 80);
    REQUIRE(resistanceEffective(500) == 80);
    REQUIRE(resistanceEffective(-100) == -100);
    REQUIRE(resistanceEffective(-101) == -100);
    REQUIRE(resistanceEffective(-250) == -100);
}

TEST_CASE("appliquerResistance : la portion perd la part resistee, sans arrondi") {
    REQUIRE_THAT(appliquerResistance(100.0f, 0), Catch::Matchers::WithinAbs(100.0, 1e-4));
    REQUIRE_THAT(appliquerResistance(100.0f, 40), Catch::Matchers::WithinAbs(60.0, 1e-4));
    REQUIRE_THAT(appliquerResistance(3.0f, 50), Catch::Matchers::WithinAbs(1.5, 1e-4));
    REQUIRE_THAT(appliquerResistance(0.0f, 50), Catch::Matchers::WithinAbs(0.0, 1e-4));
}

TEST_CASE("appliquerResistance : plafonnee a 80 %, une vulnerabilite augmente la portion") {
    REQUIRE_THAT(appliquerResistance(100.0f, 95), Catch::Matchers::WithinAbs(20.0, 1e-4));
    REQUIRE_THAT(appliquerResistance(100.0f, -50), Catch::Matchers::WithinAbs(150.0, 1e-4));
    REQUIRE_THAT(appliquerResistance(100.0f, -100), Catch::Matchers::WithinAbs(200.0, 1e-4));
    REQUIRE_THAT(appliquerResistance(100.0f, -300), Catch::Matchers::WithinAbs(200.0, 1e-4));  // ramenée à −100
}
