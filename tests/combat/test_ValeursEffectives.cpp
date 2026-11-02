// ./tests/combat/test_ValeursEffectives.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "combat/Caracteristiques.hpp"
#include "combat/ValeursEffectives.hpp"

using Catch::Approx;

namespace {
Caracteristiques avecSavoir(int savoir) {
    return Caracteristiques::seule(Caracteristique::Savoir, savoir);
}
}  // namespace

TEST_CASE("bonusDAire : 0,3 % d'aire par point d'Intellect au-dessus de la reference, rien a la reference") {
    REQUIRE(bonusDAire(avecSavoir(s_valeurDeReference)) == Approx(0.0f));
    REQUIRE(bonusDAire(avecSavoir(s_valeurDeReference + 10)) == Approx(0.03f));
    REQUIRE(bonusDAire(avecSavoir(s_valeurDeReference + 50)) == Approx(0.15f));
}

TEST_CASE("bonusDAire : leger malus sous la reference, les autres caracteristiques n'y comptent pas") {
    REQUIRE(bonusDAire(avecSavoir(0)) == Approx(-0.03f));
    REQUIRE(bonusDAire(Caracteristiques({100, 100, 100, s_valeurDeReference, 100})) == Approx(0.0f));
}

TEST_CASE("bonusDAire : plafonne a +100 % d'aire") {
    // 333 points au-dessus de la référence donnent 0,999 : juste sous le plafond ; 334 points le dépassent et sont ramenés à lui.
    REQUIRE(bonusDAire(avecSavoir(s_valeurDeReference + 333)) < s_plafondDuBonusDAire);
    REQUIRE(bonusDAire(avecSavoir(s_valeurDeReference + 334)) == s_plafondDuBonusDAire);
    REQUIRE(bonusDAire(avecSavoir(s_valeurDeReference + 500)) == s_plafondDuBonusDAire);
    REQUIRE(bonusDAire(avecSavoir(1'000'000)) == s_plafondDuBonusDAire);
}

TEST_CASE("bonusDeDuree : 0,3 % par point d'Intellect au-dessus de la reference, plafonne a +100 %") {
    REQUIRE(bonusDeDuree(avecSavoir(s_valeurDeReference)) == Approx(0.0f));
    REQUIRE(bonusDeDuree(avecSavoir(s_valeurDeReference + 10)) == Approx(0.03f));
    REQUIRE(bonusDeDuree(avecSavoir(0)) == Approx(-0.03f));
    REQUIRE(bonusDeDuree(avecSavoir(1'000'000)) == s_plafondDuBonusDeDuree);
}

TEST_CASE("rayonEffectif : le rayon grandit comme la racine carree du bonus d'aire, pas comme le bonus") {
    // +100 % d'aire, c'est un rayon de racine de 2 fois le rayon de base, pas le double.
    REQUIRE(rayonEffectif(60.0f, 1.0f, avecSavoir(1'000'000)) == Approx(60.0f * std::sqrt(2.0f)));
    // +21 % d'aire (70 points au-dessus de la reference) : le rayon grandit de 10 %, car 1,1² = 1,21.
    REQUIRE(rayonEffectif(60.0f, 1.0f, avecSavoir(s_valeurDeReference + 70)) == Approx(60.0f * std::sqrt(1.21f)));
    REQUIRE(rayonEffectif(60.0f, 1.0f, avecSavoir(s_valeurDeReference + 70)) == Approx(66.0f));
}

TEST_CASE("rayonEffectif : un coefficient nul ou une Intellect de reference rendent le rayon de base") {
    REQUIRE(rayonEffectif(60.0f, 0.0f, avecSavoir(1'000'000)) == 60.0f);
    REQUIRE(rayonEffectif(60.0f, 1.0f, avecSavoir(s_valeurDeReference)) == 60.0f);
}

TEST_CASE("rayonEffectif : le coefficient pondere le bonus d'aire avant la racine") {
    // Un demi-coefficient et +100 % d'aire : +50 % d'aire, rayon × √1,5.
    REQUIRE(rayonEffectif(60.0f, 0.5f, avecSavoir(1'000'000)) == Approx(60.0f * std::sqrt(1.5f)));
}

TEST_CASE("rayonEffectif : sous la reference, le rayon diminue tres legerement") {
    const float rayon = rayonEffectif(60.0f, 1.0f, avecSavoir(0));  // -3 % d'aire

    REQUIRE(rayon < 60.0f);
    REQUIRE(rayon == Approx(60.0f * std::sqrt(0.97f)));
}

TEST_CASE("dureeEffective : la duree grandit du bonus selon le coefficient, plafonnee a +100 %") {
    REQUIRE(dureeEffective(4.0f, 1.0f, avecSavoir(s_valeurDeReference + 10)) == Approx(4.0f * 1.03f));
    REQUIRE(dureeEffective(4.0f, 0.5f, avecSavoir(s_valeurDeReference + 10)) == Approx(4.0f * 1.015f));
    REQUIRE(dureeEffective(4.0f, 1.0f, avecSavoir(1'000'000)) == Approx(8.0f));
}

TEST_CASE("dureeEffective : un coefficient nul rend la duree de base, quelle que soit l'Intellect") {
    REQUIRE(dureeEffective(4.0f, 0.0f, avecSavoir(1'000'000)) == 4.0f);
    REQUIRE(dureeEffective(4.0f, 0.0f, avecSavoir(0)) == 4.0f);
}
