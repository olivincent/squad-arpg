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
        if (resoudreCoup(attaque, defenses, true, generateur).critique) {
            ++critiques;
        }
    }
    return critiques;
}
}  // namespace

TEST_CASE("resoudreCoup applique l'armure du defenseur") {
    Generateur generateur = generateurDeTest();

    REQUIRE(resoudreCoup(attaqueSansHasard(100), defensesDeTest(30), true, generateur).montant == appliquerArmure(100, 30));
    REQUIRE(resoudreCoup(attaqueSansHasard(50), defensesDeTest(0), true, generateur).montant == 50);
    REQUIRE_FALSE(resoudreCoup(attaqueSansHasard(100), defensesDeTest(30), true, generateur).critique);
}

TEST_CASE("resoudreCoup : un coup de 0 reste 0, meme avec une grande capacite offensive") {
    Generateur generateur = generateurDeTest();

    REQUIRE(resoudreCoup(attaqueSansHasard(0), defensesDeTest(30), true, generateur).montant == 0);
    REQUIRE(resoudreCoup({.degats = degatsPhysiques(0), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(0), true, generateur).montant == 0);
}

TEST_CASE("resoudreCoup ne tire rien quand la chance de critique est nulle ou le montant nul") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    static_cast<void>(resoudreCoup(attaqueSansHasard(100), defensesDeTest(30), true, generateur));  // capacité offensive nulle
    static_cast<void>(resoudreCoup({.degats = degatsPhysiques(0), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(0), true, generateur));

    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreCoup tire quand la chance de critique est non nulle") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    static_cast<void>(resoudreCoup({.degats = degatsPhysiques(100), .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, defensesDeTest(0), true, generateur));

    REQUIRE(generateur != intact);
}

TEST_CASE("resoudreCoup : un critique multiplie le montant avant l'armure") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(s_montant), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    bool critiqueVu = false;
    bool normalVu = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaque, defensesDeTest(s_armure), true, generateur);
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
    const int aRapportUn = nombreDeCritiques({.degats = degatsPhysiques(s_montant), .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, defensesDeTest(), 4000, generateur);
    const int auPlafond = nombreDeCritiques({.degats = degatsPhysiques(s_montant), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact}, defensesDeTest(), 4000, generateur);

    REQUIRE(aRapportUn > 130);
    REQUIRE(aRapportUn < 270);
    REQUIRE(auPlafond > 1800);
    REQUIRE(auPlafond < 2200);
}

TEST_CASE("resoudreCoup : une grande capacite defensive annule pratiquement les critiques") {
    Generateur generateur = generateurDeTest();

    const int critiques = nombreDeCritiques({.degats = degatsPhysiques(s_montant), .capaciteOffensive = 50, .nature = NatureDAttaque::Contact}, {.armure = 0, .statistiques = {.capaciteDefensive = 100000}}, 4000, generateur);

    REQUIRE(critiques == 0);
}

TEST_CASE("resoudreCoup : meme graine, memes resultats") {
    Generateur premier = generateurDeTest();
    Generateur second = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(s_montant), .capaciteOffensive = 150, .nature = NatureDAttaque::Contact};

    for (int i = 0; i < 100; ++i) {
        const ResultatDuCoup a = resoudreCoup(attaque, defensesDeTest(10), true, premier);
        const ResultatDuCoup b = resoudreCoup(attaque, defensesDeTest(10), true, second);
        REQUIRE(a.montant == b.montant);
        REQUIRE(a.critique == b.critique);
    }
}

TEST_CASE("resoudreSoin : sans capacite offensive, le soin est rendu tel quel et sans tirage") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup soin = resoudreSoin(soinSansHasard(300), generateur);

    REQUIRE(soin.montant == 300);
    REQUIRE_FALSE(soin.critique);
    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreSoin : un critique multiplie le soin, sans armure") {
    Generateur generateur = generateurDeTest();
    bool critiqueVu = false;
    bool normalVu = false;

    for (int i = 0; i < 100; ++i) {
        const ResultatDuCoup soin = resoudreSoin({.degats = degatsPhysiques(0), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact, .soin = 300}, generateur);
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
        critiques += resoudreSoin({.degats = degatsPhysiques(0), .capaciteOffensive = s_capaciteDefensiveDeReference, .nature = NatureDAttaque::Contact, .soin = 300}, generateur).critique ? 1 : 0;
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
    return {.degats = degatsPhysiques(s_montant), .capaciteOffensive = 0, .nature = nature};
}

int nombreDEsquives(const Attaque& attaque, const Defenses& defenses, int coups, Generateur& generateur) {
    int esquives = 0;
    for (int i = 0; i < coups; ++i) {
        esquives += resoudreCoup(attaque, defenses, true, generateur).esquive ? 1 : 0;
    }
    return esquives;
}
}  // namespace

TEST_CASE("resoudreCoup : un coup esquive s'arrete la, sans degats ni critique") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(s_montant), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    bool esquiveVue = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaque, esquiveur(50, 0), true, generateur);
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

    static_cast<void>(resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), esquiveur(0, 0), true, generateur));

    REQUIRE(generateur == intact);
}

namespace {
// Le nombre de nombres que ce coup a tirés dans le générateur.
int tiragesDuCoup(const Attaque& attaque, const Defenses& defenses, Generateur& generateur, bool& esquive) {
    Generateur avant = generateur;
    esquive = resoudreCoup(attaque, defenses, true, generateur).esquive;
    int tirages = 0;
    while (avant != generateur) {
        static_cast<void>(avant());
        ++tirages;
    }
    return tirages;
}
}  // namespace

TEST_CASE("resoudreCoup : une esquive possible tire, et un coup esquive ne tire pas son critique") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(s_montant), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    int esquives = 0;
    int touches = 0;

    // Chaque coup est vérifié ; à 50 %, la chance de ne voir que d'une sorte sur 200 coups est de 2^-199.
    for (int i = 0; i < 200; ++i) {
        bool esquive = false;
        const int tirages = tiragesDuCoup(attaque, esquiveur(50, 0), generateur, esquive);
        // Esquivé : un seul nombre (l'esquive). Touché : deux (l'esquive, puis le critique).
        REQUIRE(tirages == (esquive ? 1 : 2));
        ++(esquive ? esquives : touches);
    }
    REQUIRE(esquives > 0);
    REQUIRE(touches > 0);
}

TEST_CASE("resoudreCoup : l'esquive ne change pas ce que la capacite defensive et l'armure font du coup touche") {
    Generateur generateur = generateurDeTest();

    for (int i = 0; i < 100; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), {.armure = s_armure, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 50}}, true, generateur);
        if (!resultat.esquive) {
            REQUIRE(resultat.montant == appliquerArmure(s_montant, s_armure));
        }
    }
}

namespace {
// Un défenseur sans esquive dont le bouclier bloque toujours (chance 100 %) 40 points.
Defenses porteurDeBouclier(int armure = 0, int chance = 100) {
    Defenses defenses = defensesDeTest(armure);
    defenses.statistiques.chanceDeBlocage = chance;
    defenses.statistiques.montantBloque = 40;
    return defenses;
}
}  // namespace

TEST_CASE("resoudreCoup : un blocage retire un montant fixe, puis l'armure joue sur le reste") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), porteurDeBouclier(s_armure), true, generateur);

    REQUIRE(resultat.bloque == 40);
    REQUIRE(resultat.montant == appliquerArmure(s_montant - 40, s_armure));
    REQUIRE_FALSE(resultat.esquive);
}

TEST_CASE("resoudreCoup : un blocage qui absorbe tout arrete le coup, sans armure ni plancher") {
    Generateur generateur = generateurDeTest();
    const Attaque faible{.degats = degatsPhysiques(25), .capaciteOffensive = 0, .nature = NatureDAttaque::Projectile};

    const ResultatDuCoup resultat = resoudreCoup(faible, porteurDeBouclier(s_armure), true, generateur);

    REQUIRE(resultat.bloque == 25);
    REQUIRE(resultat.montant == 0);  // pas 1 : le plancher de l'armure ne s'applique pas
}

TEST_CASE("resoudreCoup : le blocage retire du montant deja critique, avant l'armure") {
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(s_montant), .capaciteOffensive = s_capaciteEnorme, .nature = NatureDAttaque::Contact};
    bool critiqueVu = false;
    bool normalVu = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaque, porteurDeBouclier(s_armure), true, generateur);
        REQUIRE(resultat.bloque == 40);
        REQUIRE(resultat.montant == appliquerArmure((resultat.critique ? 2 * s_montant : s_montant) - 40, s_armure));
        critiqueVu = critiqueVu || resultat.critique;
        normalVu = normalVu || !resultat.critique;
    }
    REQUIRE(critiqueVu);
    REQUIRE(normalVu);
}

TEST_CASE("resoudreCoup : une zone n'est jamais bloquee") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Zone), porteurDeBouclier(), true, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(resultat.montant == s_montant);
}

TEST_CASE("resoudreCoup : pendant la recuperation, aucun blocage n'est tente ni tire") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), porteurDeBouclier(), false, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(resultat.montant == s_montant);
    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreCoup : sans bouclier, aucun blocage ni tirage") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), defensesDeTest(0), true, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(generateur == intact);
}

TEST_CASE("resoudreCoup : un coup esquive n'est pas bloque") {
    Generateur generateur = generateurDeTest();
    Defenses defenses = porteurDeBouclier();
    defenses.statistiques.esquiveContact = 50;
    bool esquiveVue = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), defenses, true, generateur);
        if (resultat.esquive) {
            REQUIRE(resultat.bloque == 0);
            esquiveVue = true;
        } else {
            REQUIRE(resultat.bloque == 40);
        }
    }
    REQUIRE(esquiveVue);
}

TEST_CASE("resoudreCoup : la frequence des blocages suit la chance du bouclier") {
    Generateur generateur = generateurDeTest();
    int blocages = 0;

    for (int i = 0; i < 4000; ++i) {
        blocages += resoudreCoup(attaqueDeNature(NatureDAttaque::Contact), porteurDeBouclier(0, 30), true, generateur).bloque > 0 ? 1 : 0;
    }

    REQUIRE(blocages > 1100);  // 30 % de 4 000 : 1 200 attendus
    REQUIRE(blocages < 1300);
}

TEST_CASE("resoudreCoup : la penetration de l'attaque reduit l'armure du defenseur, une attaque sans penetration non") {
    Generateur generateur = generateurDeTest();
    Attaque perforante = attaqueSansHasard(100);
    perforante.penetration = 30;

    REQUIRE(resoudreCoup(perforante, defensesDeTest(40), true, generateur).montant == appliquerArmure(100, 40, 30));
    REQUIRE(resoudreCoup(attaqueSansHasard(100), defensesDeTest(40), true, generateur).montant == appliquerArmure(100, 40));
    REQUIRE(appliquerArmure(100, 40, 30) > appliquerArmure(100, 40));
}

TEST_CASE("resoudreCoup : un blocage total ne laisse rien a percer, la penetration n'y change rien") {
    Generateur generateur = generateurDeTest();
    Attaque perforante{.degats = degatsPhysiques(10), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact, .penetration = 75};
    Defenses defenses = defensesDeTest(1000);
    defenses.statistiques.chanceDeBlocage = 100;
    defenses.statistiques.montantBloque = 50;

    const ResultatDuCoup resultat = resoudreCoup(perforante, defenses, true, generateur);

    REQUIRE(resultat.montant == 0);
    REQUIRE(resultat.bloque == 10);
}
