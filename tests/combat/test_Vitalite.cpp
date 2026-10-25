// ./tests/combat/test_Vitalite.cpp
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "combat/Vitalite.hpp"

TEST_CASE("Vitalite commence pleine") {
    const Vitalite vitalite(100, defensesDeTest(0));

    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.getMaximum() == 100);
    REQUIRE(vitalite.getRatio() == 1.0f);
    REQUIRE(vitalite.estVivant());
}

TEST_CASE("Vitalite sans armure retire les degats entiers") {
    Vitalite vitalite(100, defensesDeTest(0));

    REQUIRE(subir(vitalite, 30) == 30);
    REQUIRE(vitalite.getValeur() == 70);
}

TEST_CASE("Vitalite definirArmure remplace l'armure sans toucher aux points de vie") {
    Vitalite vitalite(100, defensesDeTest(0));
    REQUIRE(subir(vitalite, 30) == 30);

    vitalite.definirDefenses(defensesDeTest(20));  // armure 20 : 20 dégâts de moins par coup

    REQUIRE(vitalite.getValeur() == 70);
    REQUIRE(subir(vitalite, 30) == 10);

    vitalite.definirDefenses(defensesDeTest(0));
    REQUIRE(subir(vitalite, 30) == 30);
}

TEST_CASE("Vitalite applique l'armure avant de retirer les points de vie") {
    Vitalite vitalite(100, defensesDeTest(20));  // armure 20 : 20 dégâts de moins par coup

    REQUIRE(subir(vitalite, 30) == 10);
    REQUIRE(vitalite.getValeur() == 90);
}

TEST_CASE("Vitalite renvoie les degats apres armure sans les plafonner a la vie restante") {
    Vitalite vitalite(100, defensesDeTest(0));

    REQUIRE(subir(vitalite, 90) == 90);
    REQUIRE(subir(vitalite, 50) == 50);
    REQUIRE(vitalite.getValeur() == 0);
    REQUIRE_FALSE(vitalite.estVivant());
}

TEST_CASE("Vitalite : un mort ne recoit plus de degats") {
    Vitalite vitalite(100, defensesDeTest(0));
    REQUIRE(subir(vitalite, 100) == 100);

    REQUIRE(subir(vitalite, 10) == 0);
    REQUIRE(vitalite.getValeur() == 0);
}

TEST_CASE("Vitalite : un coup de 0 ne fait rien") {
    Vitalite vitalite(100, defensesDeTest(50));

    REQUIRE(subir(vitalite, 0) == 0);
    REQUIRE(vitalite.getValeur() == 100);
}

TEST_CASE("Vitalite soigne jusqu'au maximum et renvoie les points rendus") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 30));

    REQUIRE(vitalite.soigner(10) == 10);
    REQUIRE(vitalite.soigner(50) == 20);
    REQUIRE(vitalite.getValeur() == 100);
}

TEST_CASE("Vitalite : un mort ne se soigne pas") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 100));

    REQUIRE(vitalite.soigner(50) == 0);
    REQUIRE_FALSE(vitalite.estVivant());
}

TEST_CASE("Vitalite getRatio suit les points de vie") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 25));

    REQUIRE(vitalite.getRatio() == 0.75f);
}

TEST_CASE("Vitalite : un critique retire le montant multiplie") {
    Vitalite vitalite(100000, defensesDeTest(0));
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.montant = 100, .capaciteOffensive = 100000, .nature = NatureDAttaque::Contact};  // 50 % de chance, ×2
    int critiques = 0;

    // Chaque coup est vérifié ; à 50 %, la chance de n'en voir aucun critique sur 100 est de 2^-100.
    for (int i = 0; i < 100; ++i) {
        const int avant = vitalite.getValeur();
        const ResultatDuCoup resultat = vitalite.recevoirDegats(attaque, generateur);
        REQUIRE(avant - vitalite.getValeur() == resultat.montant);
        REQUIRE(resultat.montant == (resultat.critique ? 200 : 100));
        critiques += resultat.critique ? 1 : 0;
    }
    REQUIRE(critiques > 0);
}

TEST_CASE("Vitalite : un mort ne tire rien") {
    Vitalite vitalite(100, defensesDeTest(0));
    REQUIRE(subir(vitalite, 100) == 100);
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup resultat = vitalite.recevoirDegats({.montant = 50, .capaciteOffensive = 100000, .nature = NatureDAttaque::Contact}, generateur);

    REQUIRE(resultat.montant == 0);
    REQUIRE_FALSE(resultat.critique);
    REQUIRE(generateur == intact);
}

TEST_CASE("Vitalite : la capacite defensive des defenses compte pour le critique") {
    Vitalite fragile(100000, {.armure = 0, .statistiques = {.capaciteDefensive = 1}});
    Vitalite robuste(100000, {.armure = 0, .statistiques = {.capaciteDefensive = 100000}});
    Generateur generateur = generateurDeTest();
    int critiquesDuFragile = 0;
    int critiquesDuRobuste = 0;

    for (int i = 0; i < 300; ++i) {
        critiquesDuFragile += fragile.recevoirDegats({.montant = 10, .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, generateur).critique ? 1 : 0;
        critiquesDuRobuste += robuste.recevoirDegats({.montant = 10, .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, generateur).critique ? 1 : 0;
    }

    REQUIRE(critiquesDuFragile > 100);  // rapport 100 : 50 %
    REQUIRE(critiquesDuRobuste == 0);
}

TEST_CASE("Vitalite : un coup esquive ne retire aucun point de vie") {
    Vitalite vitalite(1000, {.armure = 0, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 50}});
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.montant = 100, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
    bool esquiveVue = false;

    for (int i = 0; i < 100; ++i) {
        const int avant = vitalite.getValeur();
        const ResultatDuCoup resultat = vitalite.recevoirDegats(attaque, generateur);
        if (resultat.esquive) {
            REQUIRE(resultat.montant == 0);
            REQUIRE(vitalite.getValeur() == avant);
            esquiveVue = true;
        } else {
            REQUIRE(avant - vitalite.getValeur() == resultat.montant);
        }
    }
    REQUIRE(esquiveVue);
}

namespace {
// Une vitalité de 1000 points dont le bouclier bloque toujours (100 %) 40 points, puis 3 s de récupération.
Vitalite vitaliteAuBouclier() {
    return Vitalite(1000, {.armure = 0, .statistiques = {.capaciteDefensive = 100, .chanceDeBlocage = 100, .montantBloque = 40,
                                                         .recuperationDuBlocage = 3.0f}});
}

const Attaque s_coupDeContact{.montant = 100, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
}  // namespace

TEST_CASE("Vitalite : un blocage retire le montant bloque et lance la recuperation") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();
    REQUIRE(vitalite.blocagePret());

    const ResultatDuCoup resultat = vitalite.recevoirDegats(s_coupDeContact, generateur);

    REQUIRE(resultat.bloque == 40);
    REQUIRE(resultat.montant == 60);
    REQUIRE(vitalite.getValeur() == 1000 - 60);
    REQUIRE_FALSE(vitalite.blocagePret());
}

TEST_CASE("Vitalite : pendant la recuperation le coup passe entier, et sans tirer de blocage") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();
    static_cast<void>(vitalite.recevoirDegats(s_coupDeContact, generateur));
    const Generateur avant = generateur;

    const ResultatDuCoup resultat = vitalite.recevoirDegats(s_coupDeContact, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(resultat.montant == 100);
    REQUIRE(generateur == avant);
}

TEST_CASE("Vitalite : la recuperation s'ecoule avec le temps, puis le bouclier bloque de nouveau") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();
    static_cast<void>(vitalite.recevoirDegats(s_coupDeContact, generateur));

    vitalite.mettreAJour(2.0f);
    REQUIRE_FALSE(vitalite.blocagePret());
    REQUIRE(vitalite.recevoirDegats(s_coupDeContact, generateur).bloque == 0);

    vitalite.mettreAJour(0.9f);
    REQUIRE_FALSE(vitalite.blocagePret());  // 2,9 s sur 3

    vitalite.mettreAJour(0.2f);
    REQUIRE(vitalite.blocagePret());
    REQUIRE(vitalite.recevoirDegats(s_coupDeContact, generateur).bloque == 40);
}

TEST_CASE("Vitalite : un coup non bloque (zone) ne lance pas la recuperation") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = vitalite.recevoirDegats({.montant = 100, .capaciteOffensive = 0, .nature = NatureDAttaque::Zone}, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(vitalite.blocagePret());
}

TEST_CASE("Vitalite : un blocage total ne retire aucun point de vie") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = vitalite.recevoirDegats({.montant = 30, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact}, generateur);

    REQUIRE(resultat.bloque == 30);
    REQUIRE(resultat.montant == 0);
    REQUIRE(vitalite.getValeur() == 1000);
}

TEST_CASE("Vitalite : sans bouclier, le blocage est toujours pret") {
    Vitalite vitalite(100, defensesDeTest(0));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(s_coupDeContact, generateur));

    REQUIRE(vitalite.blocagePret());
}
