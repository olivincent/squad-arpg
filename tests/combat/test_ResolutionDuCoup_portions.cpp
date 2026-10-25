// ./tests/combat/test_ResolutionDuCoup_portions.cpp
// Le coup en portions par élément (étape 94) : critique, blocage, armure et résistances, chacun sur les bonnes portions.
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/Defense.hpp"
#include "combat/Degats.hpp"
#include "combat/Element.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/ResolutionDuCoup.hpp"
#include "combat/Vitalite.hpp"

namespace {
// Une capacité offensive assez grande pour que le critique soit au plafond : 50 % de chance, ×2.
constexpr int s_capaciteEnorme = 100000;
constexpr int s_armure = 30;

// Des dégâts de ces trois portions.
Degats portions(int physique, int feu, int sacre = 0) {
    Degats degats;
    degats[Element::Physique] = physique;
    degats[Element::Feu] = feu;
    degats[Element::Sacre] = sacre;
    return degats;
}

Attaque attaqueDe(const Degats& degats, NatureDAttaque nature = NatureDAttaque::Contact, int capaciteOffensive = 0) {
    return {.degats = degats, .capaciteOffensive = capaciteOffensive, .nature = nature};
}

// Un défenseur sans esquive, de cette armure.
Defenses defenseur(int armure = 0) {
    return defensesDeTest(armure);
}

Defenses avecResistance(Defenses defenses, Element element, int pourCent) {
    defenses.statistiques.resistances[element] = pourCent;
    return defenses;
}

// Un défenseur dont le bouclier bloque toujours (chance 100 %) 40 points.
Defenses avecBouclier(int armure = 0) {
    Defenses defenses = defensesDeTest(armure);
    defenses.statistiques.chanceDeBlocage = 100;
    defenses.statistiques.montantBloque = 40;
    return defenses;
}
}  // namespace

TEST_CASE("portions : un coup mixte subit l'armure sur le physique et la resistance sur chaque element") {
    Generateur generateur = generateurDeTest();
    const Defenses defenses = avecResistance(defenseur(s_armure), Element::Feu, 50);

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(100, 60)), defenses, true, generateur);

    REQUIRE(resultat.portions[Element::Physique] == 70);  // 100 − 30
    REQUIRE(resultat.portions[Element::Feu] == 30);       // 60 × 50 %, l'armure n'y touche pas
    REQUIRE(resultat.portions[Element::Sacre] == 0);
    REQUIRE(resultat.montant == 100);
    REQUIRE(resultat.montant == resultat.portions.total());
}

TEST_CASE("portions : l'armure ne touche pas une portion elementaire, ni la resistance une portion sans element") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup feu = resoudreCoup(attaqueDe(degatsDe(Element::Feu, 100)), defenseur(1000), true, generateur);
    const ResultatDuCoup physique = resoudreCoup(attaqueDe(degatsDe(Element::Physique, 100)),
                                                 avecResistance(defenseur(), Element::Feu, 80), true, generateur);

    REQUIRE(feu.montant == 100);
    REQUIRE(physique.montant == 100);
}

TEST_CASE("portions : la resistance physique s'applique comme les autres, apres l'armure") {
    Generateur generateur = generateurDeTest();
    const Defenses defenses = avecResistance(defenseur(s_armure), Element::Physique, 10);

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(degatsDe(Element::Physique, 100)), defenses, true, generateur);

    REQUIRE(resultat.montant == 63);  // (100 − 30) × 90 %
}

TEST_CASE("portions : le critique multiplie toutes les portions") {
    Generateur generateur = generateurDeTest();
    const Defenses defenses = avecResistance(defenseur(s_armure), Element::Feu, 50);
    const Attaque attaque = attaqueDe(portions(100, 60), NatureDAttaque::Contact, s_capaciteEnorme);
    bool critiqueVu = false;
    bool normalVu = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaque, defenses, true, generateur);
        if (resultat.critique) {
            REQUIRE(resultat.portions[Element::Physique] == 170);  // 100 × 2 − 30
            REQUIRE(resultat.portions[Element::Feu] == 60);        // 60 × 2 × 50 %
            critiqueVu = true;
        } else {
            REQUIRE(resultat.portions[Element::Physique] == 70);
            REQUIRE(resultat.portions[Element::Feu] == 30);
            normalVu = true;
        }
    }
    REQUIRE(critiqueVu);
    REQUIRE(normalVu);
}

TEST_CASE("portions : le blocage ne retire que du physique, les portions elementaires passent") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(100, 60)), avecBouclier(s_armure), true, generateur);

    REQUIRE(resultat.bloque == 40);
    REQUIRE(resultat.portions[Element::Physique] == 30);  // (100 − 40) − 30
    REQUIRE(resultat.portions[Element::Feu] == 60);
    REQUIRE(resultat.montant == 90);
}

TEST_CASE("portions : une portion physique entierement bloquee ne subit ni armure ni plancher, le reste passe") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(25, 60)), avecBouclier(1000), true, generateur);

    REQUIRE(resultat.bloque == 25);
    REQUIRE(resultat.portions[Element::Physique] == 0);
    REQUIRE(resultat.portions[Element::Feu] == 60);
    REQUIRE(resultat.montant == 60);
}

TEST_CASE("portions : un projectile purement elementaire ne tente aucun blocage et ne lance aucune recuperation") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;
    const Attaque boule = attaqueDe(degatsDe(Element::Feu, 80), NatureDAttaque::Projectile);

    const ResultatDuCoup resultat = resoudreCoup(boule, avecBouclier(), true, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(resultat.montant == 80);
    REQUIRE(generateur == intact);  // aucun tirage : ni blocage, ni critique (capacité offensive nulle)

    Vitalite vitalite(1000, avecBouclier());
    static_cast<void>(vitalite.recevoirDegats(boule, generateur));
    REQUIRE(vitalite.blocagePret());  // la récupération du blocage n'a pas été lancée
}

TEST_CASE("portions : Bloque n'est total que si tout le coup est absorbe") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup toutAbsorbe = resoudreCoup(attaqueDe(portions(25, 0)), avecBouclier(), true, generateur);
    const ResultatDuCoup partiel = resoudreCoup(attaqueDe(portions(25, 10)), avecBouclier(), true, generateur);

    REQUIRE(toutAbsorbe.montant == 0);
    REQUIRE(std::get<Bloque>(detailsDuCoup(toutAbsorbe, false).back()).total);
    REQUIRE(partiel.montant == 10);
    REQUIRE(partiel.bloque == 25);
    REQUIRE_FALSE(std::get<Bloque>(detailsDuCoup(partiel, false).back()).total);
}

TEST_CASE("portions : la resistance est plafonnee a 80 %, une vulnerabilite augmente la portion") {
    Generateur generateur = generateurDeTest();
    const Attaque feu = attaqueDe(degatsDe(Element::Feu, 100));

    REQUIRE(resoudreCoup(feu, avecResistance(defenseur(), Element::Feu, 80), true, generateur).montant == 20);
    REQUIRE(resoudreCoup(feu, avecResistance(defenseur(), Element::Feu, 95), true, generateur).montant == 20);  // plafonnée
    REQUIRE(resoudreCoup(feu, avecResistance(defenseur(), Element::Feu, -50), true, generateur).montant == 150);
    REQUIRE(resoudreCoup(feu, avecResistance(defenseur(), Element::Feu, -100), true, generateur).montant == 200);
    REQUIRE(resoudreCoup(feu, avecResistance(defenseur(), Element::Feu, -300), true, generateur).montant == 200);  // ramenée à −100
}

TEST_CASE("portions : un seul arrondi par portion, la somme des portions arrondies fait le total") {
    Generateur generateur = generateurDeTest();
    // 3 × 50 % = 1,5 pour chaque portion : chacune arrondit à 2, soit 4 ; arrondir la somme (3) donnerait 3.
    Defenses defenses = avecResistance(defenseur(), Element::Feu, 50);
    defenses.statistiques.resistances[Element::Sacre] = 50;

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(0, 3, 3)), defenses, true, generateur);

    REQUIRE(resultat.portions[Element::Feu] == 2);
    REQUIRE(resultat.portions[Element::Sacre] == 2);
    REQUIRE(resultat.montant == 4);
}

TEST_CASE("portions : un coup ni esquive ni entierement bloque inflige au moins 1, dans la plus grosse portion") {
    Generateur generateur = generateurDeTest();
    // 1 × 40 % = 0,4 et 1 × 40 % = 0,4 : tout arrondit à 0.
    Defenses defenses = avecResistance(defenseur(), Element::Feu, 60);
    defenses.statistiques.resistances[Element::Sacre] = 60;

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(0, 1, 1)), defenses, true, generateur);

    REQUIRE(resultat.montant == 1);
    REQUIRE(resultat.portions[Element::Feu] == 1);  // égalité : le premier dans l'ordre de l'énumération
    REQUIRE(resultat.portions[Element::Sacre] == 0);
}

TEST_CASE("portions : le minimum de 1 vaut aussi pour une portion physique ecrasee par une grosse armure") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(degatsDe(Element::Physique, 2)), defenseur(1000), true, generateur);

    REQUIRE(resultat.montant == 1);  // 2 × 20 % = 0,4
}

TEST_CASE("portions : la penetration ne touche que l'armure, jamais la resistance") {
    Generateur generateur = generateurDeTest();
    Attaque attaque = attaqueDe(degatsDe(Element::Feu, 100));
    attaque.penetration = 75;

    const ResultatDuCoup resultat = resoudreCoup(attaque, avecResistance(defenseur(1000), Element::Feu, 50), true, generateur);

    REQUIRE(resultat.montant == 50);
}

TEST_CASE("portions : un coup esquive n'a aucune portion") {
    Generateur generateur = generateurDeTest();
    Defenses defenses = defenseur();
    defenses.statistiques.esquiveContact = 50;
    bool esquiveVue = false;

    for (int i = 0; i < 200; ++i) {
        const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(10, 10)), defenses, true, generateur);
        if (resultat.esquive) {
            REQUIRE(resultat.montant == 0);
            REQUIRE(resultat.portions.total() == 0);
            esquiveVue = true;
        }
    }
    REQUIRE(esquiveVue);
}

TEST_CASE("portions : un coup sans aucune portion ne fait rien et ne tire rien") {
    Generateur generateur = generateurDeTest();
    const Generateur intact = generateur;

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(Degats{}, NatureDAttaque::Contact, s_capaciteEnorme), avecBouclier(), true, generateur);

    REQUIRE(resultat.montant == 0);
    REQUIRE(generateur == intact);
}
