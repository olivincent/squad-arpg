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

// Le contrôle (étape 113) : le nom de l'état, son pourcentage pour le ralentissement et l'entrave, les secondes restantes.

namespace {
EffetActif controle(EtatInflige etat, int pourcentage, float duree) {
    return {.source = IdSkill::PluieDeFleches, .dureeRestante = duree, .sorte = Controle{.etat = etat, .pourcentage = pourcentage}};
}
}  // namespace

TEST_CASE("ligneDEffetActif : un controle porte le nom de son etat, pas celui du skill") {
    REQUIRE(ligneDEffetActif(controle(EtatInflige::Etourdissement, 0, 1.5f)) == "Stunned (2 s)");
    REQUIRE(ligneDEffetActif(controle(EtatInflige::Disruption, 0, 1.0f)) == "Disrupted (1 s)");
}

TEST_CASE("ligneDEffetActif : le ralentissement et l'entrave disent leur pourcentage, avec le signe moins typographique") {
    REQUIRE(ligneDEffetActif(controle(EtatInflige::Ralentissement, 30, 3.0f)) == "Slowed: −30 % (3 s)");
    REQUIRE(ligneDEffetActif(controle(EtatInflige::Entrave, 20, 2.2f)) == "Hampered: −20 % (3 s)");
}

TEST_CASE("nomDeLEtatInflige : les quatre etats, en anglais") {
    REQUIRE(nomDeLEtatInflige(EtatInflige::Etourdissement) == "Stunned");
    REQUIRE(nomDeLEtatInflige(EtatInflige::Ralentissement) == "Slowed");
    REQUIRE(nomDeLEtatInflige(EtatInflige::Entrave) == "Hampered");
    REQUIRE(nomDeLEtatInflige(EtatInflige::Disruption) == "Disrupted");
}

TEST_CASE("ligneDEffetActif : un bouclier porte le nom de sa famille et ses points restants") {
    const EffetActif aegis{.source = IdSkill::Soin, .dureeRestante = 5.2f, .sorte = Absorption{.famille = FamilleDAbsorption::Aegis, .points = 45}};
    const EffetActif ward{.source = IdCapacite::CoupAssommant, .dureeRestante = 8.0f, .sorte = Absorption{.famille = FamilleDAbsorption::Ward, .points = 60}};

    REQUIRE(ligneDEffetActif(aegis) == "Aegis: 45 (6 s)");
    REQUIRE(ligneDEffetActif(ward) == "Ward: 60 (8 s)");
}

TEST_CASE("nomDeLaFamilleDAbsorption : Aegis et Ward") {
    REQUIRE(nomDeLaFamilleDAbsorption(FamilleDAbsorption::Aegis) == "Aegis");
    REQUIRE(nomDeLaFamilleDAbsorption(FamilleDAbsorption::Ward) == "Ward");
}
