// ./tests/combat/test_Absorption.cpp
// L'absorption dans le coup et dans les ticks (étape 114.3) : Aegis (le physique) et Ward (le non-physique) s'appliquent EN DERNIER,
// portion par portion, et se consomment chez `Vitalite`.
#include <catch2/catch_test_macros.hpp>

#include <variant>
#include <vector>

#include "aides.hpp"
#include "base/Aleatoire.hpp"
#include "combat/Attaque.hpp"
#include "combat/Defense.hpp"
#include "combat/Degats.hpp"
#include "combat/Element.hpp"
#include "combat/EvenementDeCombat.hpp"
#include "combat/FamilleDAbsorption.hpp"
#include "combat/ResolutionDuCoup.hpp"
#include "combat/Vitalite.hpp"

namespace {
// Des défenses de test (sans esquive) qui portent ces points d'absorption.
Defenses avecAbsorption(int aegis, int ward, int armure = 0) {
    Defenses defenses = defensesDeTest(armure);
    defenses.absorption = {.aegis = aegis, .ward = ward};
    return defenses;
}

Attaque attaqueDe(const Degats& degats) {
    return {.degats = degats, .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
}

Degats portions(int physique, int feu, int sacre = 0) {
    Degats degats;
    degats[Element::Physique] = physique;
    degats[Element::Feu] = feu;
    degats[Element::Sacre] = sacre;
    return degats;
}

EffetActif bouclier(SourceDEffet source, FamilleDAbsorption famille, int points, float duree = 8.0f) {
    return {.source = source, .dureeRestante = duree, .sorte = Absorption{.famille = famille, .points = points}};
}

EffetActif brulure(int montant, float duree = 4.0f) {
    return {.source = IdSkill::Explosion, .rang = 1, .dureeRestante = duree, .sorte = DegatsDansLeTemps{.element = Element::Feu, .montant = montant}};
}

EffetActif saignement(int montant, float duree = 4.0f) {
    return {.source = IdSkill::TirPercant, .rang = 1, .dureeRestante = duree, .sorte = DegatsDansLeTemps{.element = Element::Physique, .montant = montant}};
}
}  // namespace

// --- resoudreCoup ------------------------------------------------------------------------------------------------------------------

TEST_CASE("absorption coup : l'Aegis absorbe la portion physique, le reste passe") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(degatsPhysiques(40)), avecAbsorption(25, 0), true, generateur);

    REQUIRE(resultat.montant == 15);
    REQUIRE(resultat.absorbees[Element::Physique] == 25);
    REQUIRE(resultat.portions[Element::Physique] == 15);
}

TEST_CASE("absorption coup : un Aegis plus grand que le coup l'absorbe entierement") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(degatsPhysiques(40)), avecAbsorption(100, 0), true, generateur);

    REQUIRE(resultat.montant == 0);
    REQUIRE(resultat.absorbees.total() == 40);
    REQUIRE_FALSE(resultat.esquive);
}

TEST_CASE("absorption coup : le Ward absorbe le feu et le sacre, l'Aegis ne les absorbe pas") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup avecWard = resoudreCoup(attaqueDe(portions(0, 30, 10)), avecAbsorption(0, 35), true, generateur);
    const ResultatDuCoup avecAegis = resoudreCoup(attaqueDe(portions(0, 30, 10)), avecAbsorption(500, 0), true, generateur);

    REQUIRE(avecWard.montant == 5);
    REQUIRE(avecWard.absorbees[Element::Feu] == 30);
    REQUIRE(avecWard.absorbees[Element::Sacre] == 5);
    REQUIRE(avecAegis.montant == 40);
    REQUIRE(avecAegis.absorbees.total() == 0);
}

TEST_CASE("absorption coup : le Ward n'absorbe pas le physique") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(degatsPhysiques(40)), avecAbsorption(0, 500), true, generateur);

    REQUIRE(resultat.montant == 40);
    REQUIRE(resultat.absorbees.total() == 0);
}

TEST_CASE("absorption coup : un coup mixte est absorbe portion par portion, chaque famille sur la sienne") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(30, 20)), avecAbsorption(10, 15), true, generateur);

    REQUIRE(resultat.absorbees[Element::Physique] == 10);
    REQUIRE(resultat.absorbees[Element::Feu] == 15);
    REQUIRE(resultat.portions[Element::Physique] == 20);
    REQUIRE(resultat.portions[Element::Feu] == 5);
    REQUIRE(resultat.montant == 25);
}

TEST_CASE("absorption coup : s'applique apres l'armure, le bouclier absorbe ce qui aurait ete subi") {
    Generateur generateur = generateurDeTest();
    const ResultatDuCoup sansBouclier = resoudreCoup(attaqueDe(degatsPhysiques(40)), defensesDeTest(15), true, generateur);
    REQUIRE(sansBouclier.montant < 40);  // l'armure réduit bien le coup

    const ResultatDuCoup avecBouclier = resoudreCoup(attaqueDe(degatsPhysiques(40)), avecAbsorption(1000, 0, 15), true, generateur);

    REQUIRE(avecBouclier.absorbees.total() == sansBouclier.montant);  // l'Aegis n'a absorbé que le coup réduit par l'armure
    REQUIRE(avecBouclier.montant == 0);
}

TEST_CASE("absorption coup : s'applique apres la resistance") {
    Generateur generateur = generateurDeTest();
    Defenses defenses = avecAbsorption(0, 1000);
    defenses.statistiques.resistances[Element::Feu] = 50;

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(0, 40)), defenses, true, generateur);

    REQUIRE(resultat.absorbees[Element::Feu] == 20);  // 40 de feu à 50 % de résistance : 20 arrivent au Ward
    REQUIRE(resultat.montant == 0);
}

TEST_CASE("absorption coup : sans bouclier, rien n'est absorbe") {
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = resoudreCoup(attaqueDe(portions(30, 20)), defensesDeTest(0), true, generateur);

    REQUIRE(resultat.absorbees.total() == 0);
    REQUIRE(resultat.montant == 50);
}

// --- resoudreTick ------------------------------------------------------------------------------------------------------------------

TEST_CASE("absorption tick : un tick de feu va au Ward, un tick physique a l'Aegis") {
    const ResultatDuTick feu = resoudreTick(Element::Feu, 14, avecAbsorption(100, 10));
    const ResultatDuTick physique = resoudreTick(Element::Physique, 14, avecAbsorption(6, 100));

    REQUIRE(feu.absorbe == 10);
    REQUIRE(feu.montant == 4);
    REQUIRE(physique.absorbe == 6);
    REQUIRE(physique.montant == 8);
}

TEST_CASE("absorption tick : l'Aegis n'absorbe pas un tick de feu, le Ward pas un tick physique") {
    REQUIRE(resoudreTick(Element::Feu, 14, avecAbsorption(100, 0)).absorbe == 0);
    REQUIRE(resoudreTick(Element::Physique, 14, avecAbsorption(0, 100)).absorbe == 0);
}

TEST_CASE("absorption tick : s'applique apres la resistance") {
    Defenses defenses = avecAbsorption(0, 6);
    defenses.statistiques.resistances[Element::Feu] = 50;

    const ResultatDuTick resultat = resoudreTick(Element::Feu, 20, defenses);  // 20 → 10 après résistance

    REQUIRE(resultat.absorbe == 6);
    REQUIRE(resultat.montant == 4);
}

TEST_CASE("absorption tick : un bouclier assez grand absorbe tout le tick") {
    const ResultatDuTick resultat = resoudreTick(Element::Feu, 14, avecAbsorption(0, 100));

    REQUIRE(resultat.montant == 0);
    REQUIRE(resultat.absorbe == 14);
}

// --- Vitalite ----------------------------------------------------------------------------------------------------------------------

TEST_CASE("absorption vitalite : un coup physique use l'Aegis avant les points de vie") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 25));
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = vitalite.recevoirDegats(attaqueDe(degatsPhysiques(40)), generateur);

    REQUIRE(resultat.montant == 15);
    REQUIRE(vitalite.getValeur() == 85);
    REQUIRE(vitalite.getEffets().liste().empty());  // l'Aegis est épuisé : il disparaît
}

TEST_CASE("absorption vitalite : un coup plus petit que l'Aegis ne touche pas les points de vie et laisse le reste") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 60));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(attaqueDe(degatsPhysiques(40)), generateur));

    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.getEffets().pointsDAbsorption().aegis == 20);
}

TEST_CASE("absorption vitalite : le Ward ne retient pas un coup physique") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Ward, 60));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(attaqueDe(degatsPhysiques(40)), generateur));

    REQUIRE(vitalite.getValeur() == 60);
    REQUIRE(vitalite.getEffets().pointsDAbsorption().ward == 60);
}

TEST_CASE("absorption vitalite : un coup mixte use chaque famille sur sa portion") {
    Vitalite vitalite(200, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 10));
    vitalite.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Ward, 50));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(attaqueDe(portions(30, 20)), generateur));

    REQUIRE(vitalite.getValeur() == 180);  // 20 de physique passent, tout le feu est absorbé
    REQUIRE(vitalite.getEffets().pointsDAbsorption() == PointsDAbsorption{.aegis = 0, .ward = 30});
}

TEST_CASE("absorption vitalite : deux Aegis, celui qui expire le premier paie d'abord") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 40, 9.0f));
    vitalite.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Aegis, 40, 3.0f));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(attaqueDe(degatsPhysiques(50)), generateur));

    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.getEffets().liste().size() == 1);  // celui de 3 s est épuisé, celui de 9 s a payé 10
    REQUIRE(std::get<Absorption>(vitalite.getEffets().liste().front().sorte).points == 30);
}

TEST_CASE("absorption vitalite : un coup esquive ne use pas le bouclier") {
    Defenses defenses = defensesDeTest(0);
    defenses.statistiques.esquiveContact = 100;
    Vitalite vitalite(100, defenses);
    vitalite.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 1000));
    Generateur generateur = generateurDeTest();

    // Esquive plafonnée à 50 % : on frappe jusqu'à une esquive (200 coups de 1 point ne vident pas 1000 points).
    bool esquive = false;
    for (int i = 0; i < 200 && !esquive; ++i) {
        const int avant = vitalite.getEffets().pointsDAbsorption().aegis;
        esquive = vitalite.recevoirDegats(attaqueDe(degatsPhysiques(1)), generateur).esquive;
        if (esquive) {
            REQUIRE(vitalite.getEffets().pointsDAbsorption().aegis == avant);
        }
    }
    REQUIRE(esquive);
}

TEST_CASE("absorption vitalite : le Ward absorbe la brulure, le tick absorbe est rendu avec son montant absorbe") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Ward, 5));
    vitalite.poserEffet(brulure(8));

    vitalite.mettreAJour(1.0f);

    REQUIRE(vitalite.getValeur() == 97);  // 8 de feu, 5 absorbés
    const std::vector<Tick> ticks = vitalite.prendreLesTicks();
    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 3);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).absorbe == 5);
    REQUIRE(vitalite.getEffets().pointsDAbsorption().ward == 0);
}

TEST_CASE("absorption vitalite : un tick entierement absorbe est rendu quand meme, avec montant 0") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Ward, 50));
    vitalite.poserEffet(brulure(8));

    vitalite.mettreAJour(1.0f);

    REQUIRE(vitalite.getValeur() == 100);
    const std::vector<Tick> ticks = vitalite.prendreLesTicks();
    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 0);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).absorbe == 8);
    REQUIRE(vitalite.getEffets().pointsDAbsorption().ward == 42);
}

TEST_CASE("absorption vitalite : le Ward laisse passer le saignement, l'Aegis l'absorbe") {
    Vitalite avecWard(100, defensesDeTest(0));
    avecWard.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Ward, 50));
    avecWard.poserEffet(saignement(8));
    Vitalite avecAegis(100, defensesDeTest(0));
    avecAegis.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 50));
    avecAegis.poserEffet(saignement(8));

    avecWard.mettreAJour(1.0f);
    avecAegis.mettreAJour(1.0f);

    REQUIRE(avecWard.getValeur() == 92);
    REQUIRE(avecAegis.getValeur() == 100);
}

TEST_CASE("absorption vitalite : les soins ne sont pas touches") {
    Vitalite vitalite(100, defensesDeTest(0));
    Generateur generateur = generateurDeTest();
    static_cast<void>(vitalite.recevoirDegats(attaqueDe(degatsPhysiques(40)), generateur));
    vitalite.poserEffet(bouclier(IdSkill::Soin, FamilleDAbsorption::Aegis, 50));

    REQUIRE(vitalite.soigner(30) == 30);

    REQUIRE(vitalite.getValeur() == 90);
    REQUIRE(vitalite.getEffets().pointsDAbsorption().aegis == 50);
}

TEST_CASE("absorption vitalite : la mort retire les boucliers restants") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(bouclier(IdSkill::Benediction, FamilleDAbsorption::Ward, 50));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(attaqueDe(degatsPhysiques(s_coupMortel)), generateur));

    REQUIRE_FALSE(vitalite.estVivant());
    REQUIRE(vitalite.getEffets().liste().empty());
}

// --- événements --------------------------------------------------------------------------------------------------------------------

TEST_CASE("detailsDuCoup : un coup partiellement absorbe donne les degats puis l'absorption de chaque famille") {
    ResultatDuCoup resultat;
    resultat.montant = 12;
    resultat.portions = portions(12, 0);
    resultat.absorbees = portions(25, 18);

    const std::vector<DetailEvenement> details = detailsDuCoup(resultat, false);

    REQUIRE(details.size() == 3);
    REQUIRE(std::get<DegatsSubis>(details[0]).montant == 12);
    const Absorbe aegis = std::get<Absorbe>(details[1]);
    const Absorbe ward = std::get<Absorbe>(details[2]);
    REQUIRE(aegis.famille == FamilleDAbsorption::Aegis);
    REQUIRE(aegis.montant == 25);
    REQUIRE_FALSE(aegis.total);
    REQUIRE(ward.famille == FamilleDAbsorption::Ward);
    REQUIRE(ward.montant == 18);
}

TEST_CASE("detailsDuCoup : un coup entierement absorbe ne donne que l'absorption, marquee totale") {
    ResultatDuCoup resultat;
    resultat.absorbees = degatsPhysiques(40);

    const std::vector<DetailEvenement> details = detailsDuCoup(resultat, false);

    REQUIRE(details.size() == 1);
    const Absorbe aegis = std::get<Absorbe>(details[0]);
    REQUIRE(aegis.montant == 40);
    REQUIRE(aegis.total);
}

TEST_CASE("detailsDuCoup : un coup esquive n'a aucune absorption") {
    ResultatDuCoup resultat;
    resultat.esquive = true;

    const std::vector<DetailEvenement> details = detailsDuCoup(resultat, false);

    REQUIRE(details.size() == 1);
    REQUIRE(std::holds_alternative<Esquive>(details[0]));
}
