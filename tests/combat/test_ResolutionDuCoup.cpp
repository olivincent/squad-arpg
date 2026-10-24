// ./tests/combat/test_ResolutionDuCoup.cpp
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/Defense.hpp"
#include "combat/ResolutionDuCoup.hpp"

namespace {
// Une capacité offensive assez grande pour que le critique soit au plafond : 50 % de chance, ×2.
constexpr int s_capaciteEnorme = 100000;
constexpr int s_montant = 100;
constexpr int s_armure = 30;

int nombreDeCritiques(const Attaque& attaque, const Defenses& defenses, int coups, Generateur& generateur) {
    int critiques = 0;
    for (int i = 0; i < coups; ++i) {
        if (resoudreCoup(attaque, defenses, generateur).critique) {
            ++critiques;
        }
    }
    return critiques;
}
}  // namespace

TEST_CASE("resoudreCoup applique l'armure du defenseur") {
    Generateur generateur = generateurDeTest();

    REQUIRE(resoudreCoup(attaqueSansCritique(100), defensesDeTest(30), generateur).montant == appliquerArmure(100, 30));
    REQUIRE(resoudreCoup(attaqueSansCritique(50), defensesDeTest(0), generateur).montant == 50);
    REQUIRE_FALSE(resoudreCoup(attaqueSansCritique(100), defensesDeTest(30), generateur).critique);
}

TEST_CASE("resoudreCoup : un coup de 0 reste 0, meme avec une grande capacite offensive") {
    Generateur generateur = generateurDeTest();

    REQUIRE(resoudreCoup(attaqueSansCritique(0), defensesDeTest(30), generateur).montant == 0);
    REQUIRE(resoudreCoup({.montant = 0, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(0), generateur).montant == 0);
}

TEST_CASE("resoudreCoup ne tire rien quand la chance de critique est nulle ou le montant nul") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    static_cast<void>(resoudreCoup(attaqueSansCritique(100), defensesDeTest(30), generateur));  // capacité offensive nulle
    static_cast<void>(resoudreCoup({.montant = 0, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(0), generateur));

    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreCoup tire quand la chance de critique est non nulle") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    static_cast<void>(resoudreCoup({.montant = 100, .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, defensesDeTest(0), generateur));

    REQUIRE(generateur != intact);
}

TEST_CASE("resoudreCoup : un critique multiplie le montant avant l'armure") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.montant = s_montant, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    bool critiqueVu = false;
    bool normalVu = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaque, defensesDeTest(s_armure), generateur);
        if (resultat.critique) {
            REQUIRE(resultat.montant == appliquerArmure(2 * s_montant, s_armure));  // 200, puis l'armure : 170
            critiqueVu = true;
        } else {
            REQUIRE(resultat.montant == appliquerArmure(s_montant, s_armure));
            normalVu = true;
        }
    }

    REQUIRE(critiqueVu);
    REQUIRE(normalVu);
}

TEST_CASE("resoudreCoup : la frequence des critiques suit le rapport des capacites") {
    Generateur generateur = generateurDeTest();

    // Rapport 1 : 5 % ; plafond : 50 % (de 4 000 coups, 200 et 2 000 attendus).
    const int aRapportUn = nombreDeCritiques({.montant = s_montant, .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, defensesDeTest(), 4000, generateur);
    const int auPlafond = nombreDeCritiques({.montant = s_montant, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(), 4000, generateur);

    REQUIRE(aRapportUn > 130);
    REQUIRE(aRapportUn < 270);
    REQUIRE(auPlafond > 1800);
    REQUIRE(auPlafond < 2200);
}

TEST_CASE("resoudreCoup : une grande capacite defensive annule pratiquement les critiques") {
    Generateur generateur = generateurDeTest();

    const int critiques = nombreDeCritiques({.montant = s_montant, .capaciteOffensive = 50, .nature = NatureDAttaque::Contact}, {.armure = 0, .capaciteDefensive = 100000}, 4000, generateur);

    REQUIRE(critiques == 0);
}

TEST_CASE("resoudreCoup : meme graine, memes resultats") {
    Generateur premier = generateurDeTest();
    Generateur second = generateurDeTest();
    const Attaque attaque{.montant = s_montant, .capaciteOffensive = 150, .nature = NatureDAttaque::Contact};

    for (int i = 0; i < 100; ++i) {
        const ResultatDuCoup a = resoudreCoup(attaque, defensesDeTest(10), premier);
        const ResultatDuCoup b = resoudreCoup(attaque, defensesDeTest(10), second);
        REQUIRE(a.montant == b.montant);
        REQUIRE(a.critique == b.critique);
    }
}

TEST_CASE("resoudreSoin : sans capacite offensive, le soin est rendu tel quel et sans tirage") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup soin = resoudreSoin(attaqueSansCritique(300), generateur);

    REQUIRE(soin.montant == 300);
    REQUIRE_FALSE(soin.critique);
    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreSoin : un critique multiplie le soin, sans armure") {
    Generateur generateur = generateurDeTest();
    bool critiqueVu = false;
    bool normalVu = false;

    for (int i = 0; i < 100; ++i) {
        const ResultatDuCoup soin = resoudreSoin({.montant = 300, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, generateur);
        REQUIRE(soin.montant == (soin.critique ? 600 : 300));
        critiqueVu = critiqueVu || soin.critique;
        normalVu = normalVu || !soin.critique;
    }

    REQUIRE(critiqueVu);
    REQUIRE(normalVu);
}

TEST_CASE("resoudreSoin : le soin tente son critique contre une capacite defensive de reference de 100") {
    Generateur generateur = generateurDeTest();
    int critiques = 0;

    for (int i = 0; i < 4000; ++i) {
        critiques += resoudreSoin({.montant = 300, .capaciteOffensive = s_capaciteDefensiveDeReference, .nature = NatureDAttaque::Contact}, generateur).critique ? 1 : 0;
    }

    REQUIRE(critiques > 130);  // rapport 1 : 5 % de 4 000
    REQUIRE(critiques < 270);
}
