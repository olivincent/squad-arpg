// ./tests/combat/test_EffetsActifs.cpp
#include <catch2/catch_test_macros.hpp>

#include "aides.hpp"
#include "combat/EffetsActifs.hpp"

namespace {
// Un modificateur d'armure posé par ce skill, pour cette durée (en secondes).
EffetActif effetDArmure(IdSkill source, int pourcentage, float duree, int rang = 0) {
    return {.source = source,
            .rang = rang,
            .dureeRestante = duree,
            .sorte = ModificateurDeStatistique{.statistique = StatistiqueModifiable::Armure, .pourcentage = pourcentage}};
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
