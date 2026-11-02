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

// Les effets dans le temps et les piles (étape 112).

namespace {
EffetActif degatsDansLeTemps(Element element, int montant, float duree, int piles = 1) {
    return {.source = IdSkill::TirPercant,
            .rang = 1,
            .dureeRestante = duree,
            .sorte = DegatsDansLeTemps{.element = element, .montant = montant},
            .pilesMax = 3,
            .piles = piles};
}
}  // namespace

TEST_CASE("ligneDEffetActif : des degats dans le temps portent le nom de leur element, le montant par seconde et les piles") {
    REQUIRE(ligneDEffetActif(degatsDansLeTemps(Element::Physique, 7, 4.0f, 2)) == "Bleeding ×2: 14 per s (4 s)");
    REQUIRE(ligneDEffetActif(degatsDansLeTemps(Element::Physique, 7, 3.2f, 1)) == "Bleeding: 7 per s (4 s)");
    REQUIRE(ligneDEffetActif(degatsDansLeTemps(Element::Feu, 8, 2.0f)) == "Burning: 8 per s (2 s)");
}

TEST_CASE("ligneDEffetActif : un soin dans le temps porte le nom de son skill") {
    const EffetActif renew{.source = IdSkill::Renouvellement, .dureeRestante = 6.0f, .sorte = SoinDansLeTemps{.montant = 9}};

    REQUIRE(ligneDEffetActif(renew) == "Renew: 9 per s (6 s)");
}

TEST_CASE("ligneDEffetActif : un modificateur empile montre la valeur de toutes ses piles") {
    EffetActif affaiblissement = effet(IdSkill::FrappePuissante, -10, 8.0f);
    affaiblissement.pilesMax = 3;
    affaiblissement.piles = 2;

    REQUIRE(ligneDEffetActif(affaiblissement) == "Power Strike ×2: −20 % Armor (8 s)");
}
