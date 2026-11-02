// ./tests/combat/test_Vitalite.cpp
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "combat/Blocage.hpp"
#include "combat/Vitalite.hpp"

using Catch::Approx;

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
    const Attaque attaque{.degats = degatsPhysiques(100), .capaciteOffensive = 100000, .nature = NatureDAttaque::Contact};  // 50 % de chance, ×2
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

    const ResultatDuCoup resultat = vitalite.recevoirDegats({.degats = degatsPhysiques(50), .capaciteOffensive = 100000, .nature = NatureDAttaque::Contact}, generateur);

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
        critiquesDuFragile += fragile.recevoirDegats({.degats = degatsPhysiques(10), .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, generateur).critique ? 1 : 0;
        critiquesDuRobuste += robuste.recevoirDegats({.degats = degatsPhysiques(10), .capaciteOffensive = 100, .nature = NatureDAttaque::Contact}, generateur).critique ? 1 : 0;
    }

    REQUIRE(critiquesDuFragile > 100);  // rapport 100 : 50 %
    REQUIRE(critiquesDuRobuste == 0);
}

TEST_CASE("Vitalite : un coup esquive ne retire aucun point de vie") {
    Vitalite vitalite(1000, {.armure = 0, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 50}});
    Generateur generateur = generateurDeTest();
    const Attaque attaque{.degats = degatsPhysiques(100), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
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
// Une vitalité de 1000 points dont le bouclier bloque toujours (100 %) 40 points, puis `s_recuperationDuBlocage`
// (3 s, la même pour tous les boucliers) de récupération.
Vitalite vitaliteAuBouclier() {
    return Vitalite(1000, {.armure = 0, .statistiques = {.capaciteDefensive = 100, .chanceDeBlocage = 100, .montantBloque = 40}});
}

const Attaque s_coupDeContact{.degats = degatsPhysiques(100), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact};
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

    vitalite.mettreAJour(s_recuperationDuBlocage - 1.0f);
    REQUIRE_FALSE(vitalite.blocagePret());
    REQUIRE(vitalite.recevoirDegats(s_coupDeContact, generateur).bloque == 0);

    vitalite.mettreAJour(0.9f);
    REQUIRE_FALSE(vitalite.blocagePret());  // 0,1 s avant la fin

    vitalite.mettreAJour(0.2f);
    REQUIRE(vitalite.blocagePret());
    REQUIRE(vitalite.recevoirDegats(s_coupDeContact, generateur).bloque == 40);
}

TEST_CASE("Vitalite : un coup non bloque (zone) ne lance pas la recuperation") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = vitalite.recevoirDegats({.degats = degatsPhysiques(100), .capaciteOffensive = 0, .nature = NatureDAttaque::Zone}, generateur);

    REQUIRE(resultat.bloque == 0);
    REQUIRE(vitalite.blocagePret());
}

TEST_CASE("Vitalite : un blocage total ne retire aucun point de vie") {
    Vitalite vitalite = vitaliteAuBouclier();
    Generateur generateur = generateurDeTest();

    const ResultatDuCoup resultat = vitalite.recevoirDegats({.degats = degatsPhysiques(30), .capaciteOffensive = 0, .nature = NatureDAttaque::Contact}, generateur);

    REQUIRE(resultat.bloque == 30);
    REQUIRE(resultat.montant == 0);
    REQUIRE(vitalite.getValeur() == 1000);
}

namespace {
// Un modificateur d'armure de ce skill, pour cette durée (en secondes).
EffetActif effetDArmure(IdSkill source, int pourcentage, float duree) {
    return {.source = source,
            .dureeRestante = duree,
            .sorte = ModificateurDeStatistique{.statistique = StatistiqueModifiable::Armure, .pourcentage = pourcentage}};
}
}  // namespace

TEST_CASE("Vitalite : un buff d'armure s'applique au coup suivant, un affaiblissement aussi") {
    Vitalite vitalite(10000, defensesDeTest(20));
    REQUIRE(subir(vitalite, 100) == 80);  // armure 20

    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 50, 10.0f));  // armure 30
    REQUIRE(subir(vitalite, 100) == 70);

    vitalite.poserEffet(effetDArmure(IdSkill::FrappePuissante, -100, 8.0f));  // armure 20 × (100 + 50 − 100) / 100 = 10
    REQUIRE(subir(vitalite, 100) == 90);
}

TEST_CASE("Vitalite : un effet expire avec le temps et l'armure revient a sa valeur") {
    Vitalite vitalite(10000, defensesDeTest(20));
    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 50, 10.0f));
    REQUIRE(subir(vitalite, 100) == 70);

    vitalite.mettreAJour(9.0f);
    REQUIRE(vitalite.getEffets().liste().size() == 1);
    REQUIRE(subir(vitalite, 100) == 70);

    vitalite.mettreAJour(1.0f);
    REQUIRE(vitalite.getEffets().liste().empty());
    REQUIRE(subir(vitalite, 100) == 80);
}

TEST_CASE("Vitalite : les effets passent au-dessus des defenses recalculees (equipement, niveau)") {
    Vitalite vitalite(10000, defensesDeTest(20));
    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 50, 10.0f));

    vitalite.definirDefenses(defensesDeTest(40));  // armure 40, puis +50 % : 60

    REQUIRE(subir(vitalite, 100) == 40);
    REQUIRE(vitalite.getEffets().liste().size() == 1);
}

TEST_CASE("Vitalite : la mort retire tous les effets actifs") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 50, 10.0f));
    vitalite.poserEffet(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    REQUIRE(vitalite.getEffets().liste().size() == 2);

    REQUIRE(subir(vitalite, 100) == 100);

    REQUIRE_FALSE(vitalite.estVivant());
    REQUIRE(vitalite.getEffets().liste().empty());
}

TEST_CASE("Vitalite : un coup qui ne tue pas laisse les effets") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 50, 10.0f));

    REQUIRE(subir(vitalite, 30) == 30);

    REQUIRE(vitalite.getEffets().liste().size() == 1);
}

TEST_CASE("Vitalite : on ne pose pas d'effet sur un mort") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 100));

    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 50, 10.0f));

    REQUIRE(vitalite.getEffets().liste().empty());
}

TEST_CASE("Vitalite : le meme skill pose deux fois rafraichit son effet, il n'y en a qu'un") {
    Vitalite vitalite(100, defensesDeTest(20));
    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 20, 10.0f));
    vitalite.mettreAJour(6.0f);

    vitalite.poserEffet(effetDArmure(IdSkill::Soin, 20, 10.0f));

    REQUIRE(vitalite.getEffets().liste().size() == 1);
    REQUIRE(vitalite.getEffets().liste()[0].dureeRestante == 10.0f);
}

namespace {
// Un coup de contact de ces dégâts qui pose un affaiblissement d'armure de -25 % pendant 8 s.
Attaque coupQuiAffaiblit(int montant) {
    Attaque attaque = attaqueSansHasard(montant);
    attaque.effets.push_back(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    return attaque;
}
}  // namespace

TEST_CASE("Vitalite : les effets de l'attaque se posent apres le coup, ils ne jouent qu'au suivant") {
    Vitalite vitalite(10000, defensesDeTest(20));
    Generateur generateur = generateurDeTest();

    REQUIRE(vitalite.recevoirDegats(coupQuiAffaiblit(100), generateur).montant == 80);  // armure 20, pas encore affaiblie
    REQUIRE(vitalite.getEffets().liste().size() == 1);

    REQUIRE(vitalite.recevoirDegats(coupQuiAffaiblit(100), generateur).montant == 85);  // armure 15
    REQUIRE(vitalite.getEffets().liste().size() == 1);  // le même skill rafraîchit, il n'en pose pas un deuxième
}

TEST_CASE("Vitalite : un coup esquive ne pose aucun effet") {
    Generateur generateur = generateurDeTest();
    int esquives = 0;
    int touches = 0;

    for (int i = 0; i < 100; ++i) {
        Vitalite vitalite(100000, {.armure = 0, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 50}});
        const ResultatDuCoup resultat = vitalite.recevoirDegats(coupQuiAffaiblit(100), generateur);
        REQUIRE(vitalite.getEffets().liste().empty() == resultat.esquive);
        (resultat.esquive ? esquives : touches) += 1;
    }
    REQUIRE(esquives > 0);
    REQUIRE(touches > 0);
}

TEST_CASE("Vitalite : un coup qui tue ne laisse aucun effet") {
    Vitalite vitalite(100, defensesDeTest(0));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(coupQuiAffaiblit(100), generateur));

    REQUIRE_FALSE(vitalite.estVivant());
    REQUIRE(vitalite.getEffets().liste().empty());
}

TEST_CASE("Vitalite : un coup bloque en entier pose quand meme ses effets, il a atteint") {
    Vitalite vitalite(1000, {.armure = 0, .statistiques = {.capaciteDefensive = 100, .chanceDeBlocage = 100, .montantBloque = 40}});
    Generateur generateur = generateurDeTest();
    Attaque attaque = attaqueSansHasard(30);
    attaque.effets.push_back(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    const ResultatDuCoup resultat = vitalite.recevoirDegats(attaque, generateur);

    REQUIRE(resultat.bloque == 30);
    REQUIRE(vitalite.getEffets().liste().size() == 1);
}

TEST_CASE("Vitalite : sans bouclier, le blocage est toujours pret") {
    Vitalite vitalite(100, defensesDeTest(0));
    Generateur generateur = generateurDeTest();

    static_cast<void>(vitalite.recevoirDegats(s_coupDeContact, generateur));

    REQUIRE(vitalite.blocagePret());
}

// Les ticks des effets dans le temps (étape 112).

namespace {
EffetActif saignementSur(float duree, int montant, int pilesMax = 1) {
    return {.source = IdSkill::TirPercant,
            .rang = 1,
            .dureeRestante = duree,
            .sorte = DegatsDansLeTemps{.element = Element::Physique, .montant = montant},
            .pilesMax = pilesMax};
}

EffetActif brulureSur(float duree, int montant) {
    return {.source = IdSkill::Explosion,
            .rang = 1,
            .dureeRestante = duree,
            .sorte = DegatsDansLeTemps{.element = Element::Feu, .montant = montant}};
}

EffetActif soinSur(float duree, int montant) {
    return {.source = IdSkill::Soin, .dureeRestante = duree, .sorte = SoinDansLeTemps{.montant = montant}};
}
}  // namespace

TEST_CASE("ticks : un tick de degats retire des points de vie et est rendu une fois") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(saignementSur(4.0f, 10));

    vitalite.mettreAJour(1.0f);

    REQUIRE(vitalite.getValeur() == 90);
    const std::vector<Tick> ticks = vitalite.prendreLesTicks();
    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 10);
    REQUIRE(vitalite.prendreLesTicks().empty());
}

TEST_CASE("ticks : ni armure, ni esquive, ni blocage, ni critique : seule la resistance joue") {
    Defenses defenses = defensesDeTest(1000);  // une armure énorme
    defenses.statistiques.esquiveContact = 100;
    defenses.statistiques.esquiveProjectile = 100;
    defenses.statistiques.chanceDeBlocage = 100;
    defenses.statistiques.montantBloque = 1000;
    Vitalite vitalite(1000, defenses);
    vitalite.poserEffet(saignementSur(3.0f, 10));

    for (int i = 0; i < 3; ++i) {
        vitalite.mettreAJour(1.0f);
    }

    REQUIRE(vitalite.getValeur() == 970);  // trois ticks entiers
    REQUIRE(vitalite.blocagePret());
}

TEST_CASE("ticks : la resistance est lue au tick, pas a la pose") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(brulureSur(4.0f, 10));
    vitalite.mettreAJour(1.0f);
    REQUIRE(vitalite.getValeur() == 90);

    Defenses resistante = defensesDeTest(0);
    resistante.statistiques.resistances[Element::Feu] = 50;
    vitalite.definirDefenses(resistante);  // l'équipement change entre deux ticks
    vitalite.mettreAJour(1.0f);

    REQUIRE(vitalite.getValeur() == 85);
    const std::vector<Tick> ticks = vitalite.prendreLesTicks();
    REQUIRE(ticks.size() == 2);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 10);
    REQUIRE(std::get<TickDeDegats>(ticks[1]).montant == 5);
    REQUIRE(std::get<TickDeDegats>(ticks[1]).element == Element::Feu);
}

TEST_CASE("ticks : un tick entierement resiste n'est pas rendu") {
    Defenses defenses = defensesDeTest(0);
    defenses.statistiques.resistances[Element::Feu] = 80;
    Vitalite vitalite(100, defenses);
    vitalite.poserEffet(brulureSur(2.0f, 1));  // 1 × 20 % = 0,2 : arrondi à 0

    vitalite.mettreAJour(1.0f);

    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.prendreLesTicks().empty());
}

TEST_CASE("ticks : un soin dans le temps rend des points de vie, plafonnes au maximum") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 5));
    vitalite.poserEffet(soinSur(3.0f, 9));

    vitalite.mettreAJour(1.0f);
    REQUIRE(vitalite.getValeur() == 100);  // 95 + 9 plafonné à 100

    const std::vector<Tick> ticks = vitalite.prendreLesTicks();
    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeSoin>(ticks[0]).montant == 5);  // ce qui a été réellement rendu

    vitalite.mettreAJour(1.0f);  // à pleine vie : rien à rendre, rien de signalé
    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.prendreLesTicks().empty());
}

TEST_CASE("ticks : un soin dans le temps ne critique pas, meme avec une grande capacite") {
    Vitalite vitalite(1000, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 500));
    vitalite.poserEffet(soinSur(10.0f, 9));

    for (int i = 0; i < 10; ++i) {
        vitalite.mettreAJour(1.0f);
    }

    REQUIRE(vitalite.getValeur() == 590);  // 10 ticks de 9, jamais plus
}

TEST_CASE("ticks : un tick qui tue vide les effets, et les ticks suivants du meme pas n'ont pas lieu") {
    Vitalite vitalite(15, defensesDeTest(0));
    vitalite.poserEffet(saignementSur(5.0f, 10));
    vitalite.poserEffet(brulureSur(5.0f, 10));
    vitalite.poserEffet(soinSur(5.0f, 50));

    vitalite.mettreAJour(1.0f);  // trois ticks : 10 (reste 5), 10 (mort), puis le soin n'a pas lieu

    REQUIRE_FALSE(vitalite.estVivant());
    REQUIRE(vitalite.getValeur() == 0);
    REQUIRE(vitalite.getEffets().liste().empty());
    const std::vector<Tick> ticks = vitalite.prendreLesTicks();
    REQUIRE(ticks.size() == 2);
    REQUIRE(std::holds_alternative<TickDeDegats>(ticks[0]));
    REQUIRE(std::holds_alternative<TickDeDegats>(ticks[1]));
}

TEST_CASE("ticks : un mort ne subit plus aucun tick") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, 100));

    vitalite.mettreAJour(5.0f);

    REQUIRE_FALSE(vitalite.estVivant());
    REQUIRE(vitalite.prendreLesTicks().empty());
}

TEST_CASE("ticks : un coup qui pose un effet dans le temps le pose, et il bat apres le coup") {
    Vitalite vitalite(1000, defensesDeTest(0));
    Generateur generateur = generateurDeTest();
    Attaque attaque = {.degats = degatsPhysiques(50), .capaciteOffensive = 0, .nature = NatureDAttaque::Zone};
    attaque.effets.push_back(saignementSur(4.0f, 10));

    static_cast<void>(vitalite.recevoirDegats(attaque, generateur));
    REQUIRE(vitalite.getValeur() == 950);
    REQUIRE(vitalite.getEffets().liste().size() == 1);

    vitalite.mettreAJour(4.0f);
    REQUIRE(vitalite.getValeur() == 910);  // quatre ticks de 10 après le coup de 50
}

TEST_CASE("viderLesEffets : retire les effets sans toucher aux points de vie") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(saignementSur(4.0f, 10));

    vitalite.viderLesEffets();
    vitalite.mettreAJour(4.0f);

    REQUIRE(vitalite.getValeur() == 100);
    REQUIRE(vitalite.getEffets().liste().empty());
}

namespace {
// Un contrôle de cet état, de cette durée avant résistance, posé par ce skill.
EffetActif controleDe(EtatInflige etat, float duree, IdSkill source = IdSkill::FrappeSacree, int pourcentage = 0) {
    return {.source = source, .dureeRestante = duree, .sorte = Controle{.etat = etat, .pourcentage = pourcentage}};
}

float dureeDuPremierEffet(const Vitalite& vitalite) {
    return vitalite.getEffets().liste().front().dureeRestante;
}

// Des défenses avec cette résistance à cet état de contrôle.
Defenses defensesResistantes(EtatDeControle etat, float resistance) {
    Defenses defenses = defensesDeTest(0);
    defenses.statistiques.resistancesAuControle[etat] = resistance;
    return defenses;
}
}  // namespace

TEST_CASE("controle : sans resistance, il dure sa duree et la pose est notee") {
    Vitalite vitalite(100, defensesDeTest(0));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.5f));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(1.5f));
    REQUIRE(estSousEtat(vitalite.getEffets(), EtatInflige::Etourdissement));
    const std::vector<ControleApplique> poses = vitalite.prendreLesControles();
    REQUIRE(poses.size() == 1);
    REQUIRE(poses[0].etat == EtatInflige::Etourdissement);
    REQUIRE_FALSE(poses[0].immunise);
    REQUIRE(vitalite.prendreLesControles().empty());  // oublié une fois pris
}

TEST_CASE("controle : la resistance de la cible raccourcit la duree, lue a la pose") {
    Vitalite vitalite(100, defensesResistantes(EtatDeControle::Etourdissement, 20.0f));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 2.0f));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(1.6f));
}

TEST_CASE("controle : une resistance negative (vulnerabilite) allonge la duree") {
    Vitalite vitalite(100, defensesResistantes(EtatDeControle::Disruption, -50.0f));

    vitalite.poserEffet(controleDe(EtatInflige::Disruption, 2.0f));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(3.0f));
}

TEST_CASE("controle : la resistance est bornee a 80 %, la duree ne tombe jamais a zero") {
    Vitalite vitalite(100, defensesResistantes(EtatDeControle::Etourdissement, 500.0f));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 2.0f));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(0.4f));
}

TEST_CASE("controle : l'entrave est reduite par la resistance au ralentissement") {
    Vitalite vitalite(100, defensesResistantes(EtatDeControle::Ralentissement, 25.0f));

    vitalite.poserEffet(controleDe(EtatInflige::Entrave, 4.0f, IdSkill::PluieDeFleches, 20));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(3.0f));
}

TEST_CASE("controle : la resistance a un autre etat ne joue pas") {
    Vitalite vitalite(100, defensesResistantes(EtatDeControle::Disruption, 50.0f));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 2.0f));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(2.0f));
}

TEST_CASE("controle : rendement decroissant 100 %, 50 %, 25 %, puis immunite") {
    Vitalite vitalite(100, defensesDeTest(0));

    // Un skill différent à chaque fois : aucun rafraîchissement, chaque effet garde la durée posée.
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::FrappeSacree));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::FrappePuissante));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::CoupAssommant));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::Contresort));

    const std::vector<EffetActif>& effets = vitalite.getEffets().liste();
    REQUIRE(effets.size() == 3);  // la quatrième application est bloquée
    REQUIRE(effets[0].dureeRestante == Approx(4.0f));
    REQUIRE(effets[1].dureeRestante == Approx(2.0f));
    REQUIRE(effets[2].dureeRestante == Approx(1.0f));
    const std::vector<ControleApplique> poses = vitalite.prendreLesControles();
    REQUIRE(poses.size() == 4);
    REQUIRE_FALSE(poses[2].immunise);
    REQUIRE(poses[3].immunise);
}

TEST_CASE("controle : la resistance et le rendement se multiplient") {
    Vitalite vitalite(100, defensesResistantes(EtatDeControle::Etourdissement, 50.0f));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::FrappeSacree));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::FrappePuissante));

    REQUIRE(vitalite.getEffets().liste()[0].dureeRestante == Approx(2.0f));  // 4 × 0,5 × 1
    REQUIRE(vitalite.getEffets().liste()[1].dureeRestante == Approx(1.0f));  // 4 × 0,5 × 0,5
}

TEST_CASE("controle : chaque etat a son propre compteur de rendement") {
    Vitalite vitalite(100, defensesDeTest(0));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::FrappeSacree));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::FrappePuissante));
    vitalite.poserEffet(controleDe(EtatInflige::Disruption, 4.0f, IdSkill::Contresort));

    REQUIRE(vitalite.getEffets().liste()[2].dureeRestante == Approx(4.0f));
}

TEST_CASE("controle : apres 15 s sans application, la duree pleine revient") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f, IdSkill::FrappeSacree));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f, IdSkill::FrappePuissante));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f, IdSkill::CoupAssommant));
    vitalite.mettreAJour(15.0f);
    REQUIRE(vitalite.getEffets().liste().empty());

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 4.0f, IdSkill::Contresort));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(4.0f));
}

TEST_CASE("controle : la mort vide les effets et le compteur") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f, IdSkill::FrappeSacree));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f, IdSkill::FrappePuissante));
    static_cast<void>(subir(vitalite, s_coupMortel));
    REQUIRE(vitalite.getEffets().liste().empty());

    REQUIRE(vitalite.getEffets().rendements().applications(EtatInflige::Etourdissement) == 0);
}

TEST_CASE("controle : viderLesEffets remet aussi le compteur a zero") {
    Vitalite vitalite(100, defensesDeTest(0));
    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f, IdSkill::FrappeSacree));

    vitalite.viderLesEffets();

    REQUIRE(vitalite.getEffets().rendements().applications(EtatInflige::Etourdissement) == 0);
}

TEST_CASE("controle : un mort n'en recoit pas") {
    Vitalite vitalite(100, defensesDeTest(0));
    static_cast<void>(subir(vitalite, s_coupMortel));

    vitalite.poserEffet(controleDe(EtatInflige::Etourdissement, 1.0f));

    REQUIRE(vitalite.getEffets().liste().empty());
    REQUIRE(vitalite.prendreLesControles().empty());
}

TEST_CASE("controle : celui d'une attaque se pose apres le coup, par le meme chemin") {
    Vitalite vitalite(1000, defensesResistantes(EtatDeControle::Etourdissement, 50.0f));
    Generateur generateur = generateurDeTest();
    Attaque attaque = attaqueSansHasard(10);
    attaque.effets.push_back(controleDe(EtatInflige::Etourdissement, 2.0f));

    static_cast<void>(vitalite.recevoirDegats(attaque, generateur));

    REQUIRE(dureeDuPremierEffet(vitalite) == Approx(1.0f));
    REQUIRE(vitalite.prendreLesControles().size() == 1);
}
