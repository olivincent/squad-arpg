// ./tests/interface/test_AffichageEffets.cpp
#include <catch2/catch_test_macros.hpp>

#include "interface/AffichageEffets.hpp"

namespace {
EffetActif effet(IdSkill source, int pourcentage, float duree) {
    return {.source = source,
            .dureeRestante = duree,
            .sorte = ModificateurDeStatistique{.statistique = StatistiqueModifiable::Armure, .pourcentage = pourcentage}};
}
}  // namespace

TEST_CASE("ligneDEffetActif : le skill, la valeur signee et les secondes restantes, en anglais") {
    REQUIRE(ligneDEffetActif(effet(IdSkill::Benediction, 20, 10.0f)) == "Blessing: +20 % Armor (10 s)");
    REQUIRE(ligneDEffetActif(effet(IdSkill::FrappePuissante, -25, 8.0f)) == "Power Strike: −25 % Armor (8 s)");  // signe moins typographique
}

TEST_CASE("ligneDEffetActif : les secondes sont arrondies vers le haut, un effet actif n'affiche jamais 0 s") {
    REQUIRE(ligneDEffetActif(effet(IdSkill::Benediction, 20, 9.2f)) == "Blessing: +20 % Armor (10 s)");
    REQUIRE(ligneDEffetActif(effet(IdSkill::Benediction, 20, 0.1f)) == "Blessing: +20 % Armor (1 s)");
    REQUIRE(ligneDEffetActif(effet(IdSkill::Benediction, 20, 3.0f)) == "Blessing: +20 % Armor (3 s)");
}

TEST_CASE("titre et nom de statistique du bloc des effets actifs") {
    REQUIRE(titreDesEffetsActifs() == "Active effects");
    REQUIRE(nomDeLaStatistique(StatistiqueModifiable::Armure) == "Armor");
}
