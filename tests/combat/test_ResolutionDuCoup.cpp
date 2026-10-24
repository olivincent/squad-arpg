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

    REQUIRE(resoudreCoup(attaqueSansHasard(100), defensesDeTest(30), generateur).montant == appliquerArmure(100, 30));
    REQUIRE(resoudreCoup(attaqueSansHasard(50), defensesDeTest(0), generateur).montant == 50);
    REQUIRE_FALSE(resoudreCoup(attaqueSansHasard(100), defensesDeTest(30), generateur).critique);
}

TEST_CASE("resoudreCoup : un coup de 0 reste 0, meme avec une grande capacite offensive") {
    Generateur generateur = generateurDeTest();

    REQUIRE(resoudreCoup(attaqueSansHasard(0), defensesDeTest(30), generateur).montant == 0);
    REQUIRE(resoudreCoup({.montant = 0, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(0), generateur).montant == 0);
}

TEST_CASE("resoudreCoup ne tire rien quand la chance de critique est nulle ou le montant nul") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    static_cast<void>(resoudreCoup(attaqueSansHasard(100), defensesDeTest(30), generateur));  // capacité offensive nulle
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

    const int critiques = nombreDeCritiques({.montant = s_montant, .capaciteOffensive = 50, .nature = NatureDAttaque::Contact}, {.armure = 0, .statistiques = {.capaciteDefensive = 100000}}, 4000, generateur);

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

    const ResultatDuCoup soin = resoudreSoin(attaqueSansHasard(300), generateur);

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

namespace {
// Un défenseur sans armure qui esquive à ce pourcentage le contact et les projectiles.
Defenses esquiveur(int contact, int projectile) {
    Defenses defenses = defensesDeTest(0);
    defenses.statistiques.esquiveContact = contact;
    defenses.statistiques.esquiveProjectile = projectile;
    return defenses;
}

Attaque attaqueDeNature(NatureDAttaque nature) {
    return {.montant = s_montant, .capaciteOffensive = 0, .nature = nature};
}

int nombreDEsquives(const Attaque& attaque, const Defenses& defenses, int coups, Generateur& generateur) {
    int esquives = 0;
    for (int i = 0; i < coups; ++i) {
        esquives += resoudreCoup(attaque, defenses, generateur).esquive ? 1 : 0;
    }
    return esquives;
}
}  // namespace

TEST_CASE("resoudreCoup : un coup esquive s'arrete la, sans degats ni critique") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.montant = s_montant, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    bool esquiveVue = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaque, esquiveur(50, 0), generateur);
        if (resultat.esquive) {
            REQUIRE(resultat.montant == 0);
            REQUIRE_FALSE(resultat.critique);
            esquiveVue = true;
        }
    }
    REQUIRE(esquiveVue);
}

TEST_CASE("resoudreCoup : la frequence des esquives suit l'esquive du defenseur, plafonnee a 50 %") {
    Generateur generateur = generateurDeTest();
    const Attaque contact = attaqueDeNature(NatureDAttaque::Contact);

    // 5 % de 4 000 coups : 200 attendus ; 20 % : 800 ; plafond : un défenseur à 90 % esquive 50 %, soit 2 000.
    const int cinq = nombreDEsquives(contact, esquiveur(5, 0), 4000, generateur);
    const int vingt = nombreDEsquives(contact, esquiveur(20, 0), 4000, generateur);
    const int plafonne = nombreDEsquives(contact, esquiveur(90, 0), 4000, generateur);

    REQUIRE(cinq > 130);
    REQUIRE(cinq < 270);
    REQUIRE(vingt > 700);
    REQUIRE(vingt < 900);
    REQUIRE(plafonne > 1800);
    REQUIRE(plafonne < 2200);
}

TEST_CASE("resoudreCoup : l'esquive contre le contact n'esquive pas les projectiles, et inversement") {
    Generateur generateur = generateurDeTest();

    REQUIRE(nombreDEsquives(attaqueDeNature(NatureDAttaque::Projectile), esquiveur(50, 0), 500, generateur) == 0);
    REQUIRE(nombreDEsquives(attaqueDeNature(NatureDAttaque::Contact), esquiveur(0, 50), 500, generateur) == 0);
    REQUIRE(nombreDEsquives(attaqueDeNature(NatureDAttaque::Projectile), esquiveur(0, 50), 500, generateur) > 0);
    REQUIRE(nombreDEsquives(attaqueDeNature(NatureDAttaque::Contact), esquiveur(50, 0), 500, generateur) > 0);
}

TEST_CASE("resoudreCoup : une zone n'est jamais esquivee, et ne tire rien pour cela") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    REQUIRE(nombreDEsquives(attaqueDeNature(NatureDAttaque::Zone), esquiveur(50, 50), 500, generateur) == 0);

    REQUIRE(generateur == intact);  // sans chance d'esquive ni de critique : aucun tirage
}

TEST_CASE("resoudreCoup : sans esquive ni capacite offensive, aucun tirage") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    static_cast<void>(resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), esquiveur(0, 0), generateur));

    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreCoup : une esquive possible tire, et un coup esquive ne tire pas son critique") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.montant = s_montant, .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    int tiragesParCoupEsquive = 0;
    int tiragesParCoupTouche = 0;

    for (int i = 0; i < 200 && (tiragesParCoupEsquive == 0 || tiragesParCoupTouche == 0); ++i) {
        Generateur avant = generateur;
        const bool esquive = resoudreCoup(attaque, esquiveur(50, 0), generateur).esquive;
        int tirages = 0;
        while (avant != generateur) {
            static_cast<void>(avant());
            ++tirages;
        }
        (esquive ? tiragesParCoupEsquive : tiragesParCoupTouche) = tirages;
    }

    // Esquivé : un seul nombre (l'esquive). Touché : deux (l'esquive, puis le critique).
    REQUIRE(tiragesParCoupEsquive == 1);
    REQUIRE(tiragesParCoupTouche == 2);
}

TEST_CASE("resoudreCoup : l'esquive ne change pas ce que la capacite defensive et l'armure font du coup touche") {
    Generateur generateur = generateurDeTest();

    for (int i = 0; i < 100; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), {.armure = s_armure, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 50}}, generateur);
        if (!resultat.esquive) {
            REQUIRE(resultat.montant == appliquerArmure(s_montant, s_armure));
        }
    }
}
