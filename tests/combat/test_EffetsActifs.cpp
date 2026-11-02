// ./tests/combat/test_EffetsActifs.cpp
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "combat/EffetsActifs.hpp"

namespace {
// Un modificateur d'armure posé par ce skill, pour cette durée (en secondes).
EffetActif effetDArmure(IdSkill source, int pourcentage, float duree, int rang = 0, int pilesMax = 1) {
    return {.source = source,
            .rang = rang,
            .dureeRestante = duree,
            .sorte = ModificateurDeStatistique{.statistique = StatistiqueModifiable::Armure, .pourcentage = pourcentage},
            .pilesMax = pilesMax};
}

// Un affaiblissement de ce skill qui s'empile jusqu'à `pilesMax` fois.
EffetActif affaiblissementEmpilable(int pourcentage, float duree, int pilesMax) {
    return effetDArmure(IdSkill::FrappePuissante, pourcentage, duree, 0, pilesMax);
}

// Des dégâts dans le temps de ce skill (étape 112) : ce montant par tick et par pile, pour cette durée.
EffetActif saignement(float duree, int montant = 10, int pilesMax = 1, IdSkill source = IdSkill::TirPercant, int rang = 1) {
    return {.source = source,
            .rang = rang,
            .dureeRestante = duree,
            .sorte = DegatsDansLeTemps{.element = Element::Physique, .montant = montant},
            .pilesMax = pilesMax};
}

EffetActif regeneration(float duree, int montant = 9) {
    return {.source = IdSkill::Soin, .dureeRestante = duree, .sorte = SoinDansLeTemps{.montant = montant}};
}

// Fait couler `duree` secondes par pas de `pas` et rend tous les ticks, dans l'ordre.
std::vector<Tick> ticksSur(EffetsActifs& effets, float duree, float pas) {
    std::vector<Tick> tous;
    float ecoule = 0.0f;
    while (ecoule < duree - 1.0e-4f) {
        const std::vector<Tick> ticks = effets.mettreAJour(pas);
        tous.insert(tous.end(), ticks.begin(), ticks.end());
        ecoule += pas;
    }
    return tous;
}

int pourcentageDe(const EffetActif& effet) {
    return std::get<ModificateurDeStatistique>(effet.sorte).pourcentage;
}
}  // namespace

TEST_CASE("EffetsActifs : vide au depart") {
    const EffetsActifs effets;

    REQUIRE(effets.liste().empty());
}

TEST_CASE("EffetsActifs poser : ajoute l'effet, dans l'ordre") {
    EffetsActifs effets;

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(effets.liste()[0].source == IdSkill::FrappePuissante);
    REQUIRE(effets.liste()[1].source == IdSkill::Soin);
}

TEST_CASE("EffetsActifs poser : deux skills differents se cumulent") {
    EffetsActifs effets;

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.poser(effetDArmure(IdSkill::Provocation, 20, 10.0f));

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(pourcentageDe(effets.liste()[0]) == -25);
    REQUIRE(pourcentageDe(effets.liste()[1]) == 20);
}

TEST_CASE("EffetsActifs poser : un meme skill peut poser deux effets de rangs differents sur la meme entite") {
    EffetsActifs effets;

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f, 0));
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -10, 8.0f, 1));

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(pourcentageDe(effets.liste()[0]) == -25);
    REQUIRE(pourcentageDe(effets.liste()[1]) == -10);
}

TEST_CASE("EffetsActifs poser : le meme skill et le meme rang rafraichissent, chacun de son cote") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f, 0));
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -10, 8.0f, 1));
    effets.mettreAJour(5.0f);

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -30, 8.0f, 1));

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(effets.liste()[0].dureeRestante == 3.0f);
    REQUIRE(pourcentageDe(effets.liste()[0]) == -25);
    REQUIRE(effets.liste()[1].dureeRestante == 8.0f);
    REQUIRE(pourcentageDe(effets.liste()[1]) == -30);
}

TEST_CASE("EffetsActifs poser : le meme skill rafraichit la duree, il n'y a jamais deux effets du meme skill") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.mettreAJour(5.0f);  // 3 s restantes

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste()[0].dureeRestante == 8.0f);
}

TEST_CASE("EffetsActifs poser : un rafraichissement ne raccourcit jamais l'effet") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::Soin, 20, 12.0f));

    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));

    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste()[0].dureeRestante == 12.0f);
}

TEST_CASE("EffetsActifs poser : le meme skill garde la valeur la plus forte, dans les deux sens") {
    EffetsActifs buff;
    buff.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));
    buff.poser(effetDArmure(IdSkill::Soin, 30, 10.0f));
    REQUIRE(pourcentageDe(buff.liste()[0]) == 30);
    buff.poser(effetDArmure(IdSkill::Soin, 10, 10.0f));
    REQUIRE(pourcentageDe(buff.liste()[0]) == 30);

    EffetsActifs affaiblissement;
    affaiblissement.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    affaiblissement.poser(effetDArmure(IdSkill::FrappePuissante, -40, 8.0f));
    REQUIRE(pourcentageDe(affaiblissement.liste()[0]) == -40);
    affaiblissement.poser(effetDArmure(IdSkill::FrappePuissante, -10, 8.0f));
    REQUIRE(pourcentageDe(affaiblissement.liste()[0]) == -40);
}

TEST_CASE("EffetsActifs mettreAJour : la duree diminue avec le temps") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    effets.mettreAJour(3.0f);

    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste()[0].dureeRestante == 5.0f);
}

TEST_CASE("EffetsActifs mettreAJour : l'effet disparait quand sa duree atteint 0, pas avant") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    effets.mettreAJour(7.5f);
    REQUIRE(effets.liste().size() == 1);

    effets.mettreAJour(0.5f);
    REQUIRE(effets.liste().empty());
}

TEST_CASE("EffetsActifs mettreAJour : un pas qui depasse la duree retire l'effet, les autres restent") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));

    effets.mettreAJour(9.0f);

    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste()[0].source == IdSkill::Soin);
    REQUIRE(effets.liste()[0].dureeRestante == 1.0f);
}

TEST_CASE("EffetsActifs vider : retire tous les effets") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));

    effets.vider();

    REQUIRE(effets.liste().empty());
}

TEST_CASE("appliquerLesEffets : sans effet, les defenses ne changent pas") {
    const Defenses defenses = {.armure = 20, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 7}};

    const Defenses resultat = appliquerLesEffets(defenses, EffetsActifs());

    REQUIRE(resultat.armure == 20);
    REQUIRE(resultat.statistiques == defenses.statistiques);
}

TEST_CASE("appliquerLesEffets : un buff augmente l'armure, un affaiblissement la baisse") {
    EffetsActifs buff;
    buff.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));
    EffetsActifs affaiblissement;
    affaiblissement.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    REQUIRE(appliquerLesEffets(defensesDeTest(20), buff).armure == 24);
    REQUIRE(appliquerLesEffets(defensesDeTest(20), affaiblissement).armure == 15);
}

TEST_CASE("appliquerLesEffets : les pourcentages s'additionnent puis s'appliquent une seule fois") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    // Additif : 20 × (100 + 20 − 25) / 100 = 19 ; multiplicatif : 20 × 1,2 × 0,75 = 18.
    REQUIRE(appliquerLesEffets(defensesDeTest(20), effets).armure == 19);
}

TEST_CASE("appliquerLesEffets : le resultat est arrondi au plus proche") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    REQUIRE(appliquerLesEffets(defensesDeTest(10), effets).armure == 8);  // 7,5
    REQUIRE(appliquerLesEffets(defensesDeTest(5), effets).armure == 4);   // 3,75
    REQUIRE(appliquerLesEffets(defensesDeTest(1), effets).armure == 1);   // 0,75
}

TEST_CASE("appliquerLesEffets : l'armure ne descend jamais sous 0") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -100, 8.0f));
    effets.poser(effetDArmure(IdSkill::Provocation, -50, 8.0f));

    REQUIRE(appliquerLesEffets(defensesDeTest(20), effets).armure == 0);
}

TEST_CASE("appliquerLesEffets : une armure nulle reste nulle, meme avec un buff") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::Soin, 50, 10.0f));

    REQUIRE(appliquerLesEffets(defensesDeTest(0), effets).armure == 0);
}

TEST_CASE("appliquerLesEffets : les statistiques de combat restent celles de la source") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));
    const Defenses defenses = {.armure = 20, .statistiques = {.capaciteDefensive = 100, .esquiveContact = 7, .chanceDeBlocage = 30}};

    REQUIRE(appliquerLesEffets(defenses, effets).statistiques == defenses.statistiques);
}

// Les piles (étape 112) : génériques, un maximum de 1 par défaut.

TEST_CASE("piles : un effet qui vient d'etre pose a une pile, avec un maximum de 1 par defaut") {
    EffetsActifs effets;

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));

    REQUIRE(effets.liste()[0].piles == 1);
    REQUIRE(effets.liste()[0].pilesMax == 1);
}

TEST_CASE("piles : avec un maximum de 1, reposer ne change que la duree et la valeur, comme a l'etape 111") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.mettreAJour(5.0f);

    effets.poser(effetDArmure(IdSkill::FrappePuissante, -40, 6.0f));
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -10, 6.0f));

    REQUIRE(effets.liste().size() == 1);
    REQUIRE(effets.liste()[0].piles == 1);
    REQUIRE(effets.liste()[0].dureeRestante == 6.0f);
    REQUIRE(pourcentageDe(effets.liste()[0]) == -40);
}

TEST_CASE("piles : chaque application ajoute une pile, jusqu'au maximum") {
    EffetsActifs effets;

    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    REQUIRE(effets.liste()[0].piles == 1);
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    REQUIRE(effets.liste()[0].piles == 2);
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    REQUIRE(effets.liste()[0].piles == 3);
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    REQUIRE(effets.liste()[0].piles == 3);

    REQUIRE(effets.liste().size() == 1);
}

TEST_CASE("piles : toutes les piles partagent une seule duree, rafraichie a chaque application") {
    EffetsActifs effets;
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    effets.mettreAJour(5.0f);
    REQUIRE(effets.liste()[0].dureeRestante == 3.0f);

    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    REQUIRE(effets.liste()[0].piles == 2);
    REQUIRE(effets.liste()[0].dureeRestante == 8.0f);

    effets.mettreAJour(8.0f);  // les deux piles disparaissent ensemble
    REQUIRE(effets.liste().empty());
}

TEST_CASE("piles : l'application au maximum rafraichit quand meme la duree") {
    EffetsActifs effets;
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 2));
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 2));
    effets.mettreAJour(6.0f);

    effets.poser(affaiblissementEmpilable(-10, 8.0f, 2));

    REQUIRE(effets.liste()[0].piles == 2);
    REQUIRE(effets.liste()[0].dureeRestante == 8.0f);
}

TEST_CASE("piles : la valeur par pile est la plus forte des applications") {
    EffetsActifs effets;
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    effets.poser(affaiblissementEmpilable(-30, 8.0f, 3));
    effets.poser(affaiblissementEmpilable(-20, 8.0f, 3));

    REQUIRE(effets.liste()[0].piles == 3);
    REQUIRE(pourcentageDe(effets.liste()[0]) == -30);
}

TEST_CASE("piles : deux skills differents ont chacun leurs piles") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -10, 8.0f, 0, 3));
    effets.poser(effetDArmure(IdSkill::Provocation, -10, 8.0f, 0, 3));
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -10, 8.0f, 0, 3));

    REQUIRE(effets.liste().size() == 2);
    REQUIRE(effets.liste()[0].piles == 2);
    REQUIRE(effets.liste()[1].piles == 1);
}

TEST_CASE("piles : l'effet vaut la valeur par pile fois le nombre de piles") {
    EffetsActifs effets;
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));

    REQUIRE(appliquerLesEffets(defensesDeTest(100), effets).armure == 70);  // −10 % × 3 piles
}

TEST_CASE("piles : les piles d'un affaiblissement s'additionnent aux autres pourcentages avant l'application unique") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::Soin, 20, 10.0f));
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));
    effets.poser(affaiblissementEmpilable(-10, 8.0f, 3));

    REQUIRE(appliquerLesEffets(defensesDeTest(100), effets).armure == 100);  // +20 − 2 × 10
}

// Les effets dans le temps (étape 112) : un tick par seconde, le premier une seconde après la pose.

TEST_CASE("ticks : le premier tick a lieu une seconde apres la pose, pas avant") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f));

    REQUIRE(effets.mettreAJour(0.5f).empty());
    REQUIRE(effets.mettreAJour(0.4f).empty());
    const std::vector<Tick> ticks = effets.mettreAJour(0.1f);

    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 10);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).element == Element::Physique);
}

TEST_CASE("ticks : une duree de N secondes donne N ticks, par grands pas") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f));

    REQUIRE(ticksSur(effets, 4.0f, 0.5f).size() == 4);
    REQUIRE(effets.liste().empty());
}

TEST_CASE("ticks : une duree de N secondes donne N ticks, a 60 images par seconde (l'imprecision des flottants ne perd pas le dernier)") {
    for (const float duree : {1.0f, 4.0f, 6.0f, 8.0f, 10.0f, 20.0f}) {
        INFO("duree " << duree);
        EffetsActifs effets;
        effets.poser(saignement(duree));

        REQUIRE(static_cast<float>(ticksSur(effets, duree + 0.5f, 1.0f / 60.0f).size()) == duree);
    }
}

TEST_CASE("ticks : un pas plus long que l'intervalle rend plusieurs ticks, sans depasser la duree") {
    EffetsActifs effets;
    effets.poser(saignement(3.0f));
    REQUIRE(effets.mettreAJour(2.5f).size() == 2);

    effets.poser(saignement(3.0f, 10, 1, IdSkill::Explosion, 1));
    REQUIRE(effets.mettreAJour(10.0f).size() == 1 + 3);  // le premier : son 3e tick ; le second : 3 ticks, pas 10
    REQUIRE(effets.liste().empty());
}

TEST_CASE("ticks : une duree qui n'est pas entiere ne donne que les ticks entiers") {
    EffetsActifs effets;
    effets.poser(saignement(2.5f));

    REQUIRE(ticksSur(effets, 3.0f, 0.5f).size() == 2);
}

TEST_CASE("ticks : un rafraichissement ne remet pas le rythme a zero") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f));
    REQUIRE(effets.mettreAJour(0.5f).empty());

    effets.poser(saignement(4.0f));  // rafraîchit la durée à 4 s, mais le prochain tick reste dans 0,5 s

    REQUIRE(effets.mettreAJour(0.25f).empty());
    REQUIRE(effets.mettreAJour(0.25f).size() == 1);
    REQUIRE(effets.liste()[0].dureeRestante == 3.5f);
}

TEST_CASE("ticks : le montant d'un tick est celui d'une pile fois le nombre de piles") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f, 10, 3));
    effets.poser(saignement(4.0f, 10, 3));
    effets.poser(saignement(4.0f, 10, 3));

    const std::vector<Tick> ticks = effets.mettreAJour(1.0f);

    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 30);
}

TEST_CASE("ticks : le montant par pile est le plus fort des applications, le rythme celui de la premiere") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f, 8, 3));
    static_cast<void>(effets.mettreAJour(0.25f));
    effets.poser(saignement(4.0f, 12, 3));
    effets.poser(saignement(4.0f, 9, 3));

    REQUIRE(std::get<DegatsDansLeTemps>(effets.liste()[0].sorte).montant == 12);
    const std::vector<Tick> ticks = effets.mettreAJour(0.75f);
    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 12 * 3);
}

TEST_CASE("ticks : un soin dans le temps rend des ticks de soin") {
    EffetsActifs effets;
    effets.poser(regeneration(3.0f, 9));

    const std::vector<Tick> ticks = ticksSur(effets, 3.0f, 0.5f);

    REQUIRE(ticks.size() == 3);
    for (const Tick& tick : ticks) {
        REQUIRE(std::get<TickDeSoin>(tick).montant == 9);
    }
}

TEST_CASE("ticks : un modificateur de statistique n'a aucun tick") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::Soin, 20, 5.0f));

    REQUIRE(ticksSur(effets, 5.0f, 0.5f).empty());
}

TEST_CASE("ticks : deux effets dans le temps de rangs differents battent chacun a leur rythme, dans l'ordre de pose") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f, 10, 1, IdSkill::TirPercant, 1));
    static_cast<void>(effets.mettreAJour(0.5f));
    effets.poser(saignement(4.0f, 7, 1, IdSkill::TirPercant, 2));

    const std::vector<Tick> ticks = effets.mettreAJour(0.5f);  // le premier bat à 1 s, le second dans 1 s

    REQUIRE(ticks.size() == 1);
    REQUIRE(std::get<TickDeDegats>(ticks[0]).montant == 10);
    const std::vector<Tick> suivants = effets.mettreAJour(0.5f);
    REQUIRE(suivants.size() == 1);
    REQUIRE(std::get<TickDeDegats>(suivants[0]).montant == 7);
}

TEST_CASE("ticks : un effet dans le temps ne change aucune statistique") {
    EffetsActifs effets;
    effets.poser(saignement(4.0f));
    effets.poser(regeneration(4.0f));

    REQUIRE(appliquerLesEffets(defensesDeTest(20), effets).armure == 20);
}

TEST_CASE("estUnBonus : un soin dans le temps est un bonus, des degats dans le temps un malus") {
    REQUIRE(estUnBonus(regeneration(4.0f)));
    REQUIRE_FALSE(estUnBonus(saignement(4.0f)));
}

// Les marques d'un contrôle (étape 113) : une rangée jaune à part, ni dans les bonus ni dans les malus.

TEST_CASE("EffetsActifs : un controle a sa propre marque, il n'est pas dans les malus") {
    EffetsActifs effets;
    effets.poser(effetDArmure(IdSkill::FrappePuissante, -25, 8.0f));
    effets.poser({.source = IdSkill::FrappeSacree, .dureeRestante = 2.0f, .sorte = Controle{.etat = EtatInflige::Etourdissement}});
    effets.poser({.source = IdSkill::PluieDeFleches, .rang = 1, .dureeRestante = 2.0f, .sorte = Controle{.etat = EtatInflige::Entrave, .pourcentage = 20}});

    REQUIRE(effets.pilesDesMalus() == std::vector<int>{1});  // seul l'affaiblissement d'armure
    REQUIRE(effets.pilesDesBonus().empty());
    REQUIRE(effets.nombreDeControles() == 2);
}

TEST_CASE("EffetsActifs : le temps fait aussi couler le rendement decroissant du controle") {
    EffetsActifs effets;
    static_cast<void>(effets.compterUneApplicationDeControle(EtatInflige::Etourdissement));
    REQUIRE(effets.rendements().applications(EtatInflige::Etourdissement) == 1);

    static_cast<void>(effets.mettreAJour(15.0f));

    REQUIRE(effets.rendements().applications(EtatInflige::Etourdissement) == 0);
}

TEST_CASE("EffetsActifs vider : retire aussi le compteur de rendement") {
    EffetsActifs effets;
    static_cast<void>(effets.compterUneApplicationDeControle(EtatInflige::Disruption));

    effets.vider();

    REQUIRE(effets.rendements().applications(EtatInflige::Disruption) == 0);
}
